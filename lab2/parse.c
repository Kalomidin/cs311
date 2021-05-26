/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;


char * substring(int start, int end, const char *buffer) {
	char *temp = malloc(sizeof(char) * (end - start));
	memcpy(temp, &buffer[start], end-start);
	return temp;
}

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
	/** Implement this function */
	
	/* 1. Set the value */
	instr.value = (uint32_t) fromBinary(substring(0, 32, buffer));

	/* 2. Get the opcode */
	instr.opcode = fromBinary(substring(0, 6, buffer));

	/* 3. Match the instruction depeding on the opcode*/
	switch (instr.opcode)
	{
		//I format
	    case 0x9:		//ADDIU
	    case 0xc:		//ANDI
	    case 0xf:		//LUI	
	    case 0xd:		//ORI
	    case 0xb:		//SLTIU
	    case 0x23:		//LW	
	    case 0x2b:		//SW
	    case 0x4:		//BEQ
	    case 0x5:		//BNE
			instr.r_t.r_i.rs = (unsigned char) fromBinary(substring(6, 11, buffer));
			instr.r_t.r_i.rt = (unsigned char) fromBinary(substring(11, 16, buffer));
			instr.r_t.r_i.r_i.imm = (short) fromBinary(substring(16, 32, buffer));
			break;
		//R Format
		case 0x0:
			instr.r_t.r_i.rs = (unsigned char) fromBinary(substring(6, 11, buffer));
			instr.r_t.r_i.rt = (unsigned char) fromBinary(substring(11, 16, buffer));
			instr.r_t.r_i.r_i.r.rd = (unsigned char) fromBinary(substring(16, 21, buffer));
			instr.r_t.r_i.r_i.r.shamt = (unsigned char) fromBinary(substring(21, 26, buffer));
			instr.func_code = (short) fromBinary(substring(26, 32, buffer));
			break;
		case 0x2:
		case 0x3:
			instr.r_t.target = (uint32_t) fromBinary(substring(6, 32, buffer)) << 2;
			break;
	
	default:
		printf("Wrong instruction: %d\n", instr.opcode);
		exit(-1);
		break;
	}

	/* 4. Write the text to the memory*/
	mem_write_32(MEM_TEXT_START + index, instr.value);
    
	// /* 5. Increase the PC*/
	// CURRENT_STATE.PC += 4;
	
	return instr;
}



void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */
	mem_write_32(MEM_DATA_START + index, (uint32_t) fromBinary(substring(0, 32, buffer)));
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //I format
	    case 0x9:		//ADDIU
	    case 0xc:		//ANDI
	    case 0xf:		//LUI	
	    case 0xd:		//ORI
	    case 0xb:		//SLTIU
	    case 0x23:		//LW	
	    case 0x2b:		//SW
	    case 0x4:		//BEQ
	    case 0x5:		//BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //R format
	    case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //J format
	    case 0x2:		//J
	    case 0x3:		//JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
