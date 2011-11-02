#ifndef __DIRTRAV__
#define __DIRTRAV__

#include <dirent.h>
#define MAX_FILENUM 1024 

int dir_traversal(const char *dirname, char cont[MAX_FILENUM][256]);

int pars_name(char dirent[MAX_FILENUM][256], struct name *name, int num);

extern int jpeg_name(struct name *name, char *dirname);

#endif
