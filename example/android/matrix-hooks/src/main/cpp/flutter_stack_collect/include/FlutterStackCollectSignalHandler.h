//
// Created by 123 on 2022/6/1.
//收集信号处理
//

#ifndef ANDROID_FLUTTERSTACKCOLLECTSIGNALHANDLER_H
#define ANDROID_FLUTTERSTACKCOLLECTSIGNALHANDLER_H
#include <csignal>
#include <map>
#include "../../pthread/ThreadTrace.h"
namespace wechat_backtrace{
    class FlutterStackCollectSignalHandler{
    public:
        static void RegisterFlutterStackCollectSignalHandler();
        static void SendSignalToFlutterPthread(std::map<pthread_t, pthread_meta_t> &m_pthread_metas);
    private:
        static void Signalhandler(int signal,siginfo_t *info,void *context_);
    };

}

#endif //ANDROID_FLUTTERSTACKCOLLECTSIGNALHANDLER_H
