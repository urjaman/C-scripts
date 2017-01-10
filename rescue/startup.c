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



void sysctl(char *file,char *data) {
	int filedes;
	char path[62] = "/proc/sys/";
	strncat(path,file,50);
	filedes = open(path,O_WRONLY);
	if(filedes == -1) exit(2);
	write(filedes,data,strlen(data));
	close(filedes);
	}

pid_t sysctls(char *nameofme) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		sysctl("kernel/hostname",nameofme);
		sysctl("vm/bdflush","2 500 0 0 500 100 2 0 0");
		sysctl("vm/max-readahead","8");
		sysctl("vm/min-readahead","1");
		sysctl("vm/pagetable_cache","0 0");
		sysctl("vm/vm_anon_lru","1");
		sysctl("vm/overcommit_memory","1");
		sysctl("vm/kswapd","1024 64 16");
		sysctl("fs/file-max","2048");
		sysctl("kernel/panic","20");
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

pid_t e2fsck(char *device) {
	char *argv[4];
	argv[0] = "/sbin/e2fsck";
	argv[1] = "-p";
	argv[2] = device;
	argv[3] = (char *) NULL;
	return spawnprocess(argv);
	}

pid_t spawnmount(char *device,char *mp, char *fstype, int flags,char *data) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		mount(device,mp,fstype,flags,data);
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

pid_t myswapon(void) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		swapon(swappart,0);
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

pid_t ifconfig(void) {
	char *argv[5];
	argv[0] = "/bin/ifconfig";
	argv[1] = "lo";
	argv[2] = "127.0.0.1";
	argv[3] = "up";
	argv[4] = (char *) NULL;
	return spawnprocess(argv);
	}

pid_t route(void) {
	char *argv[8];
	argv[0] = "/bin/route";
	argv[1] = "add";
	argv[2] = "-net";
	argv[3] = "127.0.0.0";
	argv[4] = "netmask";
	argv[5] = "255.0.0.0";
	argv[6] = "lo";
	argv[7] = (char *) NULL;
	return spawnprocess(argv);
	}


void shell(char *tty) {
	char *argv[2];
	argv[0] = "/bin/ash";
	argv[1] = (char *) NULL;
	pid_t childpid = fork();
	if (childpid == -1) return;

	if (childpid == 0) {
		freopen(tty,"w",stdout);
		freopen(tty,"w",stderr);
		freopen(tty,"r",stdin);
		execv(argv[0],argv);
		exit(2);
	}
	}	


int main(void) {
	char nameofme[] = "rescuetux";
	int status;
	char mountrw = 1;

	pid_t pids[4];
	pids[0] = spawnmount("/proc"+1,"/proc","/proc"+1,0,"");
	pids[1] = simplecmd("/sbin/hwclock", "-s");

	waitpid(pids[1],NULL,0); /* e2fsck needs hwclock */
	pids[1] = e2fsck(rootpart);
	pids[2] = ifconfig();
	myswapon();

	if (pids[0]) waitpid(pids[0],NULL,0); /* sysctl (via proc) requires proc */
	sysctls(nameofme);

	waitpid(pids[2],NULL,0); /* route requires ifconfig */
	route();

	shell("/dev/tty5");
	waitpid(pids[1],&status,0); /* wait for e2fsck / (with status rep) */
	if (WIFEXITED(status)) 	if ((WEXITSTATUS(status) & 4) == 4) mountrw = 0;
	if (mountrw) {
		pids[0] = spawnmount(rootpart,"/",rtfstype,MS_REMOUNT,"");
		if (pids[0]) waitpid(pids[0],NULL,0);
		pids[0] = simplecmd("/bin/mount","-a");
		waitpid(pids[0],NULL,0);
 		exit(0);
		}
	printf("FS Error\nMounting tmpfs to /tmp\n");
	pids[0] = spawnmount("tmpfs","/tmp","tmpfs",0,"size=512k");
	if (pids[0]) waitpid(pids[0],NULL,0);
	exit(1);
	
	}
	
		
