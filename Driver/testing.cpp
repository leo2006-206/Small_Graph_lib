import std;
import Helper;

#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <unistd.h>

int main(void){
	Timing::perf_control pc;

	pc.start();

	pc.finish();

	return 0;
}