package app.android.USBIP;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

public class ShellProc extends Service {
	private static final String STR_USBIPD_START = "usbipd start";
	private static final String STR_BINDD_START = "bind to usbip, complete!";
	
	private static boolean isServerOn;
	private int sock_binddriver;
	private static Handler handler;
	
	@Override
	public void onStart(Intent intent, int startId) {
		super.onStart(intent, startId);
		System.loadLibrary("ndk-usbip");
		Log.i("USBIP", "ShellProc onStart");
		
		startListen();
	}
	
	public static void setHandler(Handler handler) {
		ShellProc.handler = handler;
	}
	
	private void startListen() {
		if( !isServerOn ) {
			int ret;
			if((ret = server_bind())<0) {
				isServerOn = true;
				Log.e("USBIP", "bind_error " + ret);
				sendHandler(USBIP_AppActivity.HandlerWhat_ToastShow, "서버 bind 에러 " + ret);
				return;
			}
		}
		sendHandler(USBIP_AppActivity.HandlerWhat_ServerReady, null);
		
		new Thread ( new Runnable() {
			public void run() {
				Log.d("USBIP", "USBIP listen_start");
				if(listen_usbipd()<0) {
					Log.e("USBIP", "listen_usbipd error");
					return;
				}
				Log.i("USBIP", "listen success");
				new Thread ( new Runnable() {
					public void run() {
						Log.d("USBIP", "BINDDRIVER listen_start");
						if(listen_binddriver()<0) {
							Log.e("USBIP", "listen_binddriver error");
							return;
						}
						Log.i("USBIP", "bind listen success");
						recvBINDDRIVER();
					} // BINDDRIVER Thread run()
				}).start(); // BINDDRIVER Thread
				recvUSBIPD();
			} // USBIPD Thread run()
		}).start();
	}
	
	private void sendHandler(int what, String str) {
		if(handler==null)
			return;
		Message msg = new Message();
		msg.what = what;
		if(str!=null)
			msg.obj = str;
		handler.sendMessage(msg);
	}
	
	private void recvUSBIPD() {
		String str=null;
		while(true) {
			str = recv_usbipd();
			if(str==null || str=="") {
				Log.e("USBIP", "recv Error");
				break;
			}
			if(str.startsWith("err")) {
				Log.e("USBIP", "usbipd err: " + str);
				sendHandler(USBIP_AppActivity.HandlerWhat_ToastShow, str);
			}
			else {
				if(str.startsWith(STR_USBIPD_START))
					sendHandler(USBIP_AppActivity.HandlerWhat_UsbipdOpen, null);
				Log.d("USBIP", "recv : " + str);
			}
		}
	}
	
	private void recvBINDDRIVER() {
		String str=null;
		while(true) {
			str = recv_binddriver();
			if(str==null || str=="") {
				Log.e("USBIP", "recv Error");
				break;
			}
			if(str.startsWith("err")) {
				Log.e("USBIP", "recv err: " + str);
				sendHandler(USBIP_AppActivity.HandlerWhat_ToastShow, str);
			}
			else {
				if(str.startsWith(STR_BINDD_START))
					sendHandler(USBIP_AppActivity.HandlerWhat_BindDSuccess, null);
				Log.d("USBIP", "recv : " + str);
			}
		}
	}
	
	public native int server_bind();
	public native int listen_usbipd();
	public native int listen_binddriver();
	public native String recv_usbipd();
	public native String recv_binddriver();

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}
}
