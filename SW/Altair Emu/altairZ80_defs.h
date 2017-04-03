/*	altairZ80_defs.h: MITS Altair simulator definitions
		Written by Peter Schorn, 2001
		Based on work by Charles E Owen ((c) 1997, Commercial use prohibited)
*/

#include "sim_defs.h"													/* simulator definitions					*/

/* Memory */
#define MAXMEMSIZE			65536									/* max memory size								*/
#define MAXBANKS				4											/* max number of memory banks			*/
#define KB							1024									/* kilo byte											*/
#define MEMSIZE					(cpu_unit.capac)			/* actual memory size							*/
#define ADDRMASK				(MAXMEMSIZE - 1)			/* address mask										*/
#define bootrom_size		256										/* size of boot rom								*/
#define bootrom_origin	0xFF00								/* start address of boot rom			*/
#define dsk_boot_origin	0xFE00								/* load address for dsk boot code	*/

#define BACKSPACE_CHAR	0x08									/* backspace character						*/
#define DELETE_CHAR			0x7f									/* delete character								*/
#define CONTROLZ_CHAR		26										/* control Z character						*/

/* Simulator stop codes */
#define STOP_HALT				2											/* HALT														*/
#define STOP_IBKPT			3											/* breakpoint											*/
#define STOP_OPCODE			4

#define UNIT_V_OPSTOP		(UNIT_V_UF)						/* Stop on Invalid OP?						*/
#define UNIT_OPSTOP			(1 << UNIT_V_OPSTOP)
#define UNIT_V_CHIP	 		(UNIT_V_UF+1)					/* 8080 or Z80										*/
#define UNIT_CHIP				(1 << UNIT_V_CHIP)
#define UNIT_V_MSIZE		(UNIT_V_UF+2)					/* Memory Size										*/
#define UNIT_MSIZE			(1 << UNIT_V_MSIZE)
#define UNIT_V_BANKED		(UNIT_V_UF+3)					/* Banked memory is used					*/
#define UNIT_BANKED			(1 << UNIT_V_BANKED)

/* data structure for IN/OUT instructions */
struct idev {
	int32 (*routine)(int32, int32, int32);
};

/*	The Default is to use "inline". In this case the wrapper functions for
		GetBYTE and PutBYTE need to be created. Otherwise they are not needed
		and the calls map to the original functions.																*/
#ifdef NO_INLINE
#define INLINE
#define GetBYTEWrapper GetBYTE
#define PutBYTEWrapper PutBYTE
#else
#define INLINE inline
#endif
