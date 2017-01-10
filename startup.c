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

#define SYSCTLCOUNT 10
unsigned char table_sysctl[SYSCTLCOUNT*2][32] = {
	"vm/bdflush","2 500 0 0 500 100 2 0 0",
	"vm/max-readahead","8",
	"vm/min-readahead","1",
	"vm/pagetable_cache","0 0",
	"vm/vm_anon_lru","1",
	"vm/kswapd","1024 64 16",
	"fs/file-max","2048",
	"kernel/panic","20",
	"vm/vm_vfs_scan_ratio","10",
	"vm/vm_passes","10" };

unsigned char* modprobe_table[] = {
	"i82365",
	"ds",
	"parport_pc",
	NULL
	};

void sysctl(const char *file,const char *data) {
	int filedes;
	char path[62] = "/proc/sys/";
	strncat(path,file,50);
	filedes = open(path,O_WRONLY);
	if(filedes == -1) exit(2);
	write(filedes,data,strlen(data));
	close(filedes);
	}

pid_t sysctls(const char *nameofme) {
	int x;
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		sysctl("kernel/hostname",nameofme);
		for (x=0;x<SYSCTLCOUNT;x++) 
			sysctl(table_sysctl[x<1],table_sysctl[(x<1|1)]);
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

pid_t e2fsck(const char *device) {
	const char *argv[4];
	argv[0] = "/sbin/e2fsck";
	argv[1] = "-p";
	argv[2] = device;
	argv[3] = (char *) NULL;
	return spawnprocess(argv);
	}

pid_t spawnmount(const char *device,const char *mp, const char *fstype, int flags,const char *data) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		mount(device,mp,fstype,flags,data);
		if (childpid == 0) { exit(0); } else { return 0; }
		}
	return childpid;
	}

void myswapon(void) {
	swapon(swappart,0);
}

pid_t ifconfig(const char * interface, const char *ip) {
	const char *argv[5];
	argv[0] = "/sbin/ifconfig";
	argv[1] = interface;
	argv[2] = ip;
	argv[3] = "up";
	argv[4] = (char *) NULL;
	return spawnprocess(argv);
	}

pid_t ifconfig_lo(void) {
	return ifconfig("lo","127.0.0.1");
	}

pid_t route(void) {
	const char *argv[8];
	argv[0] = "/sbin/route";
	argv[1] = "add";
	argv[2] = "-net";
	argv[3] = "127.0.0.0";
	argv[4] = "netmask";
	argv[5] = "255.0.0.0";
	argv[6] = "lo";
	argv[7] = (char *) NULL;
	return spawnprocess(argv);
	}


pid_t modprobe(void) {
	pid_t childpid = fork();
	if ((childpid == 0)||(childpid == -1)) {
		int i;
		const char *argv[3];
		argv[0] = "/sbin/modprobe";
		argv[2] = NULL;
		for(i=0;;i++) {
			pid_t cpid;
			argv[1] = modprobe_table[i];
			if (!argv[1]) break;
			cpid = spawnprocess(argv);
			if (cpid) waitpid(cpid,NULL,0);
		}
		if (childpid == 0) { exit(0); } else { return 0; }
	}
	return childpid;
}



void shell(char *tty) {
	const char *argv[2];
	argv[0] = "/bin/shell";
	argv[1] = (char *) NULL;
	pid_t childpid = fork();
	if (childpid == -1) return;

	if (childpid == 0) {
		freopen(tty,"w+",stdout);
		freopen(tty,"w+",stderr);
		freopen(tty,"r+",stdin);
		ioctl(0,TIOCSCTTY, 0);
		execv(argv[0],argv);
		exit(2);
	}
}	



int main(void) {
	const char nameofme[] = "laptop";
	const char bootpart[] = "/dev/hda1";
	int status, x;
	char mountrw = 1;
	pid_t pids[5];
	simplecmd("/usr/bin/setleds", "+scroll");
	pids[0] = spawnmount("/proc"+1,"/proc","/proc"+1,0,"");
	pids[1] = simplecmd("/sbin/hwclock", "-s");
	simplecmd("/bin/loadkeys","fi-latin9");
	pids[3] = simplecmd("/usr/bin/setfont","ter-112n");
	myswapon();
	waitpid(pids[1],NULL,0); /* e2fsck needs hwclock */
	pids[4] = e2fsck(homepart);
	pids[1] = e2fsck(rootpart);
	e2fsck(bootpart);
	pids[2] = ifconfig_lo();
	simplecmd("/sbin/rmmod","-a");
	vsimplecmd("/sbin/irqhigh");
	
	if (pids[0]) waitpid(pids[0],NULL,0); /* sysctl (via proc) requires proc */
	sysctls(nameofme);
	pids[0] = modprobe();
	waitpid(pids[2],NULL,0); /* route requires ifconfig */
	route();
	waitpid(pids[4], NULL, 0); /* wait for e2fsck of home b4 shell */
	waitpid( spawnmount(homepart, "/home", rtfstype, 0, ""), NULL, 0);
	shell("/dev/tty5");
	waitpid(pids[1],&status,0); /* wait for e2fsck / (with status rep) */
	if (WIFEXITED(status)) {
		x = WEXITSTATUS(status);
		x = (x & 0x4);
		if (x == 4) mountrw = 0;
		}
	if (mountrw) {
		pids[1] = spawnmount(rootpart,"/",rtfstype,MS_REMOUNT,"");
		if (pids[1]) waitpid(pids[0],NULL,0);
		waitpid(pids[2], NULL, 0); /* wait for home checked before mount*/
		pids[1] = simplecmd("/bin/mount","-a");
		if (pids[1]) waitpid(pids[1],NULL,0);
	} else {
		printf("RootFS Error\nMounting tmpfs to /tmp\n");
		pids[1] = spawnmount("tmpfs","/tmp","tmpfs",0,"size=512k");
		if (pids[1]) waitpid(pids[1],NULL,0);
	}
	
	unlink("/tmp/.X0-lock");
	if (pids[0]) waitpid(pids[0],NULL,0);
	vsimplecmd("/sbin/cardmgr");
	waitpid(pids[3],NULL,0); /* wait for setfont */
	simplecmd("/sbin/rmmod","-a");
	simplecmd("/usr/bin/setleds","-scroll");
 	exit(!mountrw);
}
