#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/usbdevice_fs.h>
#include <memory.h>
#include <malloc.h>

#define PORT 3700
#define MAX_RECV 10
#define MAX_SEND 1024
#define CMD_MAX 7
#define CMD_MAX_RECV 5
#define SIZE sizeof(struct sockaddr_in)
#define SIZE_USBDEVFS sizeof(struct usbdevfs_urb)
#define SIZE_ISOPACKETDESC sizeof(struct usbdevfs_iso_packet_desc)

int sockfd = -1;
unsigned char buffer[4];
struct usbdevfs_urb urb;
unsigned int seqnum = 0;

int connect_usbip()
{
	struct sockaddr_in sockin;
	int con = -1;

	if( (sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1) {
		fprintf(stderr, "socket() fail\n");
		return -1;
	}
	bzero((char*)&sockin,sizeof(sockin));
	sockin.sin_family = AF_INET;
	sockin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockin.sin_port = htons(PORT);

	con = connect(sockfd,(struct sockaddr*)&sockin, SIZE);
	if(con==-1) {
		fprintf(stderr, "connect() fail\n");
		return -1;
	}
	return 0;
}

int recv_cmd(int sockfd, char* cmd, int cmd_size) 
{
	int ret, i=0;
	char cmd_tmp;

	//do {
		ret = recv(sockfd, &cmd_tmp, 1, 0);
		if(ret<=0) {
			fprintf(stderr, "recv() error\n");
			return -1;
		}
		cmd[i] = cmd_tmp;
		i++;
	//} while(i<cmd_size && cmd_tmp!='\0');
	return ret;
}

int send_cmd(struct usbdevfs_urb *surb, unsigned char *buffer)
{
	int ret, total = 0;
	ret = send(sockfd, surb, SIZE_USBDEVFS, 0);
	if(ret<=0) {
		fprintf(stderr, "send(urb) error\n");
		return -1;
	}
	total += ret;
	ret = send(sockfd, buffer, surb->buffer_length, 0);
	if(ret<=0) {
		fprintf(stderr, "send(urb.buffer) error\n");
		return -1;
	}
	total += ret;
	ret = send(sockfd, surb->iso_frame_desc, SIZE_ISOPACKETDESC, 0);
	if(ret<=0) {
		fprintf(stderr, "send(urb.iso_frame_desc[0]) error\n");
		return -1;
	}
	total += ret;
	return total;
}


int process_cmd()
{
	char cmd[MAX_RECV];
	int cmd_num;

	do {
		if(recv_cmd(sockfd, cmd, MAX_RECV)==-1){
			fprintf(stderr, "recv_cmd() fail\n");
			return -1;
		}
		if(sscanf(cmd, "%d", &cmd_num) != 1) {
			fprintf(stderr, "cmd_num error\n");
			return -1;
		}
		if(cmd_num<=CMD_MAX_RECV) {
			if(send_cmd(cmd_num)==-1)
				return -1;
		}
		else {
			// no send
		}
	} while(cmd_num<CMD_MAX);

	set_urb_control();
	return 0;
}

int main(int argc, char* argv[])
{
	char c, c2;
	int speed=0;

	if(connect_usbip()==-1) {
		fprintf(stderr, "connect_usbip() error\n");
		return -1;
	}
	if(process_cmd()==-1) {
		fprintf(stderr, "process_cmd() error\n");
		return -1;
	}


}
