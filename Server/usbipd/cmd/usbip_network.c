/*
 * $Id$
 *
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#include "usbip_network.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

//
//uint16_t USBIP_VERSION =1.7;
//uint16_t IPPROTO_TCP =1.7;
//

#include <malloc.h>
#include <memory.h>

void pack_uint32_t(int pack, uint32_t *num)
{
	uint32_t i;

	if (pack)
		i = htonl(*num);
	else
		i = ntohl(*num);

	*num = i;
}

void pack_uint16_t(int pack, uint16_t *num)
{
	uint16_t i;

	if (pack)
		i = htons(*num);
	else
		i = ntohs(*num);

	*num = i;
}

void pack_usb_device(int pack, struct usb_device *udev)
{
	pack_uint32_t(pack, &udev->busnum);
	pack_uint32_t(pack, &udev->devnum);
	pack_uint32_t(pack, &udev->speed );

	pack_uint16_t(pack, &udev->idVendor );
	pack_uint16_t(pack, &udev->idProduct);
	pack_uint16_t(pack, &udev->bcdDevice);
}

void pack_usb_interface(int pack, struct usb_interface *udev)
{
	/* uint8_t members need nothing */
}


static ssize_t usbip_xmit(int sockfd, void *buff, size_t bufflen, int sending)
{
	ssize_t total = 0;

	if (!bufflen)
		return 0;

	do {
		ssize_t nbytes;

		if (sending)
			nbytes = send(sockfd, buff, bufflen, 0);
		else
			nbytes = recv(sockfd, buff, bufflen, MSG_WAITALL);

		if (nbytes <= 0)
			return -1;

		buff	= (void *) ((intptr_t) buff + nbytes);
		bufflen	-= nbytes;
		total	+= nbytes;

	} while (bufflen > 0);


	return total;
}

ssize_t usbip_recv(int sockfd, void *buff, size_t bufflen)
{
	return usbip_xmit(sockfd, buff, bufflen, 0);
}

ssize_t usbip_xmitv(int sockfd, struct iovec *iov, int io_count, int sending)
{
	int i,len,ret;
	struct msghdr mhdr;
	for(i=0,len=0;i<io_count;i++)
		len+=iov[i].iov_len;
	memset(&mhdr, 0, sizeof(mhdr));
	mhdr.msg_iov=iov;
	mhdr.msg_iovlen=io_count;
	if(sending)
		ret=sendmsg(sockfd, &mhdr, 0);
	else
		ret=recvmsg(sockfd, &mhdr, MSG_WAITALL);
	if(ret!=len){
		// DEV_TEST
		info("WARNING : usbip_xmitv - ret:%d, len:%d, io_count:%d", ret, len, io_count);
		if(sending)
			dbg("why sendmsg don't send all msg out? %d", ret);
		else
			dbg("why recvmsg don't recv all msg in? %d", ret);
		return -1;
		//return ret;
	}
	// info("##usbip_xmitv - ret:%d, len:%d", ret, len);
	return ret;
}

ssize_t usbip_send(int sockfd, void *buff, size_t bufflen)
{
	return usbip_xmit(sockfd, buff, bufflen, 1);
}

int usbip_send_op_common(int sockfd, uint32_t code, uint32_t status)
{
	int ret;
	struct op_common op_common;

	bzero(&op_common, sizeof(op_common));

	op_common.version	= USBIP_VERSION;
	op_common.code		= code;
	op_common.status	= status;

	PACK_OP_COMMON(1, &op_common);

	ret = usbip_send(sockfd, (void *) &op_common, sizeof(op_common));
	if (ret < 0) {
		err("send op_common");
		return -1;
	}

	return 0;
}

int usbip_recv_op_common(int sockfd, uint16_t *code)
{
	int ret;
	struct op_common op_common;

	bzero(&op_common, sizeof(op_common));

	ret = usbip_recv(sockfd, (void *) &op_common, sizeof(op_common));
	if (ret < 0) {
		err("recv op_common, %d", ret);
		goto err;
	}

	PACK_OP_COMMON(0, &op_common);

	if (op_common.version != USBIP_VERSION) {
		err("version mismatch, %d %d", op_common.version, USBIP_VERSION);
		goto err;
	}

	switch(*code) {
		case OP_UNSPEC:
			break;
		default:
			if (op_common.code != *code) {
				info("unexpected pdu %d for %d", op_common.code, *code);
				goto err;
			}
	}

	if (op_common.status != ST_OK) {
		info("request failed at peer, %d", op_common.status);
		goto err;
	}

	*code = op_common.code;

	return 0;
err:
	return -1;
}


int usbip_set_reuseaddr(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_REUSEADDR");

	return ret;
}

int usbip_set_nodelay(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt TCP_NODELAY");

	return ret;
}

int usbip_set_keepalive(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_KEEPALIVE");

	return ret;
}

/* IPv6 Ready */
/*
 * moved here from vhci_attach.c
 */
int tcp_connect(char *hostname, char *service)
{
	struct addrinfo hints, *res, *res0;
	int sockfd;
	int err;


	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	/* get all possible addresses */
	err = getaddrinfo(hostname, service, &hints, &res0);
	if (err) {
		err("%s %s: %s", hostname, service, gai_strerror(err));
		return -1;
	}

	/* try all the addresses */
	for (res = res0; res; res = res->ai_next) {
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		err = getnameinfo(res->ai_addr, res->ai_addrlen,
				hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
		if (err) {
			err("%s %s: %s", hostname, service, gai_strerror(err));
			continue;
		}

		dbg("trying %s port %s\n", hbuf, sbuf);

		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			err("socket");
			continue;
		}

		/* should set TCP_NODELAY for usbip */
		usbip_set_nodelay(sockfd);
		/* TODO: write code for heatbeat */
		usbip_set_keepalive(sockfd);

		err = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if (err < 0) {
			close(sockfd);
			continue;
		}

		/* connected */
		dbg("connected to %s:%s", hbuf, sbuf);
		freeaddrinfo(res0);
		return sockfd;
	}


	dbg("%s:%s, %s", hostname, service, "no destination to connect to");
	freeaddrinfo(res0);

	return -1;
}

#define SIZE sizeof(struct sockaddr_in)

int server_listen_accept()
{
    struct sockaddr_in server;
	int sockfd_connect;
	int sockfd_listen;
    
	if((sockfd_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("fail to call socket()\n");
		exit(1);
	}
    
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(3700);
    
	if(bind(sockfd_listen, (struct sockaddr *)&server, SIZE) == -1)
	{
		printf("fail to call bind()\n");
		exit(1);
	}
	if(listen(sockfd_listen, 5) == -1)
	{
		printf("fail to call listen()\n");
		exit(1);
	}
	printf("sockfd_listen : %d\n", sockfd_listen);
    if((sockfd_connect = accept(sockfd_listen, NULL, NULL)) ==-1)
    {
		printf("fail to call accept()\n");
		exit(1);
	}
	return sockfd_connect;
}

int send_dev(int sockfd, char* cmd, int cmd_size) {
	int ret;
	ret = send(sockfd, cmd, cmd_size, 0);
	if(ret<=0) {
		info("send_dev() send error (%s) (size:%d)", cmd, cmd_size);
		return -1;
	}
	return ret; 
}

#define SIZE_USBDEVFS sizeof(struct usbdevfs_urb)
#define SIZE_ISOPACKETDESC sizeof(struct usbdevfs_iso_packet_desc)
#define SIZE_ASYNCURB sizeof(AsyncURB)

int recv_dev(int sockfd, struct usbdevfs_urb **urb) {
	int ret, total;
	static int seqnum = 0;
	AsyncURB *aurb;
	
	if(*urb!=NULL) {
		info("recv_dev : *urb!=NULL");
		return -1;
	}
	aurb = (AsyncURB*)malloc(SIZE_ASYNCURB+12);
	if(aurb==NULL) {
		info("malloc() error");
		goto fail;
	}
	*urb = &(aurb->urb);
	ret = recv(sockfd, *urb, SIZE_USBDEVFS, 0);
	if(ret<=0) {
		info("recv_dev : recv(urb) error : sockfd(%d)", sockfd);
		goto fail;
	}
	//info("recv(urb) : %d", ret);
	total = ret;
	(*urb)->buffer = (unsigned char*)malloc((*urb)->buffer_length);
	//(*urb)->iso_frame_desc = (struct usbdevfs_iso_packet_desc*)malloc(SIZE_ISOPACKETDESC);
	if((*urb)->buffer==NULL) { // || (*urb)->iso_frame_desc==NULL
		info("malloc() error");
		goto fail;
	}
	memset((*urb)->buffer, 0, (*urb)->buffer_length);
	ret = recv(sockfd, (*urb)->buffer, (*urb)->buffer_length, 0);
	if(ret<=0) {
		info("recv_dev : recv((*urb)->buffer) error (%d)", sockfd);
		goto fail;
	}
	//info("recv((*urb)->buffer) : %d", ret);
	total += ret;
	ret = recv(sockfd, (*urb)->iso_frame_desc, SIZE_ISOPACKETDESC, 0);
	if(ret<=0) {
		info("recv_dev : recv((*urb)->iso_frame_desc[0]) error (%d)", sockfd);
		goto fail;
	}
	//info("recv((*urb)->iso_frame_desc[0]) : %d", ret);
	total += ret;
	// info("recv total : %d", total);

	aurb->seqnum = ++seqnum;
	aurb->sub_seqnum = 0;
	aurb->data_len = (*urb)->buffer_length;
	aurb->data = (char*)malloc(aurb->data_len);
	if(aurb->data==NULL) {
		info("malloc() error");
		goto fail;
	}
	memset(aurb->data, 0, aurb->data_len);
	memcpy(aurb->data, (*urb)->buffer, aurb->data_len);
	return total;

fail:
	if(*urb) {
		free((*urb)->buffer);
		//free((*urb)->iso_frame_desc);
	}
	free(*urb);
	return -1;
}

#define USBIPAPP_PORT 3650

int connect_to_usbipApp()
{
	struct sockaddr_in sockin;
	int con = -1;
	int sockfd = -1;

	if( (sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1) {
		fprintf(stderr, "socket() fail\n");
		return -1;
	}
	memset(&sockin, 0, sizeof(sockin));
	sockin.sin_family = AF_INET;
	sockin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockin.sin_port = htons(USBIPAPP_PORT);

	con = connect(sockfd,(struct sockaddr*)&sockin, sizeof(sockin));
	if(con==-1) {
		fprintf(stderr, "connect() fail\n");
		return -2;
	}
	return sockfd;
}

int printToApp(int sockfd, char* msg)
{
	int ret;
	ret = send(sockfd, msg, strlen(msg)+1, 0);
	if(ret<=0) {
		fprintf(stderr, "usbipApp send error\n");
		return -1;
	}
	return 0;
}

#define MAXLINE 1024

int errorToApp(int sockfd, char* msg)
{
	int ret;
	char errmsg[MAXLINE];
	sprintf(errmsg, "err : %s", msg);
	ret = send(sockfd, errmsg, strlen(errmsg)+1, 0);
	if(ret<=0) {
		fprintf(stderr, "usbipApp send error\n");
		return -1;
	}
	return 0;
}
