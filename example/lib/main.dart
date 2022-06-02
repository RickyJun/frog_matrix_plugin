import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:frog_matrix_plugin/frog_matrix_plugin.dart';
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
    initPlatformState();

    
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion =
          await FrogMatrixPlugin.platformVersion ?? 'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Builder(builder: (ctx){
        return Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: ListView(
          children: [
            _btn("testFps",(){
              DropFpsSample.testDropFPS(ctx);
            }),
            _btn("testDrop",(){
              BacktraceSample.testDrop();
            }),
            _btn("testDump",(){
              BacktraceSample.testDump();
            }),
            BacktraceSample.getTestWidget()
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
