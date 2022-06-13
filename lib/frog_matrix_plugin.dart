
import 'dart:async';
import 'dart:io';
import 'package:path_provider/path_provider.dart';
import 'package:flutter/services.dart';

class FrogMatrixPlugin {
  static const MethodChannel _channel = MethodChannel('frog_matrix_plugin');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
  static Future<String> testDump(int waitSecend) async {
    Directory dir = await getTemporaryDirectory();
    String path = dir.path+"/dump.json";
    File file = File(path);
    if(file.existsSync()){
      file.delete();
    }
    file.createSync(recursive: true);
    final String? version = await _channel.invokeMethod('testDump',path);
    String content = file.readAsStringSync();
    return content;
  }
  static Future<String> testReport(int waitSecend) async {
    Directory dir = await getTemporaryDirectory();
    String path = dir.path+"/dump.json";
    File file = File(path);
    if(file.existsSync()){
      file.delete();
    }
    file.createSync(recursive: true);
    final String? version = await _channel.invokeMethod('testReport',path);
    String content = file.readAsStringSync();
    return content;
  }
}
