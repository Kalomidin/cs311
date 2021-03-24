#include <string>
#include <vector>

// Checks if _ANIMALS IF DECLARED 
#ifndef _PARSER_
  
// Defines _ANIMALS_ if above 
// conditions fails 
#define _PARSER_ 
struct fn {
    std::string name;
    int32_t instr_addr;
};

void execute_file(std::vector<std::string> instructions, std::vector<fn> func_addrs);
std::string get_binary_assembles();
int get_instr_count();
bool is_number(std::string s);
std::string decToBinary(int n);
int get_fn_addr(std::string s);
int get_mem_addr(std::string s);
#endif