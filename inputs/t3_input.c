#include <stdint.h>

uint32_t t3_F(uint32_t a, uint32_t b)
{
	return (a&0xFFF)+(b<<3);
}
