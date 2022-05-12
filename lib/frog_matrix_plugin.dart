
import 'dart:async';

import 'package:flutter/services.dart';

class FrogMatrixPlugin {
  static const MethodChannel _channel = MethodChannel('frog_matrix_plugin');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
