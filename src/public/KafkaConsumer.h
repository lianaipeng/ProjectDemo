
// 参考：https://blog.csdn.net/jfu22/article/details/78194015
/*
 */

#ifndef __KAFKA_CONSUMER_H__
#define __KAFKA_CONSUMER_H__

#include <string.h>

#include <librdkafka/rdkafka.h> 

typedef void (*consumer_callback)(rd_kafka_message_t* rkmessage, void* pUser);

class CKafkaConsumer
{
public:
    CKafkaConsumer();
    ~CKafkaConsumer();
    
    //初始化
    //topic="my_test"; brokers="192.168.1.42:9092"; partitions="0,1,2"; groupId="my_group";
    int Init(char *brokers, char *topic, char *partitions, char *groupId, consumer_callback consumer_cb, void * param_cb);
    //开始获取消息
    int Start(int timeout_ms);
    //停止
    void Stop();

    static void err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque);
    static void throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque);
    static void offset_commit_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *offsets, void *opaque);
    static int stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque);
    static void logger(const rd_kafka_t *rk, int level, const char *fac, const char *buf);
    /* 处理并打印已消费的消息 */
    /* rd_kafka_message_t的成员：
     * err - 返回给应用程序的错误信号。如果该值不是零，payload字段应该是一个错误的消息，err是一个错误码（rd_kafka_resp_err_t）。
     * rkt,partition - 消息的 topic 和 partition 或错误。
     * payload,len - 消息的数据或错误的消息 (err!=0)。
     * key,key_len - 可选的消息键，生产者指定。
     * offset - 消息偏移量.
     */
    static void msg_consume(rd_kafka_message_t *rkmessage, void *opaque);
    static void sig_stop(int sig) ;
    static void sig_usr1(int sig);

private:
    int Message();

private:
    rd_kafka_t *                         m_kafka_handle;  //kafka消息生产者句柄
    rd_kafka_topic_t *                   m_kafka_topic;   //kafka消息主题名称
    rd_kafka_conf_t *                    m_kafka_conf;    //kafka消息配置
    rd_kafka_topic_conf_t *              m_kafka_topic_conf;
    rd_kafka_topic_partition_list_t *    m_kafka_topic_partition_list;
    rd_kafka_queue_t *                   m_kafka_queue;

    consumer_callback                    m_consumer_callback; //消息回调函数
    void *                               m_consumer_callback_param; //消息回调函数的参数
    bool m_bRun;
};
/*
//#include "work_thread.h"
//
public:
    int StartConsumer(); 
    int CloseConsumer();
    void kafka_consumer_process();

public:
    static void parseBrokerStr(char* str,char** pa,int & num,char mode);
    //global function 
    static void globalCreateAndStartKafkaConsumer();
    static void print_partition_list (const rd_kafka_topic_partition_list_t *partitions);
    static void rebalance_cb (rd_kafka_t *rk,rd_kafka_resp_err_t err,rd_kafka_topic_partition_list_t *partitions,void *opaque);
    static void logger(const rd_kafka_t *rk, int level,const char *fac, const char *buf);
    static void sig_usr1 (int sig);
private:
	int msg_consume (rd_kafka_message_t *rkmessage,void *opaque);
	static void proc_msg_consume(rd_kafka_message_t *rkmessage,void *opaque);
private:
    rd_kafka_t *m_rk;  
    char m_mode;
    rd_kafka_conf_t * m_conf;
    rd_kafka_topic_conf_t* m_topic_conf;
    rd_kafka_topic_partition_list_t* m_topics;
	rd_kafka_topic_t  *m_rkt;
    int m_run;
private:
    char  m_group[64];
    char * m_errstr;
    char * m_brokers[64];
    int     m_brokernum;
    char*  m_topic[64];
    int    m_topicnum;
private:
    CSCThread  m_consumerThread;
    MessageCallBack m_pCallBack;
    void*  m_pUser;
    */

#endif
