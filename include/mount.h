#ifndef __MOUNT_H__
#define __MOUNT_H__

#define TRUE !0
#define FALSE 0

struct usb{
	char *us_dev;
	char *us_path;
	int  us_flag;
};


extern void usb_init(struct usb *usb);

int usb_creatmntpath(struct usb *usb);

int usb_rmmntpath(struct usb *usb);

extern int usb_mount(struct usb *usb);

extern int usb_umount(struct usb *usb);

#endif
