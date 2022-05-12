package com.example.frog_matrix_plugin.matrix

import android.content.Context
import com.tencent.matrix.plugin.DefaultPluginListener
import com.tencent.matrix.report.Issue
import com.tencent.matrix.util.MatrixLog


class FrogMatrixPluginListener(context: Context?) : DefaultPluginListener(context) {
    val TAG = "Matrix.FrogMatrixPluginListener"
    override fun onReportIssue(issue: Issue) {
        super.onReportIssue(issue)
        MatrixLog.e(TAG, issue.toString())
        //add your code to process data
    }
}