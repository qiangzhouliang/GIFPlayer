package com.swan.gifplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;

import com.swan.gifplayer.databinding.ActivityMainBinding;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;


public class MainActivity extends AppCompatActivity {

    Bitmap bitmap;
    GifHandler gifHandler;



    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

    }


    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg){
            // 需要刷新下一帧
            // 下一帧的刷新时间
            int nextFrame = gifHandler.updateFrame(bitmap);
            handler.sendEmptyMessageDelayed(1, nextFrame);
            binding.image.setImageBitmap(bitmap);

        }
    };

    public void ndkLoadGif(View view) {
        String path = getApplicationContext().getFilesDir().getAbsoluteFile().getPath();
        File file = new File(path+"/09-1-4人脸识别中.gif");
        gifHandler = new GifHandler(file.getAbsolutePath());

        // 得到gif width height 生成bitmap
        int width = gifHandler.getWidth();
        int height = gifHandler.getHeight();
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        // 下一帧的刷新时间
        int nextFrame = gifHandler.updateFrame(bitmap);
        handler.sendEmptyMessageDelayed(1, nextFrame);
    }
}