#ifndef __DIGPHOTO__
#define __DIGPHOTO__


typedef struct {
	unsigned char *buffer;
	long x;
	long y;
} JPEG;


void *mount_pthread(void *arg);

void *disp_pthread(void *arg);

void *key_pthread(void *arg);

#endif
