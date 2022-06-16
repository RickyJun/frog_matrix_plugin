package com.example.frog_matrix_plugin

import android.util.Log
import androidx.annotation.NonNull
import com.example.frog_matrix_plugin.matrix.DynamicConfigImpl
import com.example.frog_matrix_plugin.matrix.FrogMatrixPluginListener
import com.tencent.matrix.Matrix
import com.tencent.matrix.backtrace.WeChatBacktrace
import com.tencent.matrix.hook.FlutterStackCollect
import com.tencent.matrix.hook.memory.MemoryHook
import com.tencent.matrix.hook.pthread.PthreadHook
import com.tencent.matrix.trace.TracePlugin
import com.tencent.matrix.trace.config.TraceConfig

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.embedding.engine.plugins.activity.ActivityAware
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import java.io.File
import java.util.*

/** FrogMatrixPlugin */
class FrogMatrixPlugin: FlutterPlugin, MethodCallHandler,ActivityAware {
  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private lateinit var channel : MethodChannel
  private var activityPluginBinding: ActivityPluginBinding? = null
  override fun onAttachedToEngine(@NonNull flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "frog_matrix_plugin")
    channel.setMethodCallHandler(this)

  }
  private fun initMatrix(){
//    val builder: Matrix.Builder =  Matrix.Builder(activityPluginBinding!!.activity.application) // build matrix
//
//    builder.pluginListener(FrogMatrixPluginListener(activityPluginBinding!!.activity.applicationContext)) // add general pluginListener
//
//    val dynamicConfig = DynamicConfigImpl() // dynamic config
//
//    // init plugin
//    val traceCanaryPlugin = TracePlugin(
//      TraceConfig.Builder()
//        .dynamicConfig(dynamicConfig)
//        .enableEvilMethodTrace(true)
//        .enableFPS(true)
//        .enableEvilMethodTrace(true)
//        .build()
//    )
//    //add to matrix
//    builder.plugin(traceCanaryPlugin)
//    //init matrix
//    Matrix.init(builder.build())
//    // start plugin
//    traceCanaryPlugin.start()
    //Backtrace
    PthreadHook.INSTANCE.setThreadTraceEnabled(true);
    PthreadHook.INSTANCE.hook();



//    WeChatBacktrace.instance().configure(activityPluginBinding!!.activity.applicationContext)
//      .enableOtherProcessLogger(true).savingPath(activityPluginBinding!!.activity.applicationContext?.filesDir?.parentFile
//        ?.canonicalFile?.absolutePath +"/matric_yesdesk/stackData")
//      .setBacktraceMode(WeChatBacktrace.Mode.Fp).commit();

  }
  fun getUIThread():Long?{
    var allStackTraces:Map<Thread, Array<StackTraceElement>> = Thread.getAllStackTraces()
    Log.d("TAG", "线程总数：" + allStackTraces.size);
    for (stackTrace in allStackTraces.entries) {
      var thread:Thread  = stackTrace.key
      if(thread.name.contains(".ui")){
        return thread.id
      }
     // Log.d("TAG", "线程：" + thread.getName() + ",id=" + thread.getId() + ",state=" + thread.getState());
//      var stack:Array<StackTraceElement>  = stackTrace.value
//      var strStackTrace:String  = "堆栈：";
//      for (stackTraceElement in stack) {
//      strStackTrace += stackTraceElement.toString() + "\n";
//    }
    }
    return null
  }

  override fun onMethodCall(@NonNull call: MethodCall, @NonNull result: Result) {
//    if (call.method == "getPlatformVersion") {
//      result.success("Android ${android.os.Build.VERSION.RELEASE}")
//    }else if(call.method == "testDump") {
//      var path:String = call.arguments as String;
//      PthreadHook.INSTANCE.startFlutterTrace(path);
//      result.success("Android ${android.os.Build.VERSION.RELEASE}")
//    } else
      if(call.method == "reportToFile") {
      val args:List<Any> = call.arguments as List<Any>;
      val path:String = args[0] as String;
      val timestamp:Long = args[1] as Long;
      val res = FlutterStackCollect.report(timestamp);
      val file = File(path)
      if(file.exists()){
        file.delete()
      }
      file.createNewFile()
      file.writeText(res)
      result.success("ok")
    }else if(call.method == "startCollect") {
      FlutterStackCollect.startCollect();
      result.success("ok")
    }   else {
      result.notImplemented()
    }
  }

  override fun onDetachedFromEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }

  override fun onAttachedToActivity(binding: ActivityPluginBinding) {
    activityPluginBinding = binding
    initMatrix()
  }

  override fun onDetachedFromActivityForConfigChanges() {
    activityPluginBinding = null
  }

  override fun onReattachedToActivityForConfigChanges(binding: ActivityPluginBinding) {
    activityPluginBinding = binding
  }

  override fun onDetachedFromActivity() {
    activityPluginBinding = null
  }
}
