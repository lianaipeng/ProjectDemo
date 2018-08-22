
// 参考：https://blog.csdn.net/jfu22/article/details/78194015
/*  
 *  CKafkaProducer* pProducer= new CKafkaProducer();
 *  pProducer->Init("192.168.1.41:9091", "test", 1);
 *  string sTmp = "dddddddd";
 *  pProducer->Send(sTmp.c_str(), sTmp.length());
 */

#ifndef __KAFKA_PRODUCER_H__
#define __KAFKA_PRODUCER_H__

#include <librdkafka/rdkafka.h> 

class CKafkaProducer 
{
public:
    CKafkaProducer();
    ~CKafkaProducer();
    
    int Init(char *topic, char *brokers, int partition);
    int Send(const char *str, int len);
    void Stop();

    static void err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque);
    static void throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque);
    static void offset_commit_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *offsets, void *opaque);
    static int stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque);

private:
    rd_kafka_t *                         m_kafka_handle;  //kafka消息生产者句柄
    rd_kafka_conf_t *                    m_kafka_conf;    //kafka消息配置
    rd_kafka_topic_t *                   m_kafka_topic;   //kafka消息主题名称
    rd_kafka_topic_conf_t *              m_kafka_topic_conf;
    rd_kafka_topic_partition_list_t *    m_kafka_topic_partition_list;

    int                                  m_partition;
};

#endif
