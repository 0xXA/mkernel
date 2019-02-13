#ifndef _MKERNEL_H_
#define _MKERNEL_H_

#define info(fmt, ...)                                                 \
        pr('i',                                                        \
           "\033[2K\r\033[1;49m[\033[0m\033[7;33m°\033[0m\033[1;49m]" \
           "\033[7;37m " fmt "\033[0m\n",                               \
           ##__VA_ARGS__)

#define err(fmt, ...)                                                 \
        pr('e',                                                       \
           "\033[2K\r\033[1;49m[\033[0m\033[7;31m!\033[0m\033[1;49m]" \
           "\033[7;41m " fmt "\033[0m\n",                              \
           ##__VA_ARGS__);
#define LMAX 150

bool FORCE_REBUILD = false;
int NRCPU = 8;
char *MKERNEL_VERSION = "v1.0-Beta";
char BUILD_DIR[30];
char *DEVICE_CODENAME;
char *DEVICE_CONFIG;
char *MANIFEST="MANIFEST";
char *MANIFEST_DIR;
char *BCMD;
void pr(char, const char *, ...);
void sigint(int);
void sigsegv(int);
void sigabrt(int);
int isexist(char *NAME);
void exec(const char*, ...);
void set_env(void);
void clean(void);
void compile(void);

#endif
