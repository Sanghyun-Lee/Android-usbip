package app.android.USBIP;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class USBIP_AppActivity extends Activity {
	private ThreadExec proc_USBIPD, proc_BINDDRIVER;
	private Button btn_usbipd;
	private MyListAdapter MyAdapter;
	private ListView list_device;
	private int SelectPosition;
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		
		ArrayList<MyItem> arItem = new ArrayList<MyItem>();
		MyItem mi;
		mi = new MyItem("LTouchPad", "app.android.ltouchpad");
		arItem.add(mi);
		mi = new MyItem("KatalkPCLinker", "app.android.server");
		arItem.add(mi);
		MyAdapter = new MyListAdapter(this, R.layout.icontext, arItem);

		list_device = (ListView) findViewById(R.id.list);
		list_device.setAdapter(MyAdapter);
		list_device.setOnItemClickListener(listener);
		
		btn_usbipd = (Button)findViewById(R.id.btn_connect);
		proc_USBIPD = new ThreadExec("usbipd");
		proc_BINDDRIVER = new ThreadExec( "usbip_bind_driver --usbip 2-1.2");
		
		Intent intent = new Intent(USBIP_AppActivity.this, ShellProc.class);
		Log.d("USBIP", "startService");
		ShellProc.setHandler(handler);
		startService(intent);
	}
	
	public static final int HandlerWhat_ServerReady = 1;
	public static final int HandlerWhat_UsbipdOpen = 2;
	public static final int HandlerWhat_BindDSuccess = 3;
	public static final int HandlerWhat_DeviceUsing = 4;
	public static final int HandlerWhat_ToastShow = 5;
	
	private Handler handler = new Handler() {
		public void handleMessage(Message msg) {
			switch(msg.what) {
			case HandlerWhat_ServerReady :
				btn_usbipd.setEnabled(true);
				break;
			case HandlerWhat_UsbipdOpen :
				showToast("USBIP OPEN!");
				btn_usbipd.setEnabled(false);
				list_device.setVisibility(ListView.VISIBLE);
				break;
			case HandlerWhat_BindDSuccess :
				MyAdapter.getMytItem(0).setState(MyItem.STATE_BIND);
				MyAdapter.notifyDataSetChanged();
				break;
			case HandlerWhat_DeviceUsing :
				MyAdapter.getMytItem(0).setState(MyItem.STATE_USE);
				MyAdapter.notifyDataSetChanged();
				break;
			case HandlerWhat_ToastShow :
				String str = (String)msg.obj;
				showToast(str);
				break;
			default :
				break;
			}
		}
	};
	
	private AdapterView.OnItemClickListener listener =
			new AdapterView.OnItemClickListener() {
				public void onItemClick(AdapterView<?> alist, View view,
						int position, long id) {
					ItemClickDlg(position);
				}
			};
	
	private static final CharSequence[] dlgItems = {"PC에서 사용준비", "앱 실행"};
	private void ItemClickDlg(int position) {
		SelectPosition = position;
		new AlertDialog.Builder(USBIP_AppActivity.this)
		.setTitle(MyAdapter.getMytItem(position).getName())
		.setItems(dlgItems, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int item) {
				switch(item) {
				case 0 : // PC에서 사용준비
					if(SelectPosition==0 &&
						MyAdapter.getMytItem(SelectPosition).getState()==MyItem.STATE_RED) { // 
						proc_BINDDRIVER.start();
						openPackage(MyAdapter.getMytItem(SelectPosition).getPackName());
					}
					else {
						showToast("이미 사용준비가 됐거나 USBIP로 사용할 수 없는 어플리케이션입니다.");
					}
					break;
				case 1 :
					openPackage(MyAdapter.getMytItem(SelectPosition).getPackName());
					break;
				}
			}
		})
		.setNegativeButton("취소", null)
		.show();
	}

	public void showToast(String msg) {
		Toast.makeText(USBIP_AppActivity.this, msg, Toast.LENGTH_SHORT).show();
	}
	
	private boolean openPackage(String packname) {
		Intent intent = this.getPackageManager().getLaunchIntentForPackage(packname);
		if(intent==null) {
			Log.e("USBIP", "open " + packname + " error");
			return false;
		}
		USBIP_AppActivity.this.startActivity(intent);
		return true;
	}
	
	class ThreadExec extends Thread {
		String cmd;
		Process proc;

		public ThreadExec(String cmd) {
			this.cmd = cmd;
		}
		
		public void destroyProc() {
			if(proc==null)
				return;
			try {
				Log.d("USBIP", "destroyProc()");
				this.stop();
				proc.getInputStream().close();
				proc.getOutputStream().close();
				proc.getErrorStream().close();
				try {
					proc.waitFor();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				proc.destroy();
				proc = null;
			} catch (IOException e) {
				Log.e("USBIP", "IOException-exec()");
				e.printStackTrace();
			}
		}

		public void run() {
			Runtime runtime = Runtime.getRuntime();
			try {
				/*
				proc = runtime.exec("su");
				DataOutputStream os = new DataOutputStream(proc.getOutputStream());
				Log.d("USBIP", "exec : " + cmd);
				os.writeBytes(cmd+"\n");
				*/
				proc = runtime.exec("su");
				proc = runtime.exec(cmd);
				BufferedReader in = new BufferedReader(
						new InputStreamReader(proc.getInputStream()));
				String s=null;
				while((s=in.readLine())!=null) {
					Log.i("USBIP", "in : " + s);
				}
				Log.d("USBIP", cmd + " end");
				destroyProc();
			} catch (IOException e) {
				Log.e("USBIP", "IOException-exec()");
				e.printStackTrace();
			}
		}
	}

	public void mOnClick(View v) {
		switch (v.getId()) {
		case R.id.btn_connect:
			proc_USBIPD.start();
			break;
		}
	}
}

class MyItem {
	public static final int STATE_RED = 1;
	public static final int STATE_BIND = 2;
	public static final int STATE_USE = 3;
	
	private int dev_state;
	private String dev_name;
	private String packName;
	
	MyItem(String aName, String packName) {		
		dev_name = aName;
		this.packName = packName;
		dev_state = STATE_RED;
	}

	public void setState(int state) {
		dev_state = state;
	}
	
	public int getState() {
		return dev_state;
	}
	
	public String getName() {
		return dev_name;
	}
	
	public String getPackName() {
		return packName;
	}
}

class MyListAdapter extends BaseAdapter {
	Context maincon;
	LayoutInflater Inflater;
	private ArrayList<MyItem> arSrc;
	int layout;

	public MyListAdapter(Context context, int alayout, ArrayList<MyItem> aarSrc) {
		maincon = context;
		Inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		arSrc = aarSrc;
		layout = alayout;
	}

	public int getCount() {
		return arSrc.size();
	}
	
	public MyItem getMytItem(int position) {
		return arSrc.get(position);
	}

	public Object getItem(int position) {
		return arSrc.get(position).getName();
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {
		if (convertView == null) {
			convertView = Inflater.inflate(layout, parent, false);
		}
		TextView textDevName = (TextView) convertView.findViewById(R.id.dev_name);
		textDevName.setText(arSrc.get(position).getName());
		ImageView img = (ImageView) convertView.findViewById(R.id.img);
		TextView textDevState = (TextView) convertView.findViewById(R.id.dev_state);
		
		switch(arSrc.get(position).getState()){
		case MyItem.STATE_RED:
			img.setImageResource(R.drawable.red);
			textDevState.setText("사용 대기중");
			break;
		case MyItem.STATE_BIND:
			img.setImageResource(R.drawable.bind);
			textDevState.setText("사용 가능");
			break;
		case MyItem.STATE_USE:
			img.setImageResource(R.drawable.use);
			textDevState.setText("사용중");
			break;			
		}
		
		return convertView;
	}

}