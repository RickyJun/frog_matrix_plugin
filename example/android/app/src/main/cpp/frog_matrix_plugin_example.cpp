#include <jni.h>

// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("frog_matrix_plugin_example");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("frog_matrix_plugin_example")
//      }
//    }


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_frog_1matrix_1plugin_1example_MainActivity_test(JNIEnv *env, jobject thiz,
                                                                 jboolean has_warm_up) {
    return env->NewString(reinterpret_cast<const jchar *>("fuxx"),10);
}