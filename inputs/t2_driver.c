#include "mips.h"

#include "driver_helper.h"

int main(int argc,char *argv[])
{	
	unsigned i;
	
	if(argc>2){
		fprintf(stderr, "Usage: %s [file.bin]\n", argv[0]);
		exit(1);
	}
	
	unsigned cbMem=1<<20;
	uint8_t *pMem=(uint8_t*)malloc(cbMem);
	
	Driver_LoadInstructions(argc>1 ? argv[0] : "t2_input-mips.bin", (uint32_t*)pMem);
	
	struct mips_state_t *state=mips_create(
		0,	// pc
		cbMem,
		pMem
	);
	
	int failed=0;
	
	for(i=0;i<10;i++){
		unsigned a=rand()%256, b=rand()%256;
		unsigned addr=0x20000+(rand()%1024);
		
		pMem[addr]=a;
		pMem[addr+1]=b;
		
		mips_reset(state, 0);
		
		mips_set_register(state, R_gp, 0);
		mips_set_register(state, R_sp, 0x10000);	// Set up the stack pointer
		mips_set_register(state, R_ra, 0xFFFFFFFFu); 	// Return to nothing (cause simulator to fail)
		
		// Pass pointer as first argument
		mips_set_register(state, R_a0, addr);
		
		while(1){
			if(mips_step(state)!=0)
				break;
		}
		
		uint8_t expected=(a+b)%256;
		uint8_t got=pMem[addr+2];
		
		if(expected!=got){
			fprintf(stderr, "Expected = %u, got=%u\n", expected, got);
			failed=1;
		}
	}
	
	mips_free(state);	
	free(pMem);
	pMem=0;
	
	return failed;
}
