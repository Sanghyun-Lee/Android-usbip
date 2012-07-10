package app.android.ltouchpad;

import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class TouchListenerPad implements OnTouchListener {
	private boolean isTouchMove;
	private boolean isButtonDown;
	private Point pPoint = new Point(0,0);
	private Point movePoint = new Point(0,0);

	public boolean onTouch(View v, MotionEvent event) {
		switch(event.getAction()) {
		case MotionEvent.ACTION_DOWN :
			if(isButtonDown)
				return false;
			isButtonDown = true;
			isTouchMove = false;
			pPoint.x = (int)event.getX();
			pPoint.y = (int)event.getY();
			return true;
		case MotionEvent.ACTION_MOVE :
			isTouchMove = true;
			int x2 = (int)event.getX();
			int y2 = (int)event.getY();
			movePoint.add(x2-pPoint.x, y2-pPoint.y);
			movePoint = LTouchpadActivity.Move(movePoint);
			pPoint.x = x2;
			pPoint.y = y2;
			return false;
		case MotionEvent.ACTION_UP :
			if(!isButtonDown)
				return false;
			isButtonDown = false;
			if(!isTouchMove) {
				// Click Event
				LTouchpadActivity.LButtonClick();
			}
			return true;
		default :
			return false;
		}
	}
}
