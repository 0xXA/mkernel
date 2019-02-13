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
	if (!isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info(" Intrupted");
	exit(EXIT_FAILURE);
}

void sigsegv(int NUM) {
	if (!isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info(" Malformed Manifest");
	exit(EXIT_FAILURE);
}

void sigabrt(int NUM) {
	if (!isexist(BUILD_DIR)) exec("bash -c \"rm -rf %s\"",BUILD_DIR);
	info("Aborting...");
	exit(EXIT_FAILURE);
}

void pr(char type, const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        va_end(args);
	if (type == 'e') {
		vfprintf(stderr, msg, args);
	        abort();
	} else {
		vfprintf(stdout, msg, args);
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

void exec(const char *command, ...) {

	char *buf;
        va_list args;
        va_start(args, command);
	int sbuf=vsnprintf(NULL, 0, command, args)+1;
	buf=(char *)malloc(sbuf);
	vsnprintf(buf,sbuf,command,args);
	if (!buf) err("no memory available currently");
	if (system(buf)) err("failed to execute:\n%s", buf);
	free(buf);
        va_end(args);
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
                exec("bash -c \"make -j%d O=%s &>> %s.log\"", NRCPU, BUILD_DIR,
                     DEVICE_CODENAME);
        } else {
                exec("bash -c \"make -j%d O=%d %s\"", NRCPU, BUILD_DIR,
                     DEVICE_CONFIG);
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
        while ((opt = getopt(argc, argv, ":dfm:vu")) != -1) {
                switch (opt) {
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
