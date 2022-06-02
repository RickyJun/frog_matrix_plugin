
#include <jni.h>
#include "include/FlutterStackCollect.h"
#ifdef __cplusplus
extern "C" {
#endif


jstring strToJstring(JNIEnv* env, const char* pStr)
{
    int        strLen    = strlen(pStr);
    jclass     jstrObj   = (*env).FindClass("java/lang/String");
    jmethodID  methodId  = (*env).GetMethodID(jstrObj, "<init>", "([BLjava/lang/String;)V");
    jbyteArray byteArray = (*env).NewByteArray(strLen);
    jstring    encode    = (*env).NewStringUTF("utf-8");
    (*env).SetByteArrayRegion(byteArray, 0, strLen, (jbyte*)pStr);
    return (jstring)(*env).NewObject(jstrObj, methodId, byteArray, encode);
}
JNIEXPORT void JNICALL
Java_com_tencent_matrix_hook_FlutterStackCollect_startCollect(JNIEnv
*env,
jclass clazz) {
    wechat_backtrace::FlutterStackCollect::StartCollect();
}

JNIEXPORT jstring JNICALL
Java_com_tencent_matrix_hook_FlutterStackCollect_report(JNIEnv *env, jclass clazz) {
    char* reportStr = wechat_backtrace::FlutterStackCollect::Report();
    return strToJstring(env,reportStr);
}

JNIEXPORT void JNICALL
Java_com_tencent_matrix_hook_FlutterStackCollect_endCollect(JNIEnv *env, jclass clazz) {
    wechat_backtrace::FlutterStackCollect::EndCollect();
}

#ifdef __cplusplus
}
#endif
