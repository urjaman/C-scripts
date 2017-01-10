#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

char rootpart[] = "/dev/hda1";
char swappart[] = "/dev/hda3";
char rtfstype[] = "ext2";


pid_t spawnprocess(char *argv[]) {
	pid_t childpid = fork();

	if (childpid == -1) {
		printf("Forking %s failed",argv[0]);
		exit(1);
		}

	if (childpid == 0) {
		execv(argv[0],argv);
		printf("Execving %s failed",argv[0]);
		exit(2);
	} else {
		return childpid; }
	}

pid_t simplecmd(char *cmd, char *prm) {
	char *argv[3];
	argv[0] = cmd;
	argv[1] = prm;
	argv[2] = (char *) NULL;
	return spawnprocess(argv);
	}

