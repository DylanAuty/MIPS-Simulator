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
	
	Driver_LoadInstructions(argc>1 ? argv[0] : "t6_input-mips.bin", (uint32_t*)pMem);
	
	struct mips_state_t *state=mips_create(
		0,	// pc
		cbMem,
		pMem
	);
	
	int fail=0;

/**************HERE BE TESTINGS ARGH**********************/

	uint32_t res_sim;
	
	// Run the program from the beginning
	mips_reset(state, 0);

	// Return to nothing (cause simulator to fail)
	mips_set_register(state, R_ra, 0xFFFFFFFFu);	


	uint32_t correct[1024];
	uint32_t arg1[1024];
	uint32_t arg2[1024];

	//Arguments go here
	int i = 0;		//Here be indexes

	//////////////////////////////////////////////////////
	int numinstr = 26;	//NUMBER OF INSTRUCTIONS GOES HERE	
	//////////////////////////////////////////////////////
	
	//NB: The destination ALWAYS must be $6, regardless of instruction
	//arg1 = $4
	//arg2 = $5
	//correct = $6

	arg1[0] = 0xA;				//ADD	- Vanilla
	arg2[0] = 0xF;
	correct[0] = 0x19;
	
	arg1[1] = 0x00000FFF;		//ADD	- Effective subtract
	arg2[1] = 0xFFFFFFFF;
	correct[1] = 0xFFE;

	arg1[2] = 0x000000FF;		//SUB	- Vanilla
	arg2[2] = 0x00000100;
	correct[2] = 0xFFFFFFFF;
	
	arg1[3] = 0xA;				//ADDI	- Vanilla
	arg2[3] = 0;
	correct[3] = 0x19;	
	
	arg1[4] = 0xA;				//ADDI	- With -ve imm
	arg2[4] = 0;
	correct[4] = 0x9;
	
	arg1[5] = 0xA;				//ADDIU	- Vanilla, "+ve" imm
	arg2[5] = 0;
	correct[5] = 0xB;

	arg1[6] = 0x80000000;		//ADDIU - With "-ve" imm
	arg2[6] = 0;
	correct[6] = 0x80000001;

	arg1[7] = 0xA;				//ADDU	- Vanilla
	arg2[7] = 0xF;
	correct[7] = 0x19;

	arg1[8] = 0x80000000;		//ADDU	- "-ve" arg2
	arg2[8] = 0x1;
	correct[8] = 0x80000001;

	arg1[9] = 0x0000CBE1;		//AND	- Vanilla
	arg2[9] = 0x00006B3B;
	correct[9] = 0x00004B21;

	arg1[10] = 0x0000CBE1;		//ANDI	- Vanilla
	arg2[10] = 0;
	correct[10] = 0x00004B21;

	arg1[11] = 0xAAAAAAAA;		//OR	- Vanilla
	arg2[11] = 0x55555555;
	correct[11] = 0xFFFFFFFF;

	arg1[12] = 0xAAAAAAAA;		//ORI	- Vanilla
	arg2[12] = 0;
	correct[12] = 0xAAAAFFFF;

	arg1[13] = 0xAAAAAAAA;		//XOR	- Vanilla
	arg2[13] = 0x0000FFFF;
	correct[13] = 0xAAAA5555;

	arg1[14] = 0xAAAAAAAA;		//XORI	- Vanilla
	arg2[14] = 0;
	correct[14] = 0xAAAA2C55;

	arg1[15] = 0;				//SRL
	arg2[15] = 0xAAAA0000;
	correct[15] = 0x00AAAA00;

	arg1[16] = 0x8;				//SRLV
	arg2[16] = 0xAAAA0000;
	correct[16] = 0x00AAAA00;

	arg1[17] = 0;				//SRA
	arg2[17] = 0xAAAA0000;
	correct[17] = 0xFFAAAA00;

	arg1[18] = 0x8;				//SLLV
	arg2[18] = 0x0000AAAA;
	correct[18] = 0x00AAAA00;

	arg1[19] = 0x0AA40000;		//SLT
	arg2[19] = 0x0AAA0000;
	correct[19] = 0x1;

	arg1[20] = 0x1;				//SLTI
	arg2[20] = 0;
	correct[20] = 0x1;

	arg1[21] = 0x1;				//SLTU
	arg2[21] = 0xFFFFFFFF;
	correct[21] = 0x1;

	arg1[22] = 0x1;				//SLTIU
	arg2[22] = 0;
	correct[22] = 0x1;

	arg1[23] = 0;				//LUI
	arg2[23] = 0;
	correct[23] = 0xABCD0000;



	
	//LAST TWO INSTRUCTIONS ARE ALWAYS JR and NOOP
	numinstr --;		//...Bloody 0 indexing.

	arg1[numinstr - 1] = 0;
	arg2[numinstr - 1] = 0;
	correct[numinstr - 1] = 0;
	
	arg1[numinstr] = 0;
	arg2[numinstr] = 0;
	correct[numinstr] = 0;

	//INITIALIZE THE THINGY
	
	mips_set_register(state, 4, arg1[0]);	//So, every time, operands go in registers $4 and $5
	mips_set_register(state, 5, arg2[0]);	
	
	while(!mips_step(state)){
		// Keep stepping till it attempts to return to 0xFFFFFFFF
		
		fprintf(stdout, "Instruction no. %d\n", i);

		res_sim = mips_get_register(state, R_a2);	//Fetch from $6
		int h = 0;
		for (h = 0; h < 32; h++) {
			fprintf(stdout, "r[%d] = %.8x\n", h, mips_get_register(state, h));
		}
		
		if(res_sim!=correct[i]){
			fprintf(stdout, "F(%u,%u), sim=%u, correct=%u\n", arg1[i], arg2[i], res_sim, correct[i]);
			fail=1;
		}
		else{
			fprintf(stdout, "Instruction no. %d passed!!!\n", i);
		}
		
		//Now increment the program counter and set it up for the next instruction
		i++;
		mips_set_register(state, 4, arg1[i]);
		mips_set_register(state, 5, arg2[i]);	//We'll manually set the arguments depending on what we're testing
		mips_set_register(state, 6, 0);			//Also manually reset $6 to 0 each time	

	}
	res_sim = mips_get_register(state, R_a2);	//Fetch from $6
	if(res_sim!=correct[i]){
		fprintf(stdout, "F(%u,%u), sim=%u, correct=%u\n", arg1[i], arg2[i], res_sim, correct[i]);
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
