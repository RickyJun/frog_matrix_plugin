package com.tencent.matrix.hook;

import androidx.annotation.Keep;
@Keep
public class FlutterStackCollect {
    @Keep
    public static native void startCollect();

    @Keep
    public static native String report();

    @Keep
    public static native void endCollect();

}
