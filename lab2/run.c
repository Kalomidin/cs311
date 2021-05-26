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

// Jump register operation
#define JR 0b001000

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	/** Implement this function */
    if (CURRENT_STATE.PC >= (MEM_TEXT_START + NUM_INST * 4)) {
        RUN_BIT = FALSE;
        return;
    }
    int i = (CURRENT_STATE.PC - MEM_TEXT_START) / 4;
    CURRENT_STATE.PC += 4;
    // printf("PC is: %x, i is %x, %d\n", CURRENT_STATE.PC, i, CURRENT_STATE.REGS[11]);
    switch (INST_INFO[i].opcode)
    {
    //R instructions
    case 0x0: 
    {
        short rt = INST_INFO[i].r_t.r_i.rt;
        short rs = INST_INFO[i].r_t.r_i.rs;
        short rd = INST_INFO[i].r_t.r_i.r_i.r.rd;
        int shamt = (int) INST_INFO[i].r_t.r_i.r_i.r.shamt;
        switch (INST_INFO[i].func_code)
        {
        case JR: 
            CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
            break;
        case SRL:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
            break;
        case SLL:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
            break;
        case SUBU:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
            break;
        case SLTU:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
            break;
        case OR:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
            break;
        case NOR:
            CURRENT_STATE.REGS[rd] = ~((CURRENT_STATE.REGS[rs]) | (CURRENT_STATE.REGS[rt]));
            break;
        case AND:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
            break;
        case ADDU:
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
            break;
        default:
            break;
        }
        break;
    }
    // J instructions
    case 0x2: // J
        CURRENT_STATE.PC = INST_INFO[i].r_t.target;
        break;
    case 0x3: // JAL
        CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
        CURRENT_STATE.PC = INST_INFO[i].r_t.target;
        break;
    // L instructions
    case 0x9:		//ADDIU
    case 0xc:		//ANDI
    case 0xf:		//LUI	
    case 0xd:		//ORI
    case 0xb:		//SLTIU
    case 0x23:		//LW	
    case 0x2b:		//SW
    case 0x4:		//BEQ
    case 0x5:		//BNE
    {
        int rt = (int) INST_INFO[i].r_t.r_i.rt;
        int rs = (int) INST_INFO[i].r_t.r_i.rs;
        int imm = (unsigned short) INST_INFO[i].r_t.r_i.r_i.imm;
        int sign_bit = imm >> 15;

        int sign_ext_imm = imm;
        if (sign_bit == 1) {
            sign_ext_imm = sign_ext_imm | (((1 << 16) - 1) << 16);
        }
        
        unsigned int branch_addr = sign_ext_imm << 2;

        switch (INST_INFO[i].opcode)
        {
        case 0x9:       //ADDIU
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + sign_ext_imm;
            break;
        case 0xc:       //ANDI
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & sign_ext_imm;
            break;   
        case 0xf:       //LUI
            CURRENT_STATE.REGS[rt] = imm << 16;
            break;
        case 0xd:       //ORI
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | sign_ext_imm;
            break;  
        case 0xb:       //SLTIU
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] < ((unsigned int) sign_ext_imm) ? 1: 0;
            break;   
        case 0x23:		//LW	
            CURRENT_STATE.REGS[rt] = mem_read_32((uint32_t) (CURRENT_STATE.REGS[rs] + sign_ext_imm));
            break;
        case 0x2b:		//SW
            mem_write_32(CURRENT_STATE.REGS[rs] + sign_ext_imm, CURRENT_STATE.REGS[rt]);
            break;
        case 0x4:		//BEQ
            CURRENT_STATE.PC = CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt] ? (CURRENT_STATE.PC + branch_addr): CURRENT_STATE.PC;
            break;
        case 0x5:		//BNE
            CURRENT_STATE.PC = CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt] ? (CURRENT_STATE.PC + branch_addr): CURRENT_STATE.PC;
            // printf("In bne instruction  and rt addrs: %d with rt: %d\n", CURRENT_STATE.REGS[rt], rt);
            break;
        default:
            break;
        }       
    }
    default:
        break;
    }

}
