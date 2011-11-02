#ifndef __FILESRC__
#define __FILESRC__


#define MAX_FILENUM 2048
#define MAX_FILENAME 255


char *lwtocap(char *str, char *lw);
int dir_traversal(const char *dirname, char *ext, char pathname[MAX_FILENUM][256], int *count);


#endif
