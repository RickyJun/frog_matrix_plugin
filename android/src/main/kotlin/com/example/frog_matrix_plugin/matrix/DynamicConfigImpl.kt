package com.example.frog_matrix_plugin.matrix

import com.tencent.mrs.plugin.IDynamicConfig
import java.lang.Exception


class DynamicConfigImpl : IDynamicConfig {
    val isFPSEnable: Boolean
        get() = true
    val isTraceEnable: Boolean
        get() = true
    val isMatrixEnable: Boolean
        get() = true
    val isDumpHprof: Boolean
        get() = false

    override fun get(key: String, defStr: String?): String? {
        try {
            if(IDynamicConfig.ExptEnum.clicfg_matrix_trace_care_scene_set.name == key){
                return "com.example.frog_matrix_plugin_example.MainActivity";
            }
            return defStr;
        }catch (e:Exception){
            System.out.println("fu");
            return defStr;
        }

    }

    override fun get(key: String, defInt: Int): Int {
        return defInt;
    }

    override fun get(key: String, defLong: Long): Long {
        //hook to change default values
        return defLong;
    }

    override fun get(key: String, defBool: Boolean): Boolean {
        return defBool;
    }

    override fun get(key: String, defFloat: Float): Float {
        return defFloat;
    }
}