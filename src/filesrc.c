#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "filesrc.h"

char *
lwtocap (char *s, char *lw)
{
  char *str;

  str = s;

  while (*str)
    {
      if (*str > 'A' && *str < 'Z')
	{
	  *lw = *str + ('a' - 'A');
	}
      lw++;
      str++;
    }
  *lw = '\0';

  return lw;
}

int
dir_traversal (const char *dirname, char *ext,
	       char pathname[MAX_FILENUM][256], int *count)
{
  DIR *dir;
  struct dirent ent;
  struct dirent *dirent = NULL;
  char path[MAX_FILENAME];

  if ((dir = opendir (dirname)) == NULL)
    {
      fprintf (stderr, "open a directory %s failed: %s\n", dirname,
	       strerror (errno));
      return -1;
    }
  readdir_r (dir, &ent, &dirent);
  while (dirent != NULL)
    {
      if (strcmp (ent.d_name, ".") == 0 || strcmp (ent.d_name, "..") == 0)
	{
	  readdir_r (dir, &ent, &dirent);
	  continue;
	}
      strncpy (path, dirname, MAX_FILENAME);
      if (path[strlen (path) - 1] != '/')
	strncat (path, "/", MAX_FILENAME);
      strncat (path, ent.d_name, MAX_FILENAME);

      struct stat stat;
      if (lstat (path, &stat) < 0)
	{
	  perror ("lstat");
	  return -1;
	}
      else
	{
	  if (S_ISDIR (stat.st_mode))
	    {
	      dir_traversal (path, ext, pathname, count);
	    }
	  if (S_ISLNK (stat.st_mode))
	    {
	      char linkbuff[MAX_FILENAME];

	      readlink (path, linkbuff, MAX_FILENAME - 1);
	      puts (linkbuff);
	      strncpy (path, linkbuff, MAX_FILENAME);
	    }
	  if (S_ISREG (stat.st_mode) || S_ISLNK (stat.st_mode))
	    {
	      if (strcmp (path + strlen (path) - strlen (ext), ext) == 0)
		{
		  strncpy (pathname[*count], path, MAX_FILENAME);
		  (*count)++;
		}
	    }
	}
      readdir_r (dir, &ent, &dirent);
    }

  closedir (dir);

  return 0;

}
