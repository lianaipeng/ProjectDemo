#include "KafkaConsumer.h"

#include <signal.h>

#include "Log.h"


CKafkaConsumer::CKafkaConsumer()
{
    m_kafka_handle              = NULL;
    m_kafka_topic               = NULL;
    m_kafka_conf                = NULL;
    m_kafka_topic_conf          = NULL;
    m_kafka_topic_partition_list = NULL;
    m_kafka_queue               = NULL;

    m_consumer_callback         = NULL;
    m_consumer_callback_param   = NULL;
}

CKafkaConsumer::~CKafkaConsumer()
{
    Stop();

    rd_kafka_flush(m_kafka_handle, 10*1000); //wait for max 10 seconds

    rd_kafka_queue_destroy(m_kafka_queue);
    /* 销毁topic */
    rd_kafka_topic_destroy(m_kafka_topic);
    /* 销毁consumer实例 */
    rd_kafka_destroy(m_kafka_handle);
    /* 释放topic list资源 */
    rd_kafka_topic_partition_list_destroy(m_kafka_topic_partition_list);
}

int CKafkaConsumer::Init(char *brokers, char *topic, char *partitions, char *groupId, consumer_callback consumer_cb, void * param_cb)
{
    int ret = 0;
    rd_kafka_conf_res_t ret_conf = RD_KAFKA_CONF_OK;
    char errstr[512] = {0};

    if(topic == NULL){return -1;}
    if(brokers == NULL){return -1;}
    if(groupId == NULL){return -1;}

    m_consumer_callback = consumer_cb;
    m_consumer_callback_param = param_cb;

    /* ---------Consumer config------------------- */
    m_kafka_conf = rd_kafka_conf_new();

    rd_kafka_conf_set_error_cb(m_kafka_conf, err_cb);
    rd_kafka_conf_set_throttle_cb(m_kafka_conf, throttle_cb);
    rd_kafka_conf_set_offset_commit_cb(m_kafka_conf, offset_commit_cb);
    rd_kafka_conf_set_stats_cb(m_kafka_conf, stats_cb);
    rd_kafka_conf_set_log_cb(m_kafka_conf, logger);

    ret_conf = rd_kafka_conf_set(m_kafka_conf, "queued.min.messages", "1000000", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 1; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }

    /* Consumer与集群之间的心跳 */
    ret_conf = rd_kafka_conf_set(m_kafka_conf, "session.timeout.ms", "6000", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 2; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }
    /*
    ret_conf = rd_kafka_conf_set(m_kafka_conf, "group.id", groupId, errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 3; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }
    */
    /* ---------Kafka topic config------------------- */
    m_kafka_topic_conf = rd_kafka_topic_conf_new();

    ret_conf = rd_kafka_topic_conf_set(m_kafka_topic_conf, "auto.offset.reset", "earliest", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_topic_conf_set() failed 4; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }

    /* ------------kafka partition------------------- */
    /* 可扩展长度的 主题-分区 链表
     * 创建时指定长度，通过rd_kafka_topic_partition_list_add()添加 主题-分区对，用于订阅消息。
     */
    m_kafka_topic_partition_list = rd_kafka_topic_partition_list_new(1);
    
    /* 解析字符串*/
    int cnt = 0;
    int len = strlen(partitions);
    char * pTemp = new char[len + 1];
    char * pTemp2 = pTemp;
    sprintf(pTemp, "%s", partitions); //partitions="0,1,2";

    while(*pTemp != '\0'){
        char * s = strstr(pTemp, ",");
        if(s != NULL){
            *s = '\0';
        }

        int partition = atoi(pTemp);
        rd_kafka_topic_partition_list_add(m_kafka_topic_partition_list, topic, partition); 

        if(s != NULL){
            pTemp = s + 1;
        }else{
            break;
        }
    }

    if(pTemp2){
        delete [] pTemp2;
        pTemp2 = NULL;
    }
    
    /* ---------Create Kafka handle------------------- */
    /* 创建consumer实例  总体结构
     * conf和topic_conf都是为此结构服务
     * 其中包含rk_brokers链表，rk_topics链表，是必须创建的结构。
     */
    m_kafka_handle = rd_kafka_new(RD_KAFKA_CONSUMER, m_kafka_conf, errstr, sizeof(errstr));

    if(m_kafka_handle == NULL){
        Log_Error("Failed to create Kafka producer: %s\n", errstr);
        return -1;
    }
    
    /* ---------Add broker(s)------------------- */
    /* Librdkafka需要至少一个brokers的初始化list */
    /* broker字符串 如:”172.20.51.38:9092” 不写端口，则采用默认端口9092
     * 多个broker  brokerlist = "broker1:10000,broker2"
     * 返回 成功添加的broker个数
     * 添加一个broker也可以通过 设置rd_kafka_conf_t结构中的 "bootstrap.servers" 配置项
     */
    if(brokers && rd_kafka_brokers_add(m_kafka_handle, brokers) < 1){
        Log_Error("Error: No valid brokers specified\n");
        return -2;
    }
    /*
    //  char * topic = m_kafka_topic_partition_list->elems[0].topic;
    int partition = m_kafka_topic_partition_list->elems[0].partition;
    int partition_cnt = m_kafka_topic_partition_list->cnt;
    */
    m_kafka_topic = rd_kafka_topic_new(m_kafka_handle, topic, m_kafka_topic_conf); //Explicitly create topic to avoid per-msg lookups

    /* 重定向 rd_kafka_poll()队列到consumer_poll()队列 
     * 可以使用rd_kafka_consumer_poll()进行取消息
     */
    rd_kafka_poll_set_consumer(m_kafka_handle); 

    /* 队列模式
    //-----------------------------------------
    //  int64_t seek_offset = RD_KAFKA_OFFSET_END; //RD_KAFKA_OFFSET_BEGINNING | RD_KAFKA_OFFSET_END | RD_KAFKA_OFFSET_STORED
    //  rd_kafka_resp_err_t err = rd_kafka_seek(m_kafka_topic, partition, seek_offset, 2000);
    */

    /* 
     *
     */
    m_kafka_queue = rd_kafka_queue_new(m_kafka_handle);

    /* 发布订阅模式
    //开启consumer订阅，匹配的topic将被添加到订阅列表中
    if((err = rd_kafka_subscribe(rk, topics))){
        fprintf(stderr, "%% Failed to start consuming topics: %s\n", rd_kafka_err2str(err));
        return -1;
    }
    */              

    return ret;
}

int CKafkaConsumer::Start(int timeout_ms)
{
    int ret = 0;
    m_bRun = true;

    signal(SIGINT, sig_stop);
    signal(SIGUSR1, sig_usr1);

    char * topic = m_kafka_topic_partition_list->elems[0].topic;
    int partition = m_kafka_topic_partition_list->elems[0].partition;
    int partition_cnt = m_kafka_topic_partition_list->cnt;

    /* 
     * RD_KAFKA_OFFSET_BEGINNING 
     * RD_KAFKA_OFFSET_END 
     * RD_KAFKA_OFFSET_STORED
     */
    int64_t start_offset = RD_KAFKA_OFFSET_END; 

    /* ------------从kafka服务器接收消息---------------- */
    for(int i = 0; i < partition_cnt; i++){
        int partition = m_kafka_topic_partition_list->elems[i].partition;

        int r = rd_kafka_consume_start_queue(m_kafka_topic, partition, start_offset, m_kafka_queue);

        if(r == -1){
            Log_Error("creating queue: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
            return -1;
        }
    }

    while (m_bRun){
        int r = rd_kafka_consume_callback_queue(m_kafka_queue, 1000, msg_consume, this); //Queue mode
        if(r <= 0){
            rd_kafka_poll(m_kafka_handle, 1000); 
            continue;
        }

        rd_kafka_poll(m_kafka_handle, 0); //Poll to handle stats callbacks
    }
    
    /* ----------Stop consuming------------------------------ */
    for(int i = 0; i < partition_cnt; i++){
        int r = rd_kafka_consume_stop(m_kafka_topic, (int32_t)i);
        if(r == -1){
            Log_Error("in consume_stop: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        }
    }
    return ret;
}

void CKafkaConsumer::Stop()
{
    m_bRun = false;
}

void CKafkaConsumer::err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque)
{
    Log_Error("%% ERROR CALLBACK: %s: %s: %s\n", rd_kafka_name(rk), rd_kafka_err2str((rd_kafka_resp_err_t)err), reason);
}

void CKafkaConsumer::throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque)
{
    Log_Error("%% THROTTLED %dms by %s (%"PRId32")\n", throttle_time_ms, broker_name, broker_id);
}

void CKafkaConsumer::offset_commit_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *offsets, void *opaque)
{
    int i;
    int verbosity = 1;

    if(err || verbosity >= 2){
        Log_Error("%% Offset commit of %d partition(s): %s\n", offsets->cnt, rd_kafka_err2str(err));
    }

    for(i = 0; i < offsets->cnt; i++){
        rd_kafka_topic_partition_t * rktpar = &offsets->elems[i];

        if(rktpar->err || verbosity >= 2){
            Log_Error("%%  %s [%"PRId32"] @ %"PRId64": %s\n", rktpar->topic, rktpar->partition, rktpar->offset, rd_kafka_err2str(err));
        }
    }
}

int CKafkaConsumer::stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque)
{
    Log_Info("%s\n", json);
    return 0;
}

void CKafkaConsumer::logger(const rd_kafka_t *rk, int level, const char *fac, const char *buf)
{
    Log_Info("RDKAFKA-%i-%s: %s: %s\n", level, fac, rd_kafka_name(rk), buf);
}

void CKafkaConsumer::msg_consume(rd_kafka_message_t *rkmessage, void *opaque)
{
    CKafkaConsumer * p = (CKafkaConsumer *)opaque;

    if(p && p->m_consumer_callback){
        p->m_consumer_callback(rkmessage, p->m_consumer_callback_param);
        return;
    }

    if(rkmessage->err){
        if(rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF){
            Log_Info("Consumer reached end of %s [%"PRId32"] message queue at offset %"PRId64"\n", rd_kafka_topic_name(rkmessage->rkt), rkmessage->partition, rkmessage->offset);
            return;
        }

        Log_Error("Consume error for topic \"%s\" [%"PRId32"] offset %"PRId64": %s\n", rkmessage->rkt ? rd_kafka_topic_name(rkmessage->rkt) : "", rkmessage->partition, rkmessage->offset, rd_kafka_message_errstr(rkmessage));

        return;
    }

    if(rkmessage->key_len){
        Log_Info("Key: %d: %s\n", (int)rkmessage->key_len, (char *)rkmessage->key);
    }

    Log_Info("%d: %s\n", (int)rkmessage->len, (char *)rkmessage->payload);
}

void CKafkaConsumer::sig_stop(int sig)
{
    Log_Error("## exit ##");
    int run  = 0;
    if (!run)
        exit(1);
    run = 0;
}
void CKafkaConsumer::sig_usr1(int sig)
{
    
}

#if 0
//#include "work_config.h"

/*
   void* _kafka_Process(void* pUser)
   {
   if (pUser)
   {
   CKafkaConsumer * pConsumer = (CKafkaConsumer*)pUser;
   pConsumer->kafka_consumer_process();
   }
   }

   void CKafkaConsumer::globalCreateAndStartKafkaConsumer()
   {
   }
   */
void CKafkaConsumer::parseBrokerStr(char* str, char** pa, int& num, char mode)
{
    char buf[1024] = {'\0'};
    if(str)
    {
        strcpy(buf,str);
        char* p = buf;
        int i = 0;
        int len = strlen(buf) + 1;
        do
        {
            char* q = p;
            p = strchr(p ,mode);
            if(p || i < len); 
            {
                char* tmp = (char*)malloc(128);
                if(p)
                {
                    *p = '\0';
                    p++;
                }
                strcpy(tmp,q);
                pa[num++] = tmp;
                if(p && q)
                    i += (p - q); 
            }
        }while(p && *p != '\0');
    }
}

void CKafkaConsumer::logger(const rd_kafka_t *rk, int level, const char *fac, const char *buf)
{
}

void CKafkaConsumer::stop(int sig)
{
    int run  = 0;
    if (!run)
        exit(1);
    run = 0;
}

void CKafkaConsumer::sig_usr1(int sig) 
{
    //rd_kafka_dump(stdout, rk);
}
 
int CKafkaConsumer::StartConsumer()
{
    if(m_conf == NULL)
        m_conf = rd_kafka_conf_new();

    if(m_conf == NULL)
        return 0;

    rd_kafka_conf_set_log_cb(m_conf, logger);

    char tmp[16] = {'\0'};
    snprintf(tmp, sizeof(tmp), "%i", SIGIO);
    rd_kafka_conf_set(m_conf, "internal.termination.signal", tmp, NULL, 0);
    m_topic_conf = rd_kafka_topic_conf_new();
    if(m_topic_conf == NULL)
        return 0;
    if (rd_kafka_topic_conf_set(m_topic_conf, "auto.offset.reset",
                "smallest", NULL, 0) !=RD_KAFKA_CONF_OK)
        return 0;

    signal(SIGINT, stop);
    signal(SIGUSR1, sig_usr1);

    if(strchr("CO",m_mode))
    {
        if(m_group[0] == '\0')
        {
            strcpy(m_group,CWorkConfig::GetInstance()->GetWorkKafkaGroupId().c_str());;
        }
        if(rd_kafka_conf_set(m_conf, "group.id", m_group,m_errstr, sizeof(m_errstr)) != RD_KAFKA_CONF_OK)
        {
            printf("err: %s \n",m_errstr);
            return 0;
        } 
        if (rd_kafka_topic_conf_set(m_topic_conf, "offset.store.method","broker",m_errstr, sizeof(m_errstr)) !=RD_KAFKA_CONF_OK) 
        {
            printf("%s\n", m_errstr);
            return 0;
        }

        rd_kafka_conf_set_default_topic_conf(m_conf, m_topic_conf);
        rd_kafka_conf_set_rebalance_cb(m_conf, rebalance_cb);
    }
    if(m_rk == NULL)
    {
        if (!(m_rk = rd_kafka_new(RD_KAFKA_CONSUMER, m_conf,m_errstr, sizeof(m_errstr))))
        {
            printf("Failed to create new consumer: %s\n",m_errstr);
            return 0;
        }
    }
    rd_kafka_set_log_level(m_rk, LOG_DEBUG);
    for(int i = 0; i < m_brokernum;i++)
    {
        if (rd_kafka_brokers_add(m_rk, m_brokers[i]) == 0) 
        {
            printf( "No valid brokers specified : %s\n",m_brokers[i]);
            return 0;
        }  
    }
    rd_kafka_poll_set_consumer(m_rk);
    m_topics = rd_kafka_topic_partition_list_new(m_topicnum);
    int is_subscription = 1;
    for(int i =0 ;i < 1;i++ )
    {
        unsigned int partition = -1;
        rd_kafka_topic_partition_list_add(m_topics, m_topic[0],-1)->offset = RD_KAFKA_OFFSET_END;
    }

    rd_kafka_resp_err_t err;
    if (is_subscription) 
    {
        printf("Subscribing to %d topics\n", m_topics->cnt);
        if ((err = rd_kafka_subscribe(m_rk, m_topics))) 
        {
            printf("Failed to start consuming topics: %s\n",rd_kafka_err2str(err));
            return 0;
        }
    } 
    else 
    {
        printf("Assigning %d partitions\n", m_topics->cnt);
        if ((err = rd_kafka_assign(m_rk, m_topics))) 
        {
            printf("Failed to assign partitions: %s\n",rd_kafka_err2str(err));
        }
    }

    m_consumerThread.Begin(_kafka_Process,this);
} 


int CKafkaConsumer::CloseConsumer()
{
    rd_kafka_resp_err_t err = rd_kafka_consumer_close(m_rk);
    if (err)
        printf("Failed to close consumer: %s\n",rd_kafka_err2str(err));
    else
        printf("Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(m_topics);
    rd_kafka_destroy(m_rk);
    m_run = 5;
    while (m_run-- > 0 && rd_kafka_wait_destroyed(1000) == -1)
        printf("Waiting for librdkafka to decommission\n");
}


void CKafkaConsumer::kafka_consumer_process()
{
    while(m_run && !m_consumerThread.GetStop())
    {
        rd_kafka_message_t *rkmessage = NULL;
        rkmessage = rd_kafka_consumer_poll(m_rk, 1000);
        if (rkmessage) 
        {
            msg_consume(rkmessage, NULL);
			rd_kafka_message_destroy(rkmessage);
        }
    }
}

void CKafkaConsumer::proc_msg_consume (rd_kafka_message_t *rkmessage,void *opaque)
{
	CKafkaConsumer* pconsume = (CKafkaConsumer*)opaque;
	if(pconsume)
	{
		pconsume->msg_consume(rkmessage,opaque);
	}
}

int CKafkaConsumer::msg_consume (rd_kafka_message_t *rkmessage,void *opaque)
{
    if(rkmessage == NULL)
        return 0;
    if(rkmessage->err)
    {
        if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION || rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC)
            m_run = 0;
        return 0;
    }
    if(rkmessage->len > 0)
    {
		if(m_pCallBack)
			m_pCallBack(rkmessage,m_pUser);
		return 1;
    }
	return 0;
}

void CKafkaConsumer::print_partition_list (const rd_kafka_topic_partition_list_t *partitions)
{
	
	int i;
    for (i = 0 ; i < partitions->cnt ; i++) 
	{
		partitions->elems[i].offset = RD_KAFKA_OFFSET_END;
	}

}

void CKafkaConsumer::rebalance_cb (rd_kafka_t *rk,rd_kafka_resp_err_t err,rd_kafka_topic_partition_list_t *partitions,void *opaque)
{
	switch (err)
	{
	case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
		print_partition_list(partitions);
		rd_kafka_assign(rk, partitions);
	break;
	case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
		print_partition_list(partitions);
		rd_kafka_assign(rk, NULL);
		break;
	default:
		rd_kafka_assign(rk, NULL);
		break;
	}
}
#endif
