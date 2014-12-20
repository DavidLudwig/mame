
#include "emu.h"
#include "debugger.h"
#include "arcompact.h"
#include "arcompact_common.h"

#define ARCOMPACT_LOGGING 0

#define arcompact_fatal if (ARCOMPACT_LOGGING) fatalerror
#define arcompact_log if (ARCOMPACT_LOGGING) fatalerror


void arcompact_device::execute_run()
{
	//UINT32 lres;
	//lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(this, m_pc<<1);

//		printf("new pc %04x\n", m_pc);

		if (m_delayactive)
		{
			UINT16 op = READ16((m_pc + 0) >> 1);
			m_pc = get_insruction(op);
			if (m_delaylinks) m_regs[REG_BLINK] = m_pc;

			m_pc = m_delayjump;
			m_delayactive = 0; m_delaylinks = 0;
		}
		else
		{
			UINT16 op = READ16((m_pc + 0) >> 1);
			m_pc = get_insruction(op);
		}

		m_icount--;
	}

}


#define GET_01_01_01_BRANCH_ADDR \
	INT32 address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f); \


#define GROUP_0e_GET_h \
	h =  ((op & 0x0007) << 3); \
    h |= ((op & 0x00e0) >> 5); \

#define COMMON32_GET_breg \
	int b_temp = (op & 0x07000000) >> 24; \
	int B_temp = (op & 0x00007000) >> 12; \
	int breg = b_temp | (B_temp << 3); \

#define COMMON32_GET_s12 \
		int S_temp = (op & 0x0000003f) >> 0; \
		int s_temp = (op & 0x00000fc0) >> 6; \
		int S = s_temp | (S_temp<<6); \

#define COMMON32_GET_CONDITION \
		UINT8 condition = op & 0x0000001f;


#define COMMON16_GET_breg \
	breg =  ((op & 0x0700) >>8); \

#define COMMON16_GET_creg \
	creg =  ((op & 0x00e0) >>5); \

#define COMMON16_GET_areg \
	areg =  ((op & 0x0007) >>0); \

#define COMMON16_GET_u3 \
	u =  ((op & 0x0007) >>0); \

#define COMMON16_GET_u5 \
	u =  ((op & 0x001f) >>0); \

#define COMMON16_GET_u8 \
	u =  ((op & 0x00ff) >>0); \

#define COMMON16_GET_u7 \
	u =  ((op & 0x007f) >>0); \

#define COMMON16_GET_s9 \
	s =  ((op & 0x01ff) >>0); \

// registers used in 16-bit opcodes hae a limited range
// and can only address registers r0-r3 and r12-r15

#define REG_16BIT_RANGE(_reg_) \
	if (_reg_>3) _reg_+= 8; \


#define GET_LIMM_32 \
	limm = (READ16((m_pc + 4) >> 1) << 16); \
	limm |= READ16((m_pc + 6) >> 1); \



#define PC_ALIGNED32 \
	(m_pc&0xfffffffc)


ARCOMPACT_RETTYPE arcompact_device::get_insruction(OPS_32)
{
	UINT8 instruction = ARCOMPACT_OPERATION;

	if (instruction < 0x0c)
	{
		op <<= 16;
		op |= READ16((m_pc + 2) >> 1);

		switch (instruction) // 32-bit instructions (with optional extra dword for immediate data)
		{
			case 0x00: return arcompact_handle00(PARAMS);	break; // Bcc
			case 0x01: return arcompact_handle01(PARAMS);	break; // BLcc/BRcc
			case 0x02: return arcompact_handle02(PARAMS);	break; // LD r+o
			case 0x03: return arcompact_handle03(PARAMS);	break; // ST r+o
			case 0x04: return arcompact_handle04(PARAMS);	break; // op a,b,c (basecase)
			case 0x05: return arcompact_handle05(PARAMS);	break; // op a,b,c (05 ARC ext)
			case 0x06: return arcompact_handle06(PARAMS);	break; // op a,b,c (06 ARC ext)
			case 0x07: return arcompact_handle07(PARAMS);	break; // op a,b,c (07 User ext)
			case 0x08: return arcompact_handle08(PARAMS);	break; // op a,b,c (08 User ext)
			case 0x09: return arcompact_handle09(PARAMS);	break; // op a,b,c (09 Market ext)
			case 0x0a: return arcompact_handle0a(PARAMS);	break; // op a,b,c (0a Market ext)
			case 0x0b: return arcompact_handle0b(PARAMS);	break; // op a,b,c (0b Market ext)
		}
	}
	else
	{
		switch (instruction) // 16-bit instructions
		{
			case 0x0c: return arcompact_handle0c(PARAMS);	break; // Load/Add reg-reg
			case 0x0d: return arcompact_handle0d(PARAMS);	break; // Add/Sub/Shft imm
			case 0x0e: return arcompact_handle0e(PARAMS);	break; // Mov/Cmp/Add
			case 0x0f: return arcompact_handle0f(PARAMS);	break; // op_S b,b,c (single 16-bit ops)
			case 0x10: return arcompact_handle10(PARAMS);	break; // LD_S
			case 0x11: return arcompact_handle11(PARAMS);	break; // LDB_S
			case 0x12: return arcompact_handle12(PARAMS);	break; // LDW_S
			case 0x13: return arcompact_handle13(PARAMS);	break; // LSW_S.X
			case 0x14: return arcompact_handle14(PARAMS);	break; // ST_S
			case 0x15: return arcompact_handle15(PARAMS);	break; // STB_S
			case 0x16: return arcompact_handle16(PARAMS);	break; // STW_S
			case 0x17: return arcompact_handle17(PARAMS);	break; // Shift/Sub/Bit
			case 0x18: return arcompact_handle18(PARAMS);	break; // Stack Instr
			case 0x19: return arcompact_handle19(PARAMS);	break; // GP Instr
			case 0x1a: return arcompact_handle1a(PARAMS);	break; // PCL Instr
			case 0x1b: return arcompact_handle1b(PARAMS);	break; // MOV_S
			case 0x1c: return arcompact_handle1c(PARAMS);	break; // ADD_S/CMP_S
			case 0x1d: return arcompact_handle1d(PARAMS);	break; // BRcc_S
			case 0x1e: return arcompact_handle1e(PARAMS);	break; // Bcc_S
			case 0x1f: return arcompact_handle1f(PARAMS);	break; // BL_S
		}
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00(OPS_32)
{
	UINT8 subinstr = (op & 0x00010000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle00_00(PARAMS); break; // Branch Conditionally
		case 0x01: return arcompact_handle00_01(PARAMS); break; // Branch Unconditionally Far
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01(OPS_32)
{
	UINT8 subinstr = (op & 0x00010000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle01_00(PARAMS); break; // Branh & Link
		case 0x01: return arcompact_handle01_01(PARAMS); break; // Branch on Compare
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00(OPS_32)
{
	UINT8 subinstr2 = (op & 0x00020000) >> 17;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle01_00_00dasm(PARAMS); break; // Branch and Link Conditionally
		case 0x01: return arcompact_handle01_00_01dasm(PARAMS); break; // Branch and Link Unconditional Far
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01(OPS_32)
{
	UINT8 subinstr2 = (op & 0x00000010) >> 4;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle01_01_00(PARAMS); break; // Branch on Compare Register-Register
		case 0x01: return arcompact_handle01_01_01(PARAMS); break; // Branch on Compare/Bit Test Register-Immediate
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00(OPS_32)
{
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle01_01_00_00(PARAMS); break; // BREQ (reg-reg)
		case 0x01: return arcompact_handle01_01_00_01(PARAMS); break; // BRNE (reg-reg)
		case 0x02: return arcompact_handle01_01_00_02(PARAMS); break; // BRLT (reg-reg)
		case 0x03: return arcompact_handle01_01_00_03(PARAMS); break; // BRGE (reg-reg)
		case 0x04: return arcompact_handle01_01_00_04(PARAMS); break; // BRLO (reg-reg)
		case 0x05: return arcompact_handle01_01_00_05(PARAMS); break; // BRHS (reg-reg)
		case 0x06: return arcompact_handle01_01_00_06(PARAMS); break; // reserved
		case 0x07: return arcompact_handle01_01_00_07(PARAMS); break; // reserved
		case 0x08: return arcompact_handle01_01_00_08(PARAMS); break; // reserved
		case 0x09: return arcompact_handle01_01_00_09(PARAMS); break; // reserved
		case 0x0a: return arcompact_handle01_01_00_0a(PARAMS); break; // reserved
		case 0x0b: return arcompact_handle01_01_00_0b(PARAMS); break; // reserved
		case 0x0c: return arcompact_handle01_01_00_0c(PARAMS); break; // reserved
		case 0x0d: return arcompact_handle01_01_00_0d(PARAMS); break; // reserved
		case 0x0e: return arcompact_handle01_01_00_0e(PARAMS); break; // BBIT0 (reg-reg)
		case 0x0f: return arcompact_handle01_01_00_0f(PARAMS); break; // BBIT1 (reg-reg)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01(OPS_32) //  Branch on Compare/Bit Test Register-Immediate
{
	UINT8 subinstr3 = (op & 0x0000000f) >> 0;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle01_01_01_00(PARAMS); break; // BREQ (reg-imm)
		case 0x01: return arcompact_handle01_01_01_01(PARAMS); break; // BRNE (reg-imm)
		case 0x02: return arcompact_handle01_01_01_02(PARAMS); break; // BRLT (reg-imm)
		case 0x03: return arcompact_handle01_01_01_03(PARAMS); break; // BRGE (reg-imm)
		case 0x04: return arcompact_handle01_01_01_04(PARAMS); break; // BRLO (reg-imm)
		case 0x05: return arcompact_handle01_01_01_05(PARAMS); break; // BRHS (reg-imm)
		case 0x06: return arcompact_handle01_01_01_06(PARAMS); break; // reserved
		case 0x07: return arcompact_handle01_01_01_07(PARAMS); break; // reserved
		case 0x08: return arcompact_handle01_01_01_08(PARAMS); break; // reserved
		case 0x09: return arcompact_handle01_01_01_09(PARAMS); break; // reserved
		case 0x0a: return arcompact_handle01_01_01_0a(PARAMS); break; // reserved
		case 0x0b: return arcompact_handle01_01_01_0b(PARAMS); break; // reserved
		case 0x0c: return arcompact_handle01_01_01_0c(PARAMS); break; // reserved
		case 0x0d: return arcompact_handle01_01_01_0d(PARAMS); break; // reserved
		case 0x0e: return arcompact_handle01_01_01_0e(PARAMS); break; // BBIT0 (reg-imm)
		case 0x0f: return arcompact_handle01_01_01_0f(PARAMS); break; // BBIT1 (reg-imm)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04(OPS_32)
{
	UINT8 subinstr = (op & 0x003f0000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle04_00(PARAMS); break; // ADD
		case 0x01: return arcompact_handle04_01(PARAMS); break; // ADC
		case 0x02: return arcompact_handle04_02(PARAMS); break; // SUB
		case 0x03: return arcompact_handle04_03(PARAMS); break; // SBC
		case 0x04: return arcompact_handle04_04(PARAMS); break; // AND
		case 0x05: return arcompact_handle04_05(PARAMS); break; // OR
		case 0x06: return arcompact_handle04_06(PARAMS); break; // BIC
		case 0x07: return arcompact_handle04_07(PARAMS); break; // XOR
		case 0x08: return arcompact_handle04_08(PARAMS); break; // MAX
		case 0x09: return arcompact_handle04_09(PARAMS); break; // MIN
		case 0x0a: return arcompact_handle04_0a(PARAMS); break; // MOV
		case 0x0b: return arcompact_handle04_0b(PARAMS); break; // TST
		case 0x0c: return arcompact_handle04_0c(PARAMS); break; // CMP
		case 0x0d: return arcompact_handle04_0d(PARAMS); break; // RCMP
		case 0x0e: return arcompact_handle04_0e(PARAMS); break; // RSUB
		case 0x0f: return arcompact_handle04_0f(PARAMS); break; // BSET
		case 0x10: return arcompact_handle04_10(PARAMS); break; // BCLR
		case 0x11: return arcompact_handle04_11(PARAMS); break; // BTST
		case 0x12: return arcompact_handle04_12(PARAMS); break; // BXOR
		case 0x13: return arcompact_handle04_13(PARAMS); break; // BMSK
		case 0x14: return arcompact_handle04_14(PARAMS); break; // ADD1
		case 0x15: return arcompact_handle04_15(PARAMS); break; // ADD2
		case 0x16: return arcompact_handle04_16(PARAMS); break; // ADD3
		case 0x17: return arcompact_handle04_17(PARAMS); break; // SUB1
		case 0x18: return arcompact_handle04_18(PARAMS); break; // SUB2
		case 0x19: return arcompact_handle04_19(PARAMS); break; // SUB3
		case 0x1a: return arcompact_handle04_1a(PARAMS); break; // MPY *
		case 0x1b: return arcompact_handle04_1b(PARAMS); break; // MPYH *
		case 0x1c: return arcompact_handle04_1c(PARAMS); break; // MPYHU *
		case 0x1d: return arcompact_handle04_1d(PARAMS); break; // MPYU *
		case 0x1e: return arcompact_handle04_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle04_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle04_20(PARAMS); break; // Jcc
		case 0x21: return arcompact_handle04_21(PARAMS); break; // Jcc.D
		case 0x22: return arcompact_handle04_22(PARAMS); break; // JLcc
		case 0x23: return arcompact_handle04_23(PARAMS); break; // JLcc.D
		case 0x24: return arcompact_handle04_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle04_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle04_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle04_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle04_28(PARAMS); break; // LPcc
		case 0x29: return arcompact_handle04_29(PARAMS); break; // FLAG
		case 0x2a: return arcompact_handle04_2a(PARAMS); break; // LR
		case 0x2b: return arcompact_handle04_2b(PARAMS); break; // SR
		case 0x2c: return arcompact_handle04_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle04_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle04_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle04_2f(PARAMS); break; // Sub Opcode
		case 0x30: return arcompact_handle04_30(PARAMS); break; // LD r-r
		case 0x31: return arcompact_handle04_31(PARAMS); break; // LD r-r
		case 0x32: return arcompact_handle04_32(PARAMS); break; // LD r-r
		case 0x33: return arcompact_handle04_33(PARAMS); break; // LD r-r
		case 0x34: return arcompact_handle04_34(PARAMS); break; // LD r-r
		case 0x35: return arcompact_handle04_35(PARAMS); break; // LD r-r
		case 0x36: return arcompact_handle04_36(PARAMS); break; // LD r-r
		case 0x37: return arcompact_handle04_37(PARAMS); break; // LD r-r
		case 0x38: return arcompact_handle04_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle04_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle04_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle04_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle04_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle04_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle04_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle04_3f(PARAMS); break; // illegal
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f(OPS_32)
{
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle04_2f_00(PARAMS); break; // ASL
		case 0x01: return arcompact_handle04_2f_01(PARAMS); break; // ASR
		case 0x02: return arcompact_handle04_2f_02(PARAMS); break; // LSR
		case 0x03: return arcompact_handle04_2f_03(PARAMS); break; // ROR
		case 0x04: return arcompact_handle04_2f_04(PARAMS); break; // RCC
		case 0x05: return arcompact_handle04_2f_05(PARAMS); break; // SEXB
		case 0x06: return arcompact_handle04_2f_06(PARAMS); break; // SEXW
		case 0x07: return arcompact_handle04_2f_07(PARAMS); break; // EXTB
		case 0x08: return arcompact_handle04_2f_08(PARAMS); break; // EXTW
		case 0x09: return arcompact_handle04_2f_09(PARAMS); break; // ABS
		case 0x0a: return arcompact_handle04_2f_0a(PARAMS); break; // NOT
		case 0x0b: return arcompact_handle04_2f_0b(PARAMS); break; // RLC
		case 0x0c: return arcompact_handle04_2f_0c(PARAMS); break; // EX
		case 0x0d: return arcompact_handle04_2f_0d(PARAMS); break; // illegal
		case 0x0e: return arcompact_handle04_2f_0e(PARAMS); break; // illegal
		case 0x0f: return arcompact_handle04_2f_0f(PARAMS); break; // illegal
		case 0x10: return arcompact_handle04_2f_10(PARAMS); break; // illegal
		case 0x11: return arcompact_handle04_2f_11(PARAMS); break; // illegal
		case 0x12: return arcompact_handle04_2f_12(PARAMS); break; // illegal
		case 0x13: return arcompact_handle04_2f_13(PARAMS); break; // illegal
		case 0x14: return arcompact_handle04_2f_14(PARAMS); break; // illegal
		case 0x15: return arcompact_handle04_2f_15(PARAMS); break; // illegal
		case 0x16: return arcompact_handle04_2f_16(PARAMS); break; // illegal
		case 0x17: return arcompact_handle04_2f_17(PARAMS); break; // illegal
		case 0x18: return arcompact_handle04_2f_18(PARAMS); break; // illegal
		case 0x19: return arcompact_handle04_2f_19(PARAMS); break; // illegal
		case 0x1a: return arcompact_handle04_2f_1a(PARAMS); break; // illegal
		case 0x1b: return arcompact_handle04_2f_1b(PARAMS); break; // illegal
		case 0x1c: return arcompact_handle04_2f_1c(PARAMS); break; // illegal
		case 0x1d: return arcompact_handle04_2f_1d(PARAMS); break; // illegal
		case 0x1e: return arcompact_handle04_2f_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle04_2f_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle04_2f_20(PARAMS); break; // illegal
		case 0x21: return arcompact_handle04_2f_21(PARAMS); break; // illegal
		case 0x22: return arcompact_handle04_2f_22(PARAMS); break; // illegal
		case 0x23: return arcompact_handle04_2f_23(PARAMS); break; // illegal
		case 0x24: return arcompact_handle04_2f_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle04_2f_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle04_2f_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle04_2f_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle04_2f_28(PARAMS); break; // illegal
		case 0x29: return arcompact_handle04_2f_29(PARAMS); break; // illegal
		case 0x2a: return arcompact_handle04_2f_2a(PARAMS); break; // illegal
		case 0x2b: return arcompact_handle04_2f_2b(PARAMS); break; // illegal
		case 0x2c: return arcompact_handle04_2f_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle04_2f_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle04_2f_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle04_2f_2f(PARAMS); break; // illegal
		case 0x30: return arcompact_handle04_2f_30(PARAMS); break; // illegal
		case 0x31: return arcompact_handle04_2f_31(PARAMS); break; // illegal
		case 0x32: return arcompact_handle04_2f_32(PARAMS); break; // illegal
		case 0x33: return arcompact_handle04_2f_33(PARAMS); break; // illegal
		case 0x34: return arcompact_handle04_2f_34(PARAMS); break; // illegal
		case 0x35: return arcompact_handle04_2f_35(PARAMS); break; // illegal
		case 0x36: return arcompact_handle04_2f_36(PARAMS); break; // illegal
		case 0x37: return arcompact_handle04_2f_37(PARAMS); break; // illegal
		case 0x38: return arcompact_handle04_2f_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle04_2f_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle04_2f_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle04_2f_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle04_2f_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle04_2f_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle04_2f_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle04_2f_3f(PARAMS); break; // ZOPs (Zero Operand Opcodes)
	}

	return 0;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f(OPS_32)
{
	UINT8 subinstr2 = (op & 0x0000003f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle05_2f_00(PARAMS); break; // SWAP 
		case 0x01: return arcompact_handle05_2f_01(PARAMS); break; // NORM 
		case 0x02: return arcompact_handle05_2f_02(PARAMS); break; // SAT16
		case 0x03: return arcompact_handle05_2f_03(PARAMS); break; // RND16 
		case 0x04: return arcompact_handle05_2f_04(PARAMS); break; // ABSSW 
		case 0x05: return arcompact_handle05_2f_05(PARAMS); break; // ABSS 
		case 0x06: return arcompact_handle05_2f_06(PARAMS); break; // NEGSW 
		case 0x07: return arcompact_handle05_2f_07(PARAMS); break; // NEGS 
		case 0x08: return arcompact_handle05_2f_08(PARAMS); break; // NORMW 
		case 0x09: return arcompact_handle05_2f_09(PARAMS); break; // illegal
		case 0x0a: return arcompact_handle05_2f_0a(PARAMS); break; // illegal
		case 0x0b: return arcompact_handle05_2f_0b(PARAMS); break; // illegal
		case 0x0c: return arcompact_handle05_2f_0c(PARAMS); break; // illegal
		case 0x0d: return arcompact_handle05_2f_0d(PARAMS); break; // illegal
		case 0x0e: return arcompact_handle05_2f_0e(PARAMS); break; // illegal
		case 0x0f: return arcompact_handle05_2f_0f(PARAMS); break; // illegal
		case 0x10: return arcompact_handle05_2f_10(PARAMS); break; // illegal
		case 0x11: return arcompact_handle05_2f_11(PARAMS); break; // illegal
		case 0x12: return arcompact_handle05_2f_12(PARAMS); break; // illegal
		case 0x13: return arcompact_handle05_2f_13(PARAMS); break; // illegal
		case 0x14: return arcompact_handle05_2f_14(PARAMS); break; // illegal
		case 0x15: return arcompact_handle05_2f_15(PARAMS); break; // illegal
		case 0x16: return arcompact_handle05_2f_16(PARAMS); break; // illegal
		case 0x17: return arcompact_handle05_2f_17(PARAMS); break; // illegal
		case 0x18: return arcompact_handle05_2f_18(PARAMS); break; // illegal
		case 0x19: return arcompact_handle05_2f_19(PARAMS); break; // illegal
		case 0x1a: return arcompact_handle05_2f_1a(PARAMS); break; // illegal
		case 0x1b: return arcompact_handle05_2f_1b(PARAMS); break; // illegal
		case 0x1c: return arcompact_handle05_2f_1c(PARAMS); break; // illegal
		case 0x1d: return arcompact_handle05_2f_1d(PARAMS); break; // illegal
		case 0x1e: return arcompact_handle05_2f_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle05_2f_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle05_2f_20(PARAMS); break; // illegal
		case 0x21: return arcompact_handle05_2f_21(PARAMS); break; // illegal
		case 0x22: return arcompact_handle05_2f_22(PARAMS); break; // illegal
		case 0x23: return arcompact_handle05_2f_23(PARAMS); break; // illegal
		case 0x24: return arcompact_handle05_2f_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle05_2f_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle05_2f_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle05_2f_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle05_2f_28(PARAMS); break; // illegal
		case 0x29: return arcompact_handle05_2f_29(PARAMS); break; // illegal
		case 0x2a: return arcompact_handle05_2f_2a(PARAMS); break; // illegal
		case 0x2b: return arcompact_handle05_2f_2b(PARAMS); break; // illegal
		case 0x2c: return arcompact_handle05_2f_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle05_2f_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle05_2f_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle05_2f_2f(PARAMS); break; // illegal
		case 0x30: return arcompact_handle05_2f_30(PARAMS); break; // illegal
		case 0x31: return arcompact_handle05_2f_31(PARAMS); break; // illegal
		case 0x32: return arcompact_handle05_2f_32(PARAMS); break; // illegal
		case 0x33: return arcompact_handle05_2f_33(PARAMS); break; // illegal
		case 0x34: return arcompact_handle05_2f_34(PARAMS); break; // illegal
		case 0x35: return arcompact_handle05_2f_35(PARAMS); break; // illegal
		case 0x36: return arcompact_handle05_2f_36(PARAMS); break; // illegal
		case 0x37: return arcompact_handle05_2f_37(PARAMS); break; // illegal
		case 0x38: return arcompact_handle05_2f_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle05_2f_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle05_2f_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle05_2f_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle05_2f_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle05_2f_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle05_2f_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle05_2f_3f(PARAMS); break; // ZOPs (Zero Operand Opcodes)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f(OPS_32)
{
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle04_2f_3f_00(PARAMS); break; // illegal
		case 0x01: return arcompact_handle04_2f_3f_01(PARAMS); break; // SLEEP
		case 0x02: return arcompact_handle04_2f_3f_02(PARAMS); break; // SWI / TRAP9
		case 0x03: return arcompact_handle04_2f_3f_03(PARAMS); break; // SYNC
		case 0x04: return arcompact_handle04_2f_3f_04(PARAMS); break; // RTIE
		case 0x05: return arcompact_handle04_2f_3f_05(PARAMS); break; // BRK
		case 0x06: return arcompact_handle04_2f_3f_06(PARAMS); break; // illegal
		case 0x07: return arcompact_handle04_2f_3f_07(PARAMS); break; // illegal
		case 0x08: return arcompact_handle04_2f_3f_08(PARAMS); break; // illegal
		case 0x09: return arcompact_handle04_2f_3f_09(PARAMS); break; // illegal
		case 0x0a: return arcompact_handle04_2f_3f_0a(PARAMS); break; // illegal
		case 0x0b: return arcompact_handle04_2f_3f_0b(PARAMS); break; // illegal
		case 0x0c: return arcompact_handle04_2f_3f_0c(PARAMS); break; // illegal
		case 0x0d: return arcompact_handle04_2f_3f_0d(PARAMS); break; // illegal
		case 0x0e: return arcompact_handle04_2f_3f_0e(PARAMS); break; // illegal
		case 0x0f: return arcompact_handle04_2f_3f_0f(PARAMS); break; // illegal
		case 0x10: return arcompact_handle04_2f_3f_10(PARAMS); break; // illegal
		case 0x11: return arcompact_handle04_2f_3f_11(PARAMS); break; // illegal
		case 0x12: return arcompact_handle04_2f_3f_12(PARAMS); break; // illegal
		case 0x13: return arcompact_handle04_2f_3f_13(PARAMS); break; // illegal
		case 0x14: return arcompact_handle04_2f_3f_14(PARAMS); break; // illegal
		case 0x15: return arcompact_handle04_2f_3f_15(PARAMS); break; // illegal
		case 0x16: return arcompact_handle04_2f_3f_16(PARAMS); break; // illegal
		case 0x17: return arcompact_handle04_2f_3f_17(PARAMS); break; // illegal
		case 0x18: return arcompact_handle04_2f_3f_18(PARAMS); break; // illegal
		case 0x19: return arcompact_handle04_2f_3f_19(PARAMS); break; // illegal
		case 0x1a: return arcompact_handle04_2f_3f_1a(PARAMS); break; // illegal
		case 0x1b: return arcompact_handle04_2f_3f_1b(PARAMS); break; // illegal
		case 0x1c: return arcompact_handle04_2f_3f_1c(PARAMS); break; // illegal
		case 0x1d: return arcompact_handle04_2f_3f_1d(PARAMS); break; // illegal
		case 0x1e: return arcompact_handle04_2f_3f_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle04_2f_3f_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle04_2f_3f_20(PARAMS); break; // illegal
		case 0x21: return arcompact_handle04_2f_3f_21(PARAMS); break; // illegal
		case 0x22: return arcompact_handle04_2f_3f_22(PARAMS); break; // illegal
		case 0x23: return arcompact_handle04_2f_3f_23(PARAMS); break; // illegal
		case 0x24: return arcompact_handle04_2f_3f_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle04_2f_3f_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle04_2f_3f_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle04_2f_3f_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle04_2f_3f_28(PARAMS); break; // illegal
		case 0x29: return arcompact_handle04_2f_3f_29(PARAMS); break; // illegal
		case 0x2a: return arcompact_handle04_2f_3f_2a(PARAMS); break; // illegal
		case 0x2b: return arcompact_handle04_2f_3f_2b(PARAMS); break; // illegal
		case 0x2c: return arcompact_handle04_2f_3f_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle04_2f_3f_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle04_2f_3f_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle04_2f_3f_2f(PARAMS); break; // illegal
		case 0x30: return arcompact_handle04_2f_3f_30(PARAMS); break; // illegal
		case 0x31: return arcompact_handle04_2f_3f_31(PARAMS); break; // illegal
		case 0x32: return arcompact_handle04_2f_3f_32(PARAMS); break; // illegal
		case 0x33: return arcompact_handle04_2f_3f_33(PARAMS); break; // illegal
		case 0x34: return arcompact_handle04_2f_3f_34(PARAMS); break; // illegal
		case 0x35: return arcompact_handle04_2f_3f_35(PARAMS); break; // illegal
		case 0x36: return arcompact_handle04_2f_3f_36(PARAMS); break; // illegal
		case 0x37: return arcompact_handle04_2f_3f_37(PARAMS); break; // illegal
		case 0x38: return arcompact_handle04_2f_3f_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle04_2f_3f_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle04_2f_3f_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle04_2f_3f_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle04_2f_3f_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle04_2f_3f_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle04_2f_3f_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle04_2f_3f_3f(PARAMS); break; // illegal
	}

	return 0;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f(OPS_32) // useless ZOP group, no actual opcodes
{
	UINT8 subinstr3 = (op & 0x07000000) >> 24;
	subinstr3 |= ((op & 0x00007000) >> 12) << 3;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle05_2f_3f_00(PARAMS); break; // illegal
		case 0x01: return arcompact_handle05_2f_3f_01(PARAMS); break; // illegal
		case 0x02: return arcompact_handle05_2f_3f_02(PARAMS); break; // illegal
		case 0x03: return arcompact_handle05_2f_3f_03(PARAMS); break; // illegal
		case 0x04: return arcompact_handle05_2f_3f_04(PARAMS); break; // illegal
		case 0x05: return arcompact_handle05_2f_3f_05(PARAMS); break; // illegal
		case 0x06: return arcompact_handle05_2f_3f_06(PARAMS); break; // illegal
		case 0x07: return arcompact_handle05_2f_3f_07(PARAMS); break; // illegal
		case 0x08: return arcompact_handle05_2f_3f_08(PARAMS); break; // illegal
		case 0x09: return arcompact_handle05_2f_3f_09(PARAMS); break; // illegal
		case 0x0a: return arcompact_handle05_2f_3f_0a(PARAMS); break; // illegal
		case 0x0b: return arcompact_handle05_2f_3f_0b(PARAMS); break; // illegal
		case 0x0c: return arcompact_handle05_2f_3f_0c(PARAMS); break; // illegal
		case 0x0d: return arcompact_handle05_2f_3f_0d(PARAMS); break; // illegal
		case 0x0e: return arcompact_handle05_2f_3f_0e(PARAMS); break; // illegal
		case 0x0f: return arcompact_handle05_2f_3f_0f(PARAMS); break; // illegal
		case 0x10: return arcompact_handle05_2f_3f_10(PARAMS); break; // illegal
		case 0x11: return arcompact_handle05_2f_3f_11(PARAMS); break; // illegal
		case 0x12: return arcompact_handle05_2f_3f_12(PARAMS); break; // illegal
		case 0x13: return arcompact_handle05_2f_3f_13(PARAMS); break; // illegal
		case 0x14: return arcompact_handle05_2f_3f_14(PARAMS); break; // illegal
		case 0x15: return arcompact_handle05_2f_3f_15(PARAMS); break; // illegal
		case 0x16: return arcompact_handle05_2f_3f_16(PARAMS); break; // illegal
		case 0x17: return arcompact_handle05_2f_3f_17(PARAMS); break; // illegal
		case 0x18: return arcompact_handle05_2f_3f_18(PARAMS); break; // illegal
		case 0x19: return arcompact_handle05_2f_3f_19(PARAMS); break; // illegal
		case 0x1a: return arcompact_handle05_2f_3f_1a(PARAMS); break; // illegal
		case 0x1b: return arcompact_handle05_2f_3f_1b(PARAMS); break; // illegal
		case 0x1c: return arcompact_handle05_2f_3f_1c(PARAMS); break; // illegal
		case 0x1d: return arcompact_handle05_2f_3f_1d(PARAMS); break; // illegal
		case 0x1e: return arcompact_handle05_2f_3f_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle05_2f_3f_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle05_2f_3f_20(PARAMS); break; // illegal
		case 0x21: return arcompact_handle05_2f_3f_21(PARAMS); break; // illegal
		case 0x22: return arcompact_handle05_2f_3f_22(PARAMS); break; // illegal
		case 0x23: return arcompact_handle05_2f_3f_23(PARAMS); break; // illegal
		case 0x24: return arcompact_handle05_2f_3f_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle05_2f_3f_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle05_2f_3f_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle05_2f_3f_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle05_2f_3f_28(PARAMS); break; // illegal
		case 0x29: return arcompact_handle05_2f_3f_29(PARAMS); break; // illegal
		case 0x2a: return arcompact_handle05_2f_3f_2a(PARAMS); break; // illegal
		case 0x2b: return arcompact_handle05_2f_3f_2b(PARAMS); break; // illegal
		case 0x2c: return arcompact_handle05_2f_3f_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle05_2f_3f_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle05_2f_3f_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle05_2f_3f_2f(PARAMS); break; // illegal
		case 0x30: return arcompact_handle05_2f_3f_30(PARAMS); break; // illegal
		case 0x31: return arcompact_handle05_2f_3f_31(PARAMS); break; // illegal
		case 0x32: return arcompact_handle05_2f_3f_32(PARAMS); break; // illegal
		case 0x33: return arcompact_handle05_2f_3f_33(PARAMS); break; // illegal
		case 0x34: return arcompact_handle05_2f_3f_34(PARAMS); break; // illegal
		case 0x35: return arcompact_handle05_2f_3f_35(PARAMS); break; // illegal
		case 0x36: return arcompact_handle05_2f_3f_36(PARAMS); break; // illegal
		case 0x37: return arcompact_handle05_2f_3f_37(PARAMS); break; // illegal
		case 0x38: return arcompact_handle05_2f_3f_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle05_2f_3f_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle05_2f_3f_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle05_2f_3f_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle05_2f_3f_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle05_2f_3f_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle05_2f_3f_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle05_2f_3f_3f(PARAMS); break; // illegal
	}

	return 0;
}


// this is an Extension ALU group, maybe optional on some CPUs?
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05(OPS_32)
{
	UINT8 subinstr = (op & 0x003f0000) >> 16;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle05_00(PARAMS); break; // ASL
		case 0x01: return arcompact_handle05_01(PARAMS); break; // LSR
		case 0x02: return arcompact_handle05_02(PARAMS); break; // ASR
		case 0x03: return arcompact_handle05_03(PARAMS); break; // ROR
		case 0x04: return arcompact_handle05_04(PARAMS); break; // MUL64
		case 0x05: return arcompact_handle05_05(PARAMS); break; // MULU64
		case 0x06: return arcompact_handle05_06(PARAMS); break; // ADDS
		case 0x07: return arcompact_handle05_07(PARAMS); break; // SUBS
		case 0x08: return arcompact_handle05_08(PARAMS); break; // DIVAW
		case 0x09: return arcompact_handle05_09(PARAMS); break; // illegal
		case 0x0a: return arcompact_handle05_0a(PARAMS); break; // ASLS
		case 0x0b: return arcompact_handle05_0b(PARAMS); break; // ASRS
		case 0x0c: return arcompact_handle05_0c(PARAMS); break; // illegal
		case 0x0d: return arcompact_handle05_0d(PARAMS); break; // illegal
		case 0x0e: return arcompact_handle05_0e(PARAMS); break; // illegal
		case 0x0f: return arcompact_handle05_0f(PARAMS); break; // illegal
		case 0x10: return arcompact_handle05_10(PARAMS); break; // illegal
		case 0x11: return arcompact_handle05_11(PARAMS); break; // illegal
		case 0x12: return arcompact_handle05_12(PARAMS); break; // illegal
		case 0x13: return arcompact_handle05_13(PARAMS); break; // illegal
		case 0x14: return arcompact_handle05_14(PARAMS); break; // illegal
		case 0x15: return arcompact_handle05_15(PARAMS); break; // illegal
		case 0x16: return arcompact_handle05_16(PARAMS); break; // illegal
		case 0x17: return arcompact_handle05_17(PARAMS); break; // illegal
		case 0x18: return arcompact_handle05_18(PARAMS); break; // illegal
		case 0x19: return arcompact_handle05_19(PARAMS); break; // illegal
		case 0x1a: return arcompact_handle05_1a(PARAMS); break; // illegal
		case 0x1b: return arcompact_handle05_1b(PARAMS); break; // illegal
		case 0x1c: return arcompact_handle05_1c(PARAMS); break; // illegal
		case 0x1d: return arcompact_handle05_1d(PARAMS); break; // illegal
		case 0x1e: return arcompact_handle05_1e(PARAMS); break; // illegal
		case 0x1f: return arcompact_handle05_1f(PARAMS); break; // illegal
		case 0x20: return arcompact_handle05_20(PARAMS); break; // illegal
		case 0x21: return arcompact_handle05_21(PARAMS); break; // illegal
		case 0x22: return arcompact_handle05_22(PARAMS); break; // illegal
		case 0x23: return arcompact_handle05_23(PARAMS); break; // illegal
		case 0x24: return arcompact_handle05_24(PARAMS); break; // illegal
		case 0x25: return arcompact_handle05_25(PARAMS); break; // illegal
		case 0x26: return arcompact_handle05_26(PARAMS); break; // illegal
		case 0x27: return arcompact_handle05_27(PARAMS); break; // illegal
		case 0x28: return arcompact_handle05_28(PARAMS); break; // ADDSDW
		case 0x29: return arcompact_handle05_29(PARAMS); break; // SUBSDW
		case 0x2a: return arcompact_handle05_2a(PARAMS); break; // illegal
		case 0x2b: return arcompact_handle05_2b(PARAMS); break; // illegal
		case 0x2c: return arcompact_handle05_2c(PARAMS); break; // illegal
		case 0x2d: return arcompact_handle05_2d(PARAMS); break; // illegal
		case 0x2e: return arcompact_handle05_2e(PARAMS); break; // illegal
		case 0x2f: return arcompact_handle05_2f(PARAMS); break; // SOPs
		case 0x30: return arcompact_handle05_30(PARAMS); break; // illegal
		case 0x31: return arcompact_handle05_31(PARAMS); break; // illegal
		case 0x32: return arcompact_handle05_32(PARAMS); break; // illegal
		case 0x33: return arcompact_handle05_33(PARAMS); break; // illegal
		case 0x34: return arcompact_handle05_34(PARAMS); break; // illegal
		case 0x35: return arcompact_handle05_35(PARAMS); break; // illegal
		case 0x36: return arcompact_handle05_36(PARAMS); break; // illegal
		case 0x37: return arcompact_handle05_37(PARAMS); break; // illegal
		case 0x38: return arcompact_handle05_38(PARAMS); break; // illegal
		case 0x39: return arcompact_handle05_39(PARAMS); break; // illegal
		case 0x3a: return arcompact_handle05_3a(PARAMS); break; // illegal
		case 0x3b: return arcompact_handle05_3b(PARAMS); break; // illegal
		case 0x3c: return arcompact_handle05_3c(PARAMS); break; // illegal
		case 0x3d: return arcompact_handle05_3d(PARAMS); break; // illegal
		case 0x3e: return arcompact_handle05_3e(PARAMS); break; // illegal
		case 0x3f: return arcompact_handle05_3f(PARAMS); break; // illegal
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0c_00(PARAMS); break; // LD_S
		case 0x01: return arcompact_handle0c_01(PARAMS); break; // LDB_S
		case 0x02: return arcompact_handle0c_02(PARAMS); break; // LDW_S
		case 0x03: return arcompact_handle0c_03(PARAMS); break; // ADD_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0d_00(PARAMS); break; // ADD_S
		case 0x01: return arcompact_handle0d_01(PARAMS); break; // SUB_S
		case 0x02: return arcompact_handle0d_02(PARAMS); break; // ASL_S
		case 0x03: return arcompact_handle0d_03(PARAMS); break; // ASR_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e(OPS_16)
{
	UINT8 subinstr = (op & 0x0018) >> 3;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0e_00(PARAMS); break; // ADD_S
		case 0x01: return arcompact_handle0e_01(PARAMS); break; // MOV_S
		case 0x02: return arcompact_handle0e_02(PARAMS); break; // CMP_S
		case 0x03: return arcompact_handle0e_03(PARAMS); break; // MOV_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f(OPS_16)
{
	UINT8 subinstr = (op & 0x01f) >> 0;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0f_00(PARAMS); break; // SOPs
		case 0x01: return arcompact_handle0f_01(PARAMS); break; // 0x01 <illegal>
		case 0x02: return arcompact_handle0f_02(PARAMS); break; // SUB_S
		case 0x03: return arcompact_handle0f_03(PARAMS); break; // 0x03 <illegal>
		case 0x04: return arcompact_handle0f_04(PARAMS); break; // AND_S
		case 0x05: return arcompact_handle0f_05(PARAMS); break; // OR_S
		case 0x06: return arcompact_handle0f_06(PARAMS); break; // BIC_S
		case 0x07: return arcompact_handle0f_07(PARAMS); break; // XOR_S
		case 0x08: return arcompact_handle0f_08(PARAMS); break; // 0x08 <illegal>
		case 0x09: return arcompact_handle0f_09(PARAMS); break; // 0x09 <illegal>
		case 0x0a: return arcompact_handle0f_0a(PARAMS); break; // 0x0a <illegal>
		case 0x0b: return arcompact_handle0f_0b(PARAMS); break; // TST_S
		case 0x0c: return arcompact_handle0f_0c(PARAMS); break; // MUL64_S
		case 0x0d: return arcompact_handle0f_0d(PARAMS); break; // SEXB_S
		case 0x0e: return arcompact_handle0f_0e(PARAMS); break; // SEXW_S
		case 0x0f: return arcompact_handle0f_0f(PARAMS); break; // EXTB_S
		case 0x10: return arcompact_handle0f_10(PARAMS); break; // EXTW_S
		case 0x11: return arcompact_handle0f_11(PARAMS); break; // ABS_S
		case 0x12: return arcompact_handle0f_12(PARAMS); break; // NOT_S
		case 0x13: return arcompact_handle0f_13(PARAMS); break; // NEG_S
		case 0x14: return arcompact_handle0f_14(PARAMS); break; // ADD1_S
		case 0x15: return arcompact_handle0f_15(PARAMS); break; // ADD2_S
		case 0x16: return arcompact_handle0f_16(PARAMS); break; // ADD3_S
		case 0x17: return arcompact_handle0f_17(PARAMS); break; // 0x17 <illegal>
		case 0x18: return arcompact_handle0f_18(PARAMS); break; // ASL_S (multiple)
		case 0x19: return arcompact_handle0f_19(PARAMS); break; // LSR_S (multiple)
		case 0x1a: return arcompact_handle0f_1a(PARAMS); break; // ASR_S (multiple)
		case 0x1b: return arcompact_handle0f_1b(PARAMS); break; // ASL_S (single)
		case 0x1c: return arcompact_handle0f_1c(PARAMS); break; // LSR_S (single)
		case 0x1d: return arcompact_handle0f_1d(PARAMS); break; // ASR_S (single)
		case 0x1e: return arcompact_handle0f_1e(PARAMS); break; // TRAP (not a5?)
		case 0x1f: return arcompact_handle0f_1f(PARAMS); break; // BRK_S ( 0x7fff only? )

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle0f_00_00(PARAMS); break; // J_S
		case 0x01: return arcompact_handle0f_00_01(PARAMS); break; // J_S.D
		case 0x02: return arcompact_handle0f_00_02(PARAMS); break; // JL_S
		case 0x03: return arcompact_handle0f_00_03(PARAMS); break; // JL_S.D
		case 0x04: return arcompact_handle0f_00_04(PARAMS); break; // 0x04 <illegal>
		case 0x05: return arcompact_handle0f_00_05(PARAMS); break; // 0x05 <illegal>
		case 0x06: return arcompact_handle0f_00_06(PARAMS); break; // SUB_S.NE
		case 0x07: return arcompact_handle0f_00_07(PARAMS); break; // ZOPs

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07(OPS_16)
{
	UINT8 subinstr3 = (op & 0x0700) >> 8;

	switch (subinstr3)
	{
		case 0x00: return arcompact_handle0f_00_07_00(PARAMS); break; // NOP_S
		case 0x01: return arcompact_handle0f_00_07_01(PARAMS); break; // UNIMP_S
		case 0x02: return arcompact_handle0f_00_07_02(PARAMS); break; // 0x02 <illegal>
		case 0x03: return arcompact_handle0f_00_07_03(PARAMS); break; // 0x03 <illegal>
		case 0x04: return arcompact_handle0f_00_07_04(PARAMS); break; // JEQ_S [BLINK]
		case 0x05: return arcompact_handle0f_00_07_05(PARAMS); break; // JNE_S [BLINK]
		case 0x06: return arcompact_handle0f_00_07_06(PARAMS); break; // J_S [BLINK]
		case 0x07: return arcompact_handle0f_00_07_07(PARAMS); break; // J_S.D [BLINK]

	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle17_00(PARAMS); break; // ASL_S 
		case 0x01: return arcompact_handle17_01(PARAMS); break; // LSR_S 
		case 0x02: return arcompact_handle17_02(PARAMS); break; // ASR_S
		case 0x03: return arcompact_handle17_03(PARAMS); break; // SUB_S 
		case 0x04: return arcompact_handle17_04(PARAMS); break; // BSET_S 
		case 0x05: return arcompact_handle17_05(PARAMS); break; // BCLR_S 
		case 0x06: return arcompact_handle17_06(PARAMS); break; // BMSK_S 
		case 0x07: return arcompact_handle17_07(PARAMS); break; // BTST_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18(OPS_16)
{
	UINT8 subinstr = (op & 0x00e0) >> 5;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle18_00(PARAMS); break; // LD_S (SP)
		case 0x01: return arcompact_handle18_01(PARAMS); break; // LDB_S (SP)
		case 0x02: return arcompact_handle18_02(PARAMS); break; // ST_S (SP)
		case 0x03: return arcompact_handle18_03(PARAMS); break; // STB_S (SP)
		case 0x04: return arcompact_handle18_04(PARAMS); break; // ADD_S (SP)
		case 0x05: return arcompact_handle18_05(PARAMS); break; // subtable 18_05
		case 0x06: return arcompact_handle18_06(PARAMS); break; // subtable 18_06
		case 0x07: return arcompact_handle18_07(PARAMS); break; // subtable 18_07
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05(OPS_16) 
{
	UINT8 subinstr2 = (op & 0x0700) >> 8;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_05_00(PARAMS); break; // ADD_S (SP)
		case 0x01: return arcompact_handle18_05_01(PARAMS); break; // SUB_S (SP)
		case 0x02: return arcompact_handle18_05_02(PARAMS); break; // <illegal 0x18_05_02> 
		case 0x03: return arcompact_handle18_05_03(PARAMS); break; // <illegal 0x18_05_03>
		case 0x04: return arcompact_handle18_05_04(PARAMS); break; // <illegal 0x18_05_04>
		case 0x05: return arcompact_handle18_05_05(PARAMS); break; // <illegal 0x18_05_05>
		case 0x06: return arcompact_handle18_05_06(PARAMS); break; // <illegal 0x18_05_06>
		case 0x07: return arcompact_handle18_05_07(PARAMS); break; // <illegal 0x18_05_07>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06(OPS_16) 
{
	UINT8 subinstr2 = (op & 0x001f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_06_00(PARAMS); break; // <illegal 0x18_06_00>
		case 0x01: return arcompact_handle18_06_01(PARAMS); break; // POP_S b
		case 0x02: return arcompact_handle18_06_02(PARAMS); break; // <illegal 0x18_06_02>
		case 0x03: return arcompact_handle18_06_03(PARAMS); break; // <illegal 0x18_06_03>
		case 0x04: return arcompact_handle18_06_04(PARAMS); break; // <illegal 0x18_06_04>
		case 0x05: return arcompact_handle18_06_05(PARAMS); break; // <illegal 0x18_06_05>
		case 0x06: return arcompact_handle18_06_06(PARAMS); break; // <illegal 0x18_06_06>
		case 0x07: return arcompact_handle18_06_07(PARAMS); break; // <illegal 0x18_06_07>
		case 0x08: return arcompact_handle18_06_08(PARAMS); break; // <illegal 0x18_06_08>
		case 0x09: return arcompact_handle18_06_09(PARAMS); break; // <illegal 0x18_06_09>
		case 0x0a: return arcompact_handle18_06_0a(PARAMS); break; // <illegal 0x18_06_0a>
		case 0x0b: return arcompact_handle18_06_0b(PARAMS); break; // <illegal 0x18_06_0b>
		case 0x0c: return arcompact_handle18_06_0c(PARAMS); break; // <illegal 0x18_06_0c>
		case 0x0d: return arcompact_handle18_06_0d(PARAMS); break; // <illegal 0x18_06_0d>
		case 0x0e: return arcompact_handle18_06_0e(PARAMS); break; // <illegal 0x18_06_0e>
		case 0x0f: return arcompact_handle18_06_0f(PARAMS); break; // <illegal 0x18_06_0f>
		case 0x10: return arcompact_handle18_06_10(PARAMS); break; // <illegal 0x18_06_10>
		case 0x11: return arcompact_handle18_06_11(PARAMS); break; // POP_S blink
		case 0x12: return arcompact_handle18_06_12(PARAMS); break; // <illegal 0x18_06_12>
		case 0x13: return arcompact_handle18_06_13(PARAMS); break; // <illegal 0x18_06_13>
		case 0x14: return arcompact_handle18_06_14(PARAMS); break; // <illegal 0x18_06_14>
		case 0x15: return arcompact_handle18_06_15(PARAMS); break; // <illegal 0x18_06_15>
		case 0x16: return arcompact_handle18_06_16(PARAMS); break; // <illegal 0x18_06_16>
		case 0x17: return arcompact_handle18_06_17(PARAMS); break; // <illegal 0x18_06_17>
		case 0x18: return arcompact_handle18_06_18(PARAMS); break; // <illegal 0x18_06_18>
		case 0x19: return arcompact_handle18_06_19(PARAMS); break; // <illegal 0x18_06_19>
		case 0x1a: return arcompact_handle18_06_1a(PARAMS); break; // <illegal 0x18_06_1a>
		case 0x1b: return arcompact_handle18_06_1b(PARAMS); break; // <illegal 0x18_06_1b>
		case 0x1c: return arcompact_handle18_06_1c(PARAMS); break; // <illegal 0x18_06_1c>
		case 0x1d: return arcompact_handle18_06_1d(PARAMS); break; // <illegal 0x18_06_1d>
		case 0x1e: return arcompact_handle18_06_1e(PARAMS); break; // <illegal 0x18_06_1e>
		case 0x1f: return arcompact_handle18_06_1f(PARAMS); break; // <illegal 0x18_06_1f>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07(OPS_16) 
{
	UINT8 subinstr2 = (op & 0x001f) >> 0;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle18_07_00(PARAMS); break; // <illegal 0x18_07_00>
		case 0x01: return arcompact_handle18_07_01(PARAMS); break; // PUSH_S b
		case 0x02: return arcompact_handle18_07_02(PARAMS); break; // <illegal 0x18_07_02>
		case 0x03: return arcompact_handle18_07_03(PARAMS); break; // <illegal 0x18_07_03>
		case 0x04: return arcompact_handle18_07_04(PARAMS); break; // <illegal 0x18_07_04>
		case 0x05: return arcompact_handle18_07_05(PARAMS); break; // <illegal 0x18_07_05>
		case 0x06: return arcompact_handle18_07_06(PARAMS); break; // <illegal 0x18_07_06>
		case 0x07: return arcompact_handle18_07_07(PARAMS); break; // <illegal 0x18_07_07>
		case 0x08: return arcompact_handle18_07_08(PARAMS); break; // <illegal 0x18_07_08>
		case 0x09: return arcompact_handle18_07_09(PARAMS); break; // <illegal 0x18_07_09>
		case 0x0a: return arcompact_handle18_07_0a(PARAMS); break; // <illegal 0x18_07_0a>
		case 0x0b: return arcompact_handle18_07_0b(PARAMS); break; // <illegal 0x18_07_0b>
		case 0x0c: return arcompact_handle18_07_0c(PARAMS); break; // <illegal 0x18_07_0c>
		case 0x0d: return arcompact_handle18_07_0d(PARAMS); break; // <illegal 0x18_07_0d>
		case 0x0e: return arcompact_handle18_07_0e(PARAMS); break; // <illegal 0x18_07_0e>
		case 0x0f: return arcompact_handle18_07_0f(PARAMS); break; // <illegal 0x18_07_0f>
		case 0x10: return arcompact_handle18_07_10(PARAMS); break; // <illegal 0x18_07_10>
		case 0x11: return arcompact_handle18_07_11(PARAMS); break; // PUSH_S blink
		case 0x12: return arcompact_handle18_07_12(PARAMS); break; // <illegal 0x18_07_12>
		case 0x13: return arcompact_handle18_07_13(PARAMS); break; // <illegal 0x18_07_13>
		case 0x14: return arcompact_handle18_07_14(PARAMS); break; // <illegal 0x18_07_14>
		case 0x15: return arcompact_handle18_07_15(PARAMS); break; // <illegal 0x18_07_15>
		case 0x16: return arcompact_handle18_07_16(PARAMS); break; // <illegal 0x18_07_16>
		case 0x17: return arcompact_handle18_07_17(PARAMS); break; // <illegal 0x18_07_17>
		case 0x18: return arcompact_handle18_07_18(PARAMS); break; // <illegal 0x18_07_18>
		case 0x19: return arcompact_handle18_07_19(PARAMS); break; // <illegal 0x18_07_19>
		case 0x1a: return arcompact_handle18_07_1a(PARAMS); break; // <illegal 0x18_07_1a>
		case 0x1b: return arcompact_handle18_07_1b(PARAMS); break; // <illegal 0x18_07_1b>
		case 0x1c: return arcompact_handle18_07_1c(PARAMS); break; // <illegal 0x18_07_1c>
		case 0x1d: return arcompact_handle18_07_1d(PARAMS); break; // <illegal 0x18_07_1d>
		case 0x1e: return arcompact_handle18_07_1e(PARAMS); break; // <illegal 0x18_07_1e>
		case 0x1f: return arcompact_handle18_07_1f(PARAMS); break; // <illegal 0x18_07_1f>
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19(OPS_16)
{
	UINT8 subinstr = (op & 0x0600) >> 9;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle19_00(PARAMS); break; // LD_S (GP)
		case 0x01: return arcompact_handle19_01(PARAMS); break; // LDB_S (GP)
		case 0x02: return arcompact_handle19_02(PARAMS); break; // LDW_S (GP)
		case 0x03: return arcompact_handle19_03(PARAMS); break; // ADD_S (GP)
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c(OPS_16)
{
	UINT8 subinstr = (op & 0x0080) >> 7;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1c_00(PARAMS); break; // ADD_S
		case 0x01: return arcompact_handle1c_01(PARAMS); break; // CMP_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d(OPS_16)
{
	UINT8 subinstr = (op & 0x0080) >> 7;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1d_00(PARAMS); break; // BREQ_S
		case 0x01: return arcompact_handle1d_01(PARAMS); break; // BRNE_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e(OPS_16)
{
	UINT8 subinstr = (op & 0x0600) >> 9;

	switch (subinstr)
	{
		case 0x00: return arcompact_handle1e_00(PARAMS); break; // B_S
		case 0x01: return arcompact_handle1e_01(PARAMS); break; // BEQ_S
		case 0x02: return arcompact_handle1e_02(PARAMS); break; // BNE_S
		case 0x03: return arcompact_handle1e_03(PARAMS); break; // Bcc_S
	}

	return 0;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03(OPS_16)
{
	UINT8 subinstr2 = (op & 0x01c0) >> 6;

	switch (subinstr2)
	{
		case 0x00: return arcompact_handle1e_03_00(PARAMS); break; // BGT_S
		case 0x01: return arcompact_handle1e_03_01(PARAMS); break; // BGE_S 
		case 0x02: return arcompact_handle1e_03_02(PARAMS); break; // BLT_S
		case 0x03: return arcompact_handle1e_03_03(PARAMS); break; // BLE_S
		case 0x04: return arcompact_handle1e_03_04(PARAMS); break; // BHI_S
		case 0x05: return arcompact_handle1e_03_05(PARAMS); break; // BHS_S
		case 0x06: return arcompact_handle1e_03_06(PARAMS); break; // BLO_S
		case 0x07: return arcompact_handle1e_03_07(PARAMS); break; // BLS_S 
	}

	return 0;
}

// handlers



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00_00(OPS_32)
{
	int size = 4;
	// Branch Conditionally
	arcompact_log("unimplemented Bcc %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle00_01(OPS_32)
{
	int size = 4;
	// Branch Unconditionally Far
	arcompact_log("unimplemented B %08x", op);
	return m_pc + (size>>0);

}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00_00dasm(OPS_32)
{
	int size = 4;

	// Branch and Link Conditionally
	arcompact_log("unimplemented BLcc %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_00_01dasm(OPS_32)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);	
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
//	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	UINT32 realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 1;
	}
	else
	{
		return realaddress;
	}


	return m_pc + (size>>0);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_01_01_00_helper(OPS_32, const char* optext)
{
	int size = 4;

	// Branch on Compare / Bit Test - Register-Register

	int c = (op & 0x00000fc0) >> 6;
	COMMON32_GET_breg;
	//int n = (op & 0x00000020) >> 5;


	if ((breg != LIMM_REG) && (c != LIMM_REG))
	{

	}
	else
	{
		//UINT32 limm;
		//GET_LIMM_32;
		size = 8;
	}

	arcompact_log("unimplemented %s %08x", optext, op);
	return m_pc + (size>>0);
}


// register - register cases
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_00(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BREQ"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_01(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BRNE"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_02(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BRLT"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_03(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BRGE"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_04(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BRLO"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_05(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BRHS"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0e(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BBIT0");}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0f(OPS_32)  { return arcompact_01_01_00_helper( PARAMS, "BBIT1");}

ARCOMPACT_RETTYPE arcompact_device::arcompact_01_01_01_helper(OPS_32, const char* optext)
{
	int size = 4;
	arcompact_log("unimplemented %s %08x", optext, op);
	return m_pc + (size>>0);
}

// register -immediate cases
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_00(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BREQ"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_01(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BRNE"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_02(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BRLT"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_03(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BRGE"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_04(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BRLO"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_05(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BRHS"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0e(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BBIT0"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0f(OPS_32)  { return arcompact_01_01_01_helper(PARAMS, "BBIT1"); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle02(OPS_32)
{
	int size = 4;
	COMMON32_GET_breg;

	//UINT32 limm = 0;
	if (breg == LIMM_REG)
	{
		//GET_LIMM_32;
		size = 8;
	}

	arcompact_log("unimplemented LD %08x", op);
	return m_pc + (size>>0);

}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle03(OPS_32)
{
	int size = 4;
	//UINT32 limm = 0;
	int got_limm = 0; 

	COMMON32_GET_breg;
	int C = (op & 0x00000fc0) >> 6;
	
	if (breg == LIMM_REG)
	{
		//GET_LIMM_32;
		size = 8;
		got_limm = 1;
	}

	if (C == LIMM_REG)
	{
		if (!got_limm)
		{
			//GET_LIMM_32;
			size = 8;
		}
	}
	else
	{

	}

	arcompact_log("unimplemented ST %08x", op);
	return m_pc + (size>>0);

}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_helper(OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	int size = 4;
	//UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22;
	COMMON32_GET_breg;
	
	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			//GET_LIMM_32;
			size = 8;
			got_limm = 1;
		}
		else
		{
		}
	}
	else
	{
	}


	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				//GET_LIMM_32;
				size = 8;
			}
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
		int M = (op & 0x00000020) >> 5;

		if (M == 0)
		{
			int C = (op & 0x00000fc0) >> 6;

			if (C == LIMM_REG)
			{
				if (!got_limm)
				{
					//GET_LIMM_32;
					size = 8;
				}
			}
			else
			{
			}

		}
		else if (M == 1)
		{
		}

	}

	arcompact_log("unimplemented %s %08x", optext, op);

	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_00(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x00], /*"ADD"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_01(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x01], /*"ADC"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_02(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x02], /*"SUB"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_03(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x03], /*"SBC"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_04(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x04], /*"AND"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_05(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x05], /*"OR"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_06(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x06], /*"BIC"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_07(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x07], /*"XOR"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_08(OPS_32)  
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x08], /*"MAX"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_09(OPS_32) 
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x09], /*"MIN"*/ 0,0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0a(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0a], /*"MOV"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0b(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0b], /*"TST"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0c(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0c], /*"CMP"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0d(OPS_32)
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0d], /*"RCMP"*/ 1,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0e(OPS_32)
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0e], /*"RSUB"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_0f(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x0f], /*"BSET"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_10(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x10], /*"BCLR"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_11(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x11], /*"BTST"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_12(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x12], /*"BXOR"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_13(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x13], /*"BMSK"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_14(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x14], /*"ADD1"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_15(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x15], /*"ADD2"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_16(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x16], /*"ADD3"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_17(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x17], /*"SUB1"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_18(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x18], /*"SUB2"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_19(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x19], /*"SUB3"*/ 0,0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1a(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1a], /*"MPY"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1b(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1b], /*"MPYH"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1c(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1c], /*"MPYHU"*/ 0,0);
} // *

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1d(OPS_32)  
{ 
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x1d], /*"MPYU"*/ 0,0);
} // *


// arcompact_handle04_helper format
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_20(OPS_32)
{
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22;

	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			return limm;
		}
		else
		{
			arcompact_log("unimplemented J %08x", op);
		}
	}
	else if (p == 1)
	{
		arcompact_log("unimplemented J %08x", op);
	}
	else if (p == 2)
	{
		arcompact_log("unimplemented J %08x", op);
	}
	else if (p == 3)
	{
		int M = (op & 0x00000020) >> 5;

		if (M == 0)
		{
			int C = (op & 0x00000fc0) >> 6;

			if (C == LIMM_REG)
			{
				if (!got_limm)
				{
					//GET_LIMM_32;
					size = 8;
				}

				arcompact_log("unimplemented J %08x", op);
			}
			else
			{
				arcompact_log("unimplemented J %08x", op);
			}

		}
		else if (M == 1)
		{
			arcompact_log("unimplemented J %08x", op);
		}

	}


	return m_pc + (size>>0);}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_21(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x21], /*"J.D"*/ 1,1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_22(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x22], /*"JL"*/ 1,1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_23(OPS_32)
{
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x23], /*"JL.D"*/ 1,1);
}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_28(OPS_32) // LPcc (loop setup)
{
	int size = 4;

	int p = (op & 0x00c00000) >> 22;

	if (p == 0x00)
	{
	}
	else if (p == 0x01)
	{
	}
	else if (p == 0x02) // Loop unconditional
	{
	}
	else if (p == 0x03) // Loop conditional
	{
	}

	arcompact_log("unimplemented LPcc %08x", op);
	return m_pc + (size>>0);

}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2a(OPS_32)  // Load FROM Auxiliary register TO register
{
	int size = 4;
//	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22;
	//COMMON32_GET_breg;

	if (p == 0)
	{

		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				//GET_LIMM_32;
				size = 8;
			}
	
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented LR %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2b(OPS_32)  // Store TO Auxiliary register FROM register
{	
	int size = 4;
//	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22;
	COMMON32_GET_breg;

	if (breg == LIMM_REG)
	{
		//GET_LIMM_32;
		size = 8;
		got_limm = 1;

	}
	else
	{
	}

	if (p == 0)
	{

		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				//GET_LIMM_32;
				size = 8;
			}
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented ST %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_29(OPS_32)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper(PARAMS, opcodes_04[0x29], /*"FLAG"*/ 1,1);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_helper(OPS_32, const char* optext)
{
	int size = 4;

	int p = (op & 0x00c00000) >> 22;
	//COMMON32_GET_breg;
	
	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			//UINT32 limm;
			//GET_LIMM_32;
			size = 8;	
		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_00(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ASL"); } // ASL
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_01(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ASR"); } // ASR
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_02(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "LSR"); } // LSR
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_03(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ROR"); } // ROR
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_04(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "RCC"); } // RCC
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_05(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "SEXB"); } // SEXB
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_06(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "SEXW"); } // SEXW
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_07(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "EXTB"); } // EXTB
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_08(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "EXTW"); } // EXTW
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_09(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "ABS"); } // ABS
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0a(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "NOT"); } // NOT
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0b(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "RCL"); } // RLC
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0c(OPS_32)  { return arcompact_handle04_2f_helper(PARAMS, "EX"); } // EX


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_01(OPS_32)  { arcompact_log("SLEEP (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_02(OPS_32)  { arcompact_log("SWI / TRAP0 (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_03(OPS_32)  { arcompact_log("SYNC (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_04(OPS_32)  { arcompact_log("RTIE (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_05(OPS_32)  { arcompact_log("BRK (%08x)", op); return m_pc + (4 >> 0);}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3x_helper(OPS_32, int dsize, int extend)
{
	int size = 4;
	//UINT32 limm=0;
	int got_limm = 0;


	COMMON32_GET_breg;
	int C = (op & 0x00000fc0) >> 6;



	if (breg == LIMM_REG)
	{
		//GET_LIMM_32;
		size = 8;
		got_limm = 1;

	}
	else
	{
	}

	if (C == LIMM_REG)
	{
		if (!got_limm)
		{
			//GET_LIMM_32;
			size = 8;
		}

	}
	else
	{
	}	

	arcompact_log("unimplemented LD %08x", op);
	return m_pc + (size>>0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_30(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_31(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,0,1); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_32(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,1,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_33(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,1,1); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_34(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,2,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_35(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,2,1); }
// ZZ value of 0x3 is illegal
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_36(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,3,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_37(OPS_32)  { return arcompact_handle04_3x_helper(PARAMS,3,1); }






ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_00(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASL", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_01(OPS_32)  { return arcompact_handle04_helper(PARAMS, "LSR", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_02(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASR", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_03(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ROR", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_04(OPS_32)  { return arcompact_handle04_helper(PARAMS, "MUL64", 2,0); } // special
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_05(OPS_32)  { return arcompact_handle04_helper(PARAMS, "MULU64", 2,0);} // special
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_06(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ADDS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_07(OPS_32)  { return arcompact_handle04_helper(PARAMS, "SUBS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_08(OPS_32)  { return arcompact_handle04_helper(PARAMS, "DIVAW", 0,0); }



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0a(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASLS", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0b(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ASRS", 0,0); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_28(OPS_32)  { return arcompact_handle04_helper(PARAMS, "ADDSDW", 0,0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_29(OPS_32)  { return arcompact_handle04_helper(PARAMS, "SUBSDW", 0,0); }



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0x_helper(OPS_32, const char* optext)
{
	int size = 4;

	int p = (op & 0x00c00000) >> 22;
	//COMMON32_GET_breg;
	
	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6;

		if (C == LIMM_REG)
		{
			//UINT32 limm;
			//GET_LIMM_32;
			size = 8;	

		}
		else
		{
		}
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented %s %08x", optext, op);
	return m_pc + (size>>0);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_00(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "SWAP");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_01(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NORM");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_02(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "SAT16"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_03(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "RND16"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_04(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "ABSSW"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_05(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "ABSS");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_06(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NEGSW"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_07(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NEGS");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_08(OPS_32)  { return arcompact_handle05_2f_0x_helper(PARAMS, "NORMW"); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle06(OPS_32)
{
	arcompact_log("op a,b,c (06 ARC ext) (%08x)", op );
	return m_pc + (4 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle07(OPS_32)
{
	arcompact_log("op a,b,c (07 User ext) (%08x)", op );
	return m_pc + (4 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle08(OPS_32)
{
	arcompact_log("op a,b,c (08 User ext) (%08x)", op );
	return m_pc + (4 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle09(OPS_32)
{
	arcompact_log("op a,b,c (09 Market ext) (%08x)", op );
	return m_pc + (4 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0a(OPS_32)
{
	arcompact_log("op a,b,c (0a Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0b(OPS_32)
{
	arcompact_log("op a,b,c (0b Market ext) (%08x)",  op );
	return m_pc + (4 >> 0);;
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_00(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LD_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_01(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LDB_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_02(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "LDW_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0c_03(OPS_16)
{
	return arcompact_handle0c_helper(PARAMS, "ADD_S");
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_00(OPS_16)
{
	return arcompact_handle0d_helper(PARAMS, "ADD_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_01(OPS_16)
{
	return arcompact_handle0d_helper(PARAMS, "SUB_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_02(OPS_16)
{
	return arcompact_handle0d_helper(PARAMS, "ASL_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0d_03(OPS_16)
{
	return arcompact_handle0d_helper(PARAMS, "ASR_S");
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_0x_helper(OPS_16, const char* optext, int revop)
{
	int h;// , breg;
	int size = 2;

	GROUP_0e_GET_h;
	
	if (h == LIMM_REG)
	{
		//UINT32 limm;
		//GET_LIMM;
		size = 6;
	}
	else
	{

	}

	arcompact_log("unimplemented %s %04x", optext, op);

	return m_pc+ (size>>0);

}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_00(OPS_16)
{
	return arcompact_handle0e_0x_helper(PARAMS, "ADD_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_01(OPS_16)
{
	return arcompact_handle0e_0x_helper(PARAMS, "MOV_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_02(OPS_16)
{
	return arcompact_handle0e_0x_helper(PARAMS, "CMP_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0e_03(OPS_16)
{
	return arcompact_handle0e_0x_helper(PARAMS, "MOV_S", 1);
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_00(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "J_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_01(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "J_S.D"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_02(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "JL_S");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_03(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "JL_S.D");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_06(OPS_16)  { return arcompact_handle0f_00_0x_helper(PARAMS, "SUB_S.NE"); }




// Zero parameters (ZOP)
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_00(OPS_16)  { arcompact_log("NOP_S"); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_01(OPS_16)  { arcompact_log("UNIMP_S"); return m_pc + (2 >> 0);;} // Unimplemented Instruction, same as illegal, but recommended to fill blank space
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_04(OPS_16)  { arcompact_log("JEQ_S [blink]"); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_05(OPS_16)  { arcompact_log("JNE_S [blink]"); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_06(OPS_16)  { arcompact_log("J_S [blink]"); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_07(OPS_16)  { arcompact_log("J_S.D [blink]"); return m_pc + (2 >> 0);;}





ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0x_helper(OPS_16, const char* optext, int nodst)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_02(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "SUB_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_04(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "AND_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_05(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "OR_S",0);   }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_06(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "BIC_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_07(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "XOR_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0b(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "TST_S",1);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0c(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "MUL64_S",2);  } // actual destination is special multiply registers
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0d(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "SEXB_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0e(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "SEXW_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0f(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "EXTB_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_10(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "EXTW_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_11(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ABS_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_12(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "NOT_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_13(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "NEG_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_14(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ADD1_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_15(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ADD2_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_16(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ADD3_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_18(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASL_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_19(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "LSR_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1a(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASR_S",0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1b(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASL1_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1c(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "ASR1_S",0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1d(OPS_16)  { return arcompact_handle0f_0x_helper(PARAMS, "LSR1_S",0); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1e(OPS_16)  // special
{
	arcompact_log("unimplemented TRAP_S %04x",  op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_1f(OPS_16)  // special
{
	arcompact_log("unimplemented BRK_S %04x",  op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle_ld_helper(OPS_16, const char* optext, int shift, int swap)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle10(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "LD_S", 2, 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle11(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "LDB_S", 0, 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle12(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "LDW_S", 1, 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle13(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "LDW_S.X", 1, 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle14(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "ST_S", 2, 1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle15(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "STB_S", 0, 1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle16(OPS_16)
{
	return arcompact_handle_ld_helper(PARAMS, "STW_S", 1, 1);
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle_l7_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_00(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "ASL_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_01(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "LSR_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_02(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "ASR_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_03(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "SUB_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_04(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BSET_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_05(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BCLR_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_06(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BSMK_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle17_07(OPS_16)
{
	return arcompact_handle_l7_0x_helper(PARAMS, "BTST_S");
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_0x_helper(OPS_16, const char* optext, int st)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_00(OPS_16) 
{
	return arcompact_handle18_0x_helper(PARAMS, "LD_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_01(OPS_16) 
{
	return arcompact_handle18_0x_helper(PARAMS, "LDB_S", 0);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_02(OPS_16) 
{
	return arcompact_handle18_0x_helper(PARAMS, "ST_S", 1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_03(OPS_16) 
{
	return arcompact_handle18_0x_helper(PARAMS, "STB_S", 1);
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_04(OPS_16) 
{
	return arcompact_handle18_0x_helper(PARAMS, "ADD_S", 1); // check format
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_00(OPS_16)
{
	arcompact_log("unimplemented ADD_S SP, SP %04x", op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_01(OPS_16)
{
	arcompact_log("unimplemented SUB_S SP, SP %04x", op);
	return m_pc + (2 >> 0);;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700 
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_01(OPS_16) 
{
	arcompact_log("unimplemented POP_S %04x", op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_11(OPS_16) 
{
	arcompact_log("unimplemented POP_S [BLINK] %04x", op);
	return m_pc + (2 >> 0);;
}

// op bits remaining for 0x18_07_xx subgroups 0x0700 
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_01(OPS_16) 
{
	arcompact_log("unimplemented PUSH_S %04x", op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_11(OPS_16) 
{
	arcompact_log("unimplemented PUSH_S [BLINK] %04x", op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_0x_helper(OPS_16, const char* optext, int shift, int format)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_00(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LD_S", 2, 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_01(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LDB_S", 0, 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_02(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "LDW_S", 1, 0);  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle19_03(OPS_16)  { return arcompact_handle19_0x_helper(PARAMS, "ADD_S", 2, 1); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1a(OPS_16)
{
	arcompact_log("unimplemented MOV_S x, [PCL, x] %04x",  op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1b(OPS_16)
{
	arcompact_log("unimplemented MOV_S %04x",  op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c_00(OPS_16)
{
	arcompact_log("unimplemented ADD_S %04x",  op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1c_01(OPS_16)
{
	arcompact_log("unimplemented CMP_S %04x",  op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d_00(OPS_16)  { return arcompact_handle1d_helper(PARAMS,"BREQ_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1d_01(OPS_16)  { return arcompact_handle1d_helper(PARAMS,"BRNE_S"); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_00(OPS_16)  { return arcompact_handle1e_0x_helper(PARAMS, "BL_S");  }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_01(OPS_16)  { return arcompact_handle1e_0x_helper(PARAMS, "BEQ_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_02(OPS_16)  { return arcompact_handle1e_0x_helper(PARAMS, "BNE_S"); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_0x_helper(OPS_16, const char* optext)
{
	arcompact_log("unimplemented %s %04x", optext, op);
	return m_pc + (2 >> 0);;
}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_00(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BGT_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_01(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BGE_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_02(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLT_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_03(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLE_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_04(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BHI_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_05(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BHS_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_06(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLO_S"); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1e_03_07(OPS_16)  { return arcompact_handle1e_03_0x_helper(PARAMS, "BLS_S"); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle1f(OPS_16)
{
	arcompact_log("unimplemented BL_S %04x", op);
	return m_pc + (2 >> 0);;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_06(OPS_32)  { arcompact_fatal("<illegal 01_01_00_06> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_07(OPS_32)  { arcompact_fatal("<illegal 01_01_00_07> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_08(OPS_32)  { arcompact_fatal("<illegal 01_01_00_08> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_09(OPS_32)  { arcompact_fatal("<illegal 01_01_00_09> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0a(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0a> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0b(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0b> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0c(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0c> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_00_0d(OPS_32)  { arcompact_fatal("<illegal 01_01_00_0d> (%08x)", op); return m_pc + (4 >> 0); }

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_06(OPS_32)  { arcompact_fatal("<illegal 01_01_01_06> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_07(OPS_32)  { arcompact_fatal("<illegal 01_01_01_07> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_08(OPS_32)  { arcompact_fatal("<illegal 01_01_01_08> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_09(OPS_32)  { arcompact_fatal("<illegal 01_01_01_09> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0a(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0a> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0b(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0b> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0c(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0c> (%08x)", op); return m_pc + (4 >> 0); }
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle01_01_01_0d(OPS_32)  { arcompact_fatal("<illegal 01_01_01_0d> (%08x)", op); return m_pc + (4 >> 0); }


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_1f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_24(OPS_32)  { arcompact_fatal("<illegal 0x04_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_25(OPS_32)  { arcompact_fatal("<illegal 0x04_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_26(OPS_32)  { arcompact_fatal("<illegal 0x04_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_27(OPS_32)  { arcompact_fatal("<illegal 0x04_27> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2e> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_0f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_10(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_11(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_12(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_13(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_14(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_15(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_16(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_17(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_18(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_19(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_20(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_21(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_22(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_23(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_24(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_25(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_26(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_27(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_28(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_29(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_2f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_30(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_31(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_32(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_33(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_34(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_35(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_36(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_37(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_38(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_39(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3e> (%08x)", op); return m_pc + (4 >> 0);}



ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_09(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_10(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_11(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_12(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_13(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_14(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_15(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_16(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_17(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_18(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_19(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_20(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_21(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_22(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_23(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_24(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_25(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_26(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_27(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_28(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_29(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_2f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_30(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_31(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_32(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_33(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_34(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_35(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_36(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_37(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_38(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_39(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3e> (%08x)", op); return m_pc + (4 >> 0);}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_00(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_00> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_06(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_06> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_07(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_07> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_08(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_08> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_09(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_0f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_10(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_11(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_12(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_13(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_14(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_15(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_16(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_17(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_18(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_19(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_1f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_20(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_21(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_22(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_23(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_24(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_25(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_26(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_27(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_28(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_29(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_2f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_30(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_31(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_32(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_33(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_34(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_35(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_36(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_37(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_38(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_39(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_2f_3f_3f(OPS_32)  { arcompact_fatal("<illegal 0x04_2f_3f_3f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_00(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_00> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_01(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_01> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_02(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_02> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_03(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_03> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_04(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_04> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_05(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_05> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_06(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_06> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_07(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_07> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_08(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_08> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_09(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_10(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_11(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_12(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_13(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_14(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_15(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_16(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_17(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_18(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_19(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_20(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_21(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_22(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_23(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_24(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_25(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_26(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_27(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_27> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_28(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_28> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_29(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_29> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_2f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_2f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_30(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_31(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_32(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_33(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_34(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_35(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_36(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_37(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_38(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_39(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2f_3f_3f(OPS_32)  { arcompact_fatal("<illegal 0x05_2f_3f_3f> (%08x)", op); return m_pc + (4 >> 0);}




ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_38(OPS_32)  { arcompact_fatal("<illegal 0x04_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_39(OPS_32)  { arcompact_fatal("<illegal 0x04_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3a(OPS_32)  { arcompact_fatal("<illegal 0x04_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3b(OPS_32)  { arcompact_fatal("<illegal 0x04_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3c(OPS_32)  { arcompact_fatal("<illegal 0x04_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3d(OPS_32)  { arcompact_fatal("<illegal 0x04_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3e(OPS_32)  { arcompact_fatal("<illegal 0x04_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle04_3f(OPS_32)  { arcompact_fatal("<illegal 0x04_3f> (%08x)", op); return m_pc + (4 >> 0);}


ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_09(OPS_32)  { arcompact_fatal("<illegal 0x05_09> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0c(OPS_32)  { arcompact_fatal("<illegal 0x05_0c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0d(OPS_32)  { arcompact_fatal("<illegal 0x05_0d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0e(OPS_32)  { arcompact_fatal("<illegal 0x05_0e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_0f(OPS_32)  { arcompact_fatal("<illegal 0x05_0f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_10(OPS_32)  { arcompact_fatal("<illegal 0x05_10> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_11(OPS_32)  { arcompact_fatal("<illegal 0x05_11> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_12(OPS_32)  { arcompact_fatal("<illegal 0x05_12> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_13(OPS_32)  { arcompact_fatal("<illegal 0x05_13> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_14(OPS_32)  { arcompact_fatal("<illegal 0x05_14> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_15(OPS_32)  { arcompact_fatal("<illegal 0x05_15> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_16(OPS_32)  { arcompact_fatal("<illegal 0x05_16> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_17(OPS_32)  { arcompact_fatal("<illegal 0x05_17> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_18(OPS_32)  { arcompact_fatal("<illegal 0x05_18> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_19(OPS_32)  { arcompact_fatal("<illegal 0x05_19> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1a(OPS_32)  { arcompact_fatal("<illegal 0x05_1a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1b(OPS_32)  { arcompact_fatal("<illegal 0x05_1b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1c(OPS_32)  { arcompact_fatal("<illegal 0x05_1c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1d(OPS_32)  { arcompact_fatal("<illegal 0x05_1d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1e(OPS_32)  { arcompact_fatal("<illegal 0x05_1e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_1f(OPS_32)  { arcompact_fatal("<illegal 0x05_1f> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_20(OPS_32)  { arcompact_fatal("<illegal 0x05_20> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_21(OPS_32)  { arcompact_fatal("<illegal 0x05_21> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_22(OPS_32)  { arcompact_fatal("<illegal 0x05_22> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_23(OPS_32)  { arcompact_fatal("<illegal 0x05_23> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_24(OPS_32)  { arcompact_fatal("<illegal 0x05_24> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_25(OPS_32)  { arcompact_fatal("<illegal 0x05_25> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_26(OPS_32)  { arcompact_fatal("<illegal 0x05_26> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_27(OPS_32)  { arcompact_fatal("<illegal 0x05_27> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2a(OPS_32)  { arcompact_fatal("<illegal 0x05_2a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2b(OPS_32)  { arcompact_fatal("<illegal 0x05_2b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2c(OPS_32)  { arcompact_fatal("<illegal 0x05_2c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2d(OPS_32)  { arcompact_fatal("<illegal 0x05_2d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_2e(OPS_32)  { arcompact_fatal("<illegal 0x05_2e> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_30(OPS_32)  { arcompact_fatal("<illegal 0x05_30> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_31(OPS_32)  { arcompact_fatal("<illegal 0x05_31> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_32(OPS_32)  { arcompact_fatal("<illegal 0x05_32> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_33(OPS_32)  { arcompact_fatal("<illegal 0x05_33> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_34(OPS_32)  { arcompact_fatal("<illegal 0x05_34> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_35(OPS_32)  { arcompact_fatal("<illegal 0x05_35> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_36(OPS_32)  { arcompact_fatal("<illegal 0x05_36> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_37(OPS_32)  { arcompact_fatal("<illegal 0x05_37> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_38(OPS_32)  { arcompact_fatal("<illegal 0x05_38> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_39(OPS_32)  { arcompact_fatal("<illegal 0x05_39> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3a(OPS_32)  { arcompact_fatal("<illegal 0x05_3a> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3b(OPS_32)  { arcompact_fatal("<illegal 0x05_3b> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3c(OPS_32)  { arcompact_fatal("<illegal 0x05_3c> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3d(OPS_32)  { arcompact_fatal("<illegal 0x05_3d> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3e(OPS_32)  { arcompact_fatal("<illegal 0x05_3e> (%08x)", op); return m_pc + (4 >> 0);}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle05_3f(OPS_32)  { arcompact_fatal("<illegal 0x05_3f> (%08x)", op); return m_pc + (4 >> 0);}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_04(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_00> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_05(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_00> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_02(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_07_02> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_00_07_03(OPS_16)  { arcompact_fatal("<illegal 0x0f_00_07_03> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_01(OPS_16)  { arcompact_fatal("<illegal 0x0f_01> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_03(OPS_16)  { arcompact_fatal("<illegal 0x0f_03> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_08(OPS_16)  { arcompact_fatal("<illegal 0x0f_08> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_09(OPS_16)  { arcompact_fatal("<illegal 0x0f_09> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_0a(OPS_16)  { arcompact_fatal("<illegal 0x0f_0a> (%08x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle0f_17(OPS_16)  { arcompact_fatal("<illegal 0x0f_17> (%08x)", op); return m_pc + (2 >> 0);;}

ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_02(OPS_16)  { arcompact_fatal("<illegal 0x18_05_02> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_03(OPS_16)  { arcompact_fatal("<illegal 0x18_05_03> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_04(OPS_16)  { arcompact_fatal("<illegal 0x18_05_04> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_05(OPS_16)  { arcompact_fatal("<illegal 0x18_05_05> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_06(OPS_16)  { arcompact_fatal("<illegal 0x18_05_06> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_05_07(OPS_16)  { arcompact_fatal("<illegal 0x18_05_07> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_00(OPS_16)  { arcompact_fatal("<illegal 0x18_06_00> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_02(OPS_16)  { arcompact_fatal("<illegal 0x18_06_02> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_03(OPS_16)  { arcompact_fatal("<illegal 0x18_06_03> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_04(OPS_16)  { arcompact_fatal("<illegal 0x18_06_04> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_05(OPS_16)  { arcompact_fatal("<illegal 0x18_06_05> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_06(OPS_16)  { arcompact_fatal("<illegal 0x18_06_06> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_07(OPS_16)  { arcompact_fatal("<illegal 0x18_06_07> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_08(OPS_16)  { arcompact_fatal("<illegal 0x18_06_08> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_09(OPS_16)  { arcompact_fatal("<illegal 0x18_06_09> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0a(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0a> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0b(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0b> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0c(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0c> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0d(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0d> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0e(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0e> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_0f(OPS_16)  { arcompact_fatal("<illegal 0x18_06_0f> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_10(OPS_16)  { arcompact_fatal("<illegal 0x18_06_10> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_12(OPS_16)  { arcompact_fatal("<illegal 0x18_06_12> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_13(OPS_16)  { arcompact_fatal("<illegal 0x18_06_13> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_14(OPS_16)  { arcompact_fatal("<illegal 0x18_06_14> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_15(OPS_16)  { arcompact_fatal("<illegal 0x18_06_15> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_16(OPS_16)  { arcompact_fatal("<illegal 0x18_06_16> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_17(OPS_16)  { arcompact_fatal("<illegal 0x18_06_17> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_18(OPS_16)  { arcompact_fatal("<illegal 0x18_06_18> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_19(OPS_16)  { arcompact_fatal("<illegal 0x18_06_19> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1a(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1a> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1b(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1b> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1c(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1c> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1d(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1d> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1e(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1e> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_06_1f(OPS_16)  { arcompact_fatal("<illegal 0x18_06_1f> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_00(OPS_16)  { arcompact_fatal("<illegal 0x18_07_00> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_02(OPS_16)  { arcompact_fatal("<illegal 0x18_07_02> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_03(OPS_16)  { arcompact_fatal("<illegal 0x18_07_03> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_04(OPS_16)  { arcompact_fatal("<illegal 0x18_07_04> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_05(OPS_16)  { arcompact_fatal("<illegal 0x18_07_05> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_06(OPS_16)  { arcompact_fatal("<illegal 0x18_07_06> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_07(OPS_16)  { arcompact_fatal("<illegal 0x18_07_07> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_08(OPS_16)  { arcompact_fatal("<illegal 0x18_07_08> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_09(OPS_16)  { arcompact_fatal("<illegal 0x18_07_09> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0a(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0a> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0b(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0b> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0c(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0c> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0d(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0d> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0e(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0e> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_0f(OPS_16)  { arcompact_fatal("<illegal 0x18_07_0f> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_10(OPS_16)  { arcompact_fatal("<illegal 0x18_07_10> (%04x)", op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_12(OPS_16)  { arcompact_fatal("<illegal 0x18_07_12> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_13(OPS_16)  { arcompact_fatal("<illegal 0x18_07_13> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_14(OPS_16)  { arcompact_fatal("<illegal 0x18_07_14> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_15(OPS_16)  { arcompact_fatal("<illegal 0x18_07_15> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_16(OPS_16)  { arcompact_fatal("<illegal 0x18_07_16> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_17(OPS_16)  { arcompact_fatal("<illegal 0x18_07_17> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_18(OPS_16)  { arcompact_fatal("<illegal 0x18_07_18> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_19(OPS_16)  { arcompact_fatal("<illegal 0x18_07_19> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1a(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1a> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1b(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1b> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1c(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1c> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1d(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1d> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1e(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1e> (%04x)",  op); return m_pc + (2 >> 0);;}
ARCOMPACT_RETTYPE arcompact_device::arcompact_handle18_07_1f(OPS_16)  { arcompact_fatal("<illegal 0x18_07_1f> (%04x)",  op); return m_pc + (2 >> 0);;}


