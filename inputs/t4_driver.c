#include "mips.h"

#include "driver_helper.h"

// This is the function which the simulator is running
uint32_t t4_F(uint32_t a, uint32_t *b)
{
	return a+ntohl(b[8]);
}

int main(int argc,char *argv[])
{	
	unsigned i;
	
	if(argc>2){
		fprintf(stderr, "Usage: %s [file.bin]\n", argv[0]);
		exit(1);
	}
	
	unsigned cbMem=1<<20;
	uint8_t *pMem=(uint8_t*)malloc(cbMem);
	
	Driver_LoadInstructions(argc>1 ? argv[1] : "t4_input-mips.bin", (uint32_t*)pMem);
	
	struct mips_state_t *state=mips_create(
		0,	// pc
		cbMem,
		pMem
	);
	
	int fail=0;
	
	for(i=0;i<10;i++){
		unsigned argOffset=256;
		uint32_t *pArgs=(uint32_t*)(pMem+argOffset);	// Live pointer into memory, past end of code
		
		uint32_t a=rand(), res_correct, res_sim;
		pArgs[8]=rand();
		
		// Run the program from the beginning
		mips_reset(state, 0);
		// Return to nothing (cause simulator to fail)
		mips_set_register(state, R_ra, 0xFFFFFFFFu);	
		// Set up a stack at 0x10000
		mips_set_register(state, R_sp, 0x10000);
		// This is setting up the arguments		
		mips_set_register(state, R_a0, a);
		mips_set_register(state, R_a1, argOffset);	// Passing a pointer to the function
	
		while(!mips_step(state)){
			// Keep stepping till it attempts to return to 0xFFFFFFFF
		}
		
		// Pull the result out
		res_sim=mips_get_register(state, R_v0);
		
		// Get the true result
		res_correct=t4_F(a, pArgs);
	
		if(res_sim!=res_correct){
			fprintf(stdout, "F(%u,%u), sim=%u, correct=%u\n", a, pArgs[8], res_sim, res_correct);
			fail=1;
		}
	}		
	
	mips_free(state);	
	free(pMem);
	pMem=0;
	
	return fail;
}
