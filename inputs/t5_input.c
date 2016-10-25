#include <stdint.h>

uint32_t t5_F(uint32_t a, uint32_t b)
{
	uint32_t acc=0;
	while(b>0){
		if(b&1){
			acc=acc+a;
		}
		a=a<<1;
		b=b>>1;
	}
	return acc;
}
