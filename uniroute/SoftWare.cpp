#include "SoftWare.h"

uint32_t timeInterval(uint32_t time1, uint32_t time2)
{
	if( time2 >= time1 ){
		return time2 - time1;
	} else {
		return 0xFFFFFFFF - time1 + time2 + 1;
	}
}

uint32_t timeInterval(uint32_t time1)
{
	uint32_t time2 = millis();
	if( time2 >= time1 ){
		return time2 - time1;
	} else {
		return 0xFFFFFFFF - time1 + time2 + 1;
	}
}

