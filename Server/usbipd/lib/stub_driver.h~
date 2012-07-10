/*
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#ifndef _USBIP_STUB_DRIVER_H
#define _USBIP_STUB_DRIVER_H

#include "usbip.h"

typedef struct _AsyncURB
{
    char *data;
    unsigned int seqnum;
    unsigned int sub_seqnum;
    unsigned int data_len;
    unsigned int ret_len;
    struct usbdevfs_urb urb;
} AsyncURB;

struct usbip_stub_driver {
	int ndevs;
	struct dlist *edev_list;	/* list of exported device */
};

struct usbip_endpoint {
	struct dlist * waited_urbs;
	AsyncURB * now_urb;
	unsigned char type;
	unsigned char valid;
	unsigned char intf;
	unsigned char alter;
	unsigned int  max_packet_size;
};

struct usbip_exported_device {
	struct sysfs_device *sudev;
	int32_t status;
	int usbfs_fd;
	int client_fd;
	int usbfs_gio_id;
	int client_gio_id;
	struct usbip_endpoint * eps[2];
	struct dlist * processing_urbs;
	char * desc;
	int desc_len;
	struct usb_device    udev;
	struct usb_interface uinf[];
};

extern struct usbip_stub_driver *stub_driver;

int usbip_stub_driver_open(void);
void usbip_stub_driver_close(void);

int usbip_stub_refresh_device_list(void);
int usbip_stub_export_device(struct usbip_exported_device *edev);

struct usbip_exported_device * export_device(char * busid);
void unexport_device(struct usbip_exported_device * deleted_edev);

int usbip_refresh_eps(struct usbip_exported_device * edev, int inf, int alter);

struct usbip_exported_device *usbip_stub_get_device(int num);

#endif
