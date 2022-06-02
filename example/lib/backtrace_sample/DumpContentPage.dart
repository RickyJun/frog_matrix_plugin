import 'package:flutter/material.dart';

class DumpContentPage extends StatefulWidget{
  DumpContentPage(this.content);
  final ValueNotifier<String> content;
  @override
  State<StatefulWidget> createState()=>_DumpContentPageState();
  
}
class _DumpContentPageState extends State<DumpContentPage>{
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