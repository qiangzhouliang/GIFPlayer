package com.swan.gifplayer;

import android.graphics.Bitmap;

/**
 * @ClassName GifHandler
 * @Description
 * @Author swan
 * @Date 2023/5/24 09:34
 **/
public class GifHandler {

    // ndkGif native 结构体的地址

    // 存放在 Java 为了方便传参
    private long gifAddr;

    static {
        System.loadLibrary("gifplayer");
    }
    public GifHandler(String path){
        // 加载 信使
        this.gifAddr = loadPath(path);
    }

    public int getWidth(){
        return getWidth(gifAddr);
    }

    public int getHeight(){
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap){
        return updateFrame(gifAddr, bitmap);
    }

    private native long loadPath(String path);
    private native int getWidth(long ndkGif);
    private native int getHeight(long ndkGif);
    // 隔一段时间调用一次，使静态页面动起来
    private native int updateFrame(long ndkGif, Bitmap bitmap);
}
