#include "KafkaProducer.h"

#include "Log.h"

CKafkaProducer::CKafkaProducer()
{
    m_kafka_handle              = NULL;
    m_kafka_conf                = NULL;
    m_kafka_topic               = NULL;
    m_kafka_topic_conf          = NULL;
    m_kafka_topic_partition_list = NULL;

    m_partition                 = RD_KAFKA_PARTITION_UA;
}

CKafkaProducer::~CKafkaProducer()
{
    Stop();

    /* 等待producer请求完成 */
    /* 在摧毁生产者之前调用此接口，确保正在排队和正在进行的消息被处理完成。此函数会调用rd_kafka_poll()并触发回调。
     * 成功返回：RD_KAFKA_RESP_ERR_NO_ERROR
     * 失败返回：RD_KAFKA_RESP_ERR__TIMED_OUT
     */
    rd_kafka_flush(m_kafka_handle, 10*1000); //wait for max 10 seconds
    
    /* 销毁topic */
    rd_kafka_topic_destroy(m_kafka_topic);
    /* 销毁producer实例 */
    rd_kafka_destroy(m_kafka_handle);
    /* 释放topic list资源 */
    rd_kafka_topic_partition_list_destroy(m_kafka_topic_partition_list);
}

int CKafkaProducer::Init(char *topic, char *brokers, int partition)
{
    int ret = 0;

    rd_kafka_conf_res_t ret_conf = RD_KAFKA_CONF_OK;
    char errstr[512] = {0};

    /* 创建kafk配置 */
    m_kafka_conf = rd_kafka_conf_new();

    rd_kafka_conf_set_error_cb(m_kafka_conf, err_cb);
    rd_kafka_conf_set_throttle_cb(m_kafka_conf, throttle_cb);
    rd_kafka_conf_set_offset_commit_cb(m_kafka_conf, offset_commit_cb);
    rd_kafka_conf_set_stats_cb(m_kafka_conf, stats_cb);

    /* ---------Producer config------------------- */
    /* 配置kafka各项参数 */
    ret_conf = rd_kafka_conf_set(m_kafka_conf, "queue.buffering.max.messages", "500000", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 1; ret_conf=%d; errstr:%s\n", ret_conf, errstr); 
        return -1;
    }

    ret_conf = rd_kafka_conf_set(m_kafka_conf, "message.send.max.retries", "3", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 2; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }

    ret_conf = rd_kafka_conf_set(m_kafka_conf, "retry.backoff.ms", "500", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 3; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }
        
    /* ---------Kafka topic config------------------- */
    m_kafka_topic_conf = rd_kafka_topic_conf_new();

    ret_conf = rd_kafka_topic_conf_set(m_kafka_topic_conf, "auto.offset.reset", "earliest", errstr, sizeof(errstr));
    if(ret_conf != RD_KAFKA_CONF_OK){
        Log_Error("rd_kafka_conf_set() failed 4; ret_conf=%d; errstr:%s\n", ret_conf, errstr);
        return -1;
    }

    /* 可扩展长度的 主题-分区 链表
     * 创建时指定长度，通过rd_kafka_topic_partition_list_add()添加 主题-分区对，用于订阅消息。
     */
    m_kafka_topic_partition_list = rd_kafka_topic_partition_list_new(1);

    /* 可以add一个以上的topic */
    rd_kafka_topic_partition_list_add(m_kafka_topic_partition_list, topic, partition); 

    m_partition = partition;

    /* ---------Create Kafka handle------------------- */
    /* 创建producer实例  总体结构 
     * conf和topic_conf都是为此结构服务
     * 其中包含rk_brokers链表，rk_topics链表，是必须创建的结构。
     */
    m_kafka_handle = rd_kafka_new(RD_KAFKA_PRODUCER, m_kafka_conf, errstr, sizeof(errstr));

    if(m_kafka_handle == NULL){
        Log_Error("Failed to create Kafka producer: %s\n", errstr);
        return -1;
    }
    
    /* ---------Add broker(s)------------------- */
    /* broker字符串 如:”172.20.51.38:9092” 不写端口，则采用默认端口9092
     * 多个broker  brokerlist = "broker1:10000,broker2"
     * 返回 成功添加的broker个数
     * 添加一个broker也可以通过 设置rd_kafka_conf_t结构中的 "bootstrap.servers" 配置项
     */
    if(brokers && rd_kafka_brokers_add(m_kafka_handle, brokers) < 1){
        Log_Error("No valid brokers specified\n");
        return -2;
    }

    /* Explicitly create topic to avoid per-msg lookups */
    /* 创建topic实例
     * 使用此接口生成的主题操作句柄进行发送消息。
     */
    m_kafka_topic = rd_kafka_topic_new(m_kafka_handle, topic, m_kafka_topic_conf); 
    return ret;
}

//int CKafkaProducer::Send(const string &msg)
int CKafkaProducer::Send(const char *str, int len)
{
    int ret = 0;
    int ret2 = 0;

    if(str == NULL){
        return -1;
    }
    if(len <= 0){
        return -2;
    }
    Log_Info("#######################");

    char* topic = m_kafka_topic_partition_list->elems[0].topic;
    int partition = m_kafka_topic_partition_list->elems[0].partition;

    // char * buf = (char *)malloc(len);
    //  memcpy(buf, str, len);

    /* ------------向kafka服务器发送消息---------------- */
    /* 异步调用将消息发送到指定的topic 
     * */
    /* Topic object
     * 选择分区 (RD_KAFKA_PARTITION_UA 使用内置的分区)
     * 生成payload的副本 RD_KAFKA_MSG_F_COPY; 使用完消息后，将释放消息缓存 RD_KAFKA_MSG_F_FREE
     * 消息体
     * 长度
     * 可选键及其长度
     * */
    ret = rd_kafka_produce(m_kafka_topic, partition, RD_KAFKA_MSG_F_FREE, (void *)str, len, NULL, 0, NULL);

    if(ret == -1){
        rd_kafka_resp_err_t err = rd_kafka_last_error();

        if(err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION){
            Log_Error("No such partition: %"PRId32"\n", partition);
        }else{
            /* 队列满 内部队列受限于queue.buffering.max.messages配置项 */
            Log_Error("produce error: %s%s\n", rd_kafka_err2str(err), err == RD_KAFKA_RESP_ERR__QUEUE_FULL ? " (backpressure)" : "");
        }

        /* Poll to handle delivery reports */
        /* 发送完消息后调用此接口，timeout_ms是毫秒级的时间，函数会阻塞timeout_ms 毫秒等待事件处理，调用设置的回调函数。timeout_ms为0是非阻塞状态。
         */
        rd_kafka_poll(m_kafka_handle, 10); 

        ret = -2;
        goto end;
    }
    Log_Info("#######################");
    
    /* */
    ret2 = rd_kafka_poll(m_kafka_handle, 0);
end:
    //---------------------
    //  free(buf);
    //  buf = NULL;
    return ret;
}

void CKafkaProducer::err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque)
{
    Log_Error("%% ERROR CALLBACK: %s: %s: %s\n", rd_kafka_name(rk), rd_kafka_err2str((rd_kafka_resp_err_t)err), reason);
}

void CKafkaProducer::throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque)
{
    Log_Info("%% THROTTLED %dms by %s (%"PRId32")\n", throttle_time_ms, broker_name, broker_id);
}

void CKafkaProducer::offset_commit_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *offsets, void *opaque)
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

int CKafkaProducer::stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque)
{
    Log_Info("%s\n", json);
    return 0;
}

void CKafkaProducer::Stop()
{
}
