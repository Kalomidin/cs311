//! This file defines J-MIPS instructions
//! J-instructions that are included are following:
//! - jal
//! - jr
//! - j

// Include the string library
#include <string>
#include <vector> 

// Main files
#include "parser.h"

// Global Constants
#define J 0b000010
#define JAL 0b000011
#define JR 0b001000
#define OPCODE 26
#define INSTR_MEMORY_ACCESS 20

std::string execute_j_instr(std::string opcode, std::string immediate, int * instr_count) {
    int i32_opcode = 0;
    if (opcode.compare("j") == 0) {
        i32_opcode = J;
    } else if (opcode.compare("jal") == 0) {
        i32_opcode = JAL;
    } else {
        i32_opcode = JR;
    }
    // Move 26 bits to the left
    i32_opcode = i32_opcode << OPCODE;
    
    // If immediate is a register
    size_t found = immediate.find("$");
    if (found != std::string::npos) {
        int immed = std::stoi(immediate.substr(1), nullptr, 0);
        i32_opcode = i32_opcode >> 26;
        i32_opcode += (immed << 21);
    } else if (is_number(immediate)) {
        int immed = std::stoi(immediate, nullptr, 0);
        i32_opcode += immed + (1 << INSTR_MEMORY_ACCESS);
    } else {
        int immed = get_fn_addr(immediate);
        i32_opcode = i32_opcode + immed + (1 << INSTR_MEMORY_ACCESS);
    }
    (*instr_count) = (*instr_count) + 1;
    // Convert i32_opcode to binary string
    return decToBinary(i32_opcode);
 }