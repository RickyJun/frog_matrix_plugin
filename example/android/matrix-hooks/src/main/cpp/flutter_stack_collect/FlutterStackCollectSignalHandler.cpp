//
// Created by 123 on 2022/6/1.
//
#include "include/FlutterStackCollectSignalHandler.h"
#include "backtrace/BacktraceDefine.h"
#include "include/FlutterStackCollect.h"
#include<unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <asm/sigcontext.h>
namespace wechat_backtrace{
     void FlutterStackCollectSignalHandler::RegisterFlutterStackCollectSignalHandler(){
        struct sigaction act = {};
        act.sa_sigaction = Signalhandler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
        int res = sigaction(SIGPROF,&act,NULL);
 //       int sigs1[11] = {SIGPROF,SIGILL,SIGABRT,SIGBUS,SIGFPE,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,SIGXCPU,SIGXFSZ};
//        int sigs2[11] = {SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,SIGSYS,SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM};
//        int sigs3[4] = {SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU};
//        int resI = 0;
//        for(int i = 0 ; i < 1 ; i++,resI++){
//            //默认会导致进程流产的信号有
//            res[resI] = sigaction(sigs1[i],&act,NULL);
//        }
//         for(int i = 0 ; i < 11 ; i++,resI++){
//             //默认会导致进程流产的信号有
//             res[resI] = sigaction(sigs2[i],&act,NULL);
//         }
//         for(int i = 0 ; i < 4 ; i++,resI++){
//             //默认会导致进程流产的信号有
//             res[resI] = sigaction(sigs3[i],&act,NULL);
//         }
     }
    void FlutterStackCollectSignalHandler::Signalhandler(int signal,siginfo_t *info,void *context_){
        if(signal != SIGPROF){
            std::cout << "Signalhandler:" << signal << std::endl;
            return;
        }
        auto* ucontextPtr = reinterpret_cast<ucontext_t*>(context_);
        auto ucontext= (ucontext_t)(*ucontextPtr);
        sigcontext mcontext = ucontext.uc_mcontext;
       // uptr regs[FP_MINIMAL_REG_SIZE];
        //GetFramePointerMinimalRegs(regs);
        //fpunwind得到堆栈，存储最新10秒钟的堆栈收集到内存中；50ms一次，共200次的堆栈数据
/*
 * arm
struct sigcontext {
  unsigned long trap_no;
  unsigned long error_code;
  unsigned long oldmask;
  unsigned long arm_r0;
  unsigned long arm_r1;
  unsigned long arm_r2;
  unsigned long arm_r3;
  unsigned long arm_r4;
  unsigned long arm_r5;
  unsigned long arm_r6;
  unsigned long arm_r7;
  unsigned long arm_r8;
  unsigned long arm_r9;
  unsigned long arm_r10;
  unsigned long arm_fp;
  unsigned long arm_ip;
  unsigned long arm_sp;
  unsigned long arm_lr;
  unsigned long arm_pc;
  unsigned long arm_cpsr;
  unsigned long fault_address;
};
 */
#if defined(__arm__)
        uptr regs[31];
        regs[29] = mcontext.arm_fp;
        regs[30] = mcontext.arm_pc;
        FlutterStackCollect::DoCollectStack(reinterpret_cast<uptr *>(regs),ucontext.uc_stack);
#elif defined(__aarch64__)
        FlutterStackCollect::DoCollectStack(reinterpret_cast<uptr *>(mcontext.regs),ucontext.uc_stack);
#else
        FlutterStackCollect::DoCollectStack(reinterpret_cast<uptr *>(mcontext.gregs),ucontext.uc_stack);
#endif
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
