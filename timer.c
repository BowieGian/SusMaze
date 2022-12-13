//  created by Chenting Mao
//	last edited: 2022/9/30	by:	Chenting Mao

#include "timer.h"

long long getTimeInMs(void) 
{
 	struct timespec spec;
 	clock_gettime(CLOCK_REALTIME, &spec);
 	long long seconds = spec.tv_sec;
 	long long nanoSeconds = spec.tv_nsec;
 	long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
 	return milliSeconds;
}

long long getTimeInSec(void)
{
	struct timespec spec;
 	clock_gettime(CLOCK_REALTIME, &spec);
	long long seconds = spec.tv_sec;
	return seconds;
}


long long getTimeInNs(void)
{
	struct timespec spec;
 	clock_gettime(CLOCK_REALTIME, &spec);
 	long long seconds = spec.tv_sec;
 	long long nanoSeconds = seconds * 1000000000 + spec.tv_nsec;
	return nanoSeconds;
}


void sleepForMs(long long delayInMs)
{
 	const long long NS_PER_MS = 1000 * 1000;
 	const long long NS_PER_SECOND = 1000000000;
 	long long delayNs = delayInMs * NS_PER_MS;
 	int seconds = delayNs / NS_PER_SECOND;
 	int nanoseconds = delayNs % NS_PER_SECOND;
 	struct timespec reqDelay = {seconds, nanoseconds};
 	nanosleep(&reqDelay, (struct timespec *) NULL);
}
