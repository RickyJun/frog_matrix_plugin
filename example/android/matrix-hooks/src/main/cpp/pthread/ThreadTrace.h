/*
 * Tencent is pleased to support the open source community by making wechat-matrix available.
 * Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the BSD 3-Clause License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by Yves on 2020-03-11.
//

#ifndef LIBMATRIX_HOOK_THREADTRACE_H
#define LIBMATRIX_HOOK_THREADTRACE_H

#include <pthread.h>
#include <common/HookCommon.h>
#include "PthreadHook.h"
#include <backtrace/Backtrace.h>
//wenwenjun move form ThreadTrace.cpp start
#define THREAD_NAME_LEN 16
#define PTHREAD_BACKTRACE_MAX_FRAMES MAX_FRAME_SHORT
#define PTHREAD_BACKTRACE_MAX_FRAMES_LONG MAX_FRAME_LONG_LONG
#define PTHREAD_BACKTRACE_FRAME_ELEMENTS_MAX_SIZE MAX_FRAME_NORMAL
//wenwenjun move form ThreadTrace.cpp end
//wenwenjun move form ThreadTrace.cpp start
static volatile bool   m_quicken_unwind               = false;
static volatile size_t m_pthread_backtrace_max_frames =
        m_quicken_unwind ? PTHREAD_BACKTRACE_MAX_FRAMES_LONG
                         : PTHREAD_BACKTRACE_MAX_FRAMES;
struct pthread_meta_t {
    pid_t                           tid;
    // fixme using std::string
    char                            *thread_name;
//    char  *parent_name;
    wechat_backtrace::BacktraceMode unwind_mode;

    uint64_t hash;

    wechat_backtrace::Backtrace native_backtrace;

    std::atomic<char *> java_stacktrace;

    bool exited;

    pthread_meta_t() : tid(0),
                       thread_name(nullptr),
//                       parent_name(nullptr),
                       unwind_mode(wechat_backtrace::FramePointer),
                       hash(0),
                       native_backtrace(BACKTRACE_INITIALIZER(m_pthread_backtrace_max_frames)),
                       java_stacktrace(nullptr),
                       exited(false) {
    }

    ~pthread_meta_t() = default;

    pthread_meta_t& operator=(const pthread_meta_t &src){
        this->tid              = src.tid;
        this->thread_name      = src.thread_name;
//        parent_name       = src.parent_name;
        this->unwind_mode      = src.unwind_mode;
        this->hash             = src.hash;
        this->native_backtrace = src.native_backtrace;
        this->java_stacktrace.store(src.java_stacktrace.load(std::memory_order_acquire),
                              std::memory_order_release);
        this->exited = src.exited;
        return *this;
    }
    pthread_meta_t(const pthread_meta_t &src) {
        tid              = src.tid;
        thread_name      = src.thread_name;
//        parent_name       = src.parent_name;
        unwind_mode      = src.unwind_mode;
        hash             = src.hash;
        native_backtrace = src.native_backtrace;
        java_stacktrace.store(src.java_stacktrace.load(std::memory_order_acquire),
                              std::memory_order_release);
        exited = src.exited;
    }
};
//wenwenjun move form ThreadTrace.cpp end
namespace thread_trace {
    void thread_trace_init();

    void add_hook_thread_name(const char *regex_str);

    void pthread_dump_json(const char *path);
    //wenwenjun add
    void pthread_dump_flutter_json(const char *path);

    void enable_quicken_unwind(const bool enable);

    void enable_trace_pthread_release(const bool enable);

    typedef struct {
        pthread_hook::pthread_routine_t wrapped_func;
        pthread_hook::pthread_routine_t origin_func;
        void *origin_args;
    } routine_wrapper_t;

    routine_wrapper_t* wrap_pthread_routine(pthread_hook::pthread_routine_t start_routine, void* args);

    void handle_pthread_create(const pthread_t pthread);

    void handle_pthread_setname_np(pthread_t pthread, const char* name);

    void handle_pthread_release(pthread_t pthread);
}


#endif //LIBMATRIX_HOOK_THREADTRACE_H
