//
// Created by MOMO on 16/12/15.
//

#ifndef SRC_SINGLETON_H
#define SRC_SINGLETON_H

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

#include "Thread.h"
using namespace BroadvTool;

template<typename T>
class Singleton  {
public:
    static T& Instance() {
        if(Singleton::s_instance==0) {
            CreateInstance();
        }
        return *(Singleton::s_instance);
    }

    static T* GetInstance() {
        if(Singleton::s_instance==0) {
            CreateInstance();
        }
        return Singleton::s_instance;
    }

    static T* getInstance() {
        if(Singleton::s_instance==0) {
            CreateInstance();
        }
        return Singleton::s_instance;
    }

    static void Destroy() {
        m_cs.wlock();
        if(Singleton::s_instance!=0) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
        m_cs.unlock();
    }

protected:
    Singleton()	{
        Singleton::s_instance = static_cast<T*>(this);
    }

    ~Singleton() {
        Singleton::s_instance = 0;
    }

private:
    static void CreateInstance(){
        m_cs.wlock();
        if (s_instance == NULL) {
            s_instance = new T();
        }
        m_cs.unlock();
    }

    static void DestroyInstance(T* p) {
        delete p;
    }

private:
    static T *s_instance;
    static CRWLock m_cs;

private:
    explicit Singleton(Singleton const &) { }
    Singleton& operator=(Singleton const&) { return *this; }
};

template<typename T>
T* Singleton<T>::s_instance=0;

template<typename T>
CRWLock Singleton<T>::m_cs;

#endif // BASE_SINGLETON_H_

#endif //SRC_SINGLETON_H
