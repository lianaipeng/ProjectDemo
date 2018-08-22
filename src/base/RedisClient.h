#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <list>
#include <string>
#include <map>
#include "hiredis/hiredis.h"
using namespace std;

#include "Log.h"

class CRedisClient
{
public:
    CRedisClient() {}
    CRedisClient(string sHost, int nPort);
    virtual ~CRedisClient();

    int GetRedisConnect();

    /*
     * 使用客户端向 Redis 服务器发送一个 PING ，
     * 如果服务器运作正常的话，会返回一个 PONG 。
     */
    bool ping();

    // ################## SET 相关
    /*
     * 成功返回 OK
     */
    string set(string sTable, string sKey);
    /*
     * 成功返回 OK
     */
    string setex(string sKey, string sValue, unsigned int nSeconds);
    /*
     * 当 key 不存在时，返回 nil ，否则，返回 key 的值。
     * 如果 key 不是字符串类型，那么返回一个错误。
     */
    string get(string sTable);
    /*
     * 如果 key 已经存在并且是一个字符串， APPEND 命令将 value 追加到 key 原来的值的末尾。
     * 如果 key 不存在， APPEND 就简单地将给定 key 设为 value ，就像执行 SET key value 一样。
     * 返回值 追加 value 之后， key 中字符串的长度
     */
    int append(string sTable, string sValue);


    // ################## SET 相关
    /* 添加重复返回0, 添加成功返回个数
     */
    int sadd(string sTable, string sKey);
    /* 
     * 添加成功返回个数
     */
    int srem(string sTable, string sKey);
    /*
     * 如果 member 元素是集合的成员，返回 1 。
     * 如果 member 元素不是集合的成员，或 key 不存在，返回 0 。
     */
    int sismember(string sTable, string sKey);
    /*
     * 返回集合 key 的基数(集合中元素的数量)。
     * 当 key 不存在时，返回 0 。
     */
    int scard(string sTable);
    /*
     * 移除并返回集合中的一个随机元素。
     * 当 key 不存在或 key 是空集时，返回 nil 。
     */
    string spop(string sTable);
    /*
     * 这个函数没有完全实现
     * 只返回 一个随机值 不删除
     * 如果集合为空，返回 nil
     */
    string srandmember(string sTable);
    /*
     * 返回集合 key 中的所有成员。
     * 此时理论上不会返回 nil 所有判断元素是否为空 多余单保险
     */
    int smembers(string sKey, list<string> &sValue);


    // ################## HASH 相关
    /*
     * 当 key 不存在或 key 是空集时，返回 nil 。
     */
    int hset(string sTable, string sKey, string sValue);
    /*
     * 删除单个域 返回1
     * 删除不存在的域 返回0
     */
    int hdel(string sTable, string sKey);
    /*
     * 删除给定的一个或多个 key 。
     * 被删除 key 的数量。
     */
    int del(string sTable);
    /*
     * 当给定域不存在或是给定 key 不存在时，返回 nil 。
     */
    string hget(string sTable, string sKey);
    /*
     *
     */
    int hgetall(string sTable, map<string, string> &mapTmp);

    /*
     * 执行 RPUSH 操作后，表的长度。
     */
    int rpush(string sTable, string sKey);
    /*
     * 执行 LPUSH 命令后，列表的长度。
     */
    int lpush(string sTable, string sKey);
    /*
     * 根据参数 count 的值，移除列表中与参数 value 相等的元素
     * count > 0 移除count 个 value
     * count = 0 所有
     * count < 0 从表尾到表头, 移除 count 个 value
     * 返回值 为移除个数
     */
    int lrem(string sTable, int count, string sKey);
    /*
     * 列表的尾元素。
     * 当 key 不存在时，返回 nil 。
     */
    string rpop(string sTable);
    /*
     * 执行 LPUSH 命令后，列表的长度。
     */
    int llen(string sTable,int &nlen);
    /*
     * 返回哈希表 key 中，一个或多个给定域的值。
     * 返回元素个数
     */
    int hmget(string sTable, string sKey, list<string> &lst);

private:
    bool RedisConnect(); //redis 链接

private:
    redisContext*   m_pContext;
    redisReply*     m_pReply;
    string          m_sRedisHost;
    int             m_nRedisPort;
    string          m_sSlaveRedisHost;
};

//extern CRedisClient *NewRedisConnect(string strIp, int nPort);

#endif /* __REDIS_CLIENT_H__ */
