package app.android.ltouchpad;

import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class TouchListenerScroll implements OnTouchListener {
	private boolean isTouchMove;
	private float y1;

	public boolean onTouch(View v, MotionEvent event) {
		Log.d("LTouchPad", "Scroll/onTouch : " + event.getAction());
		switch(event.getAction()) {
		case MotionEvent.ACTION_DOWN :
			y1 = event.getY();
			isTouchMove = false;
			return true;
		case MotionEvent.ACTION_MOVE :
			float y2 = event.getY();
			if(y2-y1>5) {
				LTouchpadActivity.ScrollDown();
				isTouchMove = true;
			}
			else if(y2-y1<-5) {
				LTouchpadActivity.ScrollUp();
				isTouchMove = true;
			}
			y1 = y2;
			return true;
		case MotionEvent.ACTION_UP :
			if(!isTouchMove) {
				// Click Event
				LTouchpadActivity.SButtonClick();
			}
			return true;
		default :
			return false;
		}
	}
}
