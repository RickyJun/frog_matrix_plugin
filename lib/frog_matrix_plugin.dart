
import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';
import 'package:flutter/services.dart';

class FrogMatrixPlugin {
  static const MethodChannel _channel = MethodChannel('frog_matrix_plugin');

  static void startCollect() async {
    await _channel.invokeMethod('startCollect');
  }
  static Future<bool> reportToFile(int drogDurationMSTime,{String? filePath}) async {
    if(filePath?.isEmpty??true){
      Directory dir = await getTemporaryDirectory();
      filePath = dir.path+"/dump.json";
    }else if(!filePath!.endsWith(".json")){
      throw FlutterError("filePath must with .json on trail");
    }
    //TODO wenwenjun test
    _filepath = filePath;
    //
    int timestamp = DateTime.now().millisecondsSinceEpoch - drogDurationMSTime;
    final String res = await _channel.invokeMethod('reportToFile',[filePath,timestamp]);
    if(res == "ok"){
      return true;
    }else{
      throw FlutterError(res);
    }
  }
  //TODO wenwenjun test
  static String? _filepath;
  static Future<String> getJsonContent()async{
    File file = File(_filepath!);
    if(!file.existsSync()){
      return "not exists";
    }
    String content = file.readAsStringSync();
    return content;
  } 
  // static Future<String> testDump(int waitSecend) async {
  //   Directory dir = await getTemporaryDirectory();
  //   String path = dir.path+"/dump.json";
  //   File file = File(path);
  //   if(file.existsSync()){
  //     file.delete();
  //   }
  //   file.createSync(recursive: true);
  //   final String? version = await _channel.invokeMethod('testDump',path);
  //   String content = file.readAsStringSync();
  //   return content;
  // }
}
