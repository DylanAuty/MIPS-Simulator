#include "mips.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

struct mips_state_t{	//THIS is the Struct
	uint32_t pc;
	uint32_t npc;		//Next value of PC, to be written at START of step funct to pc.
	unsigned sizeMem;
	uint8_t *pMem;
	uint32_t r[32];	//Register array - 32 registers, each 32 bits long unsigned
	uint32_t r_hi;	//HI register, for DIV operations
	uint32_t r_lo;	//LO register, for DIV operations
	uint8_t error;	//Error flag - set to 0 initially, set to 1 when something (overflow?) is wrong.
};	//

/*! Initialises state so that the addressable memory is bound
	to pMem, the processor has just been reset, and the next
	instruction to be executed is located at pc. The memory
	pointer to by pMem is guaranteed to remain valid until
	the corresponding call to mips_free.
*/

/********************ADDITIONAL FUNCTION PROTOTYPES***************************/

/* Individual subroutines to handle R, J and I type instructions.*/
void J_type(struct mips_state_t *state, uint8_t opcode, uint32_t instruction);
void R_type(struct mips_state_t *state, uint32_t instruction);
void I_EX_type(struct mips_state_t *state, uint8_t opcode, uint32_t instruction);
	//NB: I_EX_type handles I type and possible exceptions too.

/* Advance Program counter function prototype */
void advance_pc(struct mips_state_t *state, int offset);



////////////////////////////INSTRUCTION FUNCTION PROTOTYPES////////////////////

//J Type
void J(struct mips_state_t *state, uint32_t instruction);
void JAL(struct mips_state_t *state, uint32_t instruction);

//R Type
void JR(struct mips_state_t *state, uint8_t rs);
void ADDU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SLL(struct mips_state_t *state, uint8_t rd, uint8_t rt, uint8_t shift);
void SRL(struct mips_state_t *state, uint8_t rd, uint8_t rt, uint8_t shift);
void ADD(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void AND(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void DIV(struct mips_state_t *state, uint8_t rs, uint8_t rt);
void DIVU(struct mips_state_t *state, uint8_t rs, uint8_t rt);
void MFHI(struct mips_state_t *state, uint8_t rd);
void MFLO(struct mips_state_t *state, uint8_t rd);
void MULT(struct mips_state_t *state, uint8_t rs, uint8_t rt);
void MULTU(struct mips_state_t *state, uint8_t rs, uint8_t rt);
void OR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SUB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SLT(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SLTU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SUBU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void XOR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SRA(struct mips_state_t *state, uint8_t rt, uint8_t rd, uint8_t shift);
void SRLV(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);
void SLLV(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd);

//I Type
void SW(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void LB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void LBU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void SB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void ANDI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void LW(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void BEQ(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void BNE(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void ADDI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void SLTI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void SLTIU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void ORI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void XORI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void ADDIU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void BLTZAL(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void BLTZ(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void BLEZ(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void BGTZ(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void BGEZAL(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void BGEZ(struct mips_state_t *state, uint8_t rs, uint16_t imm);
void LUI(struct mips_state_t *state, uint8_t rt, uint16_t imm);
void LWL(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void LWR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);
void SH(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm);

/*****************************************************************************/

struct mips_state_t* mips_create(	//THIS is the constructor function that returns a state... maybe. I dunno.
	uint32_t pc,      //! Address of first instruction
	unsigned sizeMem, //! Number of addressable bytes
	uint8_t *pMem	  //! Pointer to sizeMem bytes
		//usage of pMem - access byte 'pc' - "blah = state->pMem[pc]"
	){
	//At this point, the memory has been malloc'ed, and pMem is already in the right place.
	//pc is also in the right place here, wherever that may be
	struct mips_state_t *state = (struct mips_state_t *)malloc(sizeof(mips_state_t));	//Declare a mips_state_t struct
	state->pc = pc;
	state->npc = 4;
	state->sizeMem = sizeMem;
	state->pMem = pMem;
	
	int i = 0;
	for (i = 0; i < 32; i++) {
		state->r[i] = 0;	//Set all of the registers to 0
	}
	state->r_hi = 0;		//Set the HI and LO registers to 0
	state->r_lo = 0;
	state->error = 0;		//error flag is initially 0; if something goes wrong, then it'll be 1. So yeah. That's a thing.
	return state;
}

uint32_t mips_get_pc(struct mips_state_t *state){
	return state->pc;
}

uint32_t mips_get_r_lo(struct mips_state_t *state){
	return state->r_lo;
}
uint32_t mips_get_r_hi(struct mips_state_t *state){
	return state->r_hi;
}
void mips_reset_hilo(struct mips_state_t *state){
	state->r_hi = 0;
	state->r_lo = 0;
	return;
}

/*! Takes an existing state, and resets the registers.
	Should be equivalent to a state just returned from
	mips_create */
void mips_reset(struct mips_state_t *state, uint32_t pc){
	state->pc = pc;
	state->npc = state->pc + 4;
	int i = 0;
	for (i = 0; i < 32; i++) {
		state->r[i] = 0;			//reset all registers in the state to 0
	}
	state->r_lo = 0;
	state->r_hi = 0;
	state->error = 0;
	return;
}

/*! Returns the current value of one of the 32 general
	purpose MIPS registers.*/
uint32_t mips_get_register(struct mips_state_t *state, unsigned index){
	return state->r[index];			//...This doesn't need commenting. Blech.
}

/*! Modifies one of the 32 general purpose MIPS registers. */
void mips_set_register(struct mips_state_t *state, unsigned index, uint32_t value){
	state->r[index] = value;		//WOO FOR ONE LINE FUNCTIONS
	return;
}

/*! Advances the processor by one instruction. If no error
	occured, then return zero. For anything
	which stops execution, return non-zero. */
int mips_step(struct mips_state_t *state){

	uint32_t instruction;
	instruction = 0x00000000;
	uint8_t cByte;	//cByte holds the current byte
	uint32_t temp;	//Bitshifting happens in here
	
	/* Right, so...
	 * 	1) System is Big Endian
	 * 	2) Casting uint8_t as uint32_t shifts it all the way to the right:
	 * 		e.g. read 0x03000000 as 8 bits, then cast = 0x00000003.
	 */

	int i = 0;
	int shiftVal = 32;
	uint8_t opcode;							
	opcode = state->pMem[state->pc];	//Read in the opcode
	opcode = opcode >> 2;				//Shift it along to remove the irrelevant bits - output = 00******.

	/*************************************************************/
	
	for (i = 0; i < 4; i++) {			//Read in the instruction, byte by byte
		shiftVal -= 8;					//set bitshift value - bitshift left by (32 - n * 8) bits
		cByte = state->pMem[state->pc];	//Read in a byte of the instruction
		
		temp = (uint32_t)cByte;			//Cast the 8 bit thing as a 32 bit thing
		temp = temp << shiftVal;
		instruction = instruction | temp;
		state->pc++;					//Increment pc, look at the next byte. 
										//We'll do this 4 times total per function call
	}
	
	//fprintf(stderr, "\nDEBUG: Instruction = %.8x\n", instruction);

	//At this point, the instruction has been fully read in.
	
	/*SwitchyswitchyswitchWOOOOO*/
	//We've already retrieved the opcode, and put it in, uh, opcode. Yeah.
	
	/* Now we differentiate between the 3 different types of instructions:
	 * 	1) R Type has opcode 0b000000
	 * 		- Once this is ascertained, the function code must be read to determine
	 * 			exactly what operation is called for.
	 * 	2) J Type has opcode 0b000010 or 0b000011
	 * 		- There are only 2 types of J instruction
	 * 	3) I Type has any other opcode
	 */

	switch (opcode){
		//J TYPE:
		case 0x02:	//J 		- Jump
		case 0x03:	//JAL 		- Jump and link
			/* Indicates J Type instruction - jump to be executed. 6 bit op, 26 bit addr.*/
			J_type(state, opcode, instruction);	//Call J_type subroutine to handle J type instructions
			break;

		//R TYPE:
		case 0x00:
			/* Indicates R Type instruction - now slice off the last 6 bits of instruction*/
			R_type(state, instruction);			//Call R_type subroutine to handle R type instructions
			break;

		//I Type/invalid opcodes
		default:
			I_EX_type(state, opcode, instruction);
			/* Indicates EITHER - I Type instruction OR Invalid instruction.*/
			break;
	}
/*
	fprintf(stderr, "\nPC = 0x%.8x\n", state->pc);
	fprintf(stderr, "nPC = 0x%.8x\n\n", state->npc);

	int j = 0;
		for (j = 0; j < 32; j++) {
			fprintf(stderr, "r[%d] = %.8x\n", j, state->r[j]);
		}	
*/
	
	if(state->error == 1){
		return 1;
	}		//Checking for an error somewhere in the functions, like an overflow or summat

	if(state->pc >= state->sizeMem){
		return 1;
	}		//If you're trying to access a bit of completely bloody wrong memory...
	else{
		return 0;
	}
}

/*! Free all resources associated with state. */
void mips_free(struct mips_state_t *state){
	free(state);
}


//////////////////////////////////////////////////////////////////////////////
//***********************DEALING WITH INSTRUCTIONS**************************//
//////////////////////////////////////////////////////////////////////////////

void J_type(struct mips_state_t *state, uint8_t opcode, uint32_t instruction){
	switch (opcode){
		case 0x02:	//J			-Jump
			J(state, instruction);
			break;

		case 0x03:	//JAL		-Jump and link
			//Jumps to the calculated address and stores the return address in r[31]
			JAL(state, instruction);
			break;
	}
	return;		//All done, back we go...
}

void R_type(struct mips_state_t *state, uint32_t instruction){
	//Opcode doesn't need to be passed since it just... is 0.
	
	//Slice up the instruction into the necessary bits.
	uint32_t temp = 0;

	uint8_t funct = 0;		//6 bits
	uint8_t rs = 0;			//5 bits
	uint8_t rt = 0;			//5 bits
	uint8_t rd = 0;			//5 bits
	uint8_t shift = 0;		//5 bits
	
	temp = instruction & 0x0000003F;	//Mask out all but funct
	funct = temp;

	temp = instruction & 0x03E00000;	//Retrieve rs	0000 0011 1110 0000 00000...
	temp = temp >> 21;
	rs = temp;

	temp = instruction & 0x001F0000;	//Retrieve rt	0000 0000 0001 1111 00000...
	temp = temp >> 16;
	rt = temp;

	temp = instruction & 0x0000F800;	//Retrieve rd	0000 0000 0000 0000 1111 1000 00......
	temp = temp >> 11;
	rd = temp;

	temp = instruction & 0x000007C0;	//Retrieve shift 0000 0000 0000 0000 0000 0111 1100 0000
	temp = temp >> 6;
	shift = temp;

	//fprintf(stderr, "funct = %.2x\n", funct);

	//Son, I am switchy switch switch mmm yum tasty
	
	switch(funct){
		
		case 0x20:		//ADD		- Add
			ADD(state, rs, rt, rd);
			break;
		case 0x21:		//ADDU		- Add unsigned
			ADDU(state, rs, rt, rd);
			break;
		case 0x24:		//AND		- Bitwise AND
			AND(state, rs, rt, rd);
			break;
		case 0x1A:		//DIV		- Divide
			DIV(state, rs, rt);
			break;
		case 0x1B:		//DIVU		- Unsigned divide
			DIVU(state, rs, rt);
			break;
		case 0x08:		//JR		- Jump to address in register
			JR(state, rs);
			break;
		case 0x10:		//MFHI		- Move from HI register
			MFHI(state, rd);
			break;
		case 0x12:		//MFLO		- Move from LO register
			MFLO(state, rd);
			break;
		case 0x18:		//MULT		- Multiply
			MULT(state, rs, rt);
			break;
		case 0x19:		//MULTU		- Unsigned multiply
			MULTU(state, rs, rt);
			break;
		case 0x25:		//OR		- Bitwise OR
			OR(state, rs, rt, rd);
			break;
		case 0x00:		//SLL		- Logical shift left OR NOOP
			if(instruction == 0x00000000){	//This is a NOOP instruction
				advance_pc(state, 4);
			break;
			}
			else{		//Execute the SLL
			SLL(state, rt, rd, shift);
			}
			break;
		case 0x2A:		//SLT		- Set to 1 if less than
			SLT(state, rs, rt, rd);
			break;
		case 0x2B:		//SLTU		- Set to 1 if less than unsigned
			SLTU(state, rs, rt, rd);
			break;
		case 0x03:		//SRA		- Arithmetic shift right (sign-extended)
			SRA(state, rt, rd, shift);
			break;
		case 0x02:		//SRL		- Logical shift right (0-extended)
			SRL(state, rt, rd, shift);
			break;
		case 0x06:
			SRLV(state, rs, rt, rd);
			break;
		case 0x22:		//SUB		- Subtract
			SUB(state, rs, rt, rd);
			break;
		case 0x23:		//SUBU		- Unsigned subtract
			SUBU(state, rs, rt, rd);
			break;
		case 0x26:		//XOR		- Bitwise XOR
			XOR(state, rs, rt, rd);
			break;
		case 0x04:		//SLLV		- Shift left logical variable
			SLLV(state, rs, rt, rd);
			break;
		/*
		case 0x00*/
		default:
			fprintf(stderr, "ERROR: INVALID COMMAND: 0x%.8x\n", instruction);
			fprintf(stderr, "EXITING PROGRAM...\n");
			exit(1);
			break;

	}
	
	return;
}

void I_EX_type(struct mips_state_t *state, uint8_t opcode, uint32_t instruction){
	uint32_t temp = 0;

	uint8_t rs = 0;			//5 bits
	uint8_t rt = 0;			//5 bits
	uint16_t imm = 0;		//16 bits

	temp = instruction & 0x03E00000;	//Retrieve rs	0000 0011 1110 0000 00000...
	temp = temp >> 21;
	rs = temp;

	temp = instruction & 0x001F0000;	//Retrieve rt	0000 0000 0001 1111 00000...
	temp = temp >> 16;
	rt = temp;

	temp = instruction & 0x0000FFFF;	//Retrieve imm	0000 0000 0000 0000 1111 1111 1111 1111
	imm = temp;

	switch(opcode){
		case 0x0F:	//LUI		- Load upper immediate
			LUI(state, rt, imm);
			break;
		case 0x0A:	//SLTI		- Set if less than immediate
			SLTI(state, rs, rt, imm);
			break;
		case 0x0B:	//SLTIU		- Set if less than immediate unsigned
			SLTIU(state, rs, rt, imm);
			break;
		case 0x24:	//LBU		- Load byte unsigned
			LBU(state, rs, rt, imm);
			break;
		case 0x08:	//ADDI		- Add immediate signed
			ADDI(state, rs, rt, imm);
			break;
		case 0x20:	//LB		- Load byte
			LB(state, rs, rt, imm);
			break;
		case 0x28:	//SB		- Store byte
			SB(state, rs, rt, imm);
			break;
		case 0x0C:	//ANDI		- Bitwise AND immediate
			ANDI(state, rs, rt, imm);
			break;
		case 0x0D:	//ORI		- Bitwise OR immediate
			ORI(state, rs, rt, imm);
			break;
		case 0x0E:	//XORI		- Bitwise XOR immediate
			XORI(state, rs, rt, imm);
			break;
		case 0x23:	//LW		- Load word
			LW(state, rs, rt, imm);
			break;
		case 0x04:	//BEQ		- Break if equal
			BEQ(state, rs, rt, imm);
			break;
		case 0x05:	//BNE		- Break if not equal
			BNE(state, rs, rt, imm);
			break;
		case 0x01:	//BGEZ		- Break if greater than or equal to 0
			BGEZ(state, rs, imm);
			break;
		case 0x09:	//ADDIU		- Add immediate unsigned
			ADDIU(state, rs, rt, imm);
			break;
		case 0x2B:	//SW		- Store word
			SW(state, rs, rt, imm);
			break;
		case 0x22:	//LWL		- Load word left
			LWL(state, rs, rt, imm);
			break;
		case 0x26:	//LWR		- Load word right
			LWR(state, rs, rt, imm);
			break;
		case 0x29:	//SH		- Store half word
			SH(state, rs, rt, imm);
			break;
		default:
			fprintf(stderr, "ERROR: INVALID COMMAND: 0x%.8x\n", instruction);
			fprintf(stderr, "EXITING PROGRAM...\n");
			exit(1);
			break;
	}
}

void advance_pc(struct mips_state_t *state, int offset){	
	state->pc = state->npc;
	state->npc += offset;
	return;
}

//////////////////////////////////////////////////////
/****************INSTRUCTION SUBROUTINES*************/
//////////////////////////////////////////////////////


/**********************I TYPE************************/

void SH(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	uint32_t address;
	address = (int32_t)offset + state->r[rs];
	uint32_t temp;
	temp = state->r[rt];
	int i = 0;
	int shiftVal = 8;
	for (i = 0; i < 2; i++) {
		state->pMem[address] = (uint8_t)(temp >> shiftVal);
		shiftVal -= 8;
		address ++;
	}
	advance_pc(state, 4);
	return;
}


void LWL(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	uint32_t address;
	uint32_t temp;
	uint32_t final;
	uint8_t cByte;
	address = state->r[rs] + (int32_t)imm;
	int shiftVal = 32;
	int i = 0;

	for (i = 0; i < 2; i++) {			
		shiftVal -= 8;					//set bitshift value - bitshift left by (32 - n * 8) bits
		cByte = state->pMem[address];			
		temp = (uint32_t)cByte;			//Cast the 8 bit thing as a 32 bit thing
		temp = temp << shiftVal;
		final = final | temp;
		address++;					 
										//We'll do this 4 times total per function call
	}
	state->r[rt] = state->r[rt] & 0x0000FFFF;		//Clear the most significant half
	state->r[rt] = state->r[rt] | final;			//Put the fetched word into the top half
	advance_pc(state, 4);
	return;
}

void LWR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	uint32_t address;
	uint32_t temp;
	uint32_t final;
	uint8_t cByte;
	address = state->r[rs] + (int32_t)imm;
	int shiftVal = 0;
	int i = 0;

	for (i = 0; i < 2; i++) {			
		cByte = state->pMem[address];			
		temp = (uint32_t)cByte;			//Cast the 8 bit thing as a 32 bit thing
		temp = temp << shiftVal;
		final = final | temp;
		shiftVal += 8;
		address--;					 
										//We'll do this 4 times total per function call
	}
	state->r[rt] = state->r[rt] & 0xFFFF0000;		//Clear the most significant half
	state->r[rt] = state->r[rt] | final;			//Put the fetched word into the top half
	advance_pc(state, 4);
	return;
}

void LUI(struct mips_state_t *state, uint8_t rt, uint16_t imm){
	uint32_t temp;
	temp = (uint32_t)imm << 16;
	state->r[rt] = temp;
	advance_pc(state, 4);
	return;
}

void SW(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;
	offset = (int16_t)imm;
	state->pMem[state->r[rs] + offset] = state->r[rt];
	advance_pc(state, 4);
	return;
}

void LBU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	state->r[rt] = state->pMem[state->r[rs] + offset];
	advance_pc(state, 4);
	return;
}

void LB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	uint64_t sum;			//Used to detect overflow
	sum = state->r[rs] + offset;
	if(sum > 0x00000000FFFFFFFF){
		//Overflow
		state->error = 1;
	}

	state->r[rt] = state->pMem[(uint32_t)sum];
	advance_pc(state, 4);
	return;
}

void SB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Complete and utter castage
	state->pMem[state->r[rs] + offset] = 0x00FF & state->r[rt];
	advance_pc(state, 4);
	return;
}

void ANDI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	state->r[rt] = state->r[rs] & (uint32_t)imm;
	advance_pc(state, 4);
	return;
}

void ORI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	state->r[rt] = state->r[rs] | (uint32_t)imm;
	advance_pc(state, 4);
	return;
}

void XORI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	state->r[rt] = state->r[rs] ^ (uint32_t)imm;
	advance_pc(state, 4);
	return;
}

void LW(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	//state->r[rt] = state->pMem[state->r[rs] + offset];
	uint32_t address = 0;
	uint32_t word = 0;
	uint32_t temp = 0;
	uint8_t cByte = 0;
	int shiftVal = 32;

	int doflag = 1;
	
	if(offset & 0x0002 != 0){	//Check the last 2 bits of offset are 0
		doflag = 0;		//If they aren't, then throw an error
	}

	address = state->r[rs] + offset;
	int i = 0;
	doflag = 1;
	for (i = 0; i < 4; i++) {			//Read in the instruction, byte by byte
		shiftVal -= 8;					//set bitshift value - bitshift left by (32 - n * 8) bits
		cByte = state->pMem[address];	//Read in a byte of the instruction
		
		temp = (uint32_t)cByte;			//Cast the 8 bit thing as a 32 bit thing
		temp = temp << shiftVal;
		word = word | temp;
		address++;						//Increment address, look at the next byte. 
										//We'll do this 4 times total per function call
										//Hence we read in the entire word
	}
	if(doflag){
		state->r[rt] = word;
	}
	advance_pc(state, 4);
	return;
}

void BEQ(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if(state->r[rs] == state->r[rt]){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BNE(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if(state->r[rs] != state->r[rt]){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BGEZ(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if((int32_t)state->r[rs] >= 0){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BGEZAL(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if((int32_t)state->r[rs] >= 0){
		state->r[31] = state->pc + 8;
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BGTZ(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if((int32_t)state->r[rs] > 0){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BLEZ(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage

	if((int32_t)state->r[rs] <= 0){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BLTZ(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if((int32_t)state->r[rs] < 0){
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void BLTZAL(struct mips_state_t *state, uint8_t rs, uint16_t imm){
	int16_t offset;			//Signed value to store offset
	offset = (int16_t)imm;	//Castage
	
	if((int32_t)state->r[rs] < 0){
		state->r[31] = state->pc + 8;
		advance_pc(state, offset << 2);
	}
	else{
		advance_pc(state, 4);
	}
	return;
}

void ADDI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t immediate;			//Signed value to store offset
	immediate = (int16_t)imm;	//Castage
	int64_t result = 0;
	uint64_t temp = 0;
	int doflag = 1;
	result = (int64_t)((int32_t)state->r[rs]) + (int64_t)immediate;	//Do it all signed
	//Now check for overflow....
	if(result >= 0){	//Positive, check without finding magnitude
		if((uint64_t)result > 0x000000007FFFFFFF){
			state->error = 1;
			doflag = 0;
		}
	}
	else{				//result is negative, so find magnitude then check
		temp = ~((uint64_t)result);
		temp++;								//Twos complementing
		temp = temp & 0x7FFFFFFFFFFFFFFF;	//Mask out the sign bit for, uh, good measure
		if(temp > 0x000000007FFFFFFF){		//Overflow is a thing
			state->error = 1;
			doflag = 0;
		}
	}
	if(doflag){
		state->r[rt] = (int32_t)result; 
	}
	advance_pc(state, 4);
	return;
}

void SLTI(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	int16_t immediate;			//Signed value to store offset
	immediate = (int16_t)imm;	//Castage
	if((int32_t)state->r[rs] < (int32_t)immediate){		//Do a signed comparison
		//if $s is less than imm, then $d = 1.
		state->r[rt] = 1;
	}
	else{
		state->r[rt] = 0;
	}
	advance_pc(state, 4);
	return;
}

void SLTIU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	if(state->r[rs] < (uint32_t)imm){
		state->r[rt] = 1;
	}
	else{
		state->r[rt] = 0;
	}
	advance_pc(state, 4);
	return;
}

void ADDIU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint16_t imm){
	state->r[rt] = state->r[rs] + (uint32_t)imm;
	advance_pc(state, 4);
	return;
}



/**********************R TYPE************************/

void SLLV(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rt] << state->r[rs];
	advance_pc(state, 4);
	return;
}

void JR(struct mips_state_t *state, uint8_t rs){
	state->pc = state->npc;
	state->npc = state->r[rs];
	return;
}

void ADDU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	//No overflow
	state->r[rd] = state->r[rt] + state->r[rs];
	advance_pc(state, 4);
	return;
}

void SLL(struct mips_state_t *state, uint8_t rt, uint8_t rd, uint8_t shift){
	state->r[rd] = state->r[rt] << shift;
	advance_pc(state, 4);
	return;
}

void SRL(struct mips_state_t *state, uint8_t rt, uint8_t rd, uint8_t shift){
	state->r[rd] = state->r[rt] >> shift;
	advance_pc(state, 4);
	return;
}

void ADD(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	//d = s + t
	int64_t result = 0;
	uint64_t temp = 0;
	int	doflag = 1;
	//First, sign extend the numbers...
	
	result = (int64_t)((int32_t)state->r[rs]) + (int64_t)((int32_t)state->r[rt]);	//Do it all signeda
	//Now check for overflow....
	if(result >= 0){	//Positive, check without finding magnitude
		if((uint64_t)result > 0x7FFFFFFF){
			state->error = 1;
			doflag = 0;	//Don't write the output!!!
		}
	}
	else{				//result is negative, so find magnitude then check
		temp = ~((uint64_t)result);
		temp++;								//Twos complementing
		temp = temp & 0x7FFFFFFFFFFFFFFF;	//Mask out the sign bit for, uh, good measure
		if(temp > 0x000000007FFFFFFF){		//Overflow is a thing
			state->error = 1;
			doflag = 0;		//Don't write output agh!
		}
	}
	
	if(doflag){
		state->r[rd] = (int32_t)result; 
	}
	advance_pc(state, 4);
	return;
}

void AND(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rs] & state->r[rt];
	advance_pc(state, 4);
	return;
}

void DIV(struct mips_state_t *state, uint8_t rs, uint8_t rt){
	int64_t quotient;
	uint64_t temp = 0;
	int doflag = 1;
	quotient = (int64_t)((int32_t)state->r[rs])/(int64_t)((int32_t)state->r[rt]);	//Do it all signeda
	//Now check for overflow....
	/*
	if(quotient >= 0){	//Positive, check without finding magnitude
		if((uint64_t)quotient > 0x000000007FFFFFFF){
			state->error = 1;
			doflag = 0;
		}
	}
	else{				//result is negative, so find magnitude then check
		temp = ~((uint64_t)quotient);
		temp++;								//Twos complementing
		temp = temp & 0x7FFFFFFFFFFFFFFF;	//Mask out the sign bit for, uh, good measure
		if(temp > 0x000000007FFFFFFF){		//Overflow is a thing
			state->error = 1;
			doflag = 0;
		}
	}
	*/
	if(doflag){
		state->r_lo = (int32_t)quotient;
		state->r_hi = (int32_t)state->r[rs]%(int32_t)state->r[rt];
	}
	advance_pc(state, 4);
	return;
	
}

void DIVU(struct mips_state_t *state, uint8_t rs, uint8_t rt){
	state->r_lo = state->r[rs]/state->r[rt];
	state->r_hi = state->r[rs]%state->r[rt];
	return;
}

void MFHI(struct mips_state_t *state, uint8_t rd){
	state->r[rd] = state->r_hi;
	advance_pc(state, 4);
	return;
}

void MFLO(struct mips_state_t *state, uint8_t rd){
	state->r[rd] = state->r_lo;
	advance_pc(state, 4);
	return;
}

void MULT(struct mips_state_t *state, uint8_t rs, uint8_t rt){
	int64_t result;
	int doflag = 1;
	result = (int64_t)((int32_t)state->r[rs])*(int64_t)((int32_t)state->r[rt]);	//Do it all signed
	if((int64_t)((int32_t)state->r[rs]) != 0 && result/(int64_t)((int32_t)state->r[rs]) != (int64_t)((int32_t)state->r[rt])){	//Overflooooooow
		state->error = 1;
		doflag = 0;
	}
	if(doflag){
		state->r_lo = (uint32_t)result;		//Cast and write the thingy to r_lo
		state->r_hi = (uint32_t)(result >> 32);
	}	//Only do the writing if there's no error
	advance_pc(state, 4);
	return;
}

void MULTU(struct mips_state_t *state, uint8_t rs, uint8_t rt){
	state->r_lo = state->r[rs] * state->r[rt];
	advance_pc(state, 4);
	return;
}

void OR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rs] | state->r[rt];
	advance_pc(state, 4);
	return;
}

void SUB(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	//ALL THE THINGS ARE SIGNED AND ALSO OVERFLOW ARGH
	int64_t result = 0;
	uint64_t temp = 0;
	int doflag = 1;
	result = (int64_t)((int32_t)state->r[rs]) - (int64_t)((int32_t)state->r[rt]);	//Do it all signed
	//Now check for overflow....
	if(result >= 0){	//Positive, check without finding magnitude
		if((uint64_t)result > 0x000000007FFFFFFF){
			state->error = 1;
			doflag = 0;
		}
	}
	else{				//result is negative, so find magnitude then check
		temp = ~((uint64_t)result);
		temp++;								//Twos complementing
		temp = temp & 0x7FFFFFFFFFFFFFFF;	//Mask out the sign bit for, uh, good measure
		if(temp > 0x000000007FFFFFFF){		//Overflow is a thing
			state->error = 1;
			doflag = 0;
		}
	}
	if(doflag){
		state->r[rd] = (int32_t)result; 
	}
	advance_pc(state, 4);
	return;
}

void SUBU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rs] - state->r[rt];
	advance_pc(state, 4);
}

void SLT(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	if((int32_t)state->r[rs] < (int32_t)state->r[rt]){		//Do a signed comparison
		//if $s is less than $t, then $d = 1.
		state->r[rd] = 1;
	}
	else{
		state->r[rd] = 0;
	}
	advance_pc(state, 4);
	return;
}

void SLTU(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	if(state->r[rs] < state->r[rt]){
		state->r[rd] = 1;
	}
	else{
		state->r[rd] = 0;
	}
	advance_pc(state, 4);
	return;
}

void XOR(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rs] ^ state->r[rt];
	advance_pc(state, 4);
	return;
}

void SRA(struct mips_state_t *state, uint8_t rt, uint8_t rd, uint8_t shift){
	
	uint32_t masker = 0xFFFFFFFF;
	state->r[rd] = state->r[rt] >> shift;
	if((int32_t)state->r[rt] < 0){
		masker = masker << 32 - shift;
		state->r[rd] = state->r[rd] | masker;
	}
	advance_pc(state, 4);
	return;
}

void SRLV(struct mips_state_t *state, uint8_t rs, uint8_t rt, uint8_t rd){
	state->r[rd] = state->r[rt] >> state->r[rs];
	advance_pc(state, 4);
	return;
}


/**********************J TYPE************************/

void J(struct mips_state_t *state, uint32_t instruction){
	//Extract 26 address for jump: bits [25:0]
	uint32_t address = 0;
	state->pc = state->npc;
	address = instruction & 0x03FFFFFF;	//Masking out the opcode
	address = address << 2;	//Doing this because this increments in 4s, so user can't end up halfway through a word
	state->npc = (state->pc & 0xF0000000) | address;
}

void JAL(struct mips_state_t *state, uint32_t instruction){
	uint32_t address = 0;
	state->r[31] = state->npc + 4;		//Stores the return address
	state->pc = state->npc;
	address = instruction & 0x03FFFFFF;
	address = address << 2;		//Exactly the same deal as J, but we store the return address in r[31].
	state->npc = (state->pc & 0xF0000000) | address;
	return;
}

