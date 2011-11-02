#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>
#include <unistd.h>
#include <jpeglib.h>
#include <jerror.h>

#include "digphoto.h"
#include "frabuff.h"

//open "/dev/fb0"
int
open_fb (FB * arg)
{
  if ((arg->fb_fd = open (FBPATH, O_RDWR)) < 0)
    {
#if defined DEBUG
      fprintf (stderr, "open fb failed: %s\n", strerror (errno));
#endif
      exit (1);
    }

  return 0;
}

//get frame information
int
ioctl_fb (FB * arg)
{
  if (ioctl (arg->fb_fd, FBIOGET_FSCREENINFO, &arg->finfo) < 0)
    {
#if defined DEBUG
      fprintf (stderr, " Reading fixed information failed :%s\n",
	       strerror (errno));
#endif
      exit (2);
    }

  if (ioctl (arg->fb_fd, FBIOGET_VSCREENINFO, &arg->vinfo) < 0)
    {
#if defined DEBUG
      fprintf (stderr, " Reading variable information failed :%s\n",
	       strerror (errno));
#endif
      exit (3);
    }

  return 0;
}

//map framebuff into memory
int
mmap_fb (FB * arg)
{
  arg->fb_size =
    arg->vinfo.xres * arg->vinfo.yres * arg->vinfo.bits_per_pixel / 8;

  if ((arg->fb_p = (unsigned char *) mmap (NULL, arg->fb_size,
					   PROT_READ | PROT_WRITE, MAP_SHARED,
					   arg->fb_fd, 0)) == (void *) -1)
    {
#if defined DEBUG
      fprintf (stderr, "map devices FBPATH failed :%s\n", strerror (errno));
#endif
      exit (4);
    }

  return 0;
}

//unmap framebuff into memory
int
munmap_fb (FB * arg)
{
  munmap (arg->fb_p, arg->fb_size);
  close (arg->fb_fd);

  return 0;
}

void
frambuff_init (FB * arg)
{
  open_fb (arg);
  ioctl_fb (arg);
  mmap_fb (arg);
}

//draw a line or spuare
int
disp_fb (FB * arg, LC * lc, CR * cr)
{
  int x, y;

  unsigned char *p = arg->fb_p;

  for (y = 0; y < lc->lenth; y++)
    {
      for (x = 0; x < lc->wide; x++)
	{
	  *(p + arg->vinfo.bits_per_pixel / 8 * x +
	    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * y +
	    lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	    lc->lc_x * arg->vinfo.bits_per_pixel / 8 + 0) = cr->rgbo.cr_b;
	  *(p + arg->vinfo.bits_per_pixel / 8 * x +
	    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * y +
	    lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	    lc->lc_x * arg->vinfo.bits_per_pixel / 8 + 1) = cr->rgbo.cr_g;
	  *(p + arg->vinfo.bits_per_pixel / 8 * x +
	    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * y +
	    lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	    lc->lc_x * arg->vinfo.bits_per_pixel / 8 + 2) = cr->rgbo.cr_r;
	  *(p + arg->vinfo.bits_per_pixel / 8 * x +
	    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * y +
	    lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	    lc->lc_x * arg->vinfo.bits_per_pixel / 8 + 3) = cr->rgbo.cr_o;
	}
    }

  return 0;
}

//jpeg shows the location specified lone
int
disp_line (FB * arg, LC * lc, JPEG * jpeg)
{
  long i, j;
  unsigned char *p = arg->fb_p;

  for (i = 0; i < jpeg->x; i++)
    {
      for (j = 0; j < 3; j++)
	{
	  *(p + arg->vinfo.bits_per_pixel / 8 * i +
	    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * jpeg->y +
	    lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	    lc->lc_x * arg->vinfo.bits_per_pixel / 8 + j) =
	    *(jpeg->buffer + i * 3 + 2 - j);
	}

      *(p + arg->vinfo.bits_per_pixel / 8 * i +
	arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * jpeg->y +
	lc->lc_y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 +
	lc->lc_x * arg->vinfo.bits_per_pixel / 8 + j) = 0xff;
    }

  return 0;
}

int
uptodown (FB * arg, LC * lc, struct jpeg_info *jinfo, long time)
{
  long y;
  unsigned char *p = arg->fb_p;

  for (y = 0; y < arg->vinfo.yres; y++)
    {
      memcpy (p + y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      jinfo->jp_buff +
	      y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8);
      usleep (time);
    }

  return 0;
}

int
linetoline (FB * arg, LC * lc, struct jpeg_info *jinfo, long time)
{
  long y;
  long x;
  unsigned char *p = arg->fb_p;

  for (y = 0; y < arg->vinfo.yres; y++)
    {
      if (y % (arg->vinfo.yres / 16) == 0)
	{
	  y += (arg->vinfo.yres / 16);
	}
      x = y % (arg->vinfo.yres) + y / (arg->vinfo.yres / 16) * 2;
      memcpy (p + x * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      jinfo->jp_buff +
	      x * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8);
    }
#if 0
  for (y = 0; y < arg->vinfo.yres; y++)
    {
      if (y % (arg->vinfo.yres / 16) == 0)
	{
	  y += (arg->vinfo.yres / 16);
	}
      memcpy (p + y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      jinfo->jp_buff +
	      y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8);
      usleep (time);
    }
#endif

  return 0;
}

int
update (FB * arg, struct jpeg_info *jinfo)
{
  unsigned char *p = arg->fb_p;
  memcpy (p, jinfo->jp_buff, arg->vinfo.xres * arg->vinfo.yres *
	  arg->vinfo.bits_per_pixel / 8);
  return 0;
}

int
downtoup (FB * arg, LC * lc, struct jpeg_info *jinfo, long time)
{
  long y;
  unsigned char *p = arg->fb_p;

  clean_screen (0x00, arg);
  for (y = arg->vinfo.yres - 1; y >= 0; y--)
    {
      memcpy (p + y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      jinfo->jp_buff +
	      y * arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8,
	      arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8);
      usleep (time);
    }

  return 0;
}

int
clean_screen (int data, FB * arg)
{
  int x, y;
  unsigned char *p = arg->fb_p;

  for (y = 0; y < arg->vinfo.yres; y++)
    {
      for (x = 0; x < arg->vinfo.xres; x++)
	{
	  *(int *) (p + arg->vinfo.bits_per_pixel / 8 * x +
		    arg->vinfo.xres * arg->vinfo.bits_per_pixel / 8 * y) =
	    data;
	}
    }

  return 0;
}

int
get_jpeg (char *filename, FB * arg, struct jpeg_info *jinfo)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *infile;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);
  // open jpeg file
  if ((infile = fopen (filename, "rb")) == NULL)
    {
#if defined DEBUG
      perror ("fopen");
#endif
      return -1;
    }

  jpeg_stdio_src (&cinfo, infile);
  jpeg_read_header (&cinfo, TRUE);
  jpeg_start_decompress (&cinfo);

  if ((cinfo.output_width > arg->vinfo.xres)
      || (cinfo.output_height > arg->vinfo.yres))
    {
#if defined DEBUG
      printf ("too large JPEG file,cannot display\n");
#endif
      munmap_fb (arg);
      fclose (infile);
      exit (1);
    }

  unsigned char *buffer =
    (unsigned char *) malloc (cinfo.output_width * cinfo.output_components);

  if (buffer == NULL)
    {
#if defined DEBUG
      perror ("malloc");
#endif
      munmap_fb (arg);
      fclose (infile);
      exit (1);
    }

  jinfo->jp_wide = cinfo.output_width;
  jinfo->jp_high = cinfo.output_height;
  int y = 0;
  while (cinfo.output_scanline < cinfo.output_height)
    {
      unsigned char *p;
      long lx, ly;
      int i, j;

      lx = (arg->vinfo.xres - cinfo.output_width) / 2;
      ly = (arg->vinfo.yres - cinfo.output_height) / 2;

      p = jinfo->jp_buff + arg->vinfo.xres * (y + ly) *
	arg->vinfo.bits_per_pixel / 8;
      jpeg_read_scanlines (&cinfo, &buffer, 1);
      for (i = 0; i < cinfo.output_width; i++)
	{
	  for (j = 0; j < 3; j++)
	    {
	      *(p + (i + lx) * arg->vinfo.bits_per_pixel / 8 + j) =
		*(buffer + i * 3 + 2 - j);
	    }
	  *(p + i * arg->vinfo.bits_per_pixel / 8 + j) = 0x0f;
	}
      y++;
    }

  fclose (infile);
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  free (buffer);

  return 0;
}

int
get_jpeg_l (char *filename, FB * arg, struct jpeg_info *jinfo, LC * lc)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *infile;
  unsigned char *p;
  long lx, ly;
  int i, j, y, z;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);
  // open jpeg file
  if ((infile = fopen (filename, "rb")) == NULL)
    {
      perror ("fopen");
      return -1;
    }

  jpeg_stdio_src (&cinfo, infile);
  jpeg_read_header (&cinfo, TRUE);
  jpeg_start_decompress (&cinfo);

  unsigned char *buffer =
    (unsigned char *) malloc (cinfo.output_width * cinfo.output_components);
  if (buffer == NULL)
    {
#if defined DEBUG
      perror ("malloc");
#endif
      munmap_fb (arg);
      fclose (infile);
      exit (1);
    }

  if (cinfo.output_width > arg->vinfo.xres)
    {
      printf ("too large JPEG file,cannot display\n");
      cinfo.output_height = arg->vinfo.yres;
      //munmap_fb(arg);
      //      fclose(infile);
      //      exit(1);
    }
  if (cinfo.output_width > arg->vinfo.xres)
    {
      printf ("too large JPEG file,cannot display\n");
      cinfo.output_width = arg->vinfo.xres;
      //munmap_fb(arg);
      //      fclose(infile);
      //      exit(1);
    }

  jinfo->jp_wide = cinfo.output_width;
  jinfo->jp_high = cinfo.output_height;

  if (lc->flag == 0)
    {
      lx = (arg->vinfo.xres - lc->wide) / 2;
      ly = (arg->vinfo.yres - lc->lenth) / 2;
    }
  else
    {
      lx = lc->lc_x;
      ly = lc->lc_y;
    }
  y = 0;
  z = 0;
  while (cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines (&cinfo, &buffer, 1);
      if ((y % (cinfo.output_height / lc->lenth) == 0) &&
	  y < (cinfo.output_height / lc->lenth) * lc->lenth)
	{
	  p = jinfo->jp_buff + arg->vinfo.xres * (z + ly) *
	    arg->vinfo.bits_per_pixel / 8;
	  for (i = 0; i < lc->wide; i++)
	    {
	      for (j = 0; j < 3; j++)
		{
		  *(p + (i + lx) * arg->vinfo.bits_per_pixel / 8 + j) =
		    *(buffer + i * 3 * (cinfo.output_width / lc->wide) + 2 -
		      j);
		}
	      *(p + (i + lx) * arg->vinfo.bits_per_pixel / 8 + j) = 0x0f;

	      if (lc->num)
		{
		  if ((z >= 0 && z < 4)
		      || (z > lc->lenth - 5 && z < lc->lenth) || (i >= 0
								  && i < 5)
		      || (i > lc->wide - 4 && i < lc->wide))
		    {
		      *(int *) (p +
				(i + lx) * arg->vinfo.bits_per_pixel / 8) =
			0xaf00f0;
		    }
		}
	    }
	  z++;
	}
      y++;
    }

  fclose (infile);
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  free (buffer);

  return 0;
}

int
get_jpeg_b (char *filename, FB * arg, struct jpeg_info *jinfo, LC * lc)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *infile;
  unsigned char *p;
  long lx, ly;
  int i, j, y, z;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);
  // open jpeg file
  if ((infile = fopen (filename, "rb")) == NULL)
    {
      perror ("fopen");
      //      return -1;
    }

  jpeg_stdio_src (&cinfo, infile);
  jpeg_read_header (&cinfo, TRUE);
  jpeg_start_decompress (&cinfo);

  unsigned char *buffer =
    (unsigned char *) malloc (cinfo.output_width * cinfo.output_components);
  if (cinfo.output_width > arg->vinfo.xres)
    {
      printf ("too large JPEG file,cannot display\n");
      cinfo.output_height = arg->vinfo.yres;
      //munmap_fb(arg);
      //      fclose(infile);
      //      exit(1);
    }
  if (cinfo.output_width > arg->vinfo.xres)
    {
      printf ("too large JPEG file,cannot display\n");
      cinfo.output_width = arg->vinfo.xres;
      //munmap_fb(arg);
      //      fclose(infile);
      //      exit(1);
    }

  if (buffer == NULL)
    {
      perror ("malloc");
      munmap_fb (arg);
      fclose (infile);
      exit (1);
    }

  jinfo->jp_wide = cinfo.output_width;
  jinfo->jp_high = cinfo.output_height;

  if (lc->flag == 0)
    {
      lx = (arg->vinfo.xres - cinfo.output_width / lc->wide) / 2;
      ly = (arg->vinfo.yres - cinfo.output_height / lc->lenth) / 2;
    }
  else
    {
      lx = lc->lc_x;
      ly = lc->lc_y;
    }

  y = 0;
  z = 0;
  while (cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines (&cinfo, &buffer, 1);
      if (y % lc->lenth == 0)
	{
	  p = jinfo->jp_buff + arg->vinfo.xres * (z + ly) *
	    arg->vinfo.bits_per_pixel / 8;
	  for (i = 0; i < cinfo.output_width / lc->wide; i++)
	    {
	      for (j = 0; j < 3; j++)
		{
		  *(p + (i + lx) * arg->vinfo.bits_per_pixel / 8 + j) =
		    *(buffer + i * 3 * lc->wide + 2 - j);
		}
	      *(p + (i + lx) * arg->vinfo.bits_per_pixel / 8 + j) = 0x0f;
	    }
	  z++;
	}
      y++;
    }
  fclose (infile);
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  free (buffer);

  return 0;
}

int
ltobig (char *filename, FB * arg, struct jpeg_info *jinfo)
{
  int i;

  LC lc;
  lc.flag = 0;
  for (i = 12; i > 0; i--)
    {
      lc.wide = lc.lenth = i;
      get_jpeg_b (filename, arg, jinfo, &lc);
      update (arg, jinfo);

      usleep (10 * (12 - i));
    }

  return 0;
}

int
btolit (char *filename, FB * arg, struct jpeg_info *jinfo)
{
  int i;

  LC lc;
  lc.flag = 0;
  for (i = 1; i <= 12; i++)
    {
      memset (jinfo->jp_buff, 0, arg->vinfo.xres *
	      arg->vinfo.yres * arg->vinfo.bits_per_pixel / 8);
      lc.wide = lc.lenth = i;
      get_jpeg_b (filename, arg, jinfo, &lc);
      update (arg, jinfo);

      usleep (1 * (12 - i));
    }

  return 0;
}
