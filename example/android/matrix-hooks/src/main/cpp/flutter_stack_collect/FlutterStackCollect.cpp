//
// Created by 123 on 2022/6/1.
//
#include "include/FlutterStackCollect.h"
#include "Utils.h"
#include "include/FlutterStackCollectSignalHandler.h"
#include <cJSON.h>
#include <backtrace/Backtrace.h>
#include <dirent.h>
#include <iostream>
#include <memory>
#define wechat_backtrace_tag "[wechat_backtrace] "
//
struct FrogBacktrace{
    wechat_backtrace::Backtrace native_backtrace;
    time_t timestamp;
};
//线程数据
static std::map<pthread_t, pthread_meta_t> m_flutter_pthread_metas;
//存储规定时间内每一次收集的堆栈，
static std::map<pthread_t,std::vector<FrogBacktrace>> m_flutter_pthread_native_backtrace_buffer;
typedef std::lock_guard<std::mutex> flutter_pthread_meta_lock;
static std::mutex  m_flutter_pthread_meta_mutex;
namespace wechat_backtrace {

    //
    uint64_t FlutterStackCollect::expireMSTime_ = 3000;
    int FlutterStackCollect::intervalMSTime_ = 50;
    CTimer* FlutterStackCollect::pTimer_ = nullptr;
    bool FlutterStackCollect::coleetingFlag_ = false;
    void FlutterStackCollect::StartCollect() {
        FlutterStackCollectSignalHandler::RegisterFlutterStackCollectSignalHandler();
        //start run
        pTimer_ = new CTimer("CTIME_wechat_backtrace_FlutterStackCollect");
        //TODO wenwenjun
//        m_flutter_pthread_metas[FlutterStackCollect::uiIdLong].tid = FlutterStackCollect::uiIdLong;
//        m_flutter_pthread_metas[FlutterStackCollect::uiIdLong].thread_name = new char[256];
//        sprintf(m_flutter_pthread_metas[FlutterStackCollect::uiIdLong].thread_name,"ui");
//        m_flutter_pthread_metas[FlutterStackCollect::uiIdLong].exited = false;
//        m_flutter_pthread_metas[FlutterStackCollect::uiIdLong].unwind_mode = wechat_backtrace::BacktraceMode::FramePointer;
        FlutterStackCollect::getFlutterUIThreadMeta();
        pTimer_->AsyncLoop(intervalMSTime_, [](auto  arg){
            if(coleetingFlag_ == false){
                FlutterStackCollectSignalHandler::SendSignalToFlutterPthread(m_flutter_pthread_metas);
            }
        }, NULL);
    }
    char* FlutterStackCollect::DumpJson2File(time_t timestamp) {
        try{

            //flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
            cJSON *report_obj = cJSON_CreateArray();
            //
            std::map<uptr,cJSON*> frameDetailJsons;
            //通过次数
            std::map<uptr,uint>  frameDetail_counts;
            //TODO wenwenjun
            //std::vector<char*>  testPc;
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
                 //TODO wenwenjun
//                 char *rel_pc1 = new char[256];
//                 snprintf(rel_pc1, 256, "%lx(%s)", detail.rel_pc,detail.map_name);
//                 testPc.push_back(rel_pc1);
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
                     snprintf(rel_pc, 256, "%lx", detail.rel_pc);
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
                std::vector<FrogBacktrace> pthread_meta_vertor = it->second;
                for(const auto& frogBacktrace:pthread_meta_vertor){
                    if(frogBacktrace.timestamp < timestamp){
                        continue;
                    }
                    if (frogBacktrace.native_backtrace.frame_size == 0) {
                        continue;
                    }
                    //恢复
                    wechat_backtrace::restore_frame_detail(frogBacktrace.native_backtrace.frames.get(),
                                                           frogBacktrace.native_backtrace.frame_size,
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
    void FlutterStackCollect::DoCollectStack(uptr *regs,struct sigaltstack &stack) {
        coleetingFlag_ = true;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpMicro
                = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        time_t currrentMSTime = tpMicro.time_since_epoch().count();
        //clear expire
        for(auto it = m_flutter_pthread_metas.rbegin() ; it != m_flutter_pthread_metas.rend() ; it++){
            pthread_meta_t meta = it->second;
            //check is expire for expireMSTime_
            for(auto jt = m_flutter_pthread_native_backtrace_buffer[it->first].begin() ;jt != m_flutter_pthread_native_backtrace_buffer[it->first].end();){
                if(currrentMSTime - jt->timestamp < expireMSTime_){
                    break;
                }
                m_flutter_pthread_native_backtrace_buffer[it->first].erase(jt);
                jt = m_flutter_pthread_native_backtrace_buffer[it->first].begin();
            }

        }
        //
        for(auto it = m_flutter_pthread_metas.rbegin() ; it != m_flutter_pthread_metas.rend() ; it++){
            pthread_meta_t meta = it->second;
            //unwind
            wechat_backtrace::unwind_pthread_adapter(meta.native_backtrace.frames.get(),regs,stack,it->first,
                                             meta.native_backtrace.max_frames,
                                             meta.native_backtrace.frame_size);
            uint64_t native_hash = hash_backtrace_frames(&(meta.native_backtrace));
            if (native_hash) {
                meta.hash = native_hash;
            }
            //save this meta
            FrogBacktrace frogBacktrace;
            frogBacktrace.timestamp = currrentMSTime;
            frogBacktrace.native_backtrace.frame_size = meta.native_backtrace.frame_size;
            frogBacktrace.native_backtrace.max_frames = meta.native_backtrace.max_frames;
            frogBacktrace.native_backtrace.frames = meta.native_backtrace.frames;
            m_flutter_pthread_native_backtrace_buffer[it->first].push_back(frogBacktrace);
        }
        coleetingFlag_ = false;
    }
    void FlutterStackCollect::on_pthread_created(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        //filter
        //FilterFlutterPtread(m_pthread_metas);
    }
    char *FlutterStackCollect::allThreadIds[256];
    int FlutterStackCollect::allThreadNum = 0;
    int FlutterStackCollect::uiThreadNums = 0;
    int FlutterStackCollect::newestUIThreadId = 0;
    void FlutterStackCollect::getFileName(char * dirPath)
    {
        DIR *dir=opendir(dirPath);
        if(dir==NULL)
        {
            printf("%s\n",strerror(errno));
            return;
        }
        chdir(dirPath);//进入到当前读取目录
        struct dirent *ent;
        while((ent=readdir(dir)) != NULL)
        {
            if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
            {
                continue;
            }
            allThreadIds[allThreadNum] = ent->d_name;
            allThreadNum++;
        }
        closedir(dir);
        chdir("..");//返回当前目录的上一级目录
    }
    void FlutterStackCollect::getFlutterUIThreadMeta(){
        int iPid = getpid();
        char buf1[256] = {};
        snprintf(buf1,256,"/proc/%d/task/",iPid);
        getFileName(buf1);
        for(int i = 0 ; i < allThreadNum ; i++){
            char buf2[256] = {};
            snprintf(buf2,256,"/proc/%d/task/%s/stat",iPid,allThreadIds[i]);
            FILE* taskFp = fopen(buf2,"r");
            if(taskFp != nullptr){
                char *str1 = new char[256];
                if(fgets(str1,256,taskFp) != nullptr){
                    std::string str2 = std::string(str1);
                    if(str2.find(".ui") != -1){
                        std::string str3 = std::string(allThreadIds[i]);
                        int32_t id = std::stoi( str3 );
                        if(id > newestUIThreadId){
                            newestUIThreadId = id;
                            //
                            m_flutter_pthread_metas.clear();
                            m_flutter_pthread_metas[newestUIThreadId].tid = id;
                            m_flutter_pthread_metas[newestUIThreadId].thread_name = new char[256];
                            sprintf(m_flutter_pthread_metas[newestUIThreadId].thread_name,"ui");
                            m_flutter_pthread_metas[newestUIThreadId].exited = false;
                            m_flutter_pthread_metas[newestUIThreadId].unwind_mode = wechat_backtrace::BacktraceMode::FramePointer;
                        }
                        uiThreadNums++;
                    }
                }
                fclose(taskFp);
            }

        }
        return;
    }
    void FlutterStackCollect::FilterFlutterPtread(std::map<pthread_t, pthread_meta_t> &m_pthread_metas) {
        //flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
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
        //flutter_pthread_meta_lock meta_lock(m_flutter_pthread_meta_mutex);
//        if (m_flutter_pthread_metas.count(pthread) && m_flutter_pthread_metas[pthread].exited) {
//            m_flutter_pthread_metas.erase(pthread);
//            m_flutter_pthread_native_backtrace_buffer->erase(pthread);
//            std::cout << wechat_backtrace_tag  << "m_flutter_pthread_metas.erase : " << m_flutter_pthread_metas[pthread].thread_name << std::endl;
//            return;
//        }
    }
}