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

int send_urb(struct usbdevfs_urb *surb, unsigned char *buffer)
{
	int ret, total = 0;
	ret = send(sockfd, surb, SIZE_USBDEVFS, 0);
	if(ret<=0) {
		fprintf(stderr, "send(urb) error\n");
		return -1;
	}
	total += ret;
	printf(" < send >\n");
	printf("  buffer[0] : %02x\n", buffer[0]);
	printf("  buffer[1] : %02x\n", buffer[1]);
	printf("  buffer[2] : %02x\n", buffer[2]);
	printf("  buffer[3] : %02x\n", buffer[3]);
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

int send_cmd(int cmd_num)
{
	unsigned char buffer[MAX_SEND];

	switch(cmd_num) {
	case 1 : 
		urb.type=2;
		urb.endpoint=0x80;
		urb.status=0;
		urb.flags=0x00000000;
        buffer[0]=0x80;
        buffer[1]=0x06;
        buffer[2]=0x00;
        buffer[3]=0x01;
        buffer[4]=0x00;
        buffer[5]=0x00;
        buffer[6]=0x12;
        buffer[7]=0x00;
        buffer[8]=0x12;
        buffer[9]=0x01;
        buffer[10]=0x00;
        buffer[11]=0x02;
        buffer[12]=0x00;
        buffer[13]=0x00;
        buffer[14]=0x00;
        buffer[15]=0x08;
        buffer[16]=0x61;
        buffer[17]=0x04;
        buffer[18]=0x16;
        buffer[19]=0x4d;
        buffer[20]=0x00;
        buffer[21]=0x02;
        buffer[22]=0x00;
        buffer[23]=0x02;
        buffer[24]=0x00;
        buffer[25]=0x01;
		urb.buffer_length=26;
		urb.actual_length=18;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;
		urb.usercontext = 0x00;
		urb.iso_frame_desc[0].length = 0x01000680;
		urb.iso_frame_desc[0].actual_length = 0x00120000;
		urb.iso_frame_desc[0].status = 0x02000112;
		break;
	case 2 :
		urb.type=2;
		urb.endpoint=0x80;
		urb.status=0;
		urb.flags=0x00000000;
     	buffer[0]=0x80;
        buffer[1]=0x06;
        buffer[2]=0x00;
        buffer[3]=0x02;
        buffer[4]=0x00;
        buffer[5]=0x00;
        buffer[6]=0x09;
        buffer[7]=0x00;
        buffer[8]=0x09;
        buffer[9]=0x02;
        buffer[10]=0x22;
        buffer[11]=0x00;
        buffer[12]=0x01;
        buffer[13]=0x01;
        buffer[14]=0x00;
        buffer[15]=0xa0;
        buffer[16]=0x32;
		urb.buffer_length=17;
		urb.actual_length=9;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;
		urb.usercontext = 0x00;
		urb.iso_frame_desc[0].length = 0x02000680;
		urb.iso_frame_desc[0].actual_length = 0x00090000;
		urb.iso_frame_desc[0].status = 0x00220209;
		break;
	case 3 : 
		urb.type=2;
		urb.endpoint=0x80;
		urb.status=0;
		urb.flags=0x00000000;
		buffer[0]=0x80;
        buffer[1]=0x06;
        buffer[2]=0x00;
        buffer[3]=0x02;
        buffer[4]=0x00;
        buffer[5]=0x00;
        buffer[6]=0x22;
        buffer[7]=0x00;
        buffer[8]=0x09;
        buffer[9]=0x02;
        buffer[10]=0x22;
        buffer[11]=0x00;
        buffer[12]=0x01;
        buffer[13]=0x01;
        buffer[14]=0x00;
        buffer[15]=0xa0;
        buffer[16]=0x32;
        buffer[17]=0x09;
        buffer[18]=0x04;
        buffer[19]=0x00;
        buffer[20]=0x00;
        buffer[21]=0x01;
        buffer[22]=0x03;
        buffer[23]=0x01;
        buffer[24]=0x02;
        buffer[25]=0x00;
        buffer[26]=0x09;
        buffer[27]=0x21;
        buffer[28]=0x11;
        buffer[29]=0x01;
        buffer[30]=0x00;
        buffer[31]=0x01;
        buffer[32]=0x22;
        buffer[33]=0x34;
        buffer[34]=0x00;
        buffer[35]=0x07;
        buffer[36]=0x05;
        buffer[37]=0x81;
        buffer[38]=0x03;
        buffer[39]=0x04;
        buffer[40]=0x00;
        buffer[41]=0x0a;
		urb.buffer_length=42;
		urb.actual_length=34;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;
		urb.usercontext = 0x00;
		urb.iso_frame_desc[0].length = 0x02000680;
		urb.iso_frame_desc[0].actual_length = 0x00220000;
		urb.iso_frame_desc[0].status = 0x00220209;
		break;
	case 4 : 
		urb.type=2;
		urb.endpoint=0x00;
		urb.status=0;
		urb.flags=0x00000000;
		buffer[0]=0x21;
        buffer[1]=0x0a;
        buffer[2]=0x00;
        buffer[3]=0x00;
        buffer[4]=0x00;
        buffer[5]=0x00;
        buffer[6]=0x00;
        buffer[7]=0x00;
		urb.buffer_length=8;
		urb.actual_length=0;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;
		urb.usercontext = 0x00;
		urb.iso_frame_desc[0].length = 0x00000a21;
		urb.iso_frame_desc[0].actual_length = 0x00000000;
		urb.iso_frame_desc[0].status = 0x0001d881;
		break;
	case 5 : 
		urb.type=2;
		urb.endpoint=0x80;
		urb.status=0;
		urb.flags=0x00000000;
		buffer[0]=0x81;
        buffer[1]=0x06;
        buffer[2]=0x00;
        buffer[3]=0x22;
        buffer[4]=0x00;
        buffer[5]=0x00;
        buffer[6]=0x74;
        buffer[7]=0x00;
        buffer[8]=0x05;
        buffer[9]=0x01;
        buffer[10]=0x09;
        buffer[11]=0x02;
        buffer[12]=0xa1;
        buffer[13]=0x01;
        buffer[14]=0x09;
        buffer[15]=0x01;
        buffer[16]=0xa1;
        buffer[17]=0x00;
        buffer[18]=0x05;
        buffer[19]=0x09;
        buffer[20]=0x19;
        buffer[21]=0x01;
        buffer[22]=0x29;
        buffer[23]=0x03;
        buffer[24]=0x15;
        buffer[25]=0x00;
        buffer[26]=0x25;
        buffer[27]=0x01;
        buffer[28]=0x75;
        buffer[29]=0x01;
        buffer[30]=0x95;
        buffer[31]=0x03;
        buffer[32]=0x81;
        buffer[33]=0x02;
        buffer[34]=0x75;
        buffer[35]=0x05;
        buffer[36]=0x95;
        buffer[37]=0x01;
        buffer[38]=0x81;
        buffer[39]=0x01;
        buffer[40]=0x05;
        buffer[41]=0x01;
        buffer[42]=0x09;
        buffer[43]=0x30;
        buffer[44]=0x09;
        buffer[45]=0x31;
        buffer[46]=0x09;
        buffer[47]=0x38;
        buffer[48]=0x15;
        buffer[49]=0x81;
        buffer[50]=0x25;
        buffer[51]=0x7f;
        buffer[52]=0x75;
        buffer[53]=0x08;
        buffer[54]=0x95;
        buffer[55]=0x03;
        buffer[56]=0x81;
        buffer[57]=0x06;
        buffer[58]=0xc0;
        buffer[59]=0xc0;
        buffer[60]=0x00;
        buffer[61]=0x00;
        buffer[62]=0x00;
        buffer[63]=0x00;
        buffer[64]=0x70;
        buffer[65]=0x68;
        buffer[66]=0xa5;
        buffer[67]=0x01;
        buffer[68]=0x00;
        buffer[69]=0x00;
        buffer[70]=0x00;
        buffer[71]=0x00;
        buffer[72]=0x03;
        buffer[73]=0x00;
        buffer[74]=0x00;
        buffer[75]=0x00;
        buffer[76]=0x00;
        buffer[77]=0x00;
        buffer[78]=0x00;
        buffer[79]=0x00;
        buffer[80]=0x2a;
        buffer[81]=0x00;
        buffer[82]=0x00;
        buffer[83]=0x00;
        buffer[84]=0x22;
        buffer[85]=0x00;
        buffer[86]=0x00;
        buffer[87]=0x00;
        buffer[88]=0x02;
        buffer[89]=0x80;
        buffer[90]=0x00;
        buffer[91]=0x00;
        buffer[92]=0x00;
        buffer[93]=0x00;
        buffer[94]=0x00;
        buffer[95]=0x00;
        buffer[96]=0x00;
        buffer[97]=0x00;
        buffer[98]=0x00;
        buffer[99]=0x00;
        buffer[100]=0x00;
        buffer[101]=0x00;
        buffer[102]=0x00;
        buffer[103]=0x00;
        buffer[104]=0x70;
        buffer[105]=0x68;
        buffer[106]=0xa5;
        buffer[107]=0x01;
        buffer[108]=0x00;
        buffer[109]=0x00;
        buffer[110]=0x00;
        buffer[111]=0x00;
        buffer[112]=0x2a;
        buffer[113]=0x00;
        buffer[114]=0x00;
        buffer[115]=0x00;
        buffer[116]=0x22;
        buffer[117]=0x00;
        buffer[118]=0x00;
        buffer[119]=0x00;
        buffer[120]=0x00;
        buffer[121]=0x00;
        buffer[122]=0x00;
        buffer[123]=0x00;
		urb.buffer_length=124;
		urb.actual_length=52;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;
		urb.usercontext = 0x00;
		urb.iso_frame_desc[0].length = 0x22000681;
		urb.iso_frame_desc[0].actual_length = 0x00740000;
		urb.iso_frame_desc[0].status = 0x02090105;
		break;
	default :
		fprintf(stderr, "set_urb cmd_num error\n");
		return -1;
	}
	return send_urb(&urb, buffer);
}

void set_urb_control()
{
	int i;
	urb.type=1;
	urb.endpoint=0x81;
	urb.status=0;
	urb.flags=0x00000000;
	urb.buffer_length=4;
	urb.actual_length=4;
	urb.start_frame=0;
	urb.number_of_packets=0;
	urb.error_count=0;
	urb.signr=0;
	urb.usercontext = 0x00;
	for(i=0; i<urb.buffer_length; i++)
		buffer[i] = 0x00;
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

int send_control(int ctr_num, int speed) 
{
	unsigned int tmp_length = 0x00000000;

	switch(ctr_num) {
	case 0 : // SCROLL_DOWN
		buffer[3] = 0x01;
		break;
	case 1 : // MOVE_LEFT+DOWN
		buffer[1] = 0xff-speed;
		buffer[2] = 0x01+speed;			
		break;
	case 2 : // MOVE_DOWN
		buffer[1] = 0x00;
		buffer[2] = 0x01+speed;
		break;
	case 3 : // MOVE_RIGHT+DOWN
		buffer[1] = 0x01+speed;
		buffer[2] = 0x01+speed;
		break;
	case 4 : // MOVE_LEFT
		buffer[1] = 0xff-speed;
		buffer[2] = 0x00;
		break;
	case 5 : // MOVE_RESET
		buffer[1] = 0x00;
		buffer[2] = 0x00;
		buffer[3] = 0x00;
		break;
	case 6 : // MOVE_RIGHT
		buffer[1] = 0x01+speed;
		buffer[2] = 0x00;
		break;
	case 7 : // MOVE_LEFT+UP
		buffer[1] = 0xff-speed;
		buffer[2] = 0xff-speed;
		break;
	case 8 : // MOVE_UP
		buffer[1] = 0x00;
		buffer[2] = 0xff-speed;
		break;
	case 9 : // MOVE_RIGHT+UP
		buffer[1] = 0x01+speed;
		buffer[2] = 0xff-speed;
		break;
	case 10 : // SCROLL_DOWN
		buffer[3] = 0xff;
		break;
	case 11 : // LBTN_DOWN
		buffer[0] += 0x01;
		break;
	case 12 : // RBTN_DOWN
		buffer[0] += 0x02;
		break;
	case 13 : // SCRBTN_DOWN
		buffer[0] += 0x04;
		break;
	case 14 : // LBTN_UP
		buffer[0] -= 0x01;
		break;
	case 15 : // RBTN_UP
		buffer[0] -= 0x02;
		break;
	case 16 : // SCRBTN_UP
		buffer[0] -= 0x04;
		break;
	default :
		fprintf(stderr, "unknown control number\n");
		return -1;
	}

	tmp_length = buffer[3];
	tmp_length = tmp_length << 8;
	tmp_length = tmp_length | buffer[2];
	tmp_length = tmp_length << 8;
	tmp_length = tmp_length | buffer[1];
	tmp_length = tmp_length << 8;
	tmp_length = tmp_length | buffer[0];
	urb.iso_frame_desc[0].length = tmp_length;
	printf("tmp_length : 0x%08x\n", tmp_length);

	if(++seqnum%2) {
		urb.iso_frame_desc[0].actual_length = 0x00000000;
		urb.iso_frame_desc[0].status = 0x00000061;
	}
	else {
		urb.iso_frame_desc[0].actual_length = 0x00740000;
		urb.iso_frame_desc[0].status = 0x00000021;
	}

	return send_urb(&urb, buffer);
}

int move(int up, int down, int left, int right, int speed)
{
	int ret;
	if(up) {
		if(left) ret=send_control(7, speed);
		else if(right) ret=send_control(9, speed);
		else ret=send_control(8, speed);
	}
	else if(down) {
		if(left) ret=send_control(1, speed);
		else if(right) ret=send_control(3, speed);
		else ret=send_control(2, speed);
	}
	else {
		if(left) ret=send_control(4, speed);
		else ret=send_control(6, speed); // right
	}

	if(ret==-1) {
		fprintf(stderr, "move - send_control() error\n");
		return -1;
	}
	// MOVE_RESET

	// TEST	
	return send_control(5, speed);
	//return ret;
}

int btn_left(int down, int up)
{
	if(down) return send_control(11, 0);
	else return send_control(14, 0); // up
}

int btn_right(int down, int up)
{
	if(down) return send_control(12, 0);
	else return send_control(15, 0); // up
}

int btn_scroll(int down, int up)
{
	if(down) return send_control(13, 0);
	else return send_control(16, 0); // up
}

int move_scroll(int up, int down)
{
	int ret;
	if(up) ret=send_control(0, 0);
	else ret=send_control(10, 0); // down

	if(ret==-1) {
		fprintf(stderr, "move_scroll - send_control() error\n");
		return -1;
	}
	// MOVE_RESET
	return send_control(5, 0);
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
	printf("input key\n");

	while(1) {
		//fflush(stdin);
		c = getchar();
		if(c=='\n'){
			printf("0792047 nanniggu\n");
			continue;
		}
		printf("input:%c\n",c);
		if(c>='1' && c<='9') { // move
			c2 = getchar();
			printf("input:%c\n",c2);
			speed = c2-'0';
		}

		switch(c) {
		case '1' : move(0,1,1,0,speed); break;
		case '2' : move(0,1,0,0,speed); break;
		case '3' : move(0,1,0,1,speed); break;
		case '4' : move(0,0,1,0,speed); break;
		case '6' : move(0,0,0,1,speed); break;
		case '7' : move(1,0,1,0,speed); break;
		case '8' : move(1,0,0,0,speed); break;
		case '9' : move(1,0,0,1,speed); break;
		case '/' : btn_left(1, 0); break;
		case '*' : btn_left(0, 1); break;
		case '-' : btn_right(1, 0); break;
		case '+' : btn_right(0, 1); break;
		case 'q' : move_scroll(1, 0); break;
		case 'a' : btn_scroll(1, 0); break;
		case 's' : btn_scroll(0, 1); break;
		case 'z' : move_scroll(0, 1); break;
		}
	}
}
