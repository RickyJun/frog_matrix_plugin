import 'dart:math';

import 'package:flutter/material.dart';

class DumpContentPage extends StatefulWidget{
  DumpContentPage(this.content);
  final ValueNotifier<String> content;
  @override
  State<StatefulWidget> createState()=>_DumpContentPageState();
  static void startEveMethod()async{
    int j = 0;
    while(true){
      int i = Random().nextInt(5);
      j++;
      if(j==100){
        j=0;
        break;
      }
      switch(i){
        case 1:_test1();
        break;
        case 2:_test2();
        break;
        case 3:_test3();
        break;
        case 4:_test4();
        break;
        default:_test5();
        break;
      }
    }
  }
  static void startEveMethod2()async{
    int j = 0;
    while(true){
      int i = Random().nextInt(5);
      j++;
      if(j==100){
        j=0;
        break;
      }
      _test1(true);
    }
  }
  static void _test1([bool stringCon = false]){
    if(stringCon){
      String a = "aaaaaaddccadscsdfadfbbbasdfadsfdsfdsbbb";
      String b = "bbb";
      if(a.contains(b)){
        print("test_1");
      }
    }
    print("test_1");
  }
  static void _test2(){
    print("test_2");
  }
  static void _test3(){
    print("test_3");
  }
  static void _test4(){
    print("test_4");
  }
  static void _test5(){
    print("test_5");
  }
}
class _DumpContentPageState extends State<DumpContentPage>{
  @override
  void initState() {
    // TODO: implement initState
    super.initState();
  }
  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: MediaQuery.of(context).size.width,
      height: 10000,
      child: ValueListenableBuilder<String>(valueListenable: widget.content,builder: (ctx,val,_){
        return Text(val,style: const TextStyle(color: Colors.black),);
      }),
    );
  }
  
  
}