#include "USBIPConnect.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/usbdevice_fs.h>
#include <string.h>

#define PORT 3650
#define MAXLINE 1024

struct sockaddr_in sv_addr;
int sock_usbipd=-1;
int sock_binddriver=-1;
int sockfd_listen = -1;

JNIEXPORT jint JNICALL Java_app_android_USBIP_ShellProc_server_1bind
  (JNIEnv *env, jobject obj) {

	if((sockfd_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		// listen socket make error
		return -1;
	}
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sv_addr.sin_port = htons(PORT);
	if(bind(sockfd_listen, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) == -1)
	{
		// bind failed
		return -2;
	}
	if(listen(sockfd_listen, 2) == -1)
	{
		// listen error
		return -3;
	}
}

JNIEXPORT jint JNICALL Java_app_android_USBIP_ShellProc_listen_1usbipd
  (JNIEnv *env, jobject obj) {

	if((sock_usbipd = accept(sockfd_listen, NULL, NULL)) ==-1)
	{
		// accept error
		return -2;
	}

	return sock_usbipd;
}

JNIEXPORT jint JNICALL Java_app_android_USBIP_ShellProc_listen_1binddriver
  (JNIEnv *env, jobject obj) {

	if((sock_binddriver = accept(sockfd_listen, NULL, NULL)) ==-1)
	{
		// accept error
		return -2;
	}

	return sock_binddriver;
}

JNIEXPORT jstring JNICALL Java_app_android_USBIP_ShellProc_recv_1usbipd
  (JNIEnv *env, jobject obj) {
	  char content_recv[MAXLINE];
	  int i;
	  jstring str;

	  memset(content_recv, 0, MAXLINE);
	  i=recv(sock_usbipd, content_recv, MAXLINE, 0);
	  if(i<=0)
		  return NULL;

	  str = (*env)->NewStringUTF(env, content_recv);
	  return str;
}

JNIEXPORT jstring JNICALL Java_app_android_USBIP_ShellProc_recv_1binddriver
  (JNIEnv *env, jobject obj) {
	  char content_recv[MAXLINE];
	  int i;
	  jstring str;

	  memset(content_recv, 0, MAXLINE);
	  i=recv(sock_binddriver, content_recv, MAXLINE, 0);
	  if(i<=0)
		  return NULL;

	  str = (*env)->NewStringUTF(env, content_recv);
	  return str;
}
