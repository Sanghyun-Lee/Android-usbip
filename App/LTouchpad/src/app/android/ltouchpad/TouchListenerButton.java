package app.android.ltouchpad;

import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class TouchListenerButton implements OnTouchListener {

	public boolean onTouch(View v, MotionEvent event) {
		Log.d("LTouchPad", "Button/onTouch : " + event.getAction());
		if(event.getPointerCount()>1) {
			LTouchpadActivity.MultiTouch(v, event);
			return false;
		}
		
		switch(event.getAction()) {
		case MotionEvent.ACTION_DOWN :
			if(v.getId()==R.id.frame_lbtn)
				LTouchpadActivity.LButtonDown();
			else // R.id.frame_Rbtn
				LTouchpadActivity.RButtonDown();
			return true;
		case MotionEvent.ACTION_UP :
			if(v.getId()==R.id.frame_lbtn) {
				LTouchpadActivity.LButtonUp();
			}
			else // R.id.frame_Rbtn
				LTouchpadActivity.RButtonUp();
			return true;
		default :
			return false;
		}
	}
}
