package app.android.ltouchpad;

class Point {
	int x;
	int y;
	Point(int x, int y) {
		this.x = x;
		this.y = y;
	}
	
	public void set(int x, int y) {
		this.x = x;
		this.y = y;
	}
	
	public void add(int x, int y) {
		this.x += x;
		this.y += y;
	}
	
	public int getBigSize() {
		if ( getAbsX() > getAbsY() )
			return getAbsX();
		else
			return getAbsY();
	}
	
	public int getSmallSize() {
		if ( getAbsX() > getAbsY() )
			return getAbsY();
		else
			return getAbsX();
	}
	
	public int getAbsX() {
		if(x<0)
			return x*(-1);
		else
			return x;
	}
	
	public int getAbsY() {
		if(y<0)
			return y*(-1);
		else
			return y;
	}
	
	public void cutMaxAbsnum(int max) {
		if(getAbsX()>max) {		
			if(x<0)
				x = -max;
			else
				x = max;
		}
		if(getAbsY()>max) {
			if(y<0)
				y = -max;
			else
				y = max;
		}
	}
	
	public void setHalf() {
		if(x>2)
			x/=2;
		if(y>2)
			y/=2;
	}
}