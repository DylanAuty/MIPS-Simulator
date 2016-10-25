#include <stdint.h>

void f(uint8_t *data)
{
	data[2]=data[0]+data[1];
}
