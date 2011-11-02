#ifndef __FRAMEBUFF__
#define __FRAMEBUFF__

#define FBPATH "/dev/fb0"

typedef struct {
	int fb_fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long fb_size;
	unsigned char *fb_p;
} FB;

typedef struct {
	unsigned long lc_x;
	unsigned long lc_y;
	unsigned long lenth;
	unsigned long wide;
	int 		num;
	int 		flag;
} LC;

typedef union {
	unsigned long cr_rgbo;
	struct {
		unsigned char cr_r;
		unsigned char cr_g;
		unsigned char cr_b;
		unsigned char cr_o;
	} rgbo;
} CR;

struct jpeg_info{
	unsigned char *jp_buff;
	long jp_wide;
	long jp_high;
};

extern int open_fb(FB *arg);

extern int ioctl_fb(FB *arg);

extern int mmap_fb(FB *arg);

extern int munmap_fb(FB *arg);

extern void frambuff_init(FB *arg);

extern int disp_fb(FB *arg, LC *lc, CR *cr);

extern int disp_line(FB *arg, LC *lc, JPEG *jpeg);

extern int clean_screen(int data, FB *arg);

extern int uptodown_disp(FB *fb_arg, char *filename);

extern int get_jpeg(char *filename, FB *arg, struct jpeg_info *jinfo);

extern int uptodown(FB *arg, LC *lc, struct jpeg_info *jinfo, long time);

extern int linetoline (FB * arg, LC * lc, struct jpeg_info *jinfo, long time);

extern int update(FB * arg, struct jpeg_info *jinfo);

extern int downtoup(FB *arg, LC *lc, struct jpeg_info *jinfo, long time);

extern int get_jpeg(char *filename, FB *arg, struct jpeg_info *jinfo);

extern int get_jpeg_l(char *filename, FB *arg, struct jpeg_info *jinfo, LC *lc);

extern int get_jpeg_b(char *filename, FB *arg, struct jpeg_info *jinfo, LC *lc);

extern int ltobig (char *filename, FB * arg, struct jpeg_info *jinfo);

extern int btolit (char *filename, FB * arg, struct jpeg_info *jinfo);
#endif
