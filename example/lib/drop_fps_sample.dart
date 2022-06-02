import 'dart:ui';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
class DropFpsSample{
  static void testDropFPS(BuildContext context){
    imageCache!.maximumSize = 0;
    Navigator.push(context, MaterialPageRoute(builder: (ctx) => DropFpsSamplePage()));
  }
}
class DropFpsSamplePage extends StatefulWidget{
  @override
  _DropFpsSamplePageState createState() => _DropFpsSamplePageState();
}
class _DropFpsSamplePageState extends State<DropFpsSamplePage>{
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: ListView(
        children: List.generate(10000, (index) => Image.network(
          "http://img.netbian.com/file/2022/0417/2159314WsId.jpg",fit:BoxFit.fitWidth
        )),
      ),
    );
  }
}