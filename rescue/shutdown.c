#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <asm/page.h>
#include <sys/swap.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "scriptlib.h"

pid_t myswapoff(void) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		swapoff(swappart);
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

pid_t myumount(void) {
	char *argv[4];
	argv[0] = "/bin/umount";
	argv[1] = "-a";
	argv[2] = "-r";
	argv[3] = (char *) NULL;
	return spawnprocess(argv);
	} 

pid_t killall(char force) {
	/* /usr/bin/killall -q -TERM/-KILL(depending on force) pppd syslogd tail X */
	char *argv[8];
	argv[0] = "/bin/killall";
	argv[1] = "-q";
	if (force) {
	argv[2] = "-KILL";
	} else {
	argv[2] = "-TERM";
	}
	argv[3] = "pppd";
	argv[4] = "syslogd";
	argv[5] = "tail";
	argv[6] = "X";
	argv[7] = (char *) NULL;
	return spawnprocess(argv);
	}

int main(void) {
	pid_t pids[4];
	pids[0] = simplecmd("/bin/hwclock","-w");
	pids[1] = myswapoff();
	pids[2] = myumount();
	pids[3] = killall(0);
	
	waitpid(pids[0],NULL,0);
	if (pids[1]) waitpid(pids[1],NULL,0);
	waitpid(pids[2],NULL,0);
	waitpid(pids[3],NULL,0);
	
	if ((mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"")) == -1) {
		pids[0] = killall(1);
		waitpid(pids[0],NULL,0);
		sleep(1);
		if ((mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"")) == -1) {
			sync();
			sleep(2);
			mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"");
			}
		}
	sync();
	return 0;
	}

	
