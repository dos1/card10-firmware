#include <stdint.h>

#define API_CALL_SEMA 0

struct api_call
{
	uint32_t id;
	uint8_t returning;
	unsigned char buf[1];
};

static struct api_call *ApiCallSpace = (struct api_call *)0x20080001;
