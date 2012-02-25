#ifndef __HELPER_H_
#define __HELPER_H_

int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char **argv);

#endif /*__HELPER_H_*/
