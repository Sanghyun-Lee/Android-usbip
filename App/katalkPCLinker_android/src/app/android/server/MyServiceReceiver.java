/*
 * Copyright (C) 2008-2009 Google Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 * 
 * 
 * @author
 * Kang In-gu
 * Department of Computer Engineering, HansungUniversity, Korea.
 * e-mail : 2002gig@naver.com
 */

package app.android.server;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class MyServiceReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		Log.d("MessagePCViewer", "on BroadcastReceive");
		if(intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED))
			Log.d("MessagePCViewer", "on Receive ACTION_BOOT_COMPLETED");
		// 부팅시 자동 시작하는 서비스 구현중
		// context.startService(new Intent(context, MessageManager.class));
	}
}
