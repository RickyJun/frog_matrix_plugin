import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';

class dropTestPage extends StatefulWidget{
  @override
  State<StatefulWidget> createState() => _dropTestPageState();
}
class _dropTestPageState extends State<dropTestPage>{
  @override
  void initState() {
    super.initState();
    Timer.periodic(Duration(milliseconds: 10), (timer) { 
      if(mounted){
        setState(() {
          
        });
      }
    });
  }
  @override
  Widget build(BuildContext context) {
    return SizedBox(
              width: 375,height: 1000,child: Stack(
              children:getTestWidget() ,
            ));
  }
  static int testNum = 7000;
  static List<_Offset> positons = List.generate(testNum, (index) => _Offset(Random().nextInt(300)*1.0,Random().nextInt(1000)*1.0));
  static List<Positioned> getTestWidget(){
    for (var element in positons) { 
      int i = Random().nextInt(4)%2;
      if(i==0){
        element.dx = element.dx+Random().nextInt(3)*element.signx;
        element.signx = element.signx*Random().nextInt(2)%2==0?1:-1;
      }else{
        element.dy = element.dy+Random().nextInt(3)*element.signy;
        element.signy = element.signy*Random().nextInt(2)%2==0?1:-1;
      }
    }
    return List.generate(testNum, (index) => Positioned(
    child: const Icon(Icons.add_location_sharp),left: positons[index].dx,top: positons[index].dy,
  ));
  }
  
}
class _Offset{
  _Offset(this.dx, this.dy);
  double dx;
  double dy;
  int signx = 1;
  int signy = 1;
}