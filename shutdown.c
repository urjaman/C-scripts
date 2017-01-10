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

pid_t myumount(void) {
	const char *argv[4];
	argv[0] = "/bin/umount";
	argv[1] = "-a";
	argv[2] = "-r";
	argv[3] = (char *) NULL;
	return spawnprocess(argv);
	} 

pid_t killall(char force) {
	/* /usr/bin/killall -q -TERM/-KILL(depending on force) pppd syslogd tail X */
	const char *argv[21];
	argv[0] = "/usr/bin/killall";
	argv[1] = "-q";
	switch (force) {
	case 0:
	argv[2] = "-KILL";
	break;
	case 1:
	argv[2] = "-TERM";
	break;
	case 2:
	argv[2] = "-11";
	break;
	}
	argv[3] = "pppd";
	argv[4] = "syslogd";
	argv[5] = "tail";
	argv[6] = "X";
	argv[7] = "ash";
	argv[8] = "sh";
	argv[9] = "bash";
	argv[10] = "klogd";
	argv[11] = "cardmgr";
	argv[12] = "top";
	argv[13] = "ssh";
	argv[14] = "dropbear";
	argv[15] = "nano";
	argv[16] = "xinit";
	argv[17] = "svncviewer";
	argv[18] = "fluxbox";
	argv[19] = "dwm";
	argv[20] = (char *) NULL;
	return spawnprocess(argv);
	}

int main(void) {
	pid_t pids[4];
	pids[0] = simplecmd("/sbin/hwclock","-w");
	waitpid(pids[0],NULL,0);
	pids[2] = myumount();
	pids[3] = killall(0);
	
	waitpid(pids[2],NULL,0);
	waitpid(pids[3],NULL,0);
	sleep(2);
	if ((mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"")) == -1) {
		pids[0] = killall(1);
		waitpid(pids[0],NULL,0);
		sleep(1);
		if ((mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"")) == -1) {
			sync();
			pids[0] = killall(2);
			sleep(2);
			waitpid(pids[0],NULL,0);
			mount(rootpart,"/",rtfstype,(MS_RDONLY|MS_REMOUNT),"");
		}
	}
	sync();
	return 0;
}

	
