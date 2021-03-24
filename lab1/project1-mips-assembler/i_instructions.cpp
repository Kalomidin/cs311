//! This file executes all R-instructions defined for Lab1 \
//! Instructions are following:
//! - addiu
//! - sll
//! - srl
//! - ori
//! - andi
//! Branch Operations
//! - bne
//! - beq
//! Comparison Operations:
//! - sltiu
//! Load Operations:
//! - la
//! - lw
//! - lui

// Logical and Arithmetic Operations
#define ADDIU 0b001001 // Add signed
#define ORI 0b001101 // Or signed
#define ANDI 0b001100 // And signed

// Shift Operations
#define SLL 0b000000
#define SRL 0b000010

// Branch Operations
#define BNE 0b000101 // Branch Not Equal
#define BEQ 0b000100 // Branch Equal

// Comparison Operations
#define SLTIU 0b001011 // Compare with immediate

// Load Operations
#define LW 0b100011 // Load a word(32 bits or 4 bytes)
#define LUI 0b001111 // Load Upper 16 bits

// Save the word
#define SW 0b101011

// Include the string library
#include <string>
#include <vector> 

// Main files
#include "parser.h"

std::string execute_i_instr(std::string opcode, std::string rt, std::string rs, std::string immediate, int *instr_count) {
    int response = 0;
    int immed = 0;
    

    // Initialize immediate if possible
    if (is_number(immediate)) {
        immed = std::stoi(immediate, nullptr, 0);
        immed = immed & ((1 << 16) - 1); 
    }
    int rs_reg = std::stoi(rs, nullptr, 0);
    int rt_reg = std::stoi(rt, nullptr, 0);
    *instr_count = *instr_count + 1;
    

    if (opcode.compare("addiu") == 0) {
        response = ADDIU;
    } else if (opcode.compare("ori") == 0) {
        response = ORI;
    } else if (opcode.compare("andi") == 0) {
        response = ANDI;
    } else if (opcode.compare("bne") == 0 || opcode.compare("beq") == 0) {
        immed = get_fn_addr(immediate);
        immed = (immed - *instr_count);
        immed = immed & ((1 << 16) - 1);
        response = BNE;
        if (opcode.compare("beq") == 0) {
            response = BEQ;
        }
        response = response << 26;
        // std::cout <<"Imed is: " << immed << " " << decToBinary(immed) << "\n";
        response = response | (rt_reg << 21) | (rs_reg << 16) | immed;
        std::string result = decToBinary(response);
        return result;
    }  else if (opcode.compare("sltiu") == 0) {
        response = SLTIU;
    } else if (opcode.compare("lw") == 0) {
        response = LW;

    } else if (opcode.compare("lui") == 0) {
        // Update  Immed to memory address
        response = LUI;
    } else if(opcode.compare("sw") == 0) {
        response = SW;
    } else if (opcode.compare("srl") == 0 || opcode.compare("sll") == 0) {
        if (opcode.compare("srl") == 0) {
            response = SRL;
        } else {
            response = 0;
        }
        response = response | (rs_reg << 16) | (rt_reg << 11) | (immed << 6);
        return decToBinary(response);
    }


    if (opcode.compare("la") !=0 ) {
        response = response << 26;
        // std::cout << opcode << " LW is: " <<  response << " "<< decToBinary(response) << "\n";
        response = response | (rs_reg << 21) | (rt_reg << 16) | immed;
        // std::cout << "Response is: " << response << decToBinary(response) << "\n";
        return decToBinary(response);
    } else {
        int mem_addr = get_mem_addr(immediate);
        immed = mem_addr >> 16;
        // std::cout << mem_addr << " " << immediate <<" printed \n";
        response = LUI << 26;
        response = response | (rs_reg << 21) | (rt_reg << 16) | immed;
        std::string result;
        result += decToBinary(response);
        if (mem_addr % (1 << 16) == 0) {
            return result;
        } else {
            // Perform ORI
            response = ORI << 26;
            immed = mem_addr & ((1 << 16) - 1);
            response = response | (rt_reg << 21) | (rt_reg << 16) | immed;
            result += decToBinary(response);
            (*instr_count) = (*instr_count) + 1;
            return result;
        }
    }
    

}