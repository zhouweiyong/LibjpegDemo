//
// Created by user on 2018/3/6.
//

#include <jni.h>

typedef unsigned char BYTE;

extern "C"
JNIEXPORT void JNICALL
Java_com_vst_libjpegdemo_NativeCompress_compress(JNIEnv *env, jobject instance, jstring outPath_,
                                                 jobject bitmap, jint ratio, jboolean isUseHoffman,
                                                 jobject nativeCallBack);

int generateJPEG(BYTE *data, int w, int h, int quality,
                 const char *outfilename, jboolean optimize);