//
// Created by 123 on 2022/6/1.
//
#include "include/FlutterStackCollect.h"
#include "Utils.h"
#include "include/FlutterStackCollectSignalHandler.h"
#include <cJSON.h>
#include <backtrace/Backtrace.h>
//线程数据
static std::map<pthread_t, pthread_meta_t> m_flutter_pthread_metas;
//存储规定时间内每一次收集的堆栈，
static std::map<pthread_t,std::vector<pthread_meta_t>> m_flutter_pthread_metas_buffer;
static std::map<pthread_t,std::vector<pthread_meta_t>> m_flutter_pthread_metas_restore_frame_detail_buffer;
namespace wechat_backtrace {
    uint64_t FlutterStackCollect::expireMSTime_ = 3000;
    int FlutterStackCollect::intervalMSTime_ = 50;
    time_t FlutterStackCollect::firstRecordMSTime_ = 0;
    CTimer* FlutterStackCollect::pTimer_ = nullptr;
    void FlutterStackCollect::StartCollect() {
        //
        FlutterStackCollectSignalHandler::RegisterFlutterStackCollectSignalHandler();
        //
        pTimer_ = new CTimer("CTIME_wechat_backtrace_FlutterStackCollect");
        pTimer_->AsyncLoop(intervalMSTime_, [](auto  arg){
            FlutterStackCollectSignalHandler::SendSignalToFlutterPthread(m_flutter_pthread_metas);
        }, NULL);
    }
    char* FlutterStackCollect::Report() {
        cJSON *report_obj = cJSON_CreateArray();
        //
        std::map<uptr,wechat_backtrace::FrameDetail*> frameDetails;
        //通过次数
        std::map<uptr,uint>  frameDetail_counts;
         auto frame_detail_lambda = [&frameDetails,&frameDetail_counts](wechat_backtrace::FrameDetail detail) -> void {
            frameDetails[detail.rel_pc] = &detail;
            if(!frameDetail_counts.count(detail.rel_pc)){
                frameDetail_counts[detail.rel_pc] = 1;
            }else{
                frameDetail_counts[detail.rel_pc]++;
            }
        };
        //恢复堆栈，恢复so虚拟地址等
        for(auto it = m_flutter_pthread_metas_buffer.rbegin() ; it != m_flutter_pthread_metas_buffer.rend() ; it++){
            std::vector<pthread_meta_t> pthread_meta_vertor = it->second;
            for(auto meta:pthread_meta_vertor){
                if (meta.native_backtrace.frame_size == 0) {
                    continue;
                }
                //恢复
                wechat_backtrace::restore_frame_detail(meta.native_backtrace.frames.get(),
                                                       meta.native_backtrace.frame_size,
                                                       frame_detail_lambda);
            }
        }
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
        for(auto it:frameDetails){
            wechat_backtrace::FrameDetail *detail = it.second;
            cJSON *item_json = cJSON_CreateObject();
            cJSON_AddItemToArray(report_obj,item_json);

            cJSON_AddStringToObject(item_json,"object_name",detail->map_name);
            cJSON_AddNumberToObject(item_json,"object_addr",0);
            //符号化得到
            cJSON_AddStringToObject(item_json,"symbol_name",detail->function_name);
            //符号化得到
            cJSON_AddNumberToObject(item_json,"symbol_addr",0);
            cJSON_AddNumberToObject(item_json,"instruction_addr",(double)(detail->rel_pc));
            cJSON_AddNumberToObject(item_json,"repeat_count",(double)frameDetail_counts.at(detail->rel_pc));
        }
        return cJSON_Print(report_obj);
    }

    void FlutterStackCollect::EndCollect() {
        pTimer_->Cancel();
    }

    void FlutterStackCollect::PushToBuffer(pthread_t key_pthread, pthread_meta_t value_meta) {
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpMicro
                = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        time_t currrentMSTime = tpMicro.time_since_epoch().count();
        //check is expire for expireMSTime_
        if(firstRecordMSTime_ == 0 || currrentMSTime - firstRecordMSTime_ > expireMSTime_){
            firstRecordMSTime_ = currrentMSTime;
            m_flutter_pthread_metas_buffer.clear();
        }
        //save this meta
        m_flutter_pthread_metas_buffer[key_pthread].push_back(pthread_meta_t(value_meta));
    }
    //收集
    void FlutterStackCollect::DoCollectStack() {
        for(auto it = m_flutter_pthread_metas.rbegin() ; it != m_flutter_pthread_metas.rend() ; it++){
            pthread_meta_t meta = it->second;
            //unwind
            wechat_backtrace::unwind_adapter(meta.native_backtrace.frames.get(),
                                             meta.native_backtrace.max_frames,
                                             meta.native_backtrace.frame_size);
            uint64_t native_hash = hash_backtrace_frames(&(meta.native_backtrace));
            if (native_hash) {
                meta.hash = native_hash;
            }
            PushToBuffer(it->first,meta);
        }
    }
    void FlutterStackCollect::on_pthread_created(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        //filter
        FilterFlutterPtread(m_pthread_metas);
    }

    void FlutterStackCollect::FilterFlutterPtread(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        for (auto it = m_pthread_metas.rbegin(); it != m_pthread_metas.rend(); it++) {
            pthread_meta_t meta = it->second;
            if(m_flutter_pthread_metas.count(it->first)){
                continue;
            }
            if(strstr(meta.thread_name,".raster") || strstr(meta.thread_name,".ui") || strstr(meta.thread_name,"Dart") || strstr(meta.thread_name,"Flutter") ){
                m_flutter_pthread_metas[it->first] = meta;
            }
        }
    }

}