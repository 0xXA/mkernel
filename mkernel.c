#include <libgen.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mkernel.h"

void sigint(int num) {
	if (!isexist(build_dir)) exec("rm -rf %s",build_dir);
	info(" Intrupted");
	exit(EXIT_FAILURE);
}

void sigsegv(int num) {
	if (!isexist(build_dir)) exec("rm -rf %s",build_dir);
	info(" Malformed Manifest");
	exit(EXIT_FAILURE);
}

void sigabrt(int num) {
	if (!isexist(build_dir)) exec("rm -rf %s",build_dir);
	info(" Aborting...");
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

int isexist(char *name) {
	char full_path[200];
        name = realpath(name, full_path);
        struct stat nf;
        stat(name, &nf);
        int type = nf.st_mode & S_IFMT;
        if (type == S_IFDIR || type == S_IFREG) return 1;
        return 0;
}

void exec(const char *cmd, ...) {
	char buff[200];
        va_list args;
        va_start(args, cmd);
	vsprintf(buff, cmd, args);
	if (!buff) err("no memory available currently");
	if (system(buff)) err("failed to execute:\n%s", buff);
        va_end(args);
}

void set_env(void) {
        FILE *mfile=NULL;
        char line[250]="";
	char str[25]="";
	char str1[150]="";

        if (!isexist(manifest)) err("%s doesn't exists", manifest);

        mfile=fopen(manifest, "r+");
	if (!mfile) err("not enough memory for %s", manifest);

        while (fgets(line, 250, mfile)) {
                line[strlen(line) - 1] = '\0';
			if (*line != '#' && sscanf(line, "%[^=]%*[=]%s", str, str1)==2) setenv(str, str1, 1);
        }
	
	memset(line,0,sizeof(line));
	device_codename = getenv("device_codename");
        device_config = getenv("device_config");
	manifest_dir = dirname(realpath(manifest,line));
        
	if ((device_codename = getenv("device_codename"))) {
		if (!build_dir && !(build_dir = getenv("build_dir"))) {
			build_dir=(char*) calloc(strlen(getenv("HOME"))+strlen(device_codename)+11,sizeof(char));
			strcpy(build_dir, getenv("HOME"));
                	strcat(build_dir, "/");
			strcat(build_dir, ".mkernel");
			strcat(build_dir, "/");
                	strcat(build_dir, device_codename);
		}
        } else {
                err("device_codename is null or not defined");
        }

        fclose(mfile);
}

void getkernel(void) {
	exec("find %s/arch/%s/boot -type f -iname '%s' -exec cp {} '%s'",
			build_dir,getenv("ARCH"),getenv("mkernel_target_kernel_img"),manifest_dir);
}

void compile(void) {
        set_env();

        if (!force_rebuild) {
                if (isexist(build_dir)) {
                        err("\n%s exists use\n'mkernel -f' to rebuild",
                            build_dir);
                }
        } else {
                exec("rm -rf %s", build_dir);
        }

       if (!isexist(build_dir)) exec("mkdir -p %s",build_dir);

        if (getenv("cust_cfg_make_cmd") && getenv("cust_img_make_cmd")) {
		exec("%s O=%s", getenv("cust_cfg_make_cmd"), build_dir);
		exec("%s O=%s", getenv("cust_img_make_cmd"), build_dir);
	} else {
		exec("make -j%d O=%s %s", nrcpu, build_dir,
                     device_config);
		exec("make -j%d o=%s", nrcpu, build_dir);
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
            mkernel_version);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
        signal(SIGINT, sigint);
        signal(SIGSEGV, sigsegv);
        signal(SIGABRT, sigabrt);

        int opt;
        while ((opt = getopt(argc, argv, ":fl:m:o:vu")) != -1) {
                switch (opt) {
			case 'f':
				force_rebuild=true;
				break;
			case 'l':
				freopen(optarg, "w", stderr );
				freopen(optarg, "w", stdout );
				break;
			case 'm':
				manifest = optarg;
				break;
			case 'o':
				build_dir = optarg;
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
