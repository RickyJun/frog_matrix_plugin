//
// Created by 123 on 2022/6/1.
//具体收集工作：收集、汇聚、上报
//

#ifndef ANDROID_FLUTTERSTACKCOLLECT_H
#define ANDROID_FLUTTERSTACKCOLLECT_H
#include <map>
#include <string>
#include "../../pthread/ThreadTrace.h"
#include "CTimer.hpp"
namespace wechat_backtrace {
    class FlutterStackCollect{
    public:
        //开始收集
        static void StartCollect();
        //上报
        static char* DumpJson2File(time_t timestamp);
        //结束收集
        static void EndCollect();
        //收集
        static void DoCollectStack(uptr *regs,struct sigaltstack &stack);
        //在创建线程后，保存flutter相关线程
        static void on_pthread_created(std::map<pthread_t, pthread_meta_t> &m_pthread_metas);
        static void on_pthread_release(pthread_t pthread);
    private:

        static int intervalMSTime_;
        //堆栈过期时间
        static uint64_t expireMSTime_;
        static CTimer *pTimer_;
        static bool coleetingFlag_;
        //检查堆栈数据是否过去
        static void FilterFlutterPtread(std::map<pthread_t, pthread_meta_t> &m_pthread_metas);
        static char *allThreadIds[256];
        static int allThreadNum;
        static int newestUIThreadId;
        static int uiThreadNums;
        static void getFlutterUIThreadMeta();
        static void getFileName(char * dirPath);

    };

}
#endif //ANDROID_FLUTTERSTACKCOLLECT_H
