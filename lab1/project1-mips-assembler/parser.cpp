#include <vector>
#include <string>
#include <iostream>
#include<string.h>
#include "parser.h"
#include "i_instructions.cpp"
#include "j_instructions.cpp"
#include "r_instructions.cpp"
#include "main.cpp"

using namespace std;

static vector<fn> fns;
static vector<fn> mem_addrs;
static string assembles;
static int instr_count;

void parser(string str) {

    // Copy the string to s
    string s = str.substr(0);

    size_t tab = s.find("\t");
    s =s.substr(tab + 1);
    size_t space = s.find("\t");
    string opcode = s.substr(0, space);
    s =s.substr(space + 1);
    
    size_t comma = s.find(",");
    // cout <<"Running parser\n";

    /* 1. `jal/j/jr Check if the command is j, jr or jal */
    if (comma != string::npos) {

        // $rd for R-MIPS
        // $rt for I-MIPS
        string first_register = s.substr(1, comma - 1);
        s = s.substr(comma + 2);

        /* 2. Check if the next `is register` */
        if (s.at(0) == '$') {
            string rs, rt, immediate;
            /*
                3. Get the second register
                $rs: For R-MIPS
                $rs: For I-MIPS
            */
            comma = s.find(",");
            rs = s.substr(1, comma - 1);

            s = s.substr(comma + 2);

            /*4. Check if the next is register rt or immediate*/
            if (s.at(0) == '$') {
                rt = s.substr(1);
                assembles += execute_r_instr(opcode, first_register, rs, rt, &instr_count);
                
            } else {
                immediate = s;
                assembles += execute_i_instr(opcode, first_register, rs, immediate, &instr_count);
            }
        } else {
            string rs;
            string immediate;

            /*4 $rt: Check if the rt is there*/
            size_t has_register = s.find("$");
            if (has_register != string::npos) {
                // Ex: lw	$8, 4($3)
                size_t first_bracket = s.find('(');
                size_t second_bracket = s.find(')');

                rs = s.substr(first_bracket + 2, second_bracket - first_bracket - 2);
                immediate = s.substr(0, first_bracket);
            } else {
                // Ex: lui	$3, 0x1000
                rs = "0";
                immediate = s;
            }
            assembles += execute_i_instr(opcode, first_register, rs, immediate, &instr_count);
        }
    } else {
        // Ex: 	j	exit
        string immediate = s;
        assembles +=execute_j_instr(opcode, immediate, &instr_count);
    }
}

void execute_file(vector<string> instructions, vector<fn> memory_addrs) {
    instr_count = 0;
    assembles = "";
    mem_addrs = memory_addrs;
    for(int i =0; i < instructions.size(); i++) {
        size_t found = instructions[i].find("\t");
        if (found == string::npos) {
            fns.push_back( fn {
                name: instructions[i].substr(0, instructions[i].size() - 1),
                instr_addr: instr_count,
            });
            continue;
        }
        parser(instructions[i]);
    }

    instr_count = 0;
    assembles = "";

    for(int i =0; i < instructions.size(); i++) {
        size_t found = instructions[i].find("\t");
        if (found == string::npos) {
            continue;
        }
        parser(instructions[i]);
    }
}

string get_binary_assembles() {
    return assembles.substr(0);
}

int get_instr_count() {
    return instr_count;
}

bool is_number(string s)
{
    std::string::const_iterator it = s.begin();
    if (s.empty()) {
        return false;
    }
    if (s.size() == 1 && s.at(0) == '0') {
        return true;
    }
    
    if (s.at(0) == '0' && s.at(1) == 'x') {
        // TODO: Check if it is hexadecimal
        return true;
    } else {
        if (s.at(0) == '-') {
            it++;
        }
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }
}

// function to convert decimal to binary 
string decToBinary(int n) 
{ 
    // array to store binary number 
    string binaryNum[32]; 
    if ( n < 0) {
        binaryNum[31] = "1";
        n = n ^ (1 << 31);
    } else {
        binaryNum[31] = "0";
    }
    string response;
    // counter for binary array 
    int i = 0; 
    while (n > 0 || i < 31) { 
  
        // storing remainder in binary array 
        binaryNum[i] = to_string(n % 2); 
        n = n / 2; 
        i++; 
    } 
    i++;
  
    // printing binary array in reverse order 
    for (int j = i - 1; j >= 0; j--) 
        response += binaryNum[j]; 
    return response;
} 

int get_fn_addr(string s) {
    for(int i=0; i <fns.size(); ++i) {
        if (fns[i].name.compare(s) == 0) {
            return fns[i].instr_addr;  
        }
    }
    return 0;
}

int get_mem_addr(string s) {
    for(int i=0; i <mem_addrs.size(); ++i) {
        if (mem_addrs[i].name.compare(s) == 0) {
            return mem_addrs[i].instr_addr;  
        }
    }
    return 0;
}


// int main() {
//     struct fn lab1 = fn {
//         name: "lab2",
//         instr_addr: 20,
//     };
//     instr_count = 24;
//     fns.push_back(lab1);
//     string s = "	bne	$11, $8, lab2"; // Expected
//     mem_addr = (1<<28) + (1 << 2);
//     cout <<mem_addr << " Before\n";
//     parser(s);
//     cout << mem_addr << " After \n";
//     cout<< "Output:"<< assembles<<", Length:" << assembles.length() << "\n";
//     return 1; 
// }