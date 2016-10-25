#include "mips.h"

#include "driver_helper.h"

// This is the function which the simulator is running
#include "t1_input.c"

int main(int argc,char *argv[])
{	
	if(argc>2){
		fprintf(stderr, "Usage: %s [file.bin]\n", argv[0]);
		exit(1);
	}
	
	unsigned cbMem=1<<20;
	uint8_t *pMem=(uint8_t*)malloc(cbMem);
	
	Driver_LoadInstructions(argc>1 ? argv[0] : "t7_input-mips.bin", (uint32_t*)pMem);
	
	struct mips_state_t *state=mips_create(
		0,	// pc
		cbMem,
		pMem
	);
	
	int fail=0;

/**************HERE BE TESTINGS ARGH**********************/

	uint32_t res_sim_hi, res_sim_lo;
	
	// Run the program from the beginning
	mips_reset(state, 0);

	// Return to nothing (cause simulator to fail)
	mips_set_register(state, R_ra, 0xFFFFFFFFu);	


	uint32_t correct_hi[1024];
	uint32_t correct_lo[1024];
	uint32_t arg1[1024];
	uint32_t arg2[1024];

	//Arguments go here
	int i = 0;		//Here be indexes

	//////////////////////////////////////////////////////
	int numinstr = 4;	//NUMBER OF INSTRUCTIONS GOES HERE	
	//////////////////////////////////////////////////////
	
	//arg1 = $4
	//arg2 = $5
	//correct_hi = $r_hi
	//correct_lo = $r_lo

	arg1[0] = 0xC;				//DIV
	arg2[0] = 0x5;
	correct_hi[0] = 0x2;	
	correct_lo[0] = 0x2;

	arg1[1] = 0xC;				//MULT
	arg2[1] = 0x5;
	correct_hi[1] = 0;	
	correct_lo[1] = 0x3C;
	//LAST TWO INSTRUCTIONS ARE ALWAYS JR and NOOP
	numinstr --;		//...Bloody 0 indexing.

	arg1[numinstr - 1] = 0;
	arg2[numinstr - 1] = 0;
	correct_hi[numinstr - 1] = 0;
	correct_lo[numinstr - 1] = 0;

	arg1[numinstr] = 0;
	arg2[numinstr] = 0;
	correct_hi[numinstr] = 0;
	correct_lo[numinstr] = 0;

	//INITIALIZE THE THINGY
	
	mips_set_register(state, 4, arg1[0]);	//So, every time, operands go in registers $4 and $5
	mips_set_register(state, 5, arg2[0]);	
	
	while(!mips_step(state)){
		// Keep stepping till it attempts to return to 0xFFFFFFFF
		
		fprintf(stdout, "Instruction no. %d\n", i);

		res_sim_hi = mips_get_r_hi(state);	//Fetch from $r_hi
		res_sim_lo = mips_get_r_lo(state);	
		
		fprintf(stdout, "r_hi = %.8x\n", mips_get_r_hi(state));
		fprintf(stdout, "r_lo = %.8x\n", mips_get_r_lo(state));
		
		if(res_sim_hi!=correct_hi[i] || res_sim_lo!=correct_lo[i]){
			fprintf(stdout, "F(%u,%u), sim_hi=%u, sim_lo=%u, correct_hi=%u, correct_lo=%u\n", arg1[i], arg2[i], res_sim_hi, res_sim_lo, correct_hi[i], correct_lo[i]);
			fail=1;
		}
		else{
			fprintf(stdout, "Instruction no. %d passed!!!\n", i);
		}
		
		//Now increment the program counter and set it up for the next instruction
		i++;
		mips_set_register(state, 4, arg1[i]);
		mips_set_register(state, 5, arg2[i]);	//We'll manually set the arguments depending on what we're testing
		mips_reset_hilo(state);
	}
	res_sim_hi = mips_get_r_hi(state);	//Fetch from $r_hi
	res_sim_lo = mips_get_r_lo(state);
	
	if(res_sim_hi!=correct_hi[i] || res_sim_lo!=correct_lo[i]){
		fprintf(stdout, "F(%u,%u), sim_hi=%u, sim_lo=%u, correct_hi=%u, correct_lo=%u\n", arg1[i], arg2[i], res_sim_hi, res_sim_lo, correct_hi[i], correct_lo[i]);
		fail=1;
	}	
	else{
		fprintf(stdout, "Instruction no. %d passed!!!\n\n", i);
	}
	
	fprintf(stdout, "END OF TEST\n\n");

	mips_free(state);	
	free(pMem);
	pMem=0;
	
	return fail;
}

