//
// Created by 123 on 2022/6/1.
//
#include "include/FlutterStackCollect.h"
#include "Utils.h"
#include "include/FlutterStackCollectSignalHandler.h"
#include <cJSON.h>
#include <backtrace/Backtrace.h>
#include<iostream>
#define wechat_backtrace_tag "[wechat_backtrace] "
//线程数据
static std::map<pthread_t, pthread_meta_t> m_flutter_pthread_metas;
//存储规定时间内每一次收集的堆栈，
static std::map<pthread_t,std::vector<const wechat_backtrace::Backtrace>> m_flutter_pthread_native_backtrace_buffer;
typedef std::lock_guard<std::mutex> flutter_pthread_meta_lock;
static std::mutex  m_flutter_pthread_meta_mutex;
namespace wechat_backtrace {
    uint64_t FlutterStackCollect::expireMSTime_ = 3000;
    int FlutterStackCollect::intervalMSTime_ = 50;
    time_t FlutterStackCollect::firstRecordMSTime_ = 0;
    CTimer* FlutterStackCollect::pTimer_ = nullptr;
    bool FlutterStackCollect::coleetingFlag_ = false;
    void FlutterStackCollect::StartCollect() {
        //
        FlutterStackCollectSignalHandler::RegisterFlutterStackCollectSignalHandler();
        //
        pTimer_ = new CTimer("CTIME_wechat_backtrace_FlutterStackCollect");
        pTimer_->AsyncLoop(intervalMSTime_, [](auto  arg){
            if(coleetingFlag_ == false){
                FlutterStackCollectSignalHandler::SendSignalToFlutterPthread(m_flutter_pthread_metas);
            }
        }, NULL);
    }
    char* FlutterStackCollect::DumpJson2File() {
        try{

            flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
            cJSON *report_obj = cJSON_CreateArray();
            //
            std::map<uptr,cJSON*> frameDetailJsons;
            //通过次数
            std::map<uptr,uint>  frameDetail_counts;
             auto frame_detail_lambda = [&frameDetailJsons,&frameDetail_counts](wechat_backtrace::FrameDetail detail) -> void {
                 //汇聚成jsonstr返回，符号化在服务端用脚本执行
                 /*
                  * {
                  * "object_name":"",so库名称
                  * "object_addr":1231,so库基地址
                  * "symbol_name":"",符号
                  * "symbol_addr":1232,符号地址
                  * "instruction_addr":123,指令地址（即pc值）
                  * "repeat_count":9 ，重复次数
                  * }
                  * */
                 if(!frameDetailJsons.count(detail.rel_pc)){
                     cJSON *item_json = cJSON_CreateObject();
                     frameDetailJsons.emplace(detail.rel_pc,item_json);
                     cJSON_AddStringToObject(item_json,"object_name",detail.map_name);
                     cJSON_AddNumberToObject(item_json,"object_addr",0);
                     //符号化得到
                     cJSON_AddStringToObject(item_json,"symbol_name",detail.function_name);
                     //符号化得到
                     cJSON_AddNumberToObject(item_json,"symbol_addr",0);
                     char *rel_pc = new char[256];
                     snprintf(rel_pc, 256, "%ld", (unsigned long)detail.rel_pc);
                     cJSON_AddStringToObject(item_json,"instruction_addr",rel_pc);
                 }
                if(!frameDetail_counts.count(detail.rel_pc)){
                    frameDetail_counts.emplace(detail.rel_pc,1);
                }else{
                    frameDetail_counts[detail.rel_pc]++;
                }
            };
            //恢复堆栈，恢复so虚拟地址等
            for(auto it = m_flutter_pthread_native_backtrace_buffer.rbegin() ; it != m_flutter_pthread_native_backtrace_buffer.rend() ; it++){
                std::vector<const wechat_backtrace::Backtrace> pthread_meta_vertor = it->second;
                for(const auto& native_backtrace:pthread_meta_vertor){
                    if (native_backtrace.frame_size == 0) {
                        continue;
                    }
                    //恢复
                    wechat_backtrace::restore_frame_detail(native_backtrace.frames.get(),
                                                           native_backtrace.frame_size,
                                                           frame_detail_lambda);
                }
            }

            for(auto it:frameDetail_counts){
                cJSON_AddNumberToObject(frameDetailJsons[it.first],"repeat_count",(double)it.second);
                cJSON_AddItemToArray(report_obj,frameDetailJsons[it.first]);
            }
            return cJSON_Print(report_obj);
        }catch (const char* errStr){
            std::cout << wechat_backtrace_tag << errStr << std::endl;
            return const_cast<char *>(errStr?errStr:"error");
        }catch (...){
            std::cout << wechat_backtrace_tag << "errStr" << std::endl;
            return const_cast<char *>("error...");
        }
    }

    void FlutterStackCollect::EndCollect() {
        if(pTimer_){
            pTimer_->Cancel();
        }
    }
    //收集
    void FlutterStackCollect::DoCollectStack(uptr *regs) {
        coleetingFlag_ = true;
        flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
        for(auto it = m_flutter_pthread_metas.rbegin() ; it != m_flutter_pthread_metas.rend() ; it++){
            pthread_meta_t meta = it->second;
            //unwind
            wechat_backtrace::unwind_pthread_adapter(meta.native_backtrace.frames.get(),regs,it->first,
                                             meta.native_backtrace.max_frames,
                                             meta.native_backtrace.frame_size);
            uint64_t native_hash = hash_backtrace_frames(&(meta.native_backtrace));
            if (native_hash) {
                meta.hash = native_hash;
            }
            //PushToBuffer
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpMicro
                    = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            time_t currrentMSTime = tpMicro.time_since_epoch().count();
            //check is expire for expireMSTime_
            if(firstRecordMSTime_ == 0 || currrentMSTime - firstRecordMSTime_ > expireMSTime_){
                firstRecordMSTime_ = currrentMSTime;
                m_flutter_pthread_native_backtrace_buffer.clear();
            }
            //save this meta
            wechat_backtrace::Backtrace native_backtrace;
            native_backtrace.frame_size = meta.native_backtrace.frame_size;
            native_backtrace.max_frames = meta.native_backtrace.max_frames;
            native_backtrace.frames = meta.native_backtrace.frames;
            m_flutter_pthread_native_backtrace_buffer[it->first].push_back(native_backtrace);
        }
        coleetingFlag_ = false;
    }
    void FlutterStackCollect::on_pthread_created(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        //filter
        FilterFlutterPtread(m_pthread_metas);
    }

    void FlutterStackCollect::FilterFlutterPtread(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
        for (auto it = m_pthread_metas.rbegin(); it != m_pthread_metas.rend(); it++) {
            pthread_meta_t meta = it->second;
            if(m_flutter_pthread_metas.count(it->first)){
                continue;
            }
            //TODO 仅收集ui线程的堆栈
            //strstr(meta.thread_name,".raster") ||
            if(strstr(meta.thread_name,".ui")){
                m_flutter_pthread_metas.clear();
                m_flutter_pthread_metas[it->first] = meta;
                std::cout << wechat_backtrace_tag << "FilterFlutterPtread : " << meta.thread_name << std::endl;
            }
        }
    }

    void
    FlutterStackCollect::on_pthread_release(pthread_t pthread) {
        flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
        if (m_flutter_pthread_metas.count(pthread) && m_flutter_pthread_metas[pthread].exited) {
            m_flutter_pthread_metas.erase(pthread);
            m_flutter_pthread_native_backtrace_buffer.erase(pthread);
            std::cout << wechat_backtrace_tag  << "m_flutter_pthread_metas.erase : " << m_flutter_pthread_metas[pthread].thread_name << std::endl;
            return;
        }
    }
}