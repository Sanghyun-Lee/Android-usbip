/*
 * $Id$
 *
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#include "usbip_network.h"
#include "utils.h"

#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>

int sock_USBIPAPP;

static const struct option longopts[] = {
	{"usbip",	required_argument,	NULL, 'u'},
	{"other",	required_argument,	NULL, 'o'},
	{"list",	no_argument,		NULL, 'l'},
	{"list2",	no_argument,		NULL, 'L'},
	{"help",	no_argument,		NULL, 'h'},
#if 0
	{"allusbip",	no_argument,		NULL, 'a'},
	{"export-to",   required_argument,	NULL, 'e'},
	{"unexport",    required_argument,	NULL, 'x'},
	{"busid",	required_argument,	NULL, 'b'},
#endif

	{NULL,		0,			NULL,  0}
};

static const char match_busid_path[] = "/sys/bus/usb/drivers/usbip/match_busid";


static void show_help(void)
{
	printf("Usage: usbip_bind_driver [OPTION]\n");
	printf("Change driver binding for USB/IP.\n");
	printf("  --usbip busid        make a device exportable\n");
	printf("  --other busid        use a device by a local driver\n");
	printf("  --list               print usb devices and their drivers\n");
	printf("  --list2              print usb devices and their drivers in parseable mode\n");
	printf("  --allusbip           make all devices exportable\n");

#if 0
	printf("  --export-to host     export the device to 'host'\n");
	printf("  --unexport host      unexport a device previously exported to 'host'\n");
	printf("  --busid busid        the busid used for --export-to\n");
#endif
}

static int modify_match_busid(char *busid, int add)
{
	int fd;
	int ret;
	char buff[BUS_ID_SIZE + 4];

	/* BUS_IS_SIZE includes NULL termination? */
	if (strnlen(busid, BUS_ID_SIZE) > BUS_ID_SIZE - 1) {
		g_warning("too long busid");
		return -1;
	}

	fd = open(match_busid_path, O_WRONLY);
	if (fd < 0)
		return -1;

	if (add)
		snprintf(buff, BUS_ID_SIZE + 4, "add %s", busid);
	else
		snprintf(buff, BUS_ID_SIZE + 4, "del %s", busid);

	g_debug("write \"%s\" to %s", buff, match_busid_path);

	ret = write(fd, buff, sizeof(buff));
	if (ret < 0) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static const char unbind_path_format[] = "/sys/bus/usb/devices/%s/driver/unbind";

/* buggy driver may cause dead lock */
static int unbind_interface_busid(char *busid)
{
	char unbind_path[PATH_MAX];
	int fd;
	int ret;

	snprintf(unbind_path, sizeof(unbind_path), unbind_path_format, busid);

	fd = open(unbind_path, O_WRONLY);
	if (fd < 0) {
		g_warning("opening unbind_path failed: %d", fd);
		return -1;
	}

	ret = write(fd, busid, strnlen(busid, BUS_ID_SIZE));
	if (ret < 0) {
		g_warning("write to unbind_path failed: %d", ret);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

/* call at unbound state */
static int bind_to_usbip(char *busid)
{
	int fd;
	struct op_export_request req;
	uint16_t code = OP_REP_EXPORT;
	int ret;

	fd = tcp_connect("127.0.0.1", USBIP_PORT_STRING);
	if(fd < 0){
		err("connect");
		return -1;	
	}
	ret = usbip_send_op_common(fd, OP_REQ_EXPORT, 0);
	if (ret < 0) {
		close(fd);
		err("send op_common");
		return -1;
	}
	snprintf(req.udev.busid, sizeof(req.udev.busid), "%s", busid);
	ret = usbip_send(fd, &req, sizeof(req));
	if(ret!=sizeof(req)){
		close(fd);
		err("send");
		return -1;
	}
	ret = usbip_recv_op_common(fd, &code);
	close(fd);
	if(ret<0){
		err("recv reply");
		return -1;
	}
	return 0;
}


static int use_device_by_usbip(char *busid)
{
	int ret;

	ret = bind_to_usbip(busid);
	if (ret < 0) {
		errorToApp(sock_USBIPAPP, "bind usbip, failed");
		return -1;
	}

	printToApp(sock_USBIPAPP, "bind to usbip, complete!");

	return 0;
}



static int use_device_by_other(char *busid)
{
	int ret;
	int config;

	/* read and write the same config value to kick probing */
	config = read_bConfigurationValue(busid);
	if (config < 0) {
		g_warning("read bConfigurationValue of %s, failed", busid);
		return -1;
	}

	ret = modify_match_busid(busid, 0);
	if (ret < 0) {
		g_warning("del %s to match_busid, failed", busid);
		return -1;
	}

	ret = write_bConfigurationValue(busid, config);
	if (ret < 0) {
		g_warning("read bConfigurationValue of %s, failed", busid);
		return -1;
	}

	g_message("bind %s to other drivers than usbip, complete!", busid);

	return 0;
}


#include <sys/types.h>
#include <regex.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>



static int is_usb_device(char *busid)
{
	int ret;

	regex_t regex;
	regmatch_t pmatch[1];

	ret = regcomp(&regex, "^[0-9]+-[0-9]+(\\.[0-9]+)*$", REG_NOSUB|REG_EXTENDED);
	if (ret < 0)
		g_error("regcomp: %s\n", strerror(errno));

	ret = regexec(&regex, busid, 0, pmatch, 0);
	if (ret)
		return 0;	/* not matched */

	return 1;
}


#include <dirent.h>
static int show_devices(void)
{
	DIR *dir;

	dir = opendir("/sys/bus/usb/devices/");
	if (!dir)
		g_error("opendir: %s", strerror(errno));

	printf("List USB devices\n");
	for (;;) {
		struct dirent *dirent;
		char *busid;

		dirent = readdir(dir);
		if (!dirent)
			break;

		busid = dirent->d_name;

		if (is_usb_device(busid)) {
			char name[100] = {'\0'};
			char driver[100] =  {'\0'};
			int conf, ninf = 0;
			int i;

			conf = read_bConfigurationValue(busid);
			ninf = read_bNumInterfaces(busid);

			getdevicename(busid, name, sizeof(name));

			printf(" - busid %s (%s)\n", busid, name);

			for (i = 0; i < ninf; i++) {
				getdriver(busid, conf, i, driver, sizeof(driver));
				printf("         %s:%d.%d -> %s\n", busid, conf, i, driver);
			}
			printf("\n");
		}
	}

	closedir(dir);

	return 0;
}

static int show_devices2(void)
{
	DIR *dir;

	dir = opendir("/sys/bus/usb/devices/");
	if (!dir)
		g_error("opendir: %s", strerror(errno));

	for (;;) {
		struct dirent *dirent;
		char *busid;

		dirent = readdir(dir);
		if (!dirent)
			break;

		busid = dirent->d_name;

		if (is_usb_device(busid)) {
			char name[100] = {'\0'};
			char driver[100] =  {'\0'};
			int conf, ninf = 0;
			int i;

			conf = read_bConfigurationValue(busid);
			ninf = read_bNumInterfaces(busid);

			getdevicename(busid, name, sizeof(name));

			printf("busid=%s#usbid=%s#", busid, name);

			for (i = 0; i < ninf; i++) {
				getdriver(busid, conf, i, driver, sizeof(driver));
				printf("%s:%d.%d=%s#", busid, conf, i, driver);
			}
			printf("\n");
		}
	}

	closedir(dir);

	return 0;
}


#if 0
static int export_to(char *host, char *busid) {

	int ret;

	if( host == NULL ) {
		printf( "no host given\n\n");
		show_help();
		return -1;
	}
	if( busid == NULL ) {
		/* XXX print device list and ask for busnumber, if none is
		 * given */
		printf( "no busid given, use --busid switch\n\n");
		show_help();
		return -1;
	}


	ret = use_device_by_usbip(busid);
	if( ret != 0 ) {
		printf( "could not bind driver to usbip\n");
		return -1;
	}

	printf( "DEBUG: exporting device '%s' to '%s'\n", busid, host );
	ret = export_busid_to_host(host, busid); /* usbip_export.[ch] */
	if( ret != 0 ) {
		printf( "could not export device to host\n" );
		printf( "   host: %s, device: %s\n", host, busid );
		use_device_by_other(busid);
		return -1;
	}

	return 0;
}

static int unexport_from(char *host, char *busid) {

	int ret;

	if (!host || !busid)
		g_error("no host or no busid\n");

	g_message("unexport_from: host: '%s', busid: '%s'", host, busid);

	ret = unexport_busid_from_host(host, busid); /* usbip_export.[ch] */
	if( ret != 0 ) {
		err( "could not unexport device from host\n" );
		err( "   host: %s, device: %s\n", host, busid );
	}

	ret = use_device_by_other(busid);
	if (ret < 0)
		g_error("could not unbind device from usbip\n");

	return 0;
}


static int allusbip(void)
{
	DIR *dir;

	dir = opendir("/sys/bus/usb/devices/");
	if (!dir)
		g_error("opendir: %s", strerror(errno));

	for (;;) {
		struct dirent *dirent;
		char *busid;

		dirent = readdir(dir);
		if (!dirent)
			break;

		busid = dirent->d_name;

		if (!is_usb_device(busid))
			continue;

		{
			char name[PATH_MAX];
			int conf, ninf = 0;
			int i;
			int be_local = 0;

			conf = read_bConfigurationValue(busid);
			ninf = read_bNumInterfaces(busid);

			getdevicename(busid, name, sizeof(name));

			for (i = 0; i < ninf; i++) {
				char driver[PATH_MAX];

				getdriver(busid, conf, i, driver, sizeof(driver));
#if 0
				if (strncmp(driver, "usbhid", 6) == 0 || strncmp(driver, "usb-storage", 11) == 0) {
					be_local = 1;
					break;
				}
#endif
			}
			
			if (be_local == 0)
				use_device_by_usbip(busid);
		}
	}

	closedir(dir);

	return 0;
}
#endif

int main(int argc, char **argv)
{
	char *busid = NULL;
	char *remote_host = NULL;

	enum {
		cmd_unknown = 0,
		cmd_use_by_usbip,
		cmd_use_by_other,
		cmd_list,
		cmd_list2,
		cmd_allusbip,
		cmd_export_to,
		cmd_unexport,
		cmd_help,
	} cmd = cmd_unknown;

	sock_USBIPAPP = connect_to_usbipApp();
	if(sock_USBIPAPP<0) {
		fprintf(stderr, "USBIPAPP connect error");
		return -1;
	}
	
	if (geteuid() != 0)
		g_warning("running non-root?");

	usbip_use_stderr = 1;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "u:o:hlLae:x:b:", longopts, &index);
		if (c == -1)
			break;

		switch (c) {
			case 'u':
				cmd = cmd_use_by_usbip;
				busid = optarg;
				break;
			case 'o' :
				cmd = cmd_use_by_other;
				busid = optarg;
				break;
			case 'l' :
				cmd = cmd_list;
				break;
			case 'L' :
				cmd = cmd_list2;
				break;
			case 'a' :
				cmd = cmd_allusbip;
				break;
			case 'b':
				busid = optarg;
				break;
			case 'e':
				cmd = cmd_export_to;
				remote_host = optarg;
				break;
			case 'x':
				cmd = cmd_unexport;
				remote_host = optarg;
				break;
			case 'h': /* fallthrough */
			case '?':
				cmd = cmd_help;
				break;
			default:
				g_error("getopt");
		}

		//if (cmd)
		//	break;
	}

	switch (cmd) {
		case cmd_use_by_usbip:
			use_device_by_usbip(busid);
			break;
		case cmd_use_by_other:
			use_device_by_other(busid);
			break;
		case cmd_list:
			show_devices();
			break;
		case cmd_list2:
			show_devices2();
			break;
#if 0
		case cmd_allusbip:
			allusbip();
			break;
		case cmd_export_to:
			export_to(remote_host, busid);
			break;
		case cmd_unexport:
			unexport_from(remote_host, busid);
			break;
#endif
		case cmd_help: /* fallthrough */
		case cmd_unknown:
			show_help();
			break;
		default:
			g_error("NOT REACHED");
	}

	return 0;
}
