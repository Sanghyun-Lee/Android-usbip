package app.android.ltouchpad;

import android.os.Message;
import android.util.Log;

public class UsbipMouse {
	private boolean connection;
	private boolean moveflag;
	private boolean clientConnection;
	
	public UsbipMouse() {
		System.loadLibrary("usb-mouse");
		moveflag = false;
	}
	
	public boolean isUsbipConnect() {
		return connection;
	}

	public boolean isClientConnect() {
		return clientConnection;
	}
	
	public boolean connect() {
		if(connect_usbip()<0) {                                                                                                              
			connection = false;
			return false;
		}
		else {
			connection = true;
			return true;
		}
	}
	
	public boolean moveTouch(int x, int y) {
		int ret;
		if(!moveflag) {
			moveflag = true;
			ret = move(x,y);
			if(ret<0) {
				closeConnection();
				moveflag = false;
				return false;
			}
			else {
				moveflag = false;
				return true;
			}
		}
		else
			return false;
	}
	
	private void closeConnection() {
		Message msg = new Message();
		msg.obj = LTouchpadActivity.HANDLE_MSG_DISCONNECT;
		LTouchpadActivity.handler.sendMessage(msg);
		connection = false;
	}
	
	public boolean processCmd() {
		if(process_cmd()<0) {
			clientConnection = false;
			return false;
		}
		else {
			clientConnection =true;
			return true;
		}
	}
	
	public boolean btnLeft(boolean down) {
		int d=0, u=0;
		if(down)
			d=1;
		else
			u=1;
		
		if(btn_left(d, u)<0)
			return false;
		else
			return true;
	}
	
	public boolean btnRight(boolean down) {
		int d=0, u=0;
		if(down)
			d=1;
		else
			u=1;
		
		if(btn_right(d, u)<0)
			return false;
		else
			return true;
	}
	
	public boolean btnScroll(boolean down) {
		int d=0, u=0;
		if(down)
			d=1;
		else
			u=1;
		
		if(btn_scroll(d, u)<0)
			return false;
		else
			return true;
	}
	
	public boolean moveScroll(boolean down) {
		int d=0, u=0;
		if(down)
			d=1;
		else
			u=1;
		
		if(move_scroll(u, d)<0)
			return false;
		else
			return true;
	}
	
	public boolean isSendable() {
		if(is_sendable()==1)
			return true;
		else
			return false;
	}
	
	public boolean recvAck() {
		int ret;
		ret=recv_ack();
		Log.d("LTouchPad", "recvAck : " + ret);
		if(ret<0) {
			return false;
		}
		else
			return true;
	}
	
	private native int connect_usbip();
	private native int process_cmd();
	
	private native int is_sendable();
	private native int recv_ack();
	
	private native int move(int x, int y);
	private native int btn_left(int down, int up);
	private native int btn_right(int down, int up);
	private native int btn_scroll(int down, int up);
	private native int move_scroll(int up, int down);
}