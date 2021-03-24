#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>

// Include Parser
#include "parser.h"

#define MEMORY_ACCESS 0x1000_0000
#define INSTRUCTION_ACCESS 0x40_0000

int main(int argc, char* argv[]){

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n"); //Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else
	{

		// To help you handle the file IO, the deafult code is provided.
		// If we use freopen, we don't need to use fscanf, fprint,..etc. 
		// You can just use scanf or printf function 
		// ** You don't need to modify this part **
		// If you are not famailiar with freopen,  you can see the following reference
		// http://www.cplusplus.com/reference/cstdio/freopen/

		//For input file read (sample_input/example*.s)

		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));

		FILE *fp;
		fp = fopen(file, "r");

		if(fp==0){
			printf("File open Error!\n");
			exit(1);
		}

		
		// Read the file
		char * line = NULL;
		size_t len = 0;
		ssize_t read;
		std::vector<std::string> values;
		while ((read = getline(&line, &len, fp)) != -1) {
			values.push_back(std::string(line));
		}

		// std::cout << "One more time\n";

		// Remove new line
		for( int i =0; i < values.size();i++) {
			values[i] = values[i].substr(0, values[i].size() - 1);
		}

		// Move data to separate array
		// Remove new line
		std::vector<std::string> datas;
		int j = 0;
		int memory_access = (1 << 28);
		std::vector<fn> memory_addrs;
		while (true) {
			if (values[0].compare("\t.data") == 0) {
				values.erase(values.begin());
				continue;
			}
			// Remove the value
			if (values[0].compare("\t.text") == 0) {
				values.erase(values.begin() + 0);
				break;
			}
			size_t found = values[0].find(":");
			if (found != std::string::npos) {
				memory_addrs.push_back( fn {
					name: values[0].substr(0, found),
					instr_addr: (memory_access + (j << 2)),
				});
			}
			datas.push_back(values[0]);
			values.erase(values.begin() + 0);
			j++;
		}

		for( int i =0; i < memory_addrs.size();i++) {
			// std::cout << "Value is: " << memory_addrs[i].name << " " << memory_addrs[i].instr_addr << "\n";
		}

		for( int i =0; i < values.size();i++) {
			// std::cout << values[i] << "\n";
		}

		// function names and store their addresses
		int i = 0;
		// Resulting output
		std::string output;

		// Call the parser
		execute_file(values, memory_addrs);
		std::string binary_assembled = get_binary_assembles();
		// std::cout << "Value is: " << binary_assembled << "\n";
		int instr_count = get_instr_count();

		output = decToBinary(instr_count << 2);
		output += decToBinary((int) (datas.size() << 2));
		output += binary_assembled;
		
		// Datas to string
		for(i = 0; i < datas.size(); i++) {
			size_t k = datas[i].rfind("\t");
			int data = stoi(datas[i].substr(k + 1, datas[i].size()), nullptr, 0);
			output += decToBinary(data);
		}


		//From now on, if you want to read string from input file, you can just use scanf function.


		// For output file write 		// //If you use printf from now on, the result will be written to the output file.

		// printf("Hello World!\n"); 
		// You can see your code's output in the sample_input/example#.o 
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer
		file[strlen(file)-1] ='o';
		char path[100] = "./sample_input";
		size_t length = strlen(path);
		for(int k = strlen(file) - 1; k > 0; k--) {
			if (file[k] == '/') {
				for(j = k; j < strlen(file); j++) {
					if (file[j]  == '0') {
						break;
					}
					path[length + j-k] = file[j];
				}
				break;
			}
		}
		
		freopen(path,"w",stdout);

		// //If you use printf from now on, the result will be written to the output file.

		std::cout << output;
	}
	return 0;
}

