package org.libsdl.app;

import android.os.Bundle;
import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.view.KeyEvent;

//import org.libsdl.app.*;

//import org.libsdl.app;

public class AmbulantSDLPlayer extends SDLActivity {
//public class AmbulantSDLPlayer {
    // required empty class, as otherwise project depending on this one will fail to build because of the
    // missing jar
	
	/** A native method that is implemented by the
	 * ‘testJNI’ native library, which is packaged
	 * with this application.
	 */

//	public native String testJNI();
//	public native String org.libsdl.app.SDLActivity.nativeInit ();
	/** Load the native library where the native method
	 * is stored.
	 */
	static {
// 		libexpat.so          libSDL2_image.so  libSDL2_ttf.so
//		libambulant.so       libgnustl_shared.so  libSDL2.so
		
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("expat");
		System.loadLibrary("SDL2");
		System.loadLibrary("SDL2_image");
		System.loadLibrary("SDL2_ttf");
		System.loadLibrary("avutil-52");
		System.loadLibrary("avcodec-55");
		System.loadLibrary("avformat-55");
		System.loadLibrary("swresample-0");
		System.loadLibrary("swscale-2");
		System.loadLibrary("ambulant");
//		System.loadLibrary("AmbulantSDLPlayer");
//		System.loadLibrary("ambulant");

//		System.loadLibrary("testJNI");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Log.i("AmbulantSDLPlayer", "onCreate() start");  

		Intent intent = getIntent();
		Uri data = intent.getData();
		if (data != null) {
			Log.i("AmbulantSDLPlayer", "onCreate() data="+data);
			AmbulantSDLPlayer.my_string = ""+data;
		}
		AmbulantSDLPlayer.data_dir = getFilesDir().getAbsolutePath();
	}
	public static String my_string;
	public static String data_dir;
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		Log.i("AmbulantSDLPlayer", "onKeyDown() start keyCode = "+keyCode);  
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			finish();
        		return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	@Override
	protected void onStart() {
		super.onStart();  // Always call the superclass method first
		Log.i("AmbulantSDLPlayer", "onStart() start");  
	}
	@Override
	protected void onStop() {
		Log.i("AmbulantSDLPlayer", "onStop() start");  
		super.onStop();
	}
	@Override
	protected void onPause() {
		Log.i("AmbulantSDLPlayer", "onPause() start");  
		super.onPause();
	}
	@Override
	protected void onResume() {
		super.onResume();  // Always call the superclass method first
		Log.i("AmbulantSDLPlayer", "onResume() start");  
	}
	@Override
	protected void onRestart() {
		super.onRestart();  // Always call the superclass method first
		Log.i("AmbulantSDLPlayer", "onRestart() start");  
	}
	@Override
	protected void onDestroy() {
		super.onDestroy();  // Always call the superclass method first
		Log.i("AmbulantSDLPlayer", "onDestroy() start");  
		System.exit(1);
	}
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		// Checks the orientation of the screen
		if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			Log.i("AmbulantSDLPlayer", " onConfigurationChanged() landscape");  
		} else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT){
			Log.i("AmbulantSDLPlayer", " onConfigurationChanged() portrait");  
		}
	}
}
