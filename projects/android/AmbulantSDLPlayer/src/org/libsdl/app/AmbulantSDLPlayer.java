package org.libsdl.app;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.net.Uri;

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
	}
	public static String my_string;
}
