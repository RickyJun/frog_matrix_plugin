//
// Created by 123 on 2022/6/1.
//
#include "include/FlutterStackCollectSignalHandler.h"
#include "include/FlutterStackCollect.h"
#include<unistd.h>
#include <sys/syscall.h>
namespace wechat_backtrace{
     void FlutterStackCollectSignalHandler::RegisterFlutterStackCollectSignalHandler(){
        struct sigaction act = {};
        act.sa_sigaction = Signalhandler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
        int res = sigaction(SIGPROF,&act,NULL);

    }
    void FlutterStackCollectSignalHandler::Signalhandler(int signal,siginfo_t *info,void *context_){
        if(signal != SIGPROF){
            return;
        }
        //fpunwind得到堆栈，存储最新10秒钟的堆栈收集到内存中；50ms一次，共200次的堆栈数据
        FlutterStackCollect::DoCollectStack();
        printf("FlutterStackCollect::DoCollectStack()");
    }

    void FlutterStackCollectSignalHandler::SendSignalToFlutterPthread(
            std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
         for(auto it = m_pthread_metas.rbegin() ; it != m_pthread_metas.rend() ; it++){
             pthread_meta_t meta = it->second;
             syscall(__NR_tgkill,getpid(),meta.tid,SIGPROF);
         }
    }

}
