package com.vst.libjpegdemo;

/**
 * Created by zwy on 2018/2/1.
 * email:16681805@qq.com
 */

public interface NativeCallBack {

    void startCompress();

    void error(int errorCode, String des);

    void finish(String filePath);
}
