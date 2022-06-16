
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:frog_matrix_plugin/frog_matrix_plugin.dart';
import 'package:frog_matrix_plugin_example/backtrace_sample/dropTestPage.dart';

import 'DumpContentPage.dart';

class BacktraceSample{
  static final ValueNotifier<int> _index = ValueNotifier<int>(0);
  static final ValueNotifier<String> _content = ValueNotifier<String>("empty");
  // static void testDump()async{
  //   _content.value = await FrogMatrixPlugin.testDump(7);
  //   _index.value = 1;
  // }
   static void testReport()async{
     await FrogMatrixPlugin.reportToFile(1000);
    _content.value = await FrogMatrixPlugin.getJsonContent();
    _index.value = 1;
  }
  static void testStartCollect()async{
    FrogMatrixPlugin.startCollect();
  }
  static Widget getTestWidget1(){
    return dropTestPage();
  }
  static Widget getTestWidget2(){
    return DumpContentPage(_content);
  }
}


