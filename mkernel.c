#include <libgen.h>
#include <regex.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mkernel.h"

void sigint(int NUM) {
	if (isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info(" Intrupted");
	exit(EXIT_FAILURE);
}

void sigsegv(int NUM) {
	if (isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info(" Malformed Manifest");
	exit(EXIT_FAILURE);
}

void sigabrt(int NUM) {
	if (isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info("Aborting...");
	exit(EXIT_FAILURE);
}

void pr(char TYPE, const char *MSG, ...) {
        va_list ARGS;
        va_start(ARGS, MSG);
        va_end(ARGS);
	if (TYPE == 'e') {
		vfprintf(stderr, MSG, ARGS);
	        abort();
	} else {
		vfprintf(stdout, MSG, ARGS);
	}
}

int isexist(char *NAME) {
        NAME = realpath(NAME, NULL);
        struct stat NF;
        stat(NAME, &NF);
        int type = NF.st_mode & S_IFMT;
        if (type == S_IFDIR || type == S_IFREG) return 1;
        return 0;
}

void exec(const char *CMD, ...) {

	char *BUFF;
        va_list ARGS;
        va_start(ARGS, CMD);
	int SBUFF=vsnprintf(NULL, 0, CMD, ARGS)+1;
	BUFF=(char *)malloc(SBUFF);
	vsnprintf(BUFF, SBUFF, CMD, ARGS);
	if (!BUFF) err("no memory available currently");
	if (system(BUFF)) err("failed to execute:\n%s", BUFF);
	free(BUFF);
        va_end(ARGS);
}

void set_env(void) {
        FILE *MFILE;
        char LINE[LMAX];
        regex_t REGX;
        size_t REG;

        if (!isexist(MANIFEST)) err("%s doesn't exists", MANIFEST);

        MFILE = fopen(MANIFEST, "r+");

        REG = regcomp(&REGX, "^([a-zA-Z0-9_]*)=", REG_EXTENDED);

        if (!MFILE) err("Not enough memory for %s", MANIFEST);

        while (fgets(LINE, LMAX, MFILE)) {
                LINE[strlen(LINE) - 1] = '\0';
                if (!regexec(&REGX, LINE, 0, NULL, 0)) {
                        setenv(strtok(LINE, "="), strtok(NULL, "="), 1);
                }
        }

        DEVICE_CODENAME = getenv("DEVICE_CODENAME");
        DEVICE_CONFIG = getenv("DEVICE_CONFIG");
	MANIFEST_DIR = dirname(realpath(MANIFEST,NULL));
        if ((DEVICE_CODENAME = getenv("DEVICE_CODENAME"))) {;
                strcpy(BUILD_DIR, getenv("HOME"));
                strcat(BUILD_DIR, "/");
                strcat(BUILD_DIR, DEVICE_CODENAME);
        } else {
                err("DEVICE_CODENAME is null or not defined");
        }

        regfree(&REGX);
        fclose(MFILE);
}

void getkernel(void) {
	exec("find %s/arch/%s/boot -type f -iname '%s' -exec cp {} '%s'",
			BUILD_DIR,getenv("ARCH"),getenv("TARGET_KERNEL_IMG"),MANIFEST_DIR);
}

void compile(void) {
        set_env();

        if (!FORCE_REBUILD) {
                if (isexist(BUILD_DIR)) {
                        err("\n%s exists use\n'mkernel -f' to rebuild",
                            BUILD_DIR);
                }
        } else {
                exec("bash -c \"rm -rf %s\"", BUILD_DIR);
        }

        if (!isexist(BUILD_DIR)) exec("bash -c \"mkdir -p %s\"", BUILD_DIR);

        if (getenv("CREATE_LOGFILE")) {
		exec("bash -c \"make -j%d O=%s %s &>/dev/null\"", NRCPU,
                     BUILD_DIR, DEVICE_CONFIG);
		if (BCMD)
			exec(BCMD);
		else
			exec("bash -c \"make -j%d O=%s &>> %s.log\"", NRCPU, BUILD_DIR,
                     DEVICE_CODENAME);
        } else {
                exec("bash -c \"make -j%d O=%d %s\"", NRCPU, BUILD_DIR,
                     DEVICE_CONFIG);
		if (BCMD)
			exec(BCMD);
		else
			exec("bash -c \"make -j%d O=%s\"", NRCPU, BUILD_DIR);
        }
}

void usage(void) {
        info("\nusage:  mkernel [-d] [-m manifest] [-v] [-u]\n\t-d Suppress the "
            "output but errors may still be "
            "shown.\n\t-m Select arbitrary manifest file.\n\t-v Show version "
            "and "
            "exit.\n\t-u Show usage and exit");
	exit(EXIT_SUCCESS);
}

void version(void) {
        info("\n\t\t   mkernel %s\nCopyright (C) 2019 Yuvraj Saxena "
            "<infectedx27@gmail.com> @TheInfected\nThis is free software see "
            "the source for copying conditions. There is NO warranty not even "
            "for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
            MKERNEL_VERSION);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
        signal(SIGINT, sigint);
        signal(SIGSEGV, sigsegv);
        signal(SIGABRT, sigabrt);

        int opt;
        while ((opt = getopt(argc, argv, ":c:dfm:vu")) != -1) {
                switch (opt) {
			case 'c':
				BCMD = optarg;
				break;
                        case 'd':
                                fclose(stdout);
                                break;
                        case 'm':
                                MANIFEST = optarg;
                                break;
			case 'f':
				FORCE_REBUILD=true;
				break;
                        case 'v':
                                version();
                                break;
                        case 'u':
                                usage();
                                break;
                        case ':':
                                err("option needs a value");
                                break;
                        case '?':
                                err("unknown option: %c\n", optopt);
                                break;
                }
        }

	compile();
        return 0;
}
