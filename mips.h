#ifndef mips_h
#define mips_h

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

struct mips_state_t;

/*! Initialises state so that the addressable memory is bound
	to pMem, the processor has just been reset, and the next
	instruction to be executed is located at pc. The memory
	pointer to by pMem is guaranteed to remain valid until
	the corresponding call to mips_free.
*/
struct mips_state_t *mips_create(
	uint32_t pc,      //! Address of first instruction
	unsigned sizeMem, //! Number of addressable bytes
	uint8_t *pMem	  //! Pointer to sizeMem bytes
);

/*! Takes an existing state, and resets the registers.
	Should be equivalent to a state just returned from
	mips_create */
void mips_reset(struct mips_state_t *state, uint32_t pc);

/*! Returns the current value of one of the 32 general
	purpose MIPS registers.*/
uint32_t mips_get_register(struct mips_state_t *state, unsigned index);

//Bunch of getters for pc, r_hi and r_lo
uint32_t mips_get_pc(struct mips_state_t *state);
uint32_t mips_get_r_hi(struct mips_state_t *state);
uint32_t mips_get_r_lo(struct mips_state_t *state);
void mips_reset_hilo(struct mips_state_t *state);

/*! Modifies one of the 32 general purpose MIPS registers. */
void mips_set_register(struct mips_state_t *state, unsigned index, uint32_t value);

/*! Advances the processor by one instruction. If no error
	occured, then return zero. For anything
	which stops execution, return non-zero. */
int mips_step(struct mips_state_t *state);

/*! Free all resources associated with state. */
void mips_free(struct mips_state_t *state);

#ifdef __cplusplus
};
#endif

#endif
