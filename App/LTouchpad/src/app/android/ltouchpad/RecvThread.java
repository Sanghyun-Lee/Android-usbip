package app.android.ltouchpad;

public class RecvThread extends Thread {
	private UsbipMouse usbipMouse;
	
	public RecvThread(UsbipMouse usbipMouse) {
		this.usbipMouse = usbipMouse;
	}
	
	public void run() {
		while(true) {
			usbipMouse.recvAck();
		}
	}
}
