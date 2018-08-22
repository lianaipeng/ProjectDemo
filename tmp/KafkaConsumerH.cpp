#include "KafkaConsumer.h"

#include "Log.h"

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


CKafkaConsumer::CKafkaConsumer(const string& brokers, const string& topics, string groupid, int32_t nPartition /*= 0*/, int64_t offset /*= 0*/)
{
    m_strBrokers                = brokers;
    m_strTopics                 = topics;
    m_strGroupid                = groupid;
    m_nPartition                = nPartition;
    m_nCurrentOffset            = offset;
}

CKafkaConsumer::~CKafkaConsumer()
{
    Stop();
}

bool CKafkaConsumer::Init()
{
    /* 创建kafka 配置 */
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if(!conf){
        Log_Error("RdKafka create global conf failed");
        return false;
    }
    std::string errstr;
    /* 设置broker list */
    if (conf->set("metadata.broker.list", m_strBrokers, errstr) != RdKafka::Conf::CONF_OK){
        Log_Error("RdKafka conf set brokerlist failed:%s", errstr.c_str());
    }

    /* 设置consumer group*/
    if (conf->set("group.id", m_strGroupid, errstr) != RdKafka::Conf::CONF_OK){
        Log_Error("RdKafka conf set group.id failed:%s", errstr.c_str());
    }

    std::string strfetch_num = "10240000";
    /* 每次从单个分区中拉取消息的最大尺寸*/
    if(conf->set("max.partition.fetch.bytes", strfetch_num, errstr) != RdKafka::Conf::CONF_OK){
        Log_Error("RdKafka conf set max.partition failed:%s", errstr.c_str());
    }

    /* 创建kafka consumer实例*/
    m_pKafkaConsumer = RdKafka::Consumer::create(conf, errstr);
    if(!m_pKafkaConsumer){
        Log_Error("failed to ceate consumer");
    }
    delete conf;

    RdKafka::Conf *tconf = nullptr;
    /* 创建kafka topic的配置*/
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    if(!tconf){
        Log_Error("RdKafka create topic conf failed");
        return false;
    }
    if(tconf->set("auto.offset.reset", "smallest", errstr) != RdKafka::Conf::CONF_OK){
        Log_Error("RdKafka conf set auto.offset.reset failed:%s", errstr.c_str());
    }
    m_pTopic = RdKafka::Topic::create(m_pKafkaConsumer, m_strTopics, tconf, errstr);
    if(!m_pTopic){
        Log_Error("RdKafka create topic failed:%s", errstr.c_str());
    }
    delete tconf;

    RdKafka::ErrorCode resp = m_pKafkaConsumer->start(m_pTopic, m_nPartition, m_nLastOffset);
    if (resp != RdKafka::ERR_NO_ERROR){
        Log_Error("failed to start consumer:%s", errstr.c_str());
    }
    return true;
}

void CKafkaConsumer::Msg(RdKafka::Message *message, void *opt)
{
    switch(message->err()){
        case RdKafka::ERR__TIMED_OUT:
            break;
        case RdKafka::ERR_NO_ERROR:
            cout<<("*s\n",
                    static_cast<int>(message->len()),
                    static_cast<const char *>(message->payload())) << endl;
            m_nLastOffset = message->offset();
            break;
        case RdKafka::ERR__PARTITION_EOF:
            Log_Error("Reached the end of the queue, offset:%d", m_nLastOffset);
            break;
        case RdKafka::ERR__UNKNOWN_TOPIC:
        case RdKafka::ERR__UNKNOWN_PARTITION:
            Log_Error("Consume failed:%s", message->errstr());
            Stop();
            break;
        default:
            Stop();
            break;
    }
}

void CKafkaConsumer::Start(int timeout_ms)
{
    RdKafka::Message *msg = nullptr;
    m_bRun = true;
    while (m_bRun){
        msg = m_pKafkaConsumer->consume(m_pTopic, m_nPartition, timeout_ms);
        Msg(msg, nullptr);
        /* 阻塞等待消息 */
        m_pKafkaConsumer->poll(0);
        delete msg;
    }
    
    /* 停止消费 */
    m_pKafkaConsumer->stop(m_pTopic, m_nPartition);

    /* 回收资源 */
    if(m_pTopic){
        delete m_pTopic;
        m_pTopic = nullptr;
    }
    if(m_pKafkaConsumer){
        delete m_pKafkaConsumer;
        m_pKafkaConsumer = nullptr;
    }
    /*销毁kafka实例*/
    RdKafka::wait_destroyed(5000);
}

void CKafkaConsumer::Stop()
{
//#if USECPP
#if 0 
#else
    fka_flush(m_kafka_handle, 10*1000); //wait for max 10 seconds

    rd_kafka_queue_destroy(m_kafka_queue);
    rd_kafka_topic_destroy(m_kafka_topic);
    rd_kafka_destroy(m_kafka_handle);
    rd_kafka_topic_partition_list_destroy(m_kafka_topic_partition_list);
#endif
    m_bRun = false;
}


#if 0
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
