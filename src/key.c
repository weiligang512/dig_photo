//#include <curses.h>
#include <stdio.h>
#include <string.h>

#include "key.h"

char *
key_get (char *key, size_t lenth)
{
  int i;

//  initscr ();
//  cbreak ();
//  noecho ();
  if (lenth < 3)
    {
      return NULL;
    }

  memset (key, '\0', lenth);

  for (i = 0; i < 3; i++)
    {
      key[i] = getchar ();
      // if (key[0] > '!' && key[0] < '~')
      if (key[0] != 27)
	{
	  break;
	}
    }
//  endwin ();

  return key;
}
