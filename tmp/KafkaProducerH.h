
/*  
 * CKafkaProducer* KafkaprClient_ = new CKafkaProducer("localhost:9092", "test", 0);
 * KafkaprClient_->Init();
 * KafkaprClient_->Send("小刚dddddddd");
 */

#ifndef __KAFKA_PRODUCER_H__
#define __KAFKA_PRODUCER_H__

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <list>
#include <vector>
#include <fstream>
using std::string;
using std::list;
using std::cout;
using std::endl;
using std::vector;
using std::fstream;

#include <librdkafka/rdkafka.h> 

#define USECPP 0

//#if USECPP
#if 0 
//#include <librdkafka/rdkafkacpp.h> 

class CKafkaProducerCallBack : public RdKafka::DeliveryReportCb 
{
public:
    void dr_cb(RdKafka::Message &message) {
        std::cout << "Message delivery for (" << message.len() << " bytes): " <<
            message.errstr() << std::endl;
        if (message.key())
            std::cout << "Key: " << *(message.key()) << ";" << std::endl;
    }
};

class CKafkaProducerEventCallBack : public RdKafka::EventCb 
{
public:
    void event_cb(RdKafka::Event &event) {
        switch (event.type())
        {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
                    event.str() << std::endl;
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                    break;
            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;
            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n",
                        event.severity(), event.fac().c_str(), event.str().c_str());
                break;
            default:
                std::cerr << "EVENT " << event.type() <<
                    " (" << RdKafka::err2str(event.err()) << "): " <<
                    event.str() << std::endl;
                break;
        }
    }
};
#endif


class CKafkaProducer 
{
public:
    CKafkaProducer(const string &sBrokers, const string &sTopic, int nPartition = 0);
    ~CKafkaProducer();
    
    int Init();
    int Send(const string &msg);
    void Stop();

//#if USECPP
#if 0 
#else
    static void err_cb(rd_kafka_t *rk, int err, const char *reason, void *opaque);
    static void throttle_cb(rd_kafka_t *rk, const char *broker_name, int32_t broker_id, int throttle_time_ms, void *opaque);
    static void offset_commit_cb(rd_kafka_t *rk, rd_kafka_resp_err_t err, rd_kafka_topic_partition_list_t *offsets, void *opaque);
    static int stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque);
#endif

private:
//#if USECPP
#if 0 
    RdKafka::Producer*          m_pProducer = NULL;
    RdKafka::Topic*             m_pTopic = NULL;
    CKafkaProducerCallBack      m_producerCallBack;
    CKafkaProducerEventCallBack m_producerEventCallBack;
#else
    rd_kafka_t *                         m_kafka_handle;  //kafka消息生产者句柄
    rd_kafka_topic_t *                   m_kafka_topic;   //kafka消息主题名称
    rd_kafka_conf_t *                    m_kafka_conf;    //kafka消息配置
    rd_kafka_topic_conf_t *              m_kafka_topic_conf;
    rd_kafka_topic_partition_list_t *    m_kafka_topic_partition_list;
#endif
    string                      m_strTopics;
    string                      m_strBroker;
    int                         m_nPartition;
    bool                        m_bRun;
};

#endif
