#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <errno.h>

#include "mount.h"


void
usb_init (struct usb *usb)
{
  memset (usb, 0, sizeof (struct usb) * 4);
  usb[0].us_dev = "/dev/sda1";
  usb[1].us_dev = "/dev/sdb1";
  usb[2].us_dev = "/dev/sdc1";
  usb[3].us_dev = "/dev/sdd1";
  usb[0].us_path = "./usb/usba";
  usb[1].us_path = "./usb/usbb";
  usb[2].us_path = "./usb/usbc";
  usb[3].us_path = "./usb/usbd";
}

int
usb_creatmntpath (struct usb *usb)
{
  int i;
  for (i = 1; i < 4; i++)
    {
      if (mkdir (usb[i].us_path, 0755) < 0)
	{
#if defined DEBUG
	  fprintf (stdout, "%s:%d:create directory failed: %s\n", __FILE__,
		   __LINE__, strerror (errno));
#endif
	}
      else
	{
#if defined DEBUG
	  fprintf (stdout, "create directory success\n");
#endif
	}
    }

  return 0;
}

int
usb_rmmntpath (struct usb *usb)
{
  int i;

  for (i = 1; i < 4; i++)
    {
      if (!usb[i].us_flag || TRUE)
	{
	  if (rmdir (usb[i].us_path) < 0)
	    {
#if defined DEBUG
	      fprintf (stdout, "%s:%d:remove directory failed: %s\n",
		       __FILE__, __LINE__, strerror (errno));
#endif
	    }
	  else
	    {
#if defined DEBUG
	      fprintf (stdout, "remove director success\n");
#endif
	    }
	}
    }

  return 0;
}

int
usb_mount (struct usb *usb)
{
  int i;

  usb_creatmntpath (usb);

  for (i = 1; i < 4; i++)
    {
      if (!usb[i].us_flag)
	{
	  if (mount
	      (usb[i].us_dev, usb[i].us_path, "vfat", MS_SYNCHRONOUS,
	       NULL) < 0)
	    {
	      usb[i].us_flag = FALSE;
#if defined DEBUG
	      fprintf (stdout, "%s:%d:mount failed: %s\n", __FILE__, __LINE__,
		       strerror (errno));
#endif
	    }
	  else
	    {
	      usb[i].us_flag = TRUE;
#if defined DEBUG
	      fprintf (stdout, "mount %s to %s success\n", usb[i].us_dev,
		       usb[i].us_path);
#endif
	    }
	}
#if defined DEBUG
      printf ("%d\n", usb[i].us_flag);
#endif
    }

  for (i = 1; i < 4; i++)
    {
      if (usb[i].us_flag == TRUE)
	return 0;
    }

  return -1;
}

int
usb_umount (struct usb *usb)
{
  int i;

  for (i = 1; i < 4; i++)
    {
      if (usb[i].us_flag || TRUE)
	{
	  if (umount (usb[i].us_path) < 0)
	    {
#if defined DEBUG
	      fprintf (stdout, "%s:%d:umouut failed: %s\n", __FILE__,
		       __LINE__, strerror (errno));
#endif
	    }
	  else
	    {
	      usb[i].us_flag = FALSE;
#if defined DEBUG
	      fprintf (stdout, "umount success\n");
#endif
	    }
	}
#if defined DEBUG
      printf ("%d\n", usb[i].us_flag);
#endif
    }

  usb_rmmntpath (usb);

  return 0;
}

#if 0

int
main (int argc, char **argv)
{
  struct usb usb[4];

  usb_init (usb);

//      usb_mount(usb);
  sleep (10);
  usb_umount (usb);



  return 0;
}
#endif
