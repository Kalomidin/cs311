#include <vector>

// Main files
#include "parser.h"

#define ADDU 0b100001
#define AND 0b100100
#define NOR 0b100111
#define OR 0b100101
#define SLTU 0b101011
#define SUBU 0b100011



std::string execute_r_instr(std::string opcode, std::string rd, std::string rs, std::string rt, int *instr_count) {
    int response;
    int rd_reg = std::stoi(rd, nullptr, 0);
    int rs_reg = std::stoi(rs, nullptr, 0);
    int rt_reg = std::stoi(rt, nullptr, 0);
    if (opcode.compare("addu") == 0) {
        response = ADDU;
    } else if (opcode.compare("and") == 0) {
        response = AND;
    } else if (opcode.compare("nor") == 0) {
        response = NOR;
    } else if (opcode.compare("or") == 0) {
        response = OR;
    } else if (opcode.compare("sltu") == 0) {
        response = SLTU;
    } else if (opcode.compare("subu") == 0) {
        response = SUBU;
    }
    response = response + (rs_reg << 21) + (rt_reg << 16) + (rd_reg << 11);
    (*instr_count) = (*instr_count) + 1;
    return decToBinary(response);
}

//	nor	$16, $17, $18 - 000000 10001 10010 10001 00000 100111
//                      000000 10001 10010 10000 00000 100111