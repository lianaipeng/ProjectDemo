#include "RedisClient.h"

CRedisClient::CRedisClient(string sHost, int nPort)
{
    m_pContext          = NULL;
    m_pReply            = NULL;
    m_sRedisHost        = sHost;
    m_nRedisPort        = nPort;
    m_sSlaveRedisHost   = "";
}

CRedisClient::~CRedisClient()
{
    if (m_pContext)
    {
        redisFree(m_pContext);
        m_pContext = NULL;
    }
}

bool CRedisClient::RedisConnect()
{
    int nRet = 0;
    int nReConnectNum = 3;
    if (GetRedisConnect() < 0) 
    {
        int count;
        for (count = 0; count < nReConnectNum; count++)
        {
            if ((nRet = GetRedisConnect()) == 0)
                return true;
        }
        if (nRet < 0)
        {
            Log_Error("reconnect %d time fail", nReConnectNum);
        }
    }
    return false;
}

int CRedisClient::GetRedisConnect()
{
    if (m_sRedisHost == "" || m_nRedisPort <= 0)
        return -1;

    int nRet = 0;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    m_pContext = redisConnectWithTimeout(m_sRedisHost.c_str() ,m_nRedisPort, timeout);
    if (m_pContext == NULL || m_pContext->err) 
    {
        if (m_pContext) 
        {
            Log_Error("Connect redis timeout, host:%s. error info:%s",
                    m_sRedisHost.c_str(), m_pContext->errstr);
            redisFree(m_pContext);
        } 
        else 
        {
            Log_Error("Connection error: can't allocate redis context");
        }

        m_pContext = NULL;
        nRet = -1;
    }

    if (nRet == 0) 
    {
        struct timeval st_time;
        st_time.tv_sec = 0;
        st_time.tv_usec = 500 * 1000;
        redisSetTimeout(m_pContext, st_time);
    }
    return nRet;
}

bool CRedisClient::ping()
{
    bool bRet = false;
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "ping");
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_STATUS && m_pReply->len == 4) 
                bRet = true;
            else 
                Log_Error("Error.execute ping reply:%p REDIS_REPLY_STATUS:%d len:%ld str:%s",
                        m_pReply, m_pReply->type, m_pReply->len, m_pReply->str);
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute ping reply:%p", m_pReply);
        }
    } 
    else 
    {
        Log_Error("Error.execute ping m_pContext:%p", m_pContext);
    }
    return bRet;
}

string CRedisClient::set(string sTable, string sKey)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "set %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STATUS && m_pReply->len > 0)
            {
                sRet = m_pReply->str;
            }
            else
            {
                Log_Error("Error.execute set reply:%p REDIS_REPLY_STATUS:%d len:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute set reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute set m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return sRet;
}

string CRedisClient::setex(string sKey, string sValue, unsigned int nSeconds)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "setex %s %d %s",
                sKey.c_str(), nSeconds, sValue.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STATUS && m_pReply->len > 0)
            {
                sRet = m_pReply->str;
                /*
                   Log_Error("Error.execute setex reply:%p key:%s value:%s sRet:%s",
                   m_pReply, sKey.c_str(), sValue.c_str(), sRet.c_str());
                   */
            }
            else
            {
                Log_Error("Error.execute setex reply:%p REDIS_REPLY_STATUS:%d len:%ld key:%s value:%s second:%d",
                        m_pReply, m_pReply->type, m_pReply->len, sKey.c_str(), sValue.c_str(), nSeconds);
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute setex reply:%p key:%s value:%s second:%d",
                    m_pReply, sKey.c_str(), sValue.c_str(), nSeconds);
        }
    }
    else
    {
        Log_Error("Error.execute setex m_pContext:%p key:%s value:%s second:%d",
                m_pContext, sKey.c_str(), sValue.c_str(), nSeconds);

    }
    return sRet;
}

string CRedisClient::get(string sTable)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "get %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STRING && m_pReply->len >= 0)
            {
                sRet = m_pReply->str;
            }
            else if (m_pReply->type != REDIS_REPLY_NIL)
            {
                Log_Error("Error.execute get reply:%p REDIS_REPLY_STATUS:%d len:%ld table:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute get reply:%p table:%s",
                    m_pReply, sTable.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute get m_pContext:%p table:%s",
                m_pContext, sTable.c_str());
    }
    return sRet;
}

int CRedisClient::append(string sTable, string sValue)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "append %s %s", sTable.c_str(), sValue.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute append reply:%p REDIS_REPLY_STATUS:%d len:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str(), sValue.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute append reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sValue.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute append m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sValue.c_str());
    }
    return nRet;
}

int CRedisClient::sadd(string sTable, string sKey)
{
    int nRet = -1;
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "sadd %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
                nRet = m_pReply->integer;
            else
                Log_Error("Error.execute sadd reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute sadd reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    } 
    else 
    {
        Log_Error("Error.execute sadd m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}

int CRedisClient::srem(string sTable, string sKey)
{
    int nRet = -1;
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "srem %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0) 
                nRet = m_pReply->integer;
            else
                Log_Error("Error.execute srem reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute srem reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    } 
    else 
    {
        Log_Error("Error.execute srem m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}

int CRedisClient::sismember(string sTable, string sKey)
{
    int nRet = -1;
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "sismember %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
                nRet = m_pReply->integer;
            else
                Log_Error("Error.execute sismember reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute sismember reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    } 
    else 
    {
        Log_Error("Error.execute sismember m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}

int CRedisClient::scard(string sTable)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "scard %s", sTable.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0) 
                nRet = m_pReply->integer;
            else
                Log_Error("Error.execute scard reply:%p REDIS_REPLY_STATUS:%d integer:%ld key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str());
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute scard reply:%p key:%s",
                    m_pReply, sTable.c_str());
        }
    } 
    else 
    {
        Log_Error("Error.execute scard m_pContext:%p key:%s",
                m_pContext, sTable.c_str());
    }
    return nRet;
}

string CRedisClient::spop(string sTable)
{
    string sRet = "";
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "spop %s", sTable.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_NIL) 
            {
                Log_Error("Error.execute spop reply:%p REDIS_REPLY_STATUS:%d key:%s, key is not exist or empty",
                        m_pReply, m_pReply->type, sTable.c_str());
            } 
            else if (m_pReply->type == REDIS_REPLY_STRING && m_pReply->len > 0) 
            {
                sRet = m_pReply->str;
                /*
                   Log_Error("Error.execute spop reply:%p key:%s %s",
                   m_pReply, sTable.c_str(), sRet.c_str());
                   */
            } 
            else 
            {
                Log_Error("Error.execute spop reply:%p REDIS_REPLY_STATUS:%d len:%ld key:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        } 
        else 
        {
            Log_Error("Error.execute spop reply:%p key:%s",
                    m_pReply, sTable.c_str());
        }
    } 
    else 
    {
        Log_Error("Error.execute spop m_pContext:%p key:%s",
                m_pContext, sTable.c_str());
    }
    return sRet;
}

string CRedisClient::srandmember(string sTable)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "srandmember %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STRING && m_pReply->len > 0) //"" 也可以存在
            {
                sRet = m_pReply->str;
            }
            else if(m_pReply->type != REDIS_REPLY_NIL)
            {
                Log_Error("Error.execute srandmember reply:%p REDIS_REPLY_STATUS:%d len:%ld key:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute srandmember reply:%p key:%s",
                    m_pReply, sTable.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute srandmember m_pContext:%p key:%s",
                m_pContext, sTable.c_str());
    }
    return sRet;
}

int CRedisClient::smembers(string sKey, list<string> &sValue)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext,"smembers %s", sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_ARRAY && m_pReply->elements >= 0)
            {
                for (int i=0; i<m_pReply->elements; i++)
                {
                    if (m_pReply->element[i]->str == NULL)
                    {
                        Log_Error("Error.execute smembers reply:%p REDIS_REPLY_STATUS:%d elements:%ld key:%s i:%d element:%s",
                                m_pReply, m_pReply->type, m_pReply->elements, sKey.c_str(), i, m_pReply->element[i]->str);
                        sValue.push_back("");
                    }
                    else
                    {
                        sValue.push_back(m_pReply->element[i]->str);
                        //Log_Error("OK.execute smembers key:%s", m_pReply->element[i]->str);
                    }
                }
                nRet = m_pReply->elements;
            }
            else if (m_pReply->type == REDIS_REPLY_ERROR)
            {
                Log_Error("Error.execute smembers reply:%p REDIS_REPLY_STATUS:%d elements:%ld key:%s",
                        m_pReply, m_pReply->type, m_pReply->elements, sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute smembers reply:%p key:%s",
                    m_pReply, sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute smembers m_pContext:%p key:%s",
                m_pContext, sKey.c_str());
    }
    return nRet;
}

int CRedisClient::hset(string sTable, string sKey, string sValue)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "hset %s %s %s", sTable.c_str(), sKey.c_str(), sValue.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute hset reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s value:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str(), sValue.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute hset reply:%p table:%s key:%s value:%s",
                    m_pReply, sTable.c_str(), sKey.c_str(), sValue.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute hset m_pContext:%p table:%s key:%s value:%s",
                m_pContext, sTable.c_str(), sKey.c_str(), sValue.c_str());
    }
    return nRet;
}

int CRedisClient::hdel(string sTable, string sKey)
{
    int nRet;
    if (m_pContext) 
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "hdel %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply) 
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute hdel reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute hdel reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute hdel m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}

int CRedisClient::del(string sTable)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "del %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute del reply:%p REDIS_REPLY_STATUS:%d len:%ld table:%s",
                        m_pReply, m_pReply->type, m_pReply->len, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute del reply:%p table:%s",
                    m_pReply, sTable.c_str());
        }
    } 
    else
    {
        Log_Error("Error.execute del m_pContext:%p table:%s",
                m_pContext, sTable.c_str());
    }
    return nRet;
}

string CRedisClient::hget(string sTable, string sKey)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "hget %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STRING && m_pReply->len >= 0)
            {
                sRet = m_pReply->str;
            }
            else if (m_pReply->type != REDIS_REPLY_NIL)
            {
                Log_Error("Error.execute hget reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute hget reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute hget m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return sRet;
}

int CRedisClient::hgetall(string sTable, map<string, string> &mapTmp)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext,"hgetall %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_ARRAY && m_pReply->elements >= 0)
            {
                for (int i=0; i<m_pReply->elements; i+=2)
                {
                    if (m_pReply->element[i]->str != NULL && m_pReply->element[i+1]->str != NULL)
                    {
                        mapTmp[m_pReply->element[i]->str] = m_pReply->element[i+1]->str;
                    }
                }
                nRet = m_pReply->elements/2;
            }
            else
            {
                Log_Error("Error.execute hgetall reply:%p REDIS_REPLY_STATUS:%d elements:%ld table:%s",
                        m_pReply, m_pReply->type, m_pReply->elements, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute hgetall reply:%p table:%s",
                    m_pReply, sTable.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute hgetall m_pContext:%p table:%s",
                m_pContext, sTable.c_str());
    }
    return nRet;
}

int CRedisClient::rpush(string sTable, string sKey)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "rpush %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute rpush reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute rpush reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute rpush m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}

int CRedisClient::lpush(string sTable, string sKey)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "lpush %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute lpush reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute lpush reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute lpush m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}


int CRedisClient::lrem(string sTable, int count, string sKey)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "lrem %s %d %s", sTable.c_str(), count, sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
            }
            else
            {
                Log_Error("Error.execute lrem reply:%p REDIS_REPLY_STATUS:%d integer:%ld table:%s count:%d key:%s",
                        m_pReply, m_pReply->type, m_pReply->integer, sTable.c_str(), count, sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute lrem reply:%p table:%s count:%d key:%s",
                    m_pReply, sTable.c_str(), count, sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute lrem m_pContext:%p table:%s count:%d key:%s",
                m_pContext, sTable.c_str(), count, sKey.c_str());
    }
    return nRet;
}

string CRedisClient::rpop(string sTable)
{
    string sRet = "";
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "rpop %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_STRING && m_pReply->len > 0)
            {
                sRet = m_pReply->str;
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute rpop reply:%p table:%s",
                    m_pReply, sTable.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute rpop m_pContext:%p table:%s",
                m_pContext, sTable.c_str());
    }
    return sRet;
}

int CRedisClient::llen(string sTable, int &nlen)
{
    int nRet = -1;
    nlen = 0;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "llen %s", sTable.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_INTEGER && m_pReply->integer >= 0)
            {
                nRet = m_pReply->integer;
                nlen = m_pReply->integer;
            }
            else if (m_pReply->type == REDIS_REPLY_ERROR)
            {
                Log_Error("Error.execute llen reply:%p REDIS_REPLY_STATUS:%d table:%s, table is not list",
                        m_pReply, m_pReply->type, sTable.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute llen reply:%p table:%s",
                    m_pReply, sTable.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute llen m_pContext:%p table:%s",
                m_pContext, sTable.c_str());
    }
    return nRet;
}

int CRedisClient::hmget(string sTable, string sKey, list<string> &lst)
{
    int nRet = -1;
    if (m_pContext)
    {
        m_pReply = (redisReply*)redisCommand(m_pContext, "hmget %s %s", sTable.c_str(), sKey.c_str());
        if (m_pReply)
        {
            if (m_pReply->type == REDIS_REPLY_ARRAY && m_pReply->elements >= 0)
            {
                for (int i=0; i<m_pReply->elements; i++) {
                    if (m_pReply->element[i]->str == NULL)
                    {
                        Log_Error("Error.execute hmget reply:%p table:%s key:%s elements%s",
                                m_pReply, sTable.c_str(), sKey.c_str(), m_pReply->element[i]->str);
                        lst.push_back("");
                    }
                    else
                    {
                        lst.push_back(m_pReply->element[i]->str);
                    }
                }
                nRet = m_pReply->elements;
            }
            else
            {
                Log_Error("Error.execute hmget reply:%p REDIS_REPLY_STATUS:%d elements:%ld table:%s key:%s",
                        m_pReply, m_pReply->type, m_pReply->elements, sTable.c_str(), sKey.c_str());
            }
            freeReplyObject(m_pReply);
            m_pReply = NULL;
        }
        else
        {
            Log_Error("Error.execute hmget reply:%p table:%s key:%s",
                    m_pReply, sTable.c_str(), sKey.c_str());
        }
    }
    else
    {
        Log_Error("Error.execute hmget m_pContext:%p table:%s key:%s",
                m_pContext, sTable.c_str(), sKey.c_str());
    }
    return nRet;
}
