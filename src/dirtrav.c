#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

#include "dirtrav.h"

int
dir_traversal (const char *dirname, char cont[MAX_FILENUM][256])
{
  DIR *dir;
  int i = 0;
  struct dirent *dirent = NULL;

  if ((dir = opendir (dirname)) == NULL)
    {
      fprintf (stderr, "open a directory %s failed: %s\n", dirname,
	       strerror (errno));
      return -1;
    }

  while ((dirent = readdir (dir)))
    {
      //cont[i]->d_type = dirent->d_type;
      strcpy (cont[i], dirname);
      strcat (cont[i], dirent->d_name);
      i++;
      if (i >= MAX_FILENUM)
	{
	  return i;
	}
    }

  return i;
}

int
pars_name (char dirent[MAX_FILENUM][256], struct name *name, int num)
{
  int i, j = 0;

  for (i = 0; i < num; i++)
    {
      //      if (dirent[i]->d_type == 8)
      //      {
      if ((strstr (dirent[i], ".jpg\0")) != NULL)
	{
	  name->argv[j] = dirent[i];
	  j++;
	}
      //      }
    }
  name->argv[j] = NULL;
  name->num = j;
  if (j == 0)
    return -1;

  return 0;
}

int
jpeg_name (struct name *name, char *dirname)
{
  char dirent[MAX_FILENUM][256];
  int num;

  num = dir_traversal (dirname, dirent);

  if (pars_name (dirent, name, num) == -1)
    return -1;

  return 0;
}
