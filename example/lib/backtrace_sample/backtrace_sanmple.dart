
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:frog_matrix_plugin/frog_matrix_plugin.dart';
import 'package:frog_matrix_plugin_example/backtrace_sample/dropTestPage.dart';

import 'DumpContentPage.dart';

class BacktraceSample{
  static ValueNotifier<int> _index = ValueNotifier<int>(0);
  static ValueNotifier<String> _content = ValueNotifier<String>("");
  static void testDump()async{
    _content.value = await FrogMatrixPlugin.testDump(7);
    _index.value = 1;
  }
  static void testDrop()async{
    _index.value = 0;
  }
  static Widget getTestWidget(){
    return ValueListenableBuilder(valueListenable: _index, builder: (ctx,val,child){
      switch(val){
        case 0:return dropTestPage();
        case 1:return DumpContentPage(_content);
      }
      return const SizedBox();
    });
  }
}


