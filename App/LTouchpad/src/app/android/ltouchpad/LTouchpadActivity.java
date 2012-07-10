package app.android.ltouchpad;

import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

public class LTouchpadActivity extends Activity {
	
	private static UsbipMouse usbipMouse;
	private static RecvThread recvThread;
	
	private static Button btnConnect;
	private static TextView textConnect;
	private static ProgressBar pbarConnect; 
	
	private static OnTouchListener tlistenerPad;
	private static OnTouchListener tlistenerScroll;
	private static OnTouchListener tlistenerButton;
	
	private static FrameLayout fTouch;
	private static FrameLayout fScroll;
	private static FrameLayout fLbutton;
	private static FrameLayout fRbutton;
	
	private static Drawable Drawable_Lbtn;
	private static Drawable Drawable_LbtnClicked;
	private static Drawable Drawable_Rbtn;
	private static Drawable Drawable_RbtnClicked;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        btnConnect = (Button)findViewById(R.id.btn_connect);
        textConnect = (TextView)findViewById(R.id.text_connect);
        pbarConnect = (ProgressBar)findViewById(R.id.progressBar_connect);
        
        tlistenerPad = new TouchListenerPad();
        tlistenerScroll = new TouchListenerScroll();
        tlistenerButton = new TouchListenerButton();
        
        fTouch = (FrameLayout)findViewById(R.id.frame_touch);
        fTouch.setOnTouchListener(tlistenerPad);
        fScroll = (FrameLayout)findViewById(R.id.frame_scroll);
        fScroll.setOnTouchListener(tlistenerScroll);
        fLbutton = (FrameLayout)findViewById(R.id.frame_lbtn);
        fLbutton.setOnTouchListener(tlistenerButton);
        fRbutton = (FrameLayout)findViewById(R.id.frame_rbtn);
        fRbutton.setOnTouchListener(tlistenerButton);
        
        Drawable_Lbtn = getResources().getDrawable(R.drawable.left);
        Drawable_LbtnClicked = getResources().getDrawable(R.drawable.left_clicked);
        Drawable_Rbtn = getResources().getDrawable(R.drawable.right);
        Drawable_RbtnClicked = getResources().getDrawable(R.drawable.right_clicked);
        
        usbipMouse = new UsbipMouse();
        recvThread = new RecvThread(usbipMouse);
        connectUsbip();
    }
    
    public static final String HANDLE_MSG_CONNECTING = "USBIP 연결중";
    public static final String HANDLE_MSG_WAITINGPC = "PC연결 대기중";
    public static final String HANDLE_MSG_FAILEDCONNECTPC = "PC연결 실패";
    public static final String HANDLE_MSG_FAILEDCONNECT = "USBIP 연결실패";
    public static final String HANDLE_MSG_SUCCESSCONNECT = "USBIP 연결완료";
    public static final String HANDLE_MSG_DISCONNECT = "연결끊김";
    
    public static Handler handler = new Handler() {
		public void handleMessage(Message msg) {
			String s = (String)msg.obj;
			if(s.equals(HANDLE_MSG_CONNECTING)) {
				btnConnect.setVisibility(Button.INVISIBLE);
				pbarConnect.setVisibility(ProgressBar.VISIBLE);
			}
			else if(s.equals(HANDLE_MSG_WAITINGPC)) {
				pbarConnect.setVisibility(ProgressBar.VISIBLE);
			}
			else if(s.equals(HANDLE_MSG_FAILEDCONNECT)) {
				btnConnect.setVisibility(Button.VISIBLE);
				pbarConnect.setVisibility(ProgressBar.INVISIBLE);
			}
			else {
				pbarConnect.setVisibility(ProgressBar.INVISIBLE);
			}
			textConnect.setText(s);
		}
	};
    
    public static void connectUsbip() {
    	if(usbipMouse.isUsbipConnect())
    		return;
    	Message msg = new Message();
    	msg.obj = HANDLE_MSG_CONNECTING;
    	handler.sendMessage(msg);
    	
    	new Thread(new Runnable() {
    		Message msg = new Message();
			 public void run() {
				 if( !usbipMouse.connect()) {
					 msg.obj = HANDLE_MSG_FAILEDCONNECT;
					 handler.sendMessage(msg);
				 }
				 else {
					 msg.obj = HANDLE_MSG_WAITINGPC;
					 handler.sendMessage(msg);
					 
					 if( !usbipMouse.processCmd() ) {
						 msg.obj = HANDLE_MSG_FAILEDCONNECTPC;
					 }
					 else {
						 msg.obj = HANDLE_MSG_SUCCESSCONNECT;
						 recvThread.start();
					 }
					 handler.sendMessage(msg);
				 }
			 }
		 }).start();
    }
    
    public static void onButton(View v) {
    	switch(v.getId()) {
    	case R.id.btn_connect :
    		connectUsbip();
    		break;
    	}
    }
    
    public static boolean LButtonDown() {
    	Log.i("LTouchPad", "LButtonDown()");
    	if(usbipMouse.btnLeft(true)) {
    		fLbutton.setBackgroundDrawable(Drawable_LbtnClicked);
    		return true;
    	}
    	else
    		return false;
    }
    
    public static boolean LButtonUp() {
    	Log.i("LTouchPad", "LButtonUp()");
    	if(usbipMouse.btnLeft(false)) {
    		fLbutton.setBackgroundDrawable(Drawable_Lbtn);
    		return true;
    	}
    	else
    		return false;
    }
    
    public static boolean LButtonClick() {
    	if(LButtonDown())
    		return LButtonUp();
    	else
    		return false;
    }
    
    public static boolean RButtonDown() {
    	Log.i("LTouchPad", "RButtonDown()");
    	if(usbipMouse.btnRight(true)) {
    		fRbutton.setBackgroundDrawable(Drawable_RbtnClicked);
    		return true;
    	}
    	else
    		return false;
    }
    
    public static boolean RButtonUp() {
    	Log.i("LTouchPad", "RButtonUp()");
    	if(usbipMouse.btnRight(false)) {
    		fRbutton.setBackgroundDrawable(Drawable_Rbtn);
    		return true;
    	}
    	else
    		return false;
    }
    
    public static boolean RButtonClick() {
    	if(RButtonDown())
    		return RButtonUp();
    	else
    		return false;
    }
    
    public static boolean SButtonClick() {
    	Log.i("LTouchPad", "SButtonClick()");
    	if(usbipMouse.btnScroll(true)) {
    		if(usbipMouse.btnScroll(false))
    			return true;
    	}
    	return false;
    }
    
    public static boolean ScrollDown() {
    	Log.i("LTouchPad", "ScrollDown()");
    	if(!usbipMouse.isSendable()) {
    		Log.w("LTouchPad", "can't send state");
    		return false;
    	}
    	if(usbipMouse.moveScroll(true))
    		return true;
    	else
    		return false;
    }
    
    public static boolean ScrollUp() {
    	Log.i("LTouchPad", "ScrollUp()");
    	if(!usbipMouse.isSendable()) {
    		Log.w("LTouchPad", "can't send state");
    		return false;
    	}
    	if(usbipMouse.moveScroll(false))
    		return true;
    	else
    		return false;
    }
    
    public static Point Move(Point movePoint) {
    	if(!usbipMouse.isClientConnect()) {
    		Log.w("LTouchPad", "Not connected");
    		movePoint.set(0, 0);
    		return movePoint;
    	}
    	if(!usbipMouse.isSendable()) {
    		Log.w("LTouchPad", "can't send state");
    		return movePoint;
    	}
    	
    	//movePoint.setHalf();
    	movePoint.cutMaxAbsnum(30);
    	if(movePoint.getBigSize()<1)
    		return movePoint;
    	
    	Log.i("LTouchPad", "Move("+movePoint.x+","+movePoint.y+")");
    	if(usbipMouse.moveTouch(movePoint.x,movePoint.y)) {
    		movePoint.set(0, 0);
    	}
    	else {
    		Log.e("LTouchPad", "move() error!");
    	}
    	return movePoint;
    }
    
    public static boolean MultiTouch(View v, MotionEvent event) {
    	if(event.getPointerCount()<2)
    		return false;
    	
    	Point p = new Point((int)event.getX(1), (int)event.getY(1));
    	p = LTouchpadActivity.getAbsPoint(v, p);
    	// need check p case
    	switch(event.getAction()) {
    	case MotionEvent.ACTION_POINTER_2_DOWN :
    		event.setAction(MotionEvent.ACTION_DOWN);
    		break;
    	case MotionEvent.ACTION_MOVE :
    		break;
    	case MotionEvent.ACTION_POINTER_2_UP :
    		event.setAction(MotionEvent.ACTION_UP);
    		break;
    	}
    	event.setLocation(event.getX(event.getPointerCount()-1),
    			event.getY(event.getPointerCount()-1));
    	tlistenerPad.onTouch(v, event);
    	
    	return true;
    }
    
    public static Point getAbsPoint(View v, Point p) {
    	View parentView = v.getRootView();
    	Point point = new Point(0,0);
    	int sumX = 0;
    	int sumY = 0;
    	boolean isEnd = false;
    	
    	while(!isEnd) {
    		sumX += v.getLeft();
    		sumY += v.getTop();
    		
    		v = (View)v.getParent();
    		if(parentView == v) {
    			isEnd = true;
    		}
    	}
    	
    	if(p!=null) {
    		point.x += p.x;
    		point.y += p.y;
    	}
    	point.x += sumX;
		point.y += sumY;
    	return point;
    }
    
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	switch (keyCode) {
        	case KeyEvent.KEYCODE_BACK:
        		moveTaskToBack(true);
        		return true;
    	}
    	return super.onKeyDown(keyCode, event);
    }
}