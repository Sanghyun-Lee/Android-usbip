package app.android.server;
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

public class RecvThread extends Thread {
	MessageManager message;
	String str;
	
	public RecvThread(MessageManager message) {
		super();
		this.message = message;
		this.str = "";
	}
	
	public void run() {
		while(true) {
			recv();
		}
	}
	
	protected boolean recv() {
		str = message.recvFromClient();
		if(str == null || str.equals("")) {
			message.sendToAndroidapp(MessageManager.MSG_NOTCONNECTED);
			return false;
		}
		return message.sendToAndroidapp(str);
	}
}
