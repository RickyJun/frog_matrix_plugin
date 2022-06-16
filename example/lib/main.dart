import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:frog_matrix_plugin/frog_matrix_plugin.dart';
import 'package:frog_matrix_plugin_example/backtrace_sample/DumpContentPage.dart';
import 'package:frog_matrix_plugin_example/backtrace_sample/backtrace_sanmple.dart';
import 'drop_fps_sample.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';

  @override
  void initState() {
    super.initState();
    //initPlatformState();
    WidgetsBinding.instance!.addPostFrameCallback((timeStamp) { 
        Timer.periodic(const Duration(milliseconds: 16), (timer) { 
          _platformVersion =  "${DateTime.now().millisecondsSinceEpoch}";
            setState(() {
              
            });
        });
    });
    
  }
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Builder(builder: (ctx){
        return Scaffold(
        appBar: AppBar(
          title: Text('Plugin example app $_platformVersion'),
        ),
        body: ListView(
          children: [
            // _btn("testFps",(){
            //   DropFpsSample.testDropFPS(ctx);
            // }),
            // _btn("testDrop",(){
            //   BacktraceSample.testDrop();
            // }),
            // _btn("testDump",(){
            //   BacktraceSample.testDump();
            // }),
            // _btn("testReport",(){
            //   DumpContentPage.startEveMethod();
            //   BacktraceSample.testReport();
            //   DumpContentPage.startEveMethod();
            // }),
            _btn("startCollect",(){
              BacktraceSample.testStartCollect();
            }),
            _btn("report",(){
              BacktraceSample.testReport();
            }),
            BacktraceSample.getTestWidget1(),
            BacktraceSample.getTestWidget2()
          ],
        ),
      );
      }),
    );
  }
  Widget _btn(String name,VoidCallback fun){
    return Padding(padding: const EdgeInsets.only(bottom: 5),child: GestureDetector(
      onTap: fun,
      child: Container(height: 30,width: 100,
    decoration: BoxDecoration(
      color: Colors.blueAccent,
      borderRadius: BorderRadius.circular(6)
    ),
    alignment: Alignment.center,
    child: Text(name,style: const TextStyle(color: Colors.white),),),
    ),);
  }
}
