#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "digphoto.h"
#include "frabuff.h"
#include "filesrc.h"
#include "mount.h"
#include "key.h"

int global_quit = 0;
int scan = 0;

int next_music = FALSE;
sem_t usb_sem;
sem_t key_sem;
pthread_rwlock_t key_rwlock;
FB *fb_arg;
LC fb_lc;
CR fb_cr;
struct jpeg_info jinfo;
struct usb usb[4];
char key[4] = { 0 };

void
sighandler (int sig)
{
#if defined DEBUG
  printf ("pid = %d, signal = %d\n", getpid (), sig);
#endif
  global_quit++;
}

void
sigchild (int sig)
{
  int statu;
#if defined DEBUG
  printf ("pid = %d, signal = %d\n", getpid (), sig);
#endif
  wait (&statu);
#if defined DEBUG
  printf ("statu = %#08x\n", statu);
#endif
  if (statu == 0)
    next_music = TRUE;
}

int
main (int argc, char **argv)
{
  pthread_t usb_p, disp_p, key_p;
  int *disp_statu, *key_statu;
  pthread_attr_t usb_attr;
  int res, music_run = FALSE;
  int count = 0, i = 0, j;
  char path[MAX_FILENAME][256];
  pid_t music_pid;
  int fd;

  signal (SIGCHLD, sighandler);
  pthread_attr_init (&usb_attr);
  sem_init (&usb_sem, 0, 1);
  pthread_rwlock_init (&key_rwlock, NULL);

  signal (SIGINT, sighandler);
  signal (SIGQUIT, sighandler);
  signal (SIGCHLD, sigchild);

  fd = open ("/dev/null", O_RDWR);
  dup2 (fd, 2);
  pthread_attr_setdetachstate (&usb_attr, PTHREAD_CREATE_DETACHED);
  res = pthread_create (&usb_p, &usb_attr, mount_pthread, (void *) 0);
  if (res != 0)
    {
#if defined DEBUG
      fprintf (stdout, "create pthread failed: %s\n", strerror (res));
#endif
      exit (1);
    }

  res = pthread_create (&disp_p, NULL, disp_pthread, (void *) 0);
  if (res != 0)
    {
#if defined DEBUG
      fprintf (stdout, "create pthread failed: %s\n", strerror (res));
#endif
      exit (1);
    }

  res = pthread_create (&key_p, NULL, key_pthread, (void *) 0);
  if (res != 0)
    {
#if defined DEBUG
      fprintf (stdout, "create pthread failed: %s\n", strerror (res));
#endif
      exit (1);
    }

  while (1)
    {
      if (!music_run)
	{
	  music_run = TRUE;

	  sleep (1);
	  for (j = 1; j < 4; j++)
	    {
	      sem_wait (&usb_sem);
	      if (usb[j].us_flag)
		{
		  dir_traversal (usb[j].us_path, ".mp3", path, &count);
		}
	      sem_post (&usb_sem);
	    }

	  if ((music_pid = fork ()) < 0)
	    {
	      perror ("fork");
	      exit (1);
	    }
	  else if (music_pid == 0)
	    {
	      setsid ();
	      execlp ("madplay", "madplay", path[i], NULL);
	      exit (0);
	    }
	}


      if (key[0] == 'q' || key[0] == 'Q')
	{
	  global_quit++;
	  break;
	}

      if (key[0] == 's')
	{
	  scan = FALSE;
	}

      if (key[0] == 'n')
	{
	  i++;
	  if (i >= count)
	    {
	      i = 0;
	    }
	  kill (music_pid, 9);
	  music_run = FALSE;
	  next_music = FALSE;
	}

      if (next_music > 0)
	{
	  next_music = FALSE;
	  i++;
	  if (i >= count)
	    {
	      i = 0;
	    }
	  music_run = FALSE;
	}

      if (key[0] == 'b')
	{
	  i--;
	  if (i < 0)
	    {
	      i = count - 1;
	    }
	  kill (music_pid, 9);
	  music_run = FALSE;
	  next_music = FALSE;
	}

      usleep (8888);
    }

  kill (music_pid, 9);
  // pthread_cancel (music_p);
  pthread_join (disp_p, (void **) &disp_statu);
  pthread_join (key_p, (void **) &key_statu);

  sem_destroy (&usb_sem);
  pthread_rwlock_destroy (&key_rwlock);
  pthread_attr_destroy (&usb_attr);
  usb_umount (usb);
  munmap_fb (fb_arg);
  free (fb_arg);
  free (jinfo.jp_buff);

  pthread_exit ((void *) 0);
}

void *
mount_pthread (void *arg)
{
  usb_init (usb);
  usb_umount (usb);

  for (; !global_quit;)
    {
      sem_wait (&usb_sem);
      usb_mount (usb);
      sem_post (&usb_sem);

      sleep (1);
    }

  pthread_exit ((void *) 1);
}

void *
disp_pthread (void *arg)
{
  char path[MAX_FILENAME][256];
  char key_d[4];
  LC lc;
  int count = 0;
  int i, j, z;
  int run = 0, stop = 0;

  pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  if ((fb_arg = (FB *) malloc (sizeof (FB))) == NULL)
    {
      fprintf (stderr, "malloc failed :%s\n", strerror (errno));
      exit (1);
    }

  frambuff_init (fb_arg);

#if defined DEBUG
  printf ("fb:%dx%d/%dbits\n", fb_arg->vinfo.xres, fb_arg->vinfo.yres,
	  fb_arg->vinfo.bits_per_pixel);
#endif

  if ((jinfo.jp_buff =
       (unsigned char *) malloc (fb_arg->vinfo.xres * fb_arg->vinfo.yres *
				 fb_arg->vinfo.bits_per_pixel / 8)) == NULL)
    {
      fprintf (stderr, "malloc failed :%s\n", strerror (errno));
      exit (1);
    }

  for (; !global_quit;)
    {
      for (j = 1; j < 4; j++)
	{
	  sem_wait (&usb_sem);
	  if (usb[j].us_flag)
	    {
	      dir_traversal (usb[j].us_path, ".jpg", path, &count);
	    }
	  sem_post (&usb_sem);

	  for (i = 0; !global_quit && i < count;)
	    {
	      if (!run && !scan)
		{
		  memset (jinfo.jp_buff, 0, fb_arg->vinfo.xres *
			  fb_arg->vinfo.yres * fb_arg->vinfo.bits_per_pixel /
			  8);
		  for (z = 0; (z + (i / 12) * 12 < count && z < 12); z++)
		    {
		      lc.lc_x = 80 + z % 4 * 160;
		      lc.lc_y = 80 + z / 4 * 160;
		      lc.lenth = 600 / 6;
		      lc.wide = 800 / 6;
		      lc.flag = 1;
		      if (z == i % 12)
			{
			  lc.num = 1;
			}
		      else
			{
			  lc.num = 0;
			}
		      get_jpeg_l (path[z + (i / 12) * 12], fb_arg, &jinfo,
				  &lc);
		    }
		  update (fb_arg, &jinfo);
		}
	      pthread_rwlock_rdlock (&key_rwlock);
	      if (memcmp (key, key_d, sizeof (key)) != 0)
		memcpy (key_d, key, sizeof (key_d));
	      pthread_rwlock_unlock (&key_rwlock);
	      if (key_d[0] == 27 && key_d[1] == 91)
		{
		  switch (key_d[2])
		    {
		    case 'A':
		      i -= 4;
		      break;
		    case 'B':
		      i += 4;
		      break;
		    case 'C':
		      i++;
		      break;
		    case 'D':
		      i--;
		      break;
		    default:
		      break;
		    }
		  if (i < 0)
		    {
		      i = count - 1;
		    }
		  if (i >= count)
		    {
		      i = 0;
		    }
		}
	      else
		{
		  switch (key_d[0])
		    {
		    case '\n':
		      run = TRUE;
		      break;
		    case 'm':
		      {
			run = FALSE;
			scan = TRUE;
			stop = FALSE;
		      }
		      break;
		    case 's':
		      {
			run = FALSE;
			scan = FALSE;
		      }
		      break;
		    default:
		      break;

		    }
		}

	      if (run && ((key_d[2] >= 'A' && key_d[2] <= 'D') || !stop))
		{
		  stop = TRUE;
		  memset (jinfo.jp_buff, 0, fb_arg->vinfo.xres *
			  fb_arg->vinfo.yres * fb_arg->vinfo.bits_per_pixel /
			  8);
		  ltobig (path[i], fb_arg, &jinfo);

		}

	      if (!run && stop)
		{
		  stop = FALSE;
		  btolit (path[i], fb_arg, &jinfo);
		}

	      for (; scan && !global_quit;)
		{
		  memset (jinfo.jp_buff, 0, fb_arg->vinfo.xres *
			  fb_arg->vinfo.yres * fb_arg->vinfo.bits_per_pixel /
			  8);
		  if (i % 3 == 0)
		    {
		      ltobig (path[i], fb_arg, &jinfo);

		    }
		  else if (i % 3 == 1)
		    {
		      get_jpeg (path[i], fb_arg, &jinfo);
		      uptodown (fb_arg, &fb_lc, &jinfo, 1000);
		      //linetoline (fb_arg, &fb_lc, &jinfo, 1000);
		    }
		  else
		    {
		      get_jpeg (path[i], fb_arg, &jinfo);
		      downtoup (fb_arg, &fb_lc, &jinfo, 1000);
		    }

		  i++;

		  if (i < 0)
		    {
		      i = count - 1;
		    }
		  if (i >= count)
		    {
		      i = 0;
		    }
		  sleep (2);
		}
	    }
	}
    }

  pthread_exit ((void *) 2);
}


void *
key_pthread (void *arg)
{
  for (; !global_quit;)
    {
      pthread_rwlock_wrlock (&key_rwlock);
      key_get (key, sizeof (key));
      pthread_rwlock_unlock (&key_rwlock);
      usleep (88888);
    }

  pthread_exit ((void *) 3);
}
