#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{
    int fd = open("foo", O_RDONLY);
    if(fd == -1){
	perror("RTC FAILED ");
	return 1;
    }
    struct rtc_time time;
    int ret, r;
    while(1){
	ret = ioctl(fd, RTC_RD_TIME, &time);
    	if(ret == -1){
	    perror("ioctl ");
	    return 2;
	}
	printf("%x:%x:%x   \r", time.tm_hour, time.tm_min, time.tm_sec);
	fflush(NULL);
	sleep(1);
    }
}
