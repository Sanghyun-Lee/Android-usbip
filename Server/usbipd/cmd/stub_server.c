/*
 * $Id$
 *
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#include "../config.h"

#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef HAVE_LIBWRAP
#include <tcpd.h>
#endif

#include "usbdevice_fs.h"

#define _GNU_SOURCE
#include <getopt.h>
#include <signal.h>

#include "usbip.h"
#include "usbip_network.h"

#include <glib.h>
#include <sys/time.h>


static const char version[] = PACKAGE_STRING
	" ($Id$)";

/*

/*
 * A basic header followed by other additional headers.
 */
struct usbip_header_basic {
#define USBIP_CMD_SUBMIT	0x0001
#define USBIP_CMD_UNLINK	0x0002
#define USBIP_RET_SUBMIT	0x0003
#define USBIP_RET_UNLINK	0x0004
#define USBIP_RESET_DEV		0xFFFF
	unsigned int command;

	 /* sequencial number which identifies requests.
	  * incremented per connections */
	unsigned int seqnum;

	/* devid is used to specify a remote USB device uniquely instead
	 * of busnum and devnum in Linux. In the case of Linux stub_driver,
	 * this value is ((busnum << 16) | devnum) */
	unsigned int devid;  

#define USBIP_DIR_OUT	0
#define USBIP_DIR_IN 	1
	unsigned int direction;
	unsigned int ep;     /* endpoint number */
} __attribute__ ((packed));

/*
 * An additional header for a CMD_SUBMIT packet.
 */
struct usbip_header_cmd_submit {
	/* these values are basically the same as in a URB. */

	/* the same in a URB. */
	unsigned int transfer_flags;

	/* set the following data size (out),
	 * or expected reading data size (in) */
	int transfer_buffer_length;

	/* it is difficult for usbip to sync frames (reserved only?) */
	int start_frame;

	/* the number of iso descriptors that follows this header */
	int number_of_packets;

	/* the maximum time within which this request works in a host
	 * controller of a server side */
	int interval;

	/* set setup packet data for a CTRL request */
	unsigned char setup[8];
}__attribute__ ((packed));

/*
 * An additional header for a RET_SUBMIT packet.
 */
struct usbip_header_ret_submit {
	int status;
	int actual_length; /* returned data length */
	int start_frame; /* ISO and INT */
	int number_of_packets;  /* ISO only */
	int error_count; /* ISO only */
}__attribute__ ((packed));

/*
 * An additional header for a CMD_UNLINK packet.
 */
struct usbip_header_cmd_unlink {
	unsigned int seqnum; /* URB's seqnum which will be unlinked */
}__attribute__ ((packed));


/*
 * An additional header for a RET_UNLINK packet.
 */
struct usbip_header_ret_unlink {
	int status;
}__attribute__ ((packed));


/* the same as usb_iso_packet_descriptor but packed for pdu */
struct usbip_iso_packet_descriptor {
	unsigned int offset;
	unsigned int length;            /* expected length */
	unsigned int actual_length;
	unsigned int status;
}__attribute__ ((packed));


/*
 * All usbip packets use a common header to keep code simple.
 */
struct usbip_header {
	struct usbip_header_basic base;

	union {
		struct usbip_header_cmd_submit	cmd_submit;
		struct usbip_header_ret_submit	ret_submit;
		struct usbip_header_cmd_unlink	cmd_unlink;
		struct usbip_header_ret_unlink	ret_unlink;
	} u;
}__attribute__ ((packed));

#define USBDEVFS_MAX_LEN 16384
#define USB_MAX_ISO_PACKET_SIZE 8192
#define USBDEVFS_MAX_ISO_LEN 32768

int sock_USBIPAPP;

void show_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	dbg("\n%lu.%lu: ",tv.tv_sec, tv.tv_usec);
}

static int send_reply_devlist(int sockfd)
{
	int ret;
	struct usbip_exported_device *edev;
	struct op_devlist_reply reply;


	reply.ndev = 0;

	/* how many devices are exported ? */
	dlist_for_each_data(stub_driver->edev_list, edev, struct usbip_exported_device) {
		reply.ndev += 1;
	}

	dbg("%d devices are exported", reply.ndev);

	ret = usbip_send_op_common(sockfd, OP_REP_DEVLIST,  ST_OK);
	if (ret < 0) {
		err("send op_common");
		return ret;
	}

	PACK_OP_DEVLIST_REPLY(1, &reply);

	ret = usbip_send(sockfd, (void *) &reply, sizeof(reply));
	if (ret < 0) {
		err("send op_devlist_reply");
		return ret;
	}

	dlist_for_each_data(stub_driver->edev_list, edev, struct usbip_exported_device) {
		struct usb_device pdu_udev;

		dump_usb_device(&edev->udev);
		memcpy(&pdu_udev, &edev->udev, sizeof(pdu_udev));
		pack_usb_device(1, &pdu_udev);

		ret = usbip_send(sockfd, (void *) &pdu_udev, sizeof(pdu_udev));
		if (ret < 0) {
			err("send pdu_udev");
			return ret;
		}

		for (int i=0; i < edev->udev.bNumInterfaces; i++) {
			struct usb_interface pdu_uinf;

			dump_usb_interface(&edev->uinf[i]);
			memcpy(&pdu_uinf, &edev->uinf[i], sizeof(pdu_uinf));
			pack_usb_interface(1, &pdu_uinf);

			ret = usbip_send(sockfd, (void *) &pdu_uinf, sizeof(pdu_uinf));
			if (ret < 0) {
				err("send pdu_uinf");
				return ret;
			}
		}
	}

	return 0;
}


static int recv_request_devlist(int sockfd)
{
	int ret;
	struct op_devlist_request req;

	bzero(&req, sizeof(req));

	ret = usbip_recv(sockfd, (void *) &req, sizeof(req));
	if (ret < 0) {
		err("recv devlist request");
		return -1;
	}

	ret = send_reply_devlist(sockfd);
	if (ret < 0) {
		err("send devlist reply");
		return -1;
	}

	return 0;
}

static int recv_request_devlist(int sockfd);
static int recv_request_import(int sockfd);

static void correct_endian_basic(struct usbip_header_basic *base, int send)
{
	if (send) {
		base->command	= htonl(base->command);
		base->seqnum	= htonl(base->seqnum);
		base->devid	= htonl(base->devid);
		base->direction	= htonl(base->direction);
		base->ep	= htonl(base->ep);
	} else {
		base->command	= ntohl(base->command);
		base->seqnum	= ntohl(base->seqnum);
		base->devid	= ntohl(base->devid);
		base->direction	= ntohl(base->direction);
		base->ep	= ntohl(base->ep);
	}
}

void cpu_to_be32s(int *a)
{
	*a=htonl((unsigned int)*a);
}

void be32_to_cpus(int *a)
{
	*a=ntohl((unsigned int)*a);
}

static void correct_endian_cmd_submit(struct usbip_header_cmd_submit *pdu, int send)
{
	if (send) {
		pdu->transfer_flags = htonl(pdu->transfer_flags);

		cpu_to_be32s(&pdu->transfer_buffer_length);
		cpu_to_be32s(&pdu->start_frame);
		cpu_to_be32s(&pdu->number_of_packets);
		cpu_to_be32s(&pdu->interval);
	} else {
		pdu->transfer_flags = ntohl(pdu->transfer_flags);

		be32_to_cpus(&pdu->transfer_buffer_length);
		be32_to_cpus(&pdu->start_frame);
		be32_to_cpus(&pdu->number_of_packets);
		be32_to_cpus(&pdu->interval);
	}
}

static void correct_endian_ret_submit(struct usbip_header_ret_submit *pdu, int send)
{
	if (send) {
		cpu_to_be32s(&pdu->status);
		cpu_to_be32s(&pdu->actual_length);
		cpu_to_be32s(&pdu->start_frame);
		cpu_to_be32s(&pdu->error_count);
	} else {
		be32_to_cpus(&pdu->status);
		be32_to_cpus(&pdu->actual_length);
		be32_to_cpus(&pdu->start_frame);
		be32_to_cpus(&pdu->error_count);
	}
}

static void correct_endian_cmd_unlink(struct usbip_header_cmd_unlink *pdu, int send)
{
	if (send)
		pdu->seqnum = htonl(pdu->seqnum);
	else
		pdu->seqnum = ntohl(pdu->seqnum);
}

static void correct_endian_ret_unlink(struct usbip_header_ret_unlink *pdu, int send)
{
	if (send)
		cpu_to_be32s(&pdu->status);
	else
		be32_to_cpus(&pdu->status);
}

void usbip_header_correct_endian(struct usbip_header *pdu, int send)
{
	unsigned int cmd = 0;

	if (send)
		cmd = pdu->base.command;

	correct_endian_basic(&pdu->base, send);

	if (!send)
		cmd = pdu->base.command;

	switch (cmd) {
		case USBIP_RESET_DEV:
			break;
		case USBIP_CMD_SUBMIT:
			correct_endian_cmd_submit(&pdu->u.cmd_submit, send);
			break;
		case USBIP_RET_SUBMIT:
			correct_endian_ret_submit(&pdu->u.ret_submit, send);
			break;

		case USBIP_CMD_UNLINK:
			correct_endian_cmd_unlink(&pdu->u.cmd_unlink, send);
			break;
		case USBIP_RET_UNLINK:
			correct_endian_ret_unlink(&pdu->u.ret_unlink, send);
			break;

		default:
			/* NOTREACHED */
			err("unknown command in pdu header: %d", cmd);
			//BUG();
	}
}

static inline int is_valid_ep(struct usbip_exported_device *edev,
		struct usbip_header *pdu, int * is_ctrl, int * usbdevfs_type)
{
	int addr, in=0;
	struct usbip_endpoint *ep;
	static int types[] = {
		USBDEVFS_URB_TYPE_CONTROL,
		USBDEVFS_URB_TYPE_ISO,
		USBDEVFS_URB_TYPE_BULK,
		USBDEVFS_URB_TYPE_INTERRUPT
	};
	if(pdu->base.direction == USBIP_DIR_IN)
		in=1;
	else if(pdu->base.direction != USBIP_DIR_OUT)
		return 0;
	addr = pdu->base.ep & USB_ENDPOINT_NUMBER_MASK;
	if(addr == 0){
		*usbdevfs_type = USBDEVFS_URB_TYPE_CONTROL;
		goto out;
	}
	ep=&edev->eps[in][addr];
	if(!ep->valid)
		return 0;
	*usbdevfs_type = types[ep->type];
out:
	if(*usbdevfs_type == USBDEVFS_URB_TYPE_CONTROL)
		*is_ctrl = 1;
	else
		*is_ctrl = 0;
	return 1;
}

static inline int is_in_ep(unsigned  int ep_num)
{
	return ep_num & USB_DIR_IN;
}

struct usbip_endpoint * get_ep(struct usbip_exported_device *edev,
		unsigned int ep)
{
	unsigned int addr;
	int in;
	addr = ep & USB_ENDPOINT_NUMBER_MASK;
	in = is_in_ep(ep)?1:0;
	return &edev->eps[in][addr];
}

static void dump_urb(struct usbdevfs_urb *urb)
{
	dbg("type:%d\n"
		"endpoint:0x%02x\n"
		"stauts:%d\n"
		"flags:0x%08x\n"
		"buffer:0x%p\n"
		"buffer_length:%d\n"
		"actual_length:%d\n"
		"start_frame:%d\n"
		"number_of_packets:%d\n"
		"error_count:%d\n"
		"signr:%u\n"
		"usercontext:0x%p\n",
		urb->type,
		urb->endpoint,
		urb->status,
		urb->flags,
		urb->buffer,
		urb->buffer_length,
		urb->actual_length,
		urb->start_frame,
		urb->number_of_packets,
		urb->error_count,
		urb->signr,
		urb->usercontext);
}

static void usbip_dump_header(struct usbip_header *pdu)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	dbg("%lu.%lu: ",tv.tv_sec, tv.tv_usec);
	dbg("BASE: cmd %u seq %u devid %u dir %u ep %u\n",
			pdu->base.command,
			pdu->base.seqnum,
			pdu->base.devid,
			pdu->base.direction,
			pdu->base.ep);

	switch(pdu->base.command) {
		case USBIP_RESET_DEV:
			dbg("CMD_RESET:\n");
		case USBIP_CMD_SUBMIT:
			dbg("CMD_SUBMIT: x_flags %u x_len %u sf %u #p %u iv %u\n",
					pdu->u.cmd_submit.transfer_flags,
					pdu->u.cmd_submit.transfer_buffer_length,
					pdu->u.cmd_submit.start_frame,
					pdu->u.cmd_submit.number_of_packets,
					pdu->u.cmd_submit.interval);
					break;
		case USBIP_CMD_UNLINK:
			dbg("CMD_UNLINK: seq %u\n", pdu->u.cmd_unlink.seqnum);
			break;
		case USBIP_RET_SUBMIT:
			dbg("RET_SUBMIT: st %d al %u sf %d ec %d\n",
					pdu->u.ret_submit.status,
					pdu->u.ret_submit.actual_length,
					pdu->u.ret_submit.start_frame,
					pdu->u.ret_submit.error_count);
			break;
		case USBIP_RET_UNLINK:
			dbg("RET_UNLINK: status %d\n", pdu->u.ret_unlink.status);
			break;
		default:
			/* NOT REACHED */
			dbg("UNKNOWN\n");
	}
}

AsyncURB * find_aurb(struct dlist * dlist, unsigned int seqnum,
		unsigned int sub_seqnum)
{
	AsyncURB * aurb;
	dlist_for_each_data(dlist, aurb, AsyncURB){
		if(aurb->seqnum==seqnum&&
				aurb->sub_seqnum==sub_seqnum){
			return aurb;
		}
	}
	return NULL;
}

int delete_aurb(struct dlist * dlist, unsigned int seqnum,
		unsigned int sub_seqnum)
{
	AsyncURB *aurb;
	dlist_for_each_data(dlist, aurb, AsyncURB){
		if(aurb->seqnum==seqnum&&
				aurb->sub_seqnum==sub_seqnum){
			dlist_delete_before(dlist);
			return 0;
		}
	}
	return -1;
}

static int valid_request(struct usbip_exported_device *edev, 
			struct usbip_header *pdu)
{
	struct usb_device *dev = &edev->udev;
	
	if(pdu->base.devid!=((dev->busnum<<16)|dev->devnum))
		return 0;
	return 1;
}

static void setup_ret_submit_pdu(struct usbip_header *rpdu, AsyncURB *aurb)
{
	struct usbdevfs_urb *urb=&aurb->urb;

	rpdu->base.seqnum = aurb->seqnum;
	rpdu->base.command = USBIP_RET_SUBMIT;

	rpdu->u.ret_submit.status		= urb->status;
	rpdu->u.ret_submit.actual_length = aurb->ret_len;
	rpdu->u.ret_submit.start_frame	= urb->start_frame;
	rpdu->u.ret_submit.error_count	= urb->error_count;
	/* FIXME */
	rpdu->u.ret_submit.number_of_packets = urb->number_of_packets;
}

int try_submit_sub_urb(struct usbip_exported_device *edev, AsyncURB *aurb)
{
	int left_len = aurb->data_len - aurb->ret_len;
	int this_len, ret;
	this_len = (left_len>USBDEVFS_MAX_LEN)?
		USBDEVFS_MAX_LEN:left_len;
	if(this_len==0)
		g_error("why this_len = 0");
	aurb->urb.buffer = aurb->data + aurb->ret_len;
	aurb->urb.buffer_length = this_len;
	ret = ioctl(edev->usbfs_fd, USBDEVFS_SUBMITURB, &aurb->urb);
	if(ret<0){
		err("ioctl last ret %d %m\n", ret);
		return -1;
	}
	return 0;
}

int prepare_iso_data_iov(struct usbdevfs_urb *urb, struct iovec iov[], int ioc)
{
	int i;
	int all, offset, base_offset;
	struct usbdevfs_iso_packet_desc * fs_iso_desc;
	if(urb->number_of_packets>128)
		g_error("why so many packets");
	all=0;
	offset=0;
	base_offset = 0;
	fs_iso_desc = &urb->iso_frame_desc[0];
	for(i=0; i<urb->number_of_packets; i++){
		if(fs_iso_desc->actual_length <
			fs_iso_desc->length||i==urb->number_of_packets-1){
			iov[ioc].iov_base = urb->buffer+base_offset;
			iov[ioc].iov_len =  offset + fs_iso_desc->actual_length
				- base_offset;
			if(iov[ioc].iov_len)
				ioc++;
			base_offset = offset + fs_iso_desc->length;
		}
		offset += fs_iso_desc->length;
		all += fs_iso_desc->actual_length;
		fs_iso_desc++;
	}
	if(all!=urb->actual_length)
		g_error("why not equal %d %d", all, urb->actual_length);
	return ioc;
}

void fix_urb_a_len(struct usbdevfs_urb * urb)
{
	int i;
	int all=0;
	for(i=0;i<urb->number_of_packets;i++){
		all+=urb->iso_frame_desc[i].actual_length;
	}
	if(all!=urb->actual_length){
		dbg("fix actual_length %d->%d\n",
				urb->actual_length,all);
		urb->actual_length = all;
	}
}

static int stub_send_ret_submit(struct usbip_exported_device *edev)
{
	AsyncURB *aurb, *last_aurb;
	struct usbip_header pdu_header;
	struct usbdevfs_urb *urb;
	int ret, len, ep, n;
	struct iovec iov[130];
	int ioc, offset, i;
	int is_ctrl;
	static int m = 0;
	struct usbip_iso_packet_descriptor ip_iso_descs[128];
	struct usbip_endpoint * big_ep;
	urb = NULL;
	do {
/* DEV_TEST
		ret = ioctl(edev->usbfs_fd, USBDEVFS_REAPURBNDELAY, &urb);
	        if (ret < 0) {
			if (errno == EAGAIN)
				return 0;
			g_error("husb: reap urb failed errno %d %m\n", errno);
		}
*/
///*
		ret = recv_dev(edev->tmp_sockfd, &urb);
		if(ret<0) {
			return 0;
		}
//*/
		aurb =  (AsyncURB *) ((char *)urb - offsetof(AsyncURB, urb));

/*
		info("========================");
		info("aurb->seqnum:%u", aurb->seqnum);
		info("aurb->sub_seqnum:%u", aurb->sub_seqnum);
		info("aurb->data_len:%u", aurb->data_len);
		info("aurb->ret_len:%u", aurb->ret_len);
		for(n=0; n<aurb->data_len; n++)
			info(" aurb->data[%d]:0x%02x", n, aurb->data[n]);
		printf("\n");

		for(n=0;n<urb->buffer_length;n++)
			info("  buffer[%d]:0x%02x", n, *(((unsigned char*)urb->buffer)+n));
		//info("buffer_length:%d", urb->buffer_length); 
		//info("actual_length:%d", urb->actual_length);
		//info("start_frame:%d", urb->start_frame);
		//info("number_of_packets:%d", urb->number_of_packets);
		//info("error_count:%d", urb->error_count);
		//info("signr:%u", urb->signr);
		//info("usercontext:0x%p", urb->usercontext);
		info("iso_frame_desc[0].length:0x%08x", urb->iso_frame_desc[0].length);
		info("iso_frame_desc[0].actual_length:0x%08x", 
			urb->iso_frame_desc[0].actual_length);
		info("iso_frame_desc[0].status:0x%08x", urb->iso_frame_desc[0].status);
		//info("sizeof(buffer) : %d", sizeof(*(urb->buffer)));
		//info("sizeof(iso_frame_desc) : %d", sizeof(*(urb->iso_frame_desc)));
		//info("sizeof(urb) : %d", sizeof(*urb));
		//}
*/

		if(aurb->sub_seqnum){
			info("##aurb->sub_seqnum");
			if (urb->type != USBDEVFS_URB_TYPE_ISO) {
				info("##urb->type != USBDEVFS_URB_TYPE_ISO");
				dbg("splited bulk urb ret");
				last_aurb = find_aurb(edev->processing_urbs,
						aurb->seqnum, 0);
				if(last_aurb==NULL){
					dbg("perhaps unlinked urb");
				} else {
					last_aurb->ret_len += urb->actual_length;
				}
				delete_aurb(edev->processing_urbs, aurb->seqnum,
					aurb->sub_seqnum);
				continue;
			} else {
				/* FIXME unlink for iso */
				dbg("splited iso urb ret %d %d",
						aurb->seqnum,
						aurb->sub_seqnum);
				big_ep = get_ep(edev, urb->endpoint);
				if(!big_ep->now_urb)
					g_error("why no urb waiting me");
				last_aurb = big_ep->now_urb;
				if(last_aurb->seqnum!=aurb->seqnum){
					g_error("why not waited me %d %d",
					last_aurb->seqnum, aurb->seqnum);
				}
				i=aurb->sub_seqnum - urb->number_of_packets;
				if(i<0||i>last_aurb->urb.number_of_packets)
					g_error("why this %d\n", i);
				if(aurb->sub_seqnum >
						last_aurb->urb.number_of_packets)
					g_error("why so big sub_seqnum %d\n",
							aurb->sub_seqnum);
				memcpy(&last_aurb->urb.iso_frame_desc[i],
					&urb->iso_frame_desc[0],
					urb->number_of_packets
					* sizeof(urb->iso_frame_desc[0]));
				last_aurb->urb.actual_length+=urb->actual_length;
				last_aurb->urb.error_count+=urb->error_count;
				last_aurb->ret_len+=urb->number_of_packets;
				if( last_aurb->ret_len <
					last_aurb->urb.number_of_packets){
					delete_aurb(edev->processing_urbs,
							aurb->seqnum,
						aurb->sub_seqnum);
					continue;
				}
				last_aurb->ret_len=0;
				delete_aurb(edev->processing_urbs,
						aurb->seqnum,
						aurb->sub_seqnum);
				if(big_ep->waited_urbs &&
						big_ep->waited_urbs->count){
					big_ep->now_urb =
					dlist_shift(big_ep->waited_urbs);
				} else
					big_ep->now_urb = NULL;
				aurb=last_aurb;
				urb=&last_aurb->urb;
				fix_urb_a_len(urb);
				dlist_push(edev->processing_urbs, aurb);
				urb->buffer = aurb->data;
				urb->buffer_length = aurb->data_len;
			}
		}

		if(aurb->urb.type == USBDEVFS_URB_TYPE_BULK &&
			is_in_ep(aurb->urb.endpoint)){
			info("##urb->type == BULK");
			dbg("bulk in urb ret");

			len = aurb->ret_len + urb->actual_length;
			if(urb->status==0
				&&urb->actual_length==urb->buffer_length
				&&len < aurb->data_len){
				/* this is not the last urb */
				aurb->ret_len = len;
				ret = try_submit_sub_urb(edev, aurb);
				if(ret<0){
					delete_aurb(edev->processing_urbs,
					aurb->seqnum,0);
					return -1;
				}
				continue;
			} else {
				/* this is a urb end
				 * we need start next urb */
				ep = aurb->urb.endpoint & USB_ENDPOINT_NUMBER_MASK;
				big_ep = &edev->eps[1][ep];
				if(big_ep->waited_urbs &&
						big_ep->waited_urbs->count){
					big_ep->now_urb =
					dlist_shift(big_ep->waited_urbs);
				} else
					big_ep->now_urb = NULL;
				if(big_ep->now_urb){
					ret = try_submit_sub_urb(edev,
							big_ep->now_urb);
					if(ret<0){
						delete_aurb(edev->processing_urbs,
						aurb->seqnum,0);
						return -1;
					}
					dlist_push(edev->processing_urbs,
							big_ep->now_urb);
				}
			}
		}

		is_ctrl = (urb->type==USBDEVFS_URB_TYPE_CONTROL);
		memset(&pdu_header, 0, sizeof(pdu_header));

		aurb->ret_len += urb->actual_length;
		setup_ret_submit_pdu(&pdu_header, aurb);
		if(urb->status<0){
			dbg("faint error %d\n", urb->endpoint);
//			ep=urb->endpoint;
//			ret=ioctl(edev->usbfs_fd, USBDEVFS_CLEAR_HALT, 
//					&ep);
//			dbg("clear halt ret %d\n", ret);
		}
		usbip_dump_header(&pdu_header);
		usbip_header_correct_endian(&pdu_header, 1);
		dump_urb(urb);
		iov[0].iov_base = &pdu_header;
		iov[0].iov_len = sizeof(pdu_header);
		ioc=1;
		info("##iov setting end");
		if(is_in_ep(urb->endpoint)&&aurb->ret_len>0){
			if(urb->type!=USBDEVFS_URB_TYPE_ISO){
				iov[ioc].iov_base = aurb->data+(is_ctrl?8:0);
				iov[ioc].iov_len = aurb->ret_len;
				ioc++;
			} else
				ioc = prepare_iso_data_iov(urb, iov, ioc);
		}
		if(urb->type==USBDEVFS_URB_TYPE_ISO){
			info("##urb->type == USBDEVFS_URB_TYPE_ISO");
			iov[ioc].iov_base = &ip_iso_descs[0];
			iov[ioc].iov_len = sizeof(ip_iso_descs[0])
				* urb->number_of_packets;
			offset=0;
			for(i=0;i<urb->number_of_packets;i++){
				ip_iso_descs[i].offset = htonl(offset);
				ip_iso_descs[i].length =
					htonl(urb->iso_frame_desc[i].length);
				ip_iso_descs[i].status =
					htonl(urb->iso_frame_desc[i].status);
				ip_iso_descs[i].actual_length =
					htonl(urb->iso_frame_desc[i].actual_length);
				offset+=urb->iso_frame_desc[i].length;
			}
			ioc++;
		}
		info("##usbip_sendv");
		ret=usbip_sendv(edev->client_fd, iov, ioc);
		if(ret<0)
			g_error("can't send pdu_header");
		info("##usbip_sendv ret : %d, aurb->seqnum : %d", ret, aurb->seqnum);
		delete_aurb(edev->processing_urbs, aurb->seqnum, 0);
		info("##delete_aurb");
	} while(1);
	return 0;
}

static int stub_send_ret_unlink(int fd, int seqnum, int status)
{
	int ret;
	struct usbip_header pdu_header;
	memset(&pdu_header, 0, sizeof(pdu_header));
	pdu_header.base.seqnum = seqnum;
	pdu_header.base.command = USBIP_RET_UNLINK;
	pdu_header.u.ret_unlink.status = status;
	usbip_header_correct_endian(&pdu_header, 1);
	ret = usbip_send(fd, &pdu_header, sizeof(pdu_header));
	if (ret != sizeof(pdu_header))
		g_error("send ret");
	return 0;
}

static int cancel_urb(struct dlist * processing_urbs, unsigned int seqnum, int fd);

static int stub_recv_cmd_unlink(struct usbip_exported_device *edev,
		struct usbip_header *pdu)
{
	int ret;
	show_time();
	ret=cancel_urb(edev->processing_urbs,
			pdu->u.cmd_unlink.seqnum,
			edev->usbfs_fd);
	stub_send_ret_unlink(edev->client_fd, pdu->base.seqnum, -ECONNRESET);
	return 0;
}

unsigned int get_transfer_flag(unsigned  int flag)
{
	//FIXME  now uurb flag = flag in kernel, but it perhaps will change
	return flag &(
	        USBDEVFS_URB_ISO_ASAP|
		USBDEVFS_URB_SHORT_NOT_OK|
		USBDEVFS_URB_NO_FSBR|
		USBDEVFS_URB_ZERO_PACKET|
		USBDEVFS_URB_NO_INTERRUPT);
}

static int cancel_urb(struct dlist * processing_urbs, unsigned int seqnum, int fd)
{
	AsyncURB * aurb;
	int ret=-1;

	dlist_for_each_data(processing_urbs, aurb, AsyncURB) {
		if (seqnum == aurb->seqnum){
			dbg("found seqnum %d, subseqnum %d\n", seqnum,
					aurb->sub_seqnum);
			ret = ioctl(fd, USBDEVFS_DISCARDURB, &aurb->urb);
			if(ret <0){
				dbg("discard urb ret %d %m\n", ret);
			} else {
				if(aurb->sub_seqnum==0)
					aurb->sub_seqnum = 0xffff;
				dbg("discard urb success");
				//dlist_delete_before(processing_urbs);
			}
			ret = 0;
		}
	}
	return ret;
}

int submit_single_urb(int fd, AsyncURB *aurb, struct dlist * processing_urbs)
{
	static int sn = 0;
	int n;
	char cmd_num[20];
	int ret;
	switch(aurb->urb.type){
		case USBDEVFS_URB_TYPE_CONTROL:
		case USBDEVFS_URB_TYPE_BULK:
		case USBDEVFS_URB_TYPE_INTERRUPT:
			if(aurb->data_len > USBDEVFS_MAX_LEN)
				goto too_big;
			break;
		case USBDEVFS_URB_TYPE_ISO:
			if(aurb->data_len > USBDEVFS_MAX_ISO_LEN)
				goto too_big;
			break;
		default:
			g_error("should never go here");
	}
	aurb->urb.buffer = aurb->data;
	aurb->urb.buffer_length = aurb->data_len;
	dump_urb(&aurb->urb);
	// info("##submit_single_urb");
/* DEV_TEST
	ret = ioctl(fd, USBDEVFS_SUBMITURB, &aurb->urb);
	if(ret<0){
		err("ioctl last ret %d %m\n", ret);
		return -1;
	}
*/

///* DEV_TEST
	//if(aurb->seqnum <= 7) {
		sprintf(cmd_num, "%d", aurb->seqnum);
		info("cmd_num:%s",cmd_num);
		send_dev(fd, cmd_num, strlen(cmd_num)+1);
		//if(aurb->seqnum <= 5)
		dlist_push(processing_urbs, (void *)aurb);
	//}
//*/
	
	return 0;
too_big:
	err("too big data_len for single urb, len: %d, type:%d\n",
			aurb->data_len,
			aurb->urb.type);
	return -1;
}

//try split urb
int submit_bulk_out_urb(int fd, AsyncURB *aurb, struct dlist * processing_urbs)
{
	int all_len, left_len, this_len, ret, sub_seqnum=0;
	AsyncURB *t_aurb;
	struct usbdevfs_urb *urb;
	left_len=all_len=aurb->data_len;
	while(left_len){
		this_len = (left_len>USBDEVFS_MAX_LEN)?
			USBDEVFS_MAX_LEN:left_len;
		left_len-=this_len;
		sub_seqnum++;
		if(left_len==0){
			//the last urb
			aurb->urb.buffer = aurb->data + all_len - this_len;
			aurb->urb.buffer_length = this_len;
			ret = ioctl(fd, USBDEVFS_SUBMITURB, &aurb->urb);
			if(ret<0){
				err("ioctl last ret %d %m\n", ret);
				goto err;
			}
			dlist_push(processing_urbs, (void *)aurb);
			return 0;
		}
		t_aurb=calloc(1, sizeof(*aurb));
		if(NULL==aurb){
			err("malloc\n");
			ret = -1;
			goto err;
		}
		memcpy(t_aurb, aurb, sizeof(*aurb));
		t_aurb->sub_seqnum = sub_seqnum;
		urb=&t_aurb->urb;
		urb->buffer = aurb->data + (all_len-left_len-this_len);
		urb->buffer_length = this_len;
		ret = ioctl(fd, USBDEVFS_SUBMITURB, urb);
		if(ret<0){
			err("ioctl mid ret %d %m\n", ret);
			goto err;
		}
		dlist_push(processing_urbs, (void *)t_aurb);
	}
	g_error("never reach here");
err:
	if(sub_seqnum>1)
		cancel_urb(processing_urbs, aurb->seqnum, fd);
	return ret;
}

int submit_sub_iso_urb(struct usbip_exported_device *edev,
		AsyncURB *aurb, int from, int to,
		int base, int  len)
{
	AsyncURB * t_aurb;
	int ret;
	int i;
	int iso_num = to - from;
	t_aurb=malloc(sizeof(*t_aurb) +
			iso_num * sizeof(t_aurb->urb.iso_frame_desc[0]));
	if(t_aurb==NULL){
		err("malloc %m");
		return -1;
	}
	memcpy(t_aurb, aurb, sizeof(*aurb));
	t_aurb->data = aurb->data + base;
	t_aurb->data_len = len;
	t_aurb->urb.number_of_packets = iso_num;
	memcpy(&t_aurb->urb.iso_frame_desc[0],
		&aurb->urb.iso_frame_desc[from],
		sizeof(t_aurb->urb.iso_frame_desc[0])*iso_num);
	t_aurb->sub_seqnum = to;
	dbg("submited seq: %d sub_seqnum %d\n", t_aurb->seqnum, to);
	ret = submit_single_urb(edev->usbfs_fd, t_aurb, edev->processing_urbs);
	if(ret<0){
		free(t_aurb);
		return -1;
	}
	return 0;
}



int submit_iso_urb(struct usbip_exported_device *edev, AsyncURB *aurb)
{
	struct usbip_endpoint * iso_ep=get_ep(edev, aurb->urb.endpoint);
	int i;
	int offset=0, base=0, base_i=0;

	if(aurb->data_len <= USBDEVFS_MAX_ISO_LEN)
		return submit_single_urb(edev->usbfs_fd, aurb,
				edev->processing_urbs);
	if(iso_ep->now_urb == NULL)
		iso_ep->now_urb = aurb;
	else {
		if(NULL==iso_ep->waited_urbs){
			iso_ep->waited_urbs=dlist_new(sizeof(AsyncURB));
			if(NULL==iso_ep->waited_urbs){
				err("can't malloc");
				return -1;
			}
		}
		dlist_push(iso_ep->waited_urbs, aurb);
	}
	for(i=0; i<aurb->urb.number_of_packets; i++){
		if(offset+aurb->urb.iso_frame_desc[i].length<=
			USBDEVFS_MAX_ISO_LEN){
			offset+=aurb->urb.iso_frame_desc[i].length;
			continue;
		}
		if(submit_sub_iso_urb(edev, aurb, base_i, i, base, offset))
			return -1;
		base += offset;
		base_i = i;
		offset = aurb->urb.iso_frame_desc[i].length;
	}
	return submit_sub_iso_urb(edev, aurb, base_i, i, base, offset);
}

int submit_bulk_in_urb(struct usbip_exported_device *edev, AsyncURB *aurb)
{
	unsigned int addr;
	struct usbip_endpoint * big_ep;
	int ret;
	addr = aurb->urb.endpoint & USB_ENDPOINT_NUMBER_MASK;
	big_ep = &edev->eps[1][addr];
	if(big_ep->now_urb==NULL && aurb->data_len <=USBDEVFS_MAX_LEN)
		return submit_single_urb(edev->usbfs_fd, aurb,
				edev->processing_urbs);
	if(big_ep->now_urb){
		if(NULL==big_ep->waited_urbs){
			big_ep->waited_urbs=dlist_new(sizeof(AsyncURB));
			if(NULL==big_ep->waited_urbs){
				err("can't malloc");
				return -1;
			}
		}
		dlist_push(big_ep->waited_urbs, aurb);
		return 0;
	}
	big_ep->now_urb = aurb;
	ret = try_submit_sub_urb(edev, aurb);
	if(0==ret){
		dlist_push(edev->processing_urbs, aurb);
		return 0;
	} else
		return -1;
}

static void send_err_ret_cmd_submit(struct usbip_exported_device *edev,
		struct usbip_header *pdu, int status)
{
	struct usbip_header reply;
	int ret;

	memcpy(&reply, pdu, sizeof(reply));
	reply.base.command = USBIP_RET_SUBMIT;
	reply.u.ret_submit.status = status;
	reply.u.ret_submit.actual_length = 0;
	reply.u.ret_submit.start_frame	= pdu->u.cmd_submit.start_frame;
	reply.u.ret_submit.error_count	= pdu->u.cmd_submit.number_of_packets;
	usbip_header_correct_endian(&reply, 1);
	ret = usbip_send(edev->client_fd, &reply, sizeof(reply));
	if (ret != sizeof(reply)){
		err("can't send out reply");
		un_imported_dev(edev);
	}
	return;
}

static int is_clear_halt_cmd(AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) aurb->data;

	return (req->bRequest == USB_REQ_CLEAR_FEATURE) &&
		 (req->bRequestType == USB_RECIP_ENDPOINT) &&
		 (req->wValue == USB_ENDPOINT_HALT);
}

static int is_set_interface_cmd(AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) aurb->data;

	return (req->bRequest == USB_REQ_SET_INTERFACE) &&
		   (req->bRequestType == USB_RECIP_INTERFACE);
}

static int is_set_configuration_cmd(AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) aurb->data;

	return (req->bRequest == USB_REQ_SET_CONFIGURATION) &&
		   (req->bRequestType == USB_RECIP_DEVICE);
}

#define le16_to_cpu(a)  (a)
#define USB_RT_PORT     (USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_RESET             4

static int is_reset_device_cmd(AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;
	__u16 value;
	__u16 index;

	req = (struct usb_ctrlrequest *) aurb->data;
	value = le16_to_cpu(req->wValue);
	index = le16_to_cpu(req->wIndex);

	if ((req->bRequest == USB_REQ_SET_FEATURE) &&
		(req->bRequestType == USB_RT_PORT) &&
		(value = USB_PORT_FEAT_RESET)) {
		dbg("reset_device_cmd, port %u\n", index);
		return 1;
	} else
		return 0;
}

static int tweak_clear_halt_cmd(int fd, AsyncURB * aurb)
{
	struct usb_ctrlrequest *req;
	int target_endp;
	int target_dir;
	int target_ep;
	int ret;

	req = (struct usb_ctrlrequest *) aurb->data;

	/*
	 * The stalled endpoint is specified in the wx09
  buffer[15]:0x01
  buffer[16]:0xa1
  buffer[17]:0x00
  buffer[18]:0x05
  buffer[19]:0x09
  buffer[20]:0x19
  buffer[21]:0x01
  buffer[22]:0x29
  buffer[23]:0x03
  buffer[24]:0x15
  buffer[25]:0x00
  buffer[26]:0x2Index value. The endpoint
	 * of the urb is the target of this clear_halt request (i.e., control
	 * endpoint).
	 */
	target_endp = le16_to_cpu(req->wIndex) & USB_ENDPOINT_NUMBER_MASK;

	/* the stalled endpoint direction is IN or OUT?. USB_DIR_IN is 0x80. */
	target_dir = le16_to_cpu(req->wIndex) & USB_ENDPOINT_DIR_MASK;

	target_ep = target_endp|target_dir;

	ret = ioctl(fd, USBDEVFS_CLEAR_HALT,  &target_ep);

	if (ret < 0){
		err("clear halt failed %d %m\n", errno);
		return -1;
	}
	return 1;
}

static int tweak_set_interface_cmd(struct usbip_exported_device *edev,
		AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;
	struct usbdevfs_setinterface st;
	int ret;

	req = (struct usb_ctrlrequest *) aurb->data;
	st.altsetting = le16_to_cpu(req->wValue);
	st.interface = le16_to_cpu(req->wIndex);

	dbg("set_interface: inf %u alt %u\n", st.interface, st.altsetting);

	ret = ioctl(edev->usbfs_fd, USBDEVFS_SETINTERFACE,  &st);
	if (ret < 0){
		err("set interface failed %d %m\n", errno);
		return -1;
	}
	if(usbip_refresh_eps(edev, st.interface, st.altsetting)<0)
		g_error("strange usb device");
	return 1;
}

static int tweak_set_configuration_cmd(int fd, AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;
	__u16 config;

	req = (struct usb_ctrlrequest *)  aurb->data;
	config = le16_to_cpu(req->wValue);

	/*
	 * I have never seen a multi-config device. Very rare.
	 * For most devices, this will be called to choose a default
	 * configuration only once in an initialization phase.
	 *
	 * set_configuration may change a device configuration and its device
	 * drivers will be unbound and assigned for a new device configuration.
	 * This means this usbip driver will be also unbound when called, then
	 * eventually reassigned to the device as far as driver matching
	 * condition is kept.
	 *
	 * Unfortunatelly, an existing usbip connection will be dropped
	 * due to this driver unbinding. So, skip here.
	 * A user may need to set a special configuration value before
	 * exporting the device.
	 */
	info("set_configuration (%d) to dev\n", config);
	info("but, skip!\n");
	return 1;
	//return usb_driver_set_configuration(urb->dev, config);
}

static int tweak_reset_device_cmd(int fd, AsyncURB *aurb)
{
	struct usb_ctrlrequest *req;
	__u16 value;
	__u16 index;
	int ret;

	req = (struct usb_ctrlrequest *) aurb->data;
	value = le16_to_cpu(req->wValue);
	index = le16_to_cpu(req->wIndex);

	info("reset_device (port %d) to dev index:%d value: %d\n",
			index, index, value);

	ret = ioctl(fd, USBDEVFS_RESET);
	if (ret < 0){
		err("set interface failed %d %m\n", errno);
		return -1;
	}
	return 1;
}

static int special_urb(struct usbip_exported_device *edev, AsyncURB *aurb)
{
	if(aurb->urb.endpoint & USB_ENDPOINT_NUMBER_MASK)
		return 0;
	if (is_clear_halt_cmd(aurb))
		/* tweak clear_halt */
		return tweak_clear_halt_cmd(edev->usbfs_fd, aurb);

	else if (is_set_interface_cmd(aurb))
		/* tweak set_interface */
		return tweak_set_interface_cmd(edev, aurb);

	else if (is_set_configuration_cmd(aurb))
		/* tweak set_configuration */
		return tweak_set_configuration_cmd(edev->usbfs_fd, aurb);

	else if (is_reset_device_cmd(aurb))
		return tweak_reset_device_cmd(edev->usbfs_fd, aurb);
	else
		dbg("no need to tweak\n");
	return 0;
}

static void stub_recv_cmd_submit(struct usbip_exported_device *edev,
		struct usbip_header *pdu)
{
	AsyncURB *aurb=NULL;
	struct usbdevfs_urb *urb;
	struct usbip_iso_packet_descriptor ip_iso_descs[128], *ip_iso_desc;
	struct usbdevfs_iso_packet_desc * fs_iso_desc;
	int ret, data_len, iso_num=0, iso_len, is_ctrl, usbdevfs_type;
	struct iovec iov[2];
	int ioc=0, offset, i, m;
	struct usbip_endpoint * iso_ep;

	if(!is_valid_ep(edev, pdu, &is_ctrl, &usbdevfs_type)){
		err("not a valid endpoint, fix your client driver");
		goto failed;
	}
	if(usbdevfs_type == USBDEVFS_URB_TYPE_ISO){
		iso_num = pdu->u.cmd_submit.number_of_packets;
		if (iso_num < 1 || iso_num > 128 ){
			err("kernel not support this, fix your client driver\n");
			goto failed;
		}
	}
	if (pdu->u.cmd_submit.transfer_buffer_length > 0||is_ctrl) {
		data_len = pdu->u.cmd_submit.transfer_buffer_length +
			(is_ctrl?8:0);
	} else
		data_len = 0;
	iso_len =  sizeof(aurb->urb.iso_frame_desc[0]) * iso_num;
	aurb = malloc( sizeof(*aurb) + iso_len + data_len );
	if(NULL == aurb){
		err("can't malloc aurb");
		goto failed;
	}
	memset(aurb, 0, sizeof(*aurb) + iso_len );
	aurb->data = (char *)(aurb+1) + iso_len;
	aurb->data_len = data_len;
	aurb->seqnum = pdu->base.seqnum;
	urb=&aurb->urb;
	urb->endpoint = pdu->base.ep & USB_ENDPOINT_NUMBER_MASK;
	if(pdu->base.direction == USBIP_DIR_IN)
		urb->endpoint|=USB_DIR_IN;

	if(is_ctrl)
		memcpy(aurb->data, pdu->u.cmd_submit.setup, 8);
	if(pdu->base.direction==USBIP_DIR_OUT &&
		pdu->u.cmd_submit.transfer_buffer_length > 0){
		iov[ioc].iov_base = aurb->data + (is_ctrl?8:0);
		iov[ioc].iov_len = pdu->u.cmd_submit.transfer_buffer_length;
		ioc++;
	}
	if(iso_num){
		iov[ioc].iov_base = &ip_iso_descs[0];
		iov[ioc].iov_len = sizeof(ip_iso_descs[0])*iso_num;
		ioc++;
	}
	if(ioc){
		ret=usbip_recvv(edev->client_fd, iov, ioc);
		if(ret<0){
			err("can't recv all data");
			goto failed;
		}
	}
	if(iso_num){
		fs_iso_desc = urb->iso_frame_desc;
		ip_iso_desc = ip_iso_descs;
		offset = 0;
		iso_ep = get_ep(edev, urb->endpoint);
		for(i=0; i<iso_num;i++){
			if(ntohl(ip_iso_desc->offset)!=offset){
				err("i can't deal with this");
				goto failed;
			}
			fs_iso_desc->length = ntohl(ip_iso_desc->length);
			if(pdu->base.direction == USBIP_DIR_OUT){
				if(fs_iso_desc->length >
					iso_ep->max_packet_size){
					err("client driver error, too big packet");
					goto failed;
				}
			} else {
				if(fs_iso_desc->length >
						iso_ep->max_packet_size){
					dbg("fix client driver");
					fs_iso_desc->length =
						iso_ep->max_packet_size;
				}
			}
			offset+=ntohl(ip_iso_desc->length);
			fs_iso_desc++;
			ip_iso_desc++;
		}
		if(offset!=data_len){
			err("i can't deal with this %d %d", offset, data_len);
			goto failed;
		}
	}
	urb->flags = get_transfer_flag(pdu->u.cmd_submit.transfer_flags);
	urb->start_frame = pdu->u.cmd_submit.start_frame;
	urb->number_of_packets = pdu->u.cmd_submit.number_of_packets;
	urb->type = usbdevfs_type;
	if(usbdevfs_type == USBDEVFS_URB_TYPE_ISO)
		urb->flags|=USBDEVFS_URB_ISO_ASAP;
	if((ret=special_urb(edev, aurb))){
		if(ret>0)
			send_err_ret_cmd_submit(edev, pdu, 0);
		else
			send_err_ret_cmd_submit(edev, pdu, ret);
		free(aurb);
		return;
	}
	switch(urb->type){
		case USBDEVFS_URB_TYPE_CONTROL:
		case USBDEVFS_URB_TYPE_INTERRUPT:
			// DEV_TEST
			//ret = submit_single_urb(edev->usbfs_fd, aurb, edev->processing_urbs);
			ret = submit_single_urb(edev->tmp_sockfd, aurb, edev->processing_urbs);
			break;
		case USBDEVFS_URB_TYPE_BULK:
			if(pdu->base.direction == USBIP_DIR_IN)
				ret = submit_bulk_in_urb(edev, aurb);
			else
				ret = submit_bulk_out_urb(edev->usbfs_fd, aurb,
					edev->processing_urbs);
			break;
		case USBDEVFS_URB_TYPE_ISO:
			ret = submit_iso_urb(edev, aurb);
			break;
		default:
			// never reach here
			g_error("end of the world?");
			break;
	}
	if(ret<0){
		dbg("submit ret %d %d %m\n",ret, errno);
		goto failed;
	}

	/* urb is now submited */
	return;
failed:
	if(aurb)
		free(aurb);
	info("unexport dev, perhaps client driver error\n");
	un_imported_dev(edev);
	return;
}

static void reset_dev(struct usbip_exported_device *edev, struct usbip_header *req)
{
	int ret;
	struct usbip_header reply;
	ret = ioctl(edev->usbfs_fd, USBDEVFS_RESET);
	memset(&reply,0,sizeof(reply));
	memcpy(&reply.base, &req->base, sizeof(reply.base));
	usbip_header_correct_endian(&reply, 1);
	ret = usbip_send(edev->client_fd, &reply, sizeof(reply));
	if (ret != sizeof(reply))
		g_error("send ret");
	return;
}

static int recv_client_pdu(struct usbip_exported_device *edev,int sockfd)
{
	int ret;
	struct usbip_header pdu;

	memset(&pdu, 0, sizeof(pdu));
	ret = usbip_recv(sockfd, &pdu, sizeof(pdu));
	if (ret !=sizeof(pdu)){
		err("recv a header, %d %m", ret);
		un_imported_dev(edev);
		return -1;
	}
	usbip_header_correct_endian(&pdu, 0);
	dbg("recv header %d\n",ret);
	usbip_dump_header(&pdu);
	if (!valid_request(edev, &pdu)) {
		g_error("recv invalid request\n");
		return 0;
	}
	switch (pdu.base.command) {
		case USBIP_RESET_DEV:
			reset_dev(edev, &pdu);
			break;
		case USBIP_CMD_UNLINK:
			stub_recv_cmd_unlink(edev, &pdu);
			break;

		case USBIP_CMD_SUBMIT:
			stub_recv_cmd_submit(edev, &pdu);
			break;

		default:
			/* NOTREACHED */
			err("unknown pdu\n");
			return -1;
	}
	return 0;
}

void un_imported_dev(struct usbip_exported_device *edev)
{

	AsyncURB * aurb;
	struct usbdevfs_urb *urb;
	int i;
	struct usbip_endpoint * big_in_ep;
	g_source_remove(edev->client_gio_id);
	close(edev->client_fd);
	edev->client_fd=-1;
	edev->status = SDEV_ST_AVAILABLE;

	/* Clear all of submited urb */
	dlist_for_each_data(edev->processing_urbs, aurb, AsyncURB) {
		ioctl(edev->usbfs_fd, USBDEVFS_DISCARDURB, &aurb->urb);
	}
	while(0==ioctl(edev->usbfs_fd, USBDEVFS_REAPURBNDELAY, &urb));
	dlist_destroy(edev->processing_urbs);
	edev->processing_urbs = NULL;
	for(i=1; i<USB_ENDPOINT_NUMBER_MASK+1;i++){
		big_in_ep = &edev->eps[1][i];
		if(!big_in_ep->waited_urbs)
			continue;
		dlist_destroy(big_in_ep->waited_urbs);
		big_in_ep->waited_urbs =NULL;
	}
}

gboolean process_client_pdu(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	int ret;
	int client_fd;
	struct usbip_exported_device *edev
			= (struct usbip_exported_device *) data;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)){
		un_imported_dev(edev);
		return FALSE;
	}

	if (condition & G_IO_IN) {
		client_fd = g_io_channel_unix_get_fd(gio);
		if(client_fd != edev->client_fd)
			g_error("fd corrupt?");
		ret = recv_client_pdu(edev, client_fd);
		if (ret < 0)
			err("process recieved pdu");
	}
	return TRUE;
}

gboolean process_device_urb(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	int fd;
	struct usbip_exported_device *edev = (struct usbip_exported_device *) data;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)){
		dbg("device disconnected?\n");
		if(edev->status == SDEV_ST_USED)
			un_imported_dev(edev);
		else if(edev->status != SDEV_ST_AVAILABLE)
			g_error("why it is not available\n");
		g_source_remove(edev->usbfs_gio_id);
		close(edev->usbfs_fd);
		edev->usbfs_fd=-1;
		unexport_device(edev);
		return FALSE;
	}
	if (condition & G_IO_OUT) {
		fd = g_io_channel_unix_get_fd(gio);
		if(fd != edev->usbfs_fd)
			g_error("fd corrupt?");
		if(edev->status != SDEV_ST_USED)
			g_error("why I have event when not imported?\n");
		// info("##process_device_urb");
		stub_send_ret_submit(edev);
	}
	return TRUE;
}

// DEV_TEST
gboolean process_device_urb_test(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	int fd;
	struct usbip_exported_device *edev = (struct usbip_exported_device *) data;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)){
		dbg("device disconnected?\n");
		if(edev->status == SDEV_ST_USED)
			un_imported_dev(edev);
		else if(edev->status != SDEV_ST_AVAILABLE)
			g_error("why it is not available\n");
		g_source_remove(edev->usbfs_gio_id);
		close(edev->usbfs_fd);
		edev->usbfs_fd=-1;
		unexport_device(edev);
		return FALSE;
	}
	if (condition & G_IO_IN) {
		fd = g_io_channel_unix_get_fd(gio);
		if(fd != edev->tmp_sockfd)
			g_error("fd corrupt?");
		if(edev->status != SDEV_ST_USED)
			g_error("why I have event when not imported?\n");
		stub_send_ret_submit(edev);
	}
	return TRUE;
}

static int recv_request_export(int sockfd)
{
	int ret;
	struct op_export_request req;
	struct op_common reply;
	struct usbip_exported_device *edev;
	int found = 0;
	struct sockaddr sa;
	socklen_t len = sizeof(sa);
	GIOChannel *gio;

	bzero(&sa, sizeof(sa));
	ret = getpeername(sockfd,(struct sockaddr *)&sa,  &len);
	if(ret<0){
		g_error("can't getpeername");
	}
	if((sa.sa_family ==AF_INET6
	  &&!memcmp(&((struct sockaddr_in6 *)&sa)->sin6_addr, &in6addr_loopback
		  ,sizeof(in6addr_loopback)))
	 ){
		dbg("%d\n",sa.sa_family);
		dbg(" connect from localhost/tcp");
//		return -1;
	}
	bzero(&req, sizeof(req));
	bzero(&reply, sizeof(reply));

	ret = usbip_recv(sockfd, (void *) &req, sizeof(req));
	if (ret < 0) {
		err("recv export request");
		return -1;
	}
	PACK_OP_EXPORT_REQUEST(0, &req);

	dlist_for_each_data(stub_driver->edev_list, edev, struct usbip_exported_device) {
		if (!strncmp(req.udev.busid, edev->udev.busid, SYSFS_BUS_ID_SIZE)) {
			dbg("found requested device %s", req.udev.busid);
			found = 1;
			info("found requested device %s", req.udev.busid);
			break;
		}
	}
	do {
		ret = 0;
		if(found)
			break;

		edev = export_device(req.udev.busid);
		if(!edev){
			info("export_device error");
			printf("export_device error");
			ret = -1;
			break;
		}
/* DEV_TEST
		gio = g_io_channel_unix_new(edev->usbfs_fd);
		edev->usbfs_gio_id = g_io_add_watch(gio,
			(G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
			process_device_urb, edev);
		g_io_channel_unref(gio);
*/
///*
		gio = g_io_channel_unix_new(edev->tmp_sockfd);
		edev->usbfs_gio_id = g_io_add_watch(gio,
			(G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
			process_device_urb_test, edev);
		g_io_channel_unref(gio);
//*/
	} while(0);

	ret = usbip_send_op_common(sockfd, OP_REP_EXPORT, (ret==0 ? ST_OK : ST_NA));
	if (ret < 0) {
		err("send import reply");
		return -1;
	}
	return 0;
}

static int recv_request_import(int sockfd)
{
	int ret;
	struct op_import_request req;
	struct op_common reply;
	struct usbip_exported_device *edev;
	int found = 0;
	int error = 1;
	GIOChannel *gio;
	struct usb_device pdu_udev;

	bzero(&req, sizeof(req));
	bzero(&reply, sizeof(reply));

	ret = usbip_recv(sockfd, (void *) &req, sizeof(req));
	if (ret < 0) {
		err("recv import request");
		return -1;
	}

	PACK_OP_IMPORT_REQUEST(0, &req);

	dlist_for_each_data(stub_driver->edev_list, edev, struct usbip_exported_device) {
		if (!strncmp(req.busid, edev->udev.busid, SYSFS_BUS_ID_SIZE)) {
			dbg("found requested device %s", req.busid);
			found = 1;
			break;
		}
	}

	if(found && edev->status==SDEV_ST_AVAILABLE)
		error = 0;

	ret = usbip_send_op_common(sockfd, OP_REP_IMPORT, (!error ? ST_OK : ST_NA));
	if (ret < 0) {
		err("send import reply");
		goto fail;
	}

	if (error)
		goto fail;

	memcpy(&pdu_udev, &edev->udev, sizeof(pdu_udev));
	pack_usb_device(1, &pdu_udev);

	ret = usbip_send(sockfd, (void *) &pdu_udev, sizeof(pdu_udev));
	if (ret < 0) {
		err("send devinfo");
		goto fail;
	}
	edev->processing_urbs = dlist_new(sizeof(AsyncURB));
	if(NULL==edev->processing_urbs)
		g_error("can't malloc\n");
	edev->status = SDEV_ST_USED;
	usbip_set_nodelay(sockfd);
	edev->client_fd = sockfd;
	gio = g_io_channel_unix_new(sockfd);
	edev->client_gio_id = g_io_add_watch(gio,
		(G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
		process_client_pdu, edev);
		g_io_channel_unref(gio);
	return 0;
fail:
	close(sockfd);
	return -1;
}

static int recv_pdu(int sockfd)
{
	int ret;
	uint16_t code = OP_UNSPEC;


	ret = usbip_recv_op_common(sockfd, &code);
	if (ret < 0) {
		err("recv op_common, %d", ret);
		return ret;
	}

	switch(code) {
		case OP_REQ_DEVLIST:
			ret = recv_request_devlist(sockfd);
			close(sockfd);
			break;

		case OP_REQ_IMPORT:
			ret = recv_request_import(sockfd);
			/* don't close sockfd */
			break;
		case OP_REQ_EXPORT:
			ret = recv_request_export(sockfd);
			close(sockfd);
			break;
		case OP_REQ_DEVINFO:
		case OP_REQ_CRYPKEY:

		default:
			close(sockfd);
			err("unknown op_code, %d", code);
			ret = -1;
	}


	return ret;
}




static void log_addrinfo(struct addrinfo *ai)
{
	int ret;
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV];

	ret = getnameinfo(ai->ai_addr, ai->ai_addrlen, hbuf, sizeof(hbuf),
			sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

	info("listen at [%s]:%s", hbuf, sbuf);
}

static struct addrinfo *my_getaddrinfo(char *host, int ai_family)
{
	int ret;
	struct addrinfo hints, *ai_head;

	bzero(&hints, sizeof(hints));

	hints.ai_family   = ai_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	ret = getaddrinfo(host, USBIP_PORT_STRING, &hints, &ai_head);
	if (ret) {
		err("%s: %s", USBIP_PORT_STRING, gai_strerror(ret));
		return NULL;
	}

	return ai_head;
}

#define MAXSOCK 20
static int listen_all_addrinfo(struct addrinfo *ai_head, int lsock[])
{
	struct addrinfo *ai;
	int n = 0;		/* number of sockets */
	
	printf("start\n");
	for (ai = ai_head; ai && n < MAXSOCK; ai = ai->ai_next) {
		int ret;

		lsock[n] = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (lsock[n] < 0){
			printf("socket\n");
			continue;
		}

		usbip_set_reuseaddr(lsock[n]);
		usbip_set_nodelay(lsock[n]);

		if (lsock[n] >= FD_SETSIZE) {
			printf("if (lsock[n] >= FD_SETSIZE)\n");
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = bind(lsock[n], ai->ai_addr, ai->ai_addrlen);
		if (ret < 0) {
			printf("bind\n");
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = listen(lsock[n], SOMAXCONN);
		if (ret < 0) {
			printf("listen\n");
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		log_addrinfo(ai);

		/* next if succeed */
		n++;
	}
	if (n == 0) {
		errorToApp(sock_USBIPAPP, "no socket to listen to");
		return -1;
	}

	dbg("listen %d address%s", n, (n==1)?"":"es");

	return n;
}

#ifdef HAVE_LIBWRAP
static int tcpd_auth(int csock)
{
	int ret;
	struct request_info request;

	request_init(&request, RQ_DAEMON, "usbipd", RQ_FILE, csock, 0);

	fromhost(&request);

	ret = hosts_access(&request);
	if (!ret)
		return -1;

	return 0;
}
#endif

static int my_accept(int lsock)
{
	int csock;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, sizeof(host), port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

#ifdef HAVE_LIBWRAP
	ret = tcpd_auth(csock);
	if (ret < 0) {
		info("deny access from %s", host);
		close(csock);
		return -1;
	}
#endif

	info("connected from %s:%s", host, port);
printf("connected from %s:%s", host, port);

	return csock;
}


GMainLoop *main_loop;

static void signal_handler(int i)
{
	dbg("signal catched, code %d", i);

	if (main_loop)
		g_main_loop_quit(main_loop);
}

static void set_signal(void)
{
	struct sigaction act;

	bzero(&act, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
}

gboolean process_comming_request(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	int ret;

	UNUSED(data);

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		g_error("unknown condition 3");


	if (condition & G_IO_IN) {
		int lsock;
		int csock;

		lsock = g_io_channel_unix_get_fd(gio);

		csock = my_accept(lsock);
		if (csock < 0)
			return TRUE;

		ret = recv_pdu(csock);
		if (ret < 0)
			err("process recieved pdu");
	}

	return TRUE;
}

static void do_standalone_mode(gboolean daemonize)
{
	int ret;
	int lsock[MAXSOCK];
	struct addrinfo *ai_head;
	int n;
	GIOChannel *gio;

	ret = usbip_names_init(USBIDS_FILE);

	if (ret)
		err("open usb.ids");

	ret = usbip_stub_driver_open();

	if (ret < 0)
		g_error("driver open failed");

	if (daemonize) {
		if (daemon(0,0) < 0)
			g_error("daemonizing failed: %s", g_strerror(errno));

		usbip_use_syslog = 1;
	}

	set_signal();

	ai_head = my_getaddrinfo(NULL, PF_UNSPEC);
	if (!ai_head)
		return;

	n = listen_all_addrinfo(ai_head, lsock);
/*
	if (n <= 0)
		errorToApp(sock_USBIPAPP, "no socket to listen to");
*/
	for (int i = 0; i < n; i++) {

		gio = g_io_channel_unix_new(lsock[i]);
		g_io_add_watch(gio, (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
				process_comming_request, NULL);
	}

	info("usbipd start (%s)", version);
	printToApp(sock_USBIPAPP, "usbipd start");

	main_loop = g_main_loop_new(FALSE, FALSE);
	g_main_loop_run(main_loop);

	errorToApp(sock_USBIPAPP, "shutdown");

	freeaddrinfo(ai_head);
	usbip_names_free();
	usbip_stub_driver_close();

	return;
}


static const char help_message[] = "\
Usage: usbipd [options]				\n\
	-D, --daemon				\n\
		Run as a daemon process.	\n\
						\n\
	-d, --debug				\n\
		Print debugging information.	\n\
						\n\
	-v, --version				\n\
		Show version.			\n\
						\n\
	-h, --help 				\n\
		Print this help.		\n";

static void show_help(void)
{
	printf("%s", help_message);
}

static const struct option longopts[] = {
	{"daemon",	no_argument,	NULL, 'D'},
	{"debug",	no_argument,	NULL, 'd'},
	{"version",	no_argument,	NULL, 'v'},
	{"help",	no_argument,	NULL, 'h'},
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	gboolean daemonize = FALSE;
	
	enum {
		cmd_standalone_mode = 1,
		cmd_help,
		cmd_version
	} cmd = cmd_standalone_mode;

	sock_USBIPAPP = connect_to_usbipApp();
	if(sock_USBIPAPP<0) {
		fprintf(stderr, "USBIPAPP connect error");
		return -1;
	}

	usbip_use_stderr = 1;
	usbip_use_syslog = 0;

/*
	if (geteuid() != 0)
		errorToApp(sock_USBIPAPP, "running non-root?");
*/
	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "vhdD", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'd':
				usbip_use_debug = 1;
				continue;
			case 'v':
				cmd = cmd_version;
				break;
			case 'h':
				cmd = cmd_help;
				break;
			case 'D':
				daemonize = TRUE;
				break;
			case '?':
				show_help();
				exit(EXIT_FAILURE);
			default:
				err("getopt");
		}
	}

	switch (cmd) {
		case cmd_standalone_mode:
			do_standalone_mode(daemonize);
			break;
		case cmd_version:
			printf("%s\n", version);
			break;
		case cmd_help:
			show_help();
			break;
		default:
			info("unknown cmd");
			show_help();
	}

	return 0;
}
