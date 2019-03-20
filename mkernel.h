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

bool force_rebuild = false;
int nrcpu = 8;
char *mkernel_version = "v1.0";
char *build_dir=NULL;
char *device_codename=NULL;
char *device_config=NULL;
char *manifest="manifest";
//char *mkernel_toolchain_dir=NULL;
char *manifest_dir=NULL;
void pr(char, const char*, ...);
void sigint(int);
void sigsegv(int);
void sigabrt(int);
int isexist(char*);
void exec(const char*, ...);
void set_env(void);
void clean(void);
void compile(void);

#endif
