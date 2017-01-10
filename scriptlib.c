#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

const char rootpart[] = "/dev/hda2";
const char swappart[] = "/dev/hda3";
const char homepart[] = "/dev/hda4";
const char rtfstype[] = "ext2";

pid_t spawnprocess(const char *argv[]) {
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
		return childpid;
	}
}

pid_t simplecmd(const char *cmd, const char *prm) {
	const char *argv[3];
	argv[0] = cmd;
	argv[1] = prm;
	argv[2] = (char *) NULL;
	return spawnprocess(argv);
}

pid_t vsimplecmd(const char * cmd) {
	const char *argv[2];
	argv[0] = cmd;
	argv[1] = (char *) NULL;
	return spawnprocess(argv);
}

