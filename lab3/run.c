/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"



#define ADDU 0b100001
#define AND 0b100100
#define NOR 0b100111
#define OR 0b100101
#define SLTU 0b101011
#define SUBU 0b100011

// Shift Operations
#define SLL 0b000000
#define SRL 0b000010

#define J 0x2
#define JAL 0x3
#define JR 0x8
#define ADDIU 0x9
#define ANDI 0xc
#define LUI 0xf
#define ORI 0xd
#define SLTIU 0xb
#define LW 0x23
#define SW 0x2b
#define BEQ 0x4
#define BNE 0x5


/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) { 
    // printf("Pc value is: %d, %d, %d\n", pc, MEM_TEXT_START, NUM_INST);
    if (pc >= MEM_TEXT_START + NUM_INST * BYTES_PER_WORD || pc < MEM_TEXT_START) {
        return NULL;
    }
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

bool load_hazard() {
    // printf("Dest is: %d, rs is: %d, rt is: %d\n", CURRENT_STATE.EX_MEM_DEST, CURRENT_STATE.ID_EX_RS, CURRENT_STATE.ID_EX_RT);
    if(
        CURRENT_STATE.EX_MEM_READ && CURRENT_STATE.EX_MEM_DEST &&
        (
            CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RS
            ||
            CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RT
      )
      ) {
        return true;
    }
    return false;
}

void clean_IF_ID(bool set_pipe) {
    CURRENT_STATE.IF_ID_INST = malloc(sizeof(instruction));
    CURRENT_STATE.IF_ID_NPC = 0;
    if (set_pipe) {
        CURRENT_STATE.PIPE[IF_STAGE] = -1;
    }
}

void clean_ID_EX(bool set_pipe) {
    // printf("ID EX NPC before clean is: %x\n", CURRENT_STATE.ID_EX_NPC);
    CURRENT_STATE.ID_EX_NPC = 0;
    CURRENT_STATE.ID_EX_REG1 = 0;
    CURRENT_STATE.ID_EX_REG2 = 0;
 	CURRENT_STATE.ID_EX_OP_CODE = 0;
	CURRENT_STATE.ID_EX_FUNC = 0;
	CURRENT_STATE.ID_EX_SHAMT = 0;
	CURRENT_STATE.ID_EX_RT = 0;
	CURRENT_STATE.ID_EX_RS = 0;
	CURRENT_STATE.ID_EX_RD = 0;
    CURRENT_STATE.ID_EX_IMM = 0;
	CURRENT_STATE.ID_EX_DEST = 0;
	CURRENT_STATE.EX_STALL = false;
	CURRENT_STATE.J_FLUSH = false;
    if (set_pipe) {
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
    }
}

void clean_EX_MEM() {
	CURRENT_STATE.EX_MEM_NPC = 0;

	CURRENT_STATE.EX_MEM_READ = 0;
	CURRENT_STATE.EX_MEM_WRITE = 0;
	CURRENT_STATE.EX_MEM_ALU_OUT = 0;
    CURRENT_STATE.EX_MEM_BR_TARGET = 0;
	CURRENT_STATE.EX_MEM_BR_TAKE = 0;
	CURRENT_STATE.EX_MEM_DEST = 0;
	CURRENT_STATE.EX_MEM_REG_WRITE = 0;
}

void process_IF() {
    if (CURRENT_STATE.PIPE[IF_STAGE] == -1) {
        CURRENT_STATE.PIPE[IF_STAGE] = 0;
        return;
    }

    instruction *instr = get_inst_info(CURRENT_STATE.PC);

    if (instr == NULL) {
        CURRENT_STATE.IF_ID_NPC = 0;
        CURRENT_STATE.PIPE[IF_STAGE] = 0; 
        return;
    }
    CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
    CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC; 
    memcpy(CURRENT_STATE.IF_ID_INST, instr, sizeof(instr));
    CURRENT_STATE.PC += 4;
}

void process_ID() {

    CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;
    CURRENT_STATE.PIPE[ID_STAGE] = CURRENT_STATE.ID_EX_NPC;
    if (!CURRENT_STATE.PIPE[ID_STAGE]) return;

    instruction *instr = CURRENT_STATE.IF_ID_INST;
    CURRENT_STATE.ID_EX_OP_CODE = OPCODE(instr);
    CURRENT_STATE.ID_EX_FUNC = FUNC(instr);
    CURRENT_STATE.ID_EX_RT = RT(instr);
    CURRENT_STATE.ID_EX_RS = RS(instr);
    CURRENT_STATE.ID_EX_RD = RD(instr);
    // printf("\n\nrs, rt, rd: %d, %d, %d\n\n", CURRENT_STATE.ID_EX_RS, CURRENT_STATE.ID_EX_RT, CURRENT_STATE.ID_EX_RD);
    CURRENT_STATE.ID_EX_SHAMT = SHAMT(instr);
    CURRENT_STATE.ID_EX_IMM = IMM(instr);
    CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_RS];
    CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_RT];
    CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;

    if (load_hazard()) {
        CURRENT_STATE.EX_STALL = true;
    }

    switch (CURRENT_STATE.ID_EX_OP_CODE)
    {
    case JAL:
        CURRENT_STATE.REGS[31] = CURRENT_STATE.ID_EX_NPC + BYTES_PER_WORD;
    case J:
        CURRENT_STATE.J_TARGET= TARGET(instr) << 2;
        CURRENT_STATE.J_FLUSH = true;
        break;
    case 0:
        switch (CURRENT_STATE.ID_EX_FUNC) {
        case JR:
            CURRENT_STATE.J_TARGET = CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_RS];
            CURRENT_STATE.J_FLUSH = true;    
        default:
            break;   
        }
    default:
        break;
    }
}

uint32_t forwardA() {
    if (CURRENT_STATE.EX_MEM_DEST && CURRENT_STATE.EX_MEM_REG_WRITE && CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RS) {
        return CURRENT_STATE.EX_MEM_ALU_OUT;
    }
    if (CURRENT_STATE.MEM_WB_DEST && CURRENT_STATE.MEM_WR_REG_WRITE && CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_RS) {
        if (CURRENT_STATE.MEM_WB_MEM_REG) {
            return CURRENT_STATE.MEM_WB_MEM_OUT;
        } else {
            return CURRENT_STATE.MEM_WB_ALU_OUT;
        }
    }
    return CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_RS];
}

uint32_t forwardB() {
    if (CURRENT_STATE.EX_MEM_DEST && CURRENT_STATE.EX_MEM_REG_WRITE && CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RT) {
        return CURRENT_STATE.EX_MEM_ALU_OUT;
    }
    // printf("dest and rt: %x, %x\n", CURRENT_STATE.MEM_WB_DEST, CURRENT_STATE.ID_EX_RT);
    if (CURRENT_STATE.MEM_WB_DEST && CURRENT_STATE.MEM_WR_REG_WRITE && CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_RT) {
        if (CURRENT_STATE.MEM_WB_MEM_REG) {
            return CURRENT_STATE.MEM_WB_MEM_OUT;
        } else {
            return CURRENT_STATE.MEM_WB_ALU_OUT;
        }
    }
    return CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_RT];
}

void process_EX() {
    CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.ID_EX_NPC;
    CURRENT_STATE.PIPE[EX_STAGE] = CURRENT_STATE.ID_EX_NPC;
    // printf("ID EX npc value: %x\n", CURRENT_STATE.ID_EX_NPC);
    if(!CURRENT_STATE.PIPE[EX_STAGE]) {
        clean_EX_MEM();
        return;
    }
    
    uint32_t rs_val = forwardA();
    uint32_t rt_val = forwardB();
    CURRENT_STATE.EX_MEM_READ = 0;
    CURRENT_STATE.EX_MEM_WRITE = 0;
    
    short op_code = CURRENT_STATE.ID_EX_OP_CODE;
    short func = CURRENT_STATE.ID_EX_FUNC;

    short shamt = CURRENT_STATE.ID_EX_SHAMT;

    int imm = (unsigned short) CURRENT_STATE.ID_EX_IMM;
    int sign_bit = imm >> 15;

    int sign_ext_imm = imm;
    if (sign_bit == 1) {
        sign_ext_imm = sign_ext_imm | (((1 << 16) - 1) << 16);
    }
    
    unsigned int branch_addr = sign_ext_imm << 2;

    switch (op_code)
    {
        case 0:
            CURRENT_STATE.EX_MEM_REG_WRITE = true;
            CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_RD;

            switch (func)
            {
                case SRL:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rt_val >> shamt;
                    break;
                case SLL:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rt_val << shamt;
                    break;
                case SUBU:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rs_val - rt_val;
                    break;
                case SLTU:
                    CURRENT_STATE.EX_MEM_ALU_OUT = (rs_val > rt_val) ? 0 : 1;
                    break;
                case OR:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rs_val | rt_val;
                    break;
                case NOR:
                    CURRENT_STATE.EX_MEM_ALU_OUT = ~(rs_val | rt_val);
                    break;
                case AND:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rs_val & rt_val;
                    break;
                case ADDU:
                    CURRENT_STATE.EX_MEM_ALU_OUT = rs_val + rt_val;

                    break;
                default:
                    CURRENT_STATE.EX_MEM_REG_WRITE = false;                
                    break;
            }
            break;
        default:
            CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_RT;
            CURRENT_STATE.EX_MEM_REG_WRITE = true;
            switch (op_code)
            {

            case ADDIU:
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val + sign_ext_imm;
                // printf("rs val: %d, rs: %d, rt: %d\n", CURRENT_STATE.ID_EX_REG2, CURRENT_STATE.ID_EX_RS, CURRENT_STATE.ID_EX_RT);
                break;
            case ANDI:
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val & sign_ext_imm;
                break;
            case LUI:
                CURRENT_STATE.EX_MEM_ALU_OUT = imm << 16;
                break;
            case ORI:
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val | sign_ext_imm;
                break;
            case SLTIU:
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val < ((unsigned int) sign_ext_imm) ? 1 : 0;
                break;
            case LW:
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val + sign_ext_imm;
                CURRENT_STATE.EX_MEM_READ = 1;
                // printf("Alu out: %x,%x, %x\n", CURRENT_STATE.EX_MEM_ALU_OUT, sign_ext_imm, mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT));
                break;
            case SW:
                CURRENT_STATE.EX_MEM_REG_WRITE = false;
                CURRENT_STATE.EX_MEM_WRITE = 1;
                CURRENT_STATE.EX_MEM_ALU_OUT = rs_val + sign_ext_imm;
                break;
            case BEQ:
                CURRENT_STATE.EX_MEM_REG_WRITE = false;
                CURRENT_STATE.EX_MEM_ALU_OUT = (rs_val == rt_val) ? 1: 0;
                break;

            case BNE:
                CURRENT_STATE.EX_MEM_REG_WRITE = false;
                CURRENT_STATE.EX_MEM_ALU_OUT = (rs_val != rt_val) ? 1 : 0;
                break;
            default:
                CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_RT;
                CURRENT_STATE.EX_MEM_REG_WRITE = false;
                break;
            }
            break;
    }
    // printf("Next stage bne is checking\n");
    if((op_code == BNE && CURRENT_STATE.EX_MEM_ALU_OUT) || (op_code == BEQ && CURRENT_STATE.EX_MEM_ALU_OUT)) {
        CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.PC + branch_addr - BYTES_PER_WORD;
        CURRENT_STATE.EX_MEM_BR_TAKE = 1;
    }
}

void process_MEM() {
    CURRENT_STATE.PIPE[MEM_STAGE] = CURRENT_STATE.EX_MEM_NPC;
    CURRENT_STATE.MEM_WB_NPC = CURRENT_STATE.EX_MEM_NPC;
    if (!CURRENT_STATE.PIPE[MEM_STAGE]) return;

    CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.MEM_WR_REG_WRITE = CURRENT_STATE.EX_MEM_REG_WRITE;

    if (CURRENT_STATE.EX_MEM_BR_TAKE)
    {
        CURRENT_STATE.PC = CURRENT_STATE.EX_MEM_BR_TARGET;
        CURRENT_STATE.EX_MEM_BR_TAKE = 0;
        clean_IF_ID(true);
        clean_ID_EX(true);
        clean_EX_MEM();
    }

    if (CURRENT_STATE.EX_MEM_READ) {
        // printf("Alu out: %x, %x\n", CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.REGS[3]);
        CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
        CURRENT_STATE.EX_MEM_READ = 0;
        // printf("Alu out: %x\n", CURRENT_STATE.MEM_WB_MEM_OUT);
        CURRENT_STATE.MEM_WB_MEM_REG = 1;
    } else {
        CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
        CURRENT_STATE.MEM_WB_MEM_REG = 0;
    }
    if (CURRENT_STATE.EX_MEM_WRITE) {
        mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.REGS[CURRENT_STATE.EX_MEM_DEST]);
    }
    
}

void process_WB() {
    CURRENT_STATE.PIPE[WB_STAGE] = CURRENT_STATE.MEM_WB_NPC;
    if(!CURRENT_STATE.PIPE[WB_STAGE]) return;
    // printf("Dest reg: %d, %x, %d\n", CURRENT_STATE.MEM_WB_DEST, CURRENT_STATE.MEM_WB_ALU_OUT, CURRENT_STATE.MEM_WR_REG_WRITE);
    if(CURRENT_STATE.MEM_WR_REG_WRITE) {
        CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_MEM_REG ? CURRENT_STATE.MEM_WB_MEM_OUT: CURRENT_STATE.MEM_WB_ALU_OUT;    
    }
    // printf("Reg 9 is: %d\n", CURRENT_STATE.REGS[9]);
    INSTRUCTION_COUNT++;
}

void handle_stall_flush() {
    if (CURRENT_STATE.J_FLUSH) {
        clean_IF_ID(false);
        CURRENT_STATE.PC = CURRENT_STATE.J_TARGET;
    }
    if (CURRENT_STATE.EX_STALL) {
        CURRENT_STATE.ID_EX_PREV_NPC = CURRENT_STATE.ID_EX_NPC;
        CURRENT_STATE.ID_EX_NPC = 0;
    }
    CURRENT_STATE.J_FLUSH = false;
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){

	/** Your implementation here */
    process_WB();
    process_MEM();
    process_EX();   
    if (!CURRENT_STATE.EX_STALL) {
        process_ID();
        process_IF();
    } else {
        CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.ID_EX_PREV_NPC;
        CURRENT_STATE.EX_STALL = false;
    }

    handle_stall_flush();
    RUN_BIT =  (CURRENT_STATE.PIPE[IF_STAGE] || CURRENT_STATE.PIPE[ID_STAGE] || CURRENT_STATE.PIPE[EX_STAGE] || CURRENT_STATE.PIPE[MEM_STAGE]);
    RUN_BIT = RUN_BIT || !CURRENT_STATE.PIPE[WB_STAGE];
    // if (CYCLE_COUNT > 20) {
    //     RUN_BIT = false;
    // }
    // printf("Run bit: %d, %x, %x, %d, cycle count: %ld\n", RUN_BIT, CURRENT_STATE.PC, CURRENT_STATE.REGS[31], CURRENT_STATE.REGS[9], CYCLE_COUNT);
}