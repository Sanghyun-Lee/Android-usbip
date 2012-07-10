/*
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "usbip.h"

struct usbip_stub_driver *stub_driver;

static void usbip_exported_device_delete(void *dev)
{
	struct usbip_exported_device *edev =
		(struct usbip_exported_device *) dev;
	free(edev->eps[0]);
	free(edev->eps[1]);
	free(edev->desc);
	sysfs_close_device(edev->sudev);
	free(dev);
}

static void * seek_to_next_desc(void *buf, size_t size, unsigned int * offset,
		unsigned char type)
{
	unsigned int o=*offset;
	struct usb_descriptor_header * desc;
	if(o>=size)
		return NULL;
	do {
		if(o + sizeof(*desc) > size)
			return NULL;
		desc = buf + o;
		if(desc->bLength + o > size)
			return NULL;
		o+=desc->bLength;
		if(desc->bDescriptorType == type){
			*offset = o;
			return desc;
		}
	}while(1);
}

static int add_ep_info(struct usbip_exported_device *edev,
		struct usb_endpoint_descriptor *ep, int intf, int alter)
{
	int in, addr;
	struct usbip_endpoint * uep;
	int mult;
	if(ep->bEndpointAddress&USB_ENDPOINT_DIR_MASK)
		in=1;
	else
		in=0;
	addr=ep->bEndpointAddress&USB_ENDPOINT_NUMBER_MASK;
	if(addr==0){
		err("err, ep addr equal zero?");
		return -1;
	}
	uep=&edev->eps[in][addr];
	if(uep->valid){
		err("duplicate endpoints");
		return -1;
	}
	uep->type = ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	uep->valid = 1;
	uep->intf = intf;
	uep->alter = alter;
	uep->max_packet_size =  ep->wMaxPacketSize;
	if(uep->type == USB_ENDPOINT_XFER_ISOC &&
		edev->udev.speed == USB_SPEED_HIGH){
		mult = 1 + ((uep->max_packet_size >> 11) & 0x03);
		uep->max_packet_size &= 0x7ff;
		uep->max_packet_size *= mult;
	}
	return 0;
}

static int parse_interface(struct usbip_exported_device *edev,
		struct usb_config_descriptor *config,
		unsigned int *offset, int int_num, int alter)
{
	struct usb_interface_descriptor * intf;
	struct usb_endpoint_descriptor * ep;
	int i;
	do {
		intf = seek_to_next_desc(config, config->wTotalLength,
			offset, USB_DT_INTERFACE);
		if(NULL==intf){
			err("can't get intf");
			return -1;
		}
		if(intf->bLength!=sizeof(*intf)){
			err("intf size error");
			return -1;
		}
		if(intf->bInterfaceNumber<int_num)
			continue;
		if(intf->bAlternateSetting<alter)
			continue;
		if(intf->bInterfaceNumber<int_num){
			err("can't found int %d alter %d\n",int_num,alter);
			return -1;
		}
		if(intf->bAlternateSetting>alter){
			err("can't found int %d alter %d\n",int_num,alter);
			return -1;
		}
		break;
	}while(1);
	for(i=0;i<intf->bNumEndpoints;i++){
		ep = seek_to_next_desc(config, config->wTotalLength,
			offset, USB_DT_ENDPOINT);
		if(NULL==ep){
			err("can't get ep");
			return -1;
		}
		if(ep->bLength!=USB_DT_ENDPOINT_SIZE&&
			ep->bLength!=USB_DT_ENDPOINT_AUDIO_SIZE){
			err("ep size error");
			return -1;
		}
		add_ep_info(edev, ep, int_num, alter);
	}
	return 0;
}

static int parse_config(struct usbip_exported_device * edev,
		struct usb_config_descriptor * config)
{
	int i, ret;
	unsigned int offset=0;
	for(i=0; i< config->bNumInterfaces; i++){
		ret=parse_interface(edev, config,
				&offset, i, 0);
		if(ret<0)
			return -1;
	}
	return 0;
}

static int usb_parse_desc(struct usbip_exported_device *edev, int config_v)
{
	/* FIXME I suppose little endian */
#define MAX_DESC_SIZE 65536
	char * buf;
	int ret, i;
	unsigned int offset=0;
	struct usb_device_descriptor * dev;
	struct usb_config_descriptor * config;
	buf = malloc(MAX_DESC_SIZE);
	if(NULL==buf){
		err("can't malloc %d bytes %m\n", MAX_DESC_SIZE);
		return -1;
	}
	// DEV_TEST
	/* ret=read(edev->usbfs_fd, buf, MAX_DESC_SIZE);
	if(ret<0){
		err("can't read desc %m");
		free(buf);
		return -1;
	}
	if(ret==sizeof(MAX_DESC_SIZE)){
		err("too big desc");
		free(buf);
		return -1;
	}
	*/
 	buf[0] = 0x12;
 	buf[1] = 0x1;
 	buf[2] = 0x0;
 	buf[3] = 0x2;
 	buf[4] = 0x0;
 	buf[5] = 0x0;
 	buf[6] = 0x0;
 	buf[7] = 0x8;
 	buf[8] = 0x61;
 	buf[9] = 0x4;
 	buf[10] = 0x16;
 	buf[11] = 0x4d;
 	buf[12] = 0x0;
 	buf[13] = 0x2;
 	buf[14] = 0x0;
 	buf[15] = 0x2;
 	buf[16] = 0x0;
	buf[17] = 0x1;
 	buf[18] = 0x9;
 	buf[19] = 0x2;
 	buf[20] = 0x22;
 	buf[21] = 0x0;
 	buf[22] = 0x1;
 	buf[23] = 0x1;
 	buf[24] = 0x0;
 	buf[25] = 0xa0;
 	buf[26] = 0x32;
 	buf[27] = 0x9;
 	buf[28] = 0x4;
 	buf[29] = 0x0;
 	buf[30] = 0x0;
 	buf[31] = 0x1;
 	buf[32] = 0x3;
 	buf[33] = 0x1;
 	buf[34] = 0x2;
 	buf[35] = 0x0;
 	buf[36] = 0x9;
 	buf[37] = 0x21;
 	buf[38] = 0x11;
 	buf[39] = 0x1;
 	buf[40] = 0x0;
 	buf[41] = 0x1;
 	buf[42] = 0x22;
 	buf[43] = 0x34;
 	buf[44] = 0x0;
 	buf[45] = 0x7;
 	buf[46] = 0x5;
 	buf[47] = 0x81;
 	buf[48] = 0x3;
 	buf[49] = 0x4;
 	buf[50] = 0x0;
 	buf[51] = 0xa;
	ret = 52;
	/*
	info("====descriptor====");
	for(i=0; i<ret; i++)
		info(" buf[%d] : %x", i, buf[i]);
	*/
	edev->desc = realloc(buf, ret);
	if(NULL == edev->desc){
		err("can't realloc");
		free(buf);
		return -1;
	}
	edev->desc_len = ret;
	buf = edev->desc;
	dev = (struct usb_device_descriptor *)buf;
	if(ret<sizeof(*dev)){
		err("too short desc");
		return -1;
	}
	if(config_v < 1 || config_v > dev->bNumConfigurations){
		err("no such config");
		return -1;
	}
	for(i=0;i<config_v;i++){
		config = seek_to_next_desc(buf, ret, &offset, USB_DT_CONFIG);
		if(NULL==config)
			break;
	}
	if(config->bLength!=sizeof(*config)){
		err("error length short config desc");
		return -1;
	}
	if(offset + config->wTotalLength - config->bLength > ret){
		err("error too long desc offset:%d len:%d",
				offset, config->wTotalLength);
		return -1;
	}
	if(config->bConfigurationValue != config_v){
		err("error, can't find this config_v %d\n", config_v);
		return -1;
	}
	return parse_config(edev, config);
}

static void show_eps(struct usbip_exported_device *edev)
{
	int i,j;
	for(i=0; i<2;i++){
		for(j=1;j<16;j++){
		    if(edev->eps[i][j].valid){
			    dbg("in:%d addr:%d type:%d int:%d alter:%d ps:%d\n",
				i,j,edev->eps[i][j].type,
				edev->eps[i][j].intf,edev->eps[i][j].alter,
				edev->eps[i][j].max_packet_size);
		    }
		}
	}
}

int usbip_refresh_eps(struct usbip_exported_device *edev, int inf, int alter)
{
	int i, j, ret;
	unsigned int offset = 0;
	struct usb_config_descriptor *config;
	for(i=0;i<2;i++){
		for(j=1;j<16;j++){
			if(edev->eps[i][j].intf==inf)
				edev->eps[i][j].valid=0;
		}
	}
	config = seek_to_next_desc(edev->desc, edev->desc_len,
			&offset, USB_DT_CONFIG);
	if(NULL==config){
		err("error can'get config desc");
		return -1;
	}
	if(config->bLength!=sizeof(*config)){
		err("error length short config desc");
		return -1;
	}
	if(offset + config->wTotalLength - config->bLength > edev->desc_len){
		err("error too long desc offset:%d len:%d",
				offset, config->wTotalLength);
		return -1;
	}
	offset = 0;
	ret = parse_interface(edev, config, &offset, inf, alter);
	show_eps(edev);
	return ret;
}

static int usb_host_claim_interfaces(struct usbip_exported_device *edev)
{
	int interface, ret;
	struct usbdevfs_ioctl ctrl;
	for (interface = 0; interface < edev->udev.bNumInterfaces; interface++) {
		ctrl.ioctl_code = USBDEVFS_DISCONNECT;
		ctrl.ifno = interface;
		ret = ioctl(edev->usbfs_fd, USBDEVFS_IOCTL, &ctrl);
		if (ret < 0 && errno != ENODATA) {
			err("USBDEVFS_DISCONNECT");
	                goto fail;
		}
	}

    /* XXX: only grab if all interfaces are free */
	for (interface = 0; interface < edev->udev.bNumInterfaces; interface++) {
		ret = ioctl(edev->usbfs_fd, USBDEVFS_CLAIMINTERFACE, &interface);
		if (ret < 0) {
			if (errno == EBUSY) {
				dbg("device already grabbed\n");
			} else {
				err("husb: failed to claim interface");
			}
			goto fail;
		}
	}
//	do_init_dev(edev->usbfs_fd); //for buggy usb disk
	dbg("husb: %d interfaces claimed\n",
           interface);
	return 0;
fail:
	return -1;
 
}

#define usb_host_device_path "/proc/bus/usb"

static int claim_dev(struct usbip_exported_device *edev)
{
    int fd = -1;
    struct usb_device *dev = &edev->udev;
    char buf[1024];

    dbg("husb: open device %d.%d\n", dev->busnum, dev->devnum);

    snprintf(buf, sizeof(buf), "%s/%03d/%03d", usb_host_device_path,
             dev->busnum, dev->devnum);

	/*//DEV_TEST
    fd = open(buf, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
	err("can't open file %s, perhaps you haven't mount usbfs?", buf);
        goto fail;
    }
	*/

    ///* DEV_TEST
	info("\nconnect device application...");
    edev->tmp_sockfd = server_listen_accept();
	info("edev->tmp_sockfd : %d", edev->tmp_sockfd);
	//*/

    edev->usbfs_fd = fd;
    edev->client_fd = -1;
    dbg("opened %s\n", buf);
    if (usb_parse_desc(edev, 1))
	goto fail;
	/*//DEV_TEST
    show_eps(edev);
    if (usb_host_claim_interfaces(edev))
        goto fail;
	*/
#if 0
    ret = ioctl(fd, USBDEVFS_CONNECTINFO, &ci);
    if (ret < 0) {
        err("usb_host_device_open: USBDEVFS_CONNECTINFO");
        goto fail;
    }
    dbg("husb: grabbed usb device %d.%d\n", bus_num, addr);

    ret = usb_linux_update_endp_table(dev);
    if (ret)
        goto fail;
#endif
    return 0;
fail:
    if(fd>=0)
	close(fd);
    return -1;
}

static struct usbip_exported_device *usbip_exported_device_new(char *sdevpath)
{
	struct usbip_exported_device *edev = NULL;
	int i;

	edev = (struct usbip_exported_device *) calloc(1, sizeof(*edev));
	if (!edev) {
		err("alloc device");
		return NULL;
	}

	// DEV_TEST
	// edev->sudev = sysfs_open_device_path(sdevpath);
	edev->sudev = (struct sysfs_device*)malloc(sizeof(struct sysfs_device));
	if(!edev->sudev)	{
		err("alloc sudev");
		goto err;
	}
	strcpy(edev->sudev->name, "2-1.2");
	strcpy(edev->sudev->path, sdevpath);
	strcpy(edev->sudev->bus_id, "2-1.2");
	strcpy(edev->sudev->driver_name, "usb");
	strcpy(edev->sudev->subsystem, "usb");

	info("edev->sudev success");

	for(i=0;i<2;i++){
		edev->eps[i]=calloc(sizeof(*edev->eps[i]),
				USB_ENDPOINT_NUMBER_MASK+1);
		if(!edev->eps[i]){
			err("alloc eps");
			goto err;
		}
	}
	
	//DEV_TEST
	//read_usb_device(edev->sudev, &edev->udev);
	strcpy(edev->udev.path, edev->sudev->path);
	strcpy(edev->udev.busid, edev->sudev->bus_id);
	edev->udev.busnum = 0x00000002;
	edev->udev.devnum = 0x00000006;
	edev->udev.speed = 0x00000001;
	edev->udev.idVendor = 0x0461;
	edev->udev.idProduct = 0x4d16;
	edev->udev.bcdDevice = 0x0200;
	edev->udev.bDeviceClass = 0x00;
	edev->udev.bDeviceSubClass = 0x00;
	edev->udev.bDeviceProtocol = 0x00;
	edev->udev.bConfigurationValue = 0x01;
	edev->udev.bNumConfigurations = 0x01;
	edev->udev.bNumInterfaces = 0x01;

	edev->status = SDEV_ST_AVAILABLE;

	/* reallocate buffer to include usb interface data */
	size_t size = sizeof(*edev) + edev->udev.bNumInterfaces * sizeof(struct usb_interface);
	edev = (struct usbip_exported_device *) realloc(edev, size);
	if (!edev) {
		err("alloc device");
		goto err;
	}

	// DEV_TEST
	/*
	for (int i=0; i < edev->udev.bNumInterfaces; i++)
		read_usb_interface(&edev->udev, i, &edev->uinf[i]);
	*/
	edev->uinf[0].bInterfaceClass = 0x03;
	edev->uinf[0].bInterfaceSubClass = 0x01;
	edev->uinf[0].bInterfaceProtocol = 0x02;
	edev->uinf[0].padding = 0x00;

/*
	info("#####exported_device_new");
	info("edev->sudev->name : %s", edev->sudev->name);
	info("edev->sudev->path : %s", edev->sudev->path);
	info("edev->sudev->bus_id : %s", edev->sudev->bus_id);
	info("edev->sudev->bus : %s", edev->sudev->bus);
	info("edev->sudev->driver_name : %s", edev->sudev->driver_name);
	info("edev->sudev->subsystem : %s", edev->sudev->subsystem);
	info("edev->sudev->attrlist : %p", edev->sudev->attrlist);
	info("edev->sudev->parent : %p", edev->sudev->parent);
	info("edev->sudev->children : %s", edev->sudev->children);
	info("edev->udev.path : %s", edev->udev.path);
	info("edev->udev.busid : %s", edev->udev.busid);
	info("edev->udev.busnum : %08x", edev->udev.busnum);
	info("edev->udev.devnum : %08x", edev->udev.devnum);
	info("edev->udev.speed : %08x", edev->udev.speed);
	info("edev->udev.idVendor : %04x", edev->udev.idVendor);
	info("edev->udev.idProduct : %04x", edev->udev.idProduct);
	info("edev->udev.bcdDevice : %04x", edev->udev.bcdDevice);
	info("edev->udev.bDeviceClass : %02x", edev->udev.bDeviceClass);
	info("edev->udev.bDeviceSubClass : %02x", edev->udev.bDeviceSubClass);
	info("edev->udev.bDeviceProtocol : %02x", edev->udev.bDeviceProtocol);
	info("edev->udev.bConfigurationValue : %02x", edev->udev.bConfigurationValue);
	info("edev->udev.bNumConfigurations : %02x", edev->udev.bNumConfigurations);
	info("edev->udev.bNumInterfaces : %02x", edev->udev.bNumInterfaces);
	for (int i=0; i < edev->udev.bNumInterfaces; i++) {
		info("edev->uinf[%d].bInterfaceClass : %02x", i, edev->uinf[i].bInterfaceClass);
		info("edev->uinf[%d].bInterfaceSubClass : %02x", i, edev->uinf[i].bInterfaceSubClass);
		info("edev->uinf[%d].bInterfaceProtocol : %02x", i, edev->uinf[i].bInterfaceProtocol);
		info("edev->uinf[%d].padding : %02x", i, edev->uinf[i].padding);
*/
	if(claim_dev(edev)){
		err("claim device");
		goto err;
	}

	return edev;

err:
	if (edev && edev->processing_urbs)
		dlist_destroy(edev->processing_urbs);
	if (edev && edev->sudev)
		free(edev->sudev);
		//sysfs_close_device(edev->sudev);
	if (edev && edev->eps[0])
		free(edev->eps[0]);
	if (edev && edev->eps[1])
		free(edev->eps[1]);
	if (edev && edev->desc)
		free(edev->desc);
	return NULL;
}

void unexport_device(struct usbip_exported_device * deleted_edev)
{
	struct usbip_exported_device * edev;
	dlist_for_each_data(stub_driver->edev_list, edev,
			struct usbip_exported_device) {
		if (edev!=deleted_edev)
			continue;
		dlist_delete_before(stub_driver->edev_list);
		dbg("delete edev ok\n");
		stub_driver->ndevs--;
		return;
	}
	err("can't found edev to deleted\n");
}

struct usbip_exported_device * export_device(char *busid)
{
	struct sysfs_device	*sudev;  /* sysfs_device of usb_device */
	struct usbip_exported_device *edev;

	/* sudev=sysfs_open_device("usb", busid);
	if(!sudev){
		err("can't export devce busid %s", busid);
		return NULL;
	}
	*/
	info("##export_device : busid = %s", busid);
	edev = usbip_exported_device_new(busid);
	if (!edev) {
		err("usbip_exported_device new");
		return NULL;
	}
	dbg("export dev edev: %p, usbfs fd: %d\n", edev, edev->usbfs_fd);
	dlist_unshift(stub_driver->edev_list, (void *) edev);
	stub_driver->ndevs++;
	info("%d devices exported", stub_driver->ndevs);
	dbg("%d devices exported\n", stub_driver->ndevs);
	return edev;
}

int usbip_stub_driver_open(void)
{
	stub_driver = (struct usbip_stub_driver *) calloc(1, sizeof(*stub_driver));
	if (!stub_driver) {
		err("alloc stub_driver");
		return -1;
	}

	stub_driver->ndevs = 0;

	stub_driver->edev_list = dlist_new_with_delete(sizeof(struct usbip_exported_device),
			usbip_exported_device_delete);
	if (!stub_driver->edev_list) {
		err("alloc dlist");
		goto err;
	}
#if 0
	ret = refresh_exported_devices();
	if (ret < 0)
		goto err;
#endif
	return 0;


err:
	if (stub_driver->edev_list)
		dlist_destroy(stub_driver->edev_list);
	free(stub_driver);

	stub_driver = NULL;
	return -1;
}


void usbip_stub_driver_close(void)
{
	if (!stub_driver)
		return;

	if (stub_driver->edev_list)
		dlist_destroy(stub_driver->edev_list);
	free(stub_driver);

	stub_driver = NULL;
}

struct usbip_exported_device *usbip_stub_get_device(int num)
{
	struct usbip_exported_device *edev;
	struct dlist		*dlist = stub_driver->edev_list;
	int count = 0;

	dlist_for_each_data(dlist, edev, struct usbip_exported_device) {
		if (num == count)
			return edev;
		else
			count++ ;
	}
	return NULL;
}
