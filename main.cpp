
#include "header.hpp"
#include <bitset>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>


typedef struct ITypeInstruction_Arithmetic_Imm {
    std::string inst;
    std::string rd;
    std::string rs1;
    std::string imm;
    std::string opcode;
    std::string funct3;

    std::string label;

} itype_arithmetic_imm;

typedef struct ITypeInstruction_Arithmetic_Shamt {
    std::string rd;
    std::string rs1;
    std::string sham;
    std::string opcode;
    std::string funct3;
    std::string funct7;

    std::string label;

} itype_arithmetic_shamt;

typedef struct ITypeInstruction_Load {
    std::string rd;
    std::string offset;
    std::string rs1;
    std::string opcode;
    std::string funct3;

    std::string label;

} itype_load;

typedef struct ITypeInstruction_Jump {
    std::string rd;
    std::string offset;
    std::string rs1;
    std::string opcode;

    std::string label;

} itype_jump;



std::string registerToBinary(const std::string registerName) {
    std::unordered_map<std::string, int> registerMap = {
        {"zero", 0}, 
        {"ra", 1}, 
        {"sp", 2}, 
        {"gp", 3}, 
        {"tp", 4}, 
        {"t0", 5},
        {"t1", 6},
        {"t2", 7},
        {"s0", 8}, // s0 or fp
        {"s1", 9},
        {"a0", 10},
        {"a1", 11},
        {"a2", 12},
        {"a3", 13},
        {"a4", 14},
        {"a5", 15},
        {"a6", 16},
        {"a7", 17},
        {"s2", 18},
        {"s3", 19},
        {"s4", 20},
        {"s5", 21},
        {"s6", 22},
        {"s7", 23},
        {"s8", 24},
        {"s9", 25},
        {"s10", 26},
        {"s11", 27},
        {"t3", 28},
        {"t4", 29},
        {"t5", 30},
        {"t6", 31},

    };

    int binary = registerMap[registerName];
    return std::bitset<5>(binary).to_string();

}


int convert_IType_Arithmetic_Imm(std::string instructionInput) {
    // tokenize to make instruction
    std:std::istringstream sstream(instructionInput);
    std::string instructionToken;
    std::vector<std::string> instructionTokens;

    while(sstream >> instructionToken) { // while sep by whitespace
        instructionTokens.push_back(instructionToken);

    }

    itype_arithmetic_imm instruction;
    
    instruction.inst=instructionTokens[0];
    instruction.rd=instructionTokens[1].substr(0, 2); // get rid of comma
    instruction.rs1=instructionTokens[2].substr(0, 2); // get rid of comma
    instruction.imm=instructionTokens[3];
    instruction.opcode="0010011";

    if(instruction.inst=="addi") {
        instruction.funct3="000";

    } else if(instruction.inst=="slti") {
        instruction.funct3="010";

    } else if(instruction.inst=="sltiu") {
        instruction.funct3="011";
        
    } else if(instruction.inst=="xori") {
        instruction.funct3="100";
        
    } else if(instruction.inst=="ori") {
        instruction.funct3="110";
        
    } else if(instruction.inst=="andi") {
        instruction.funct3="111";
        
    } 

    instruction.rd = registerToBinary(instruction.rd);
    instruction.rs1 = registerToBinary(instruction.rs1);
    instruction.imm = std::bitset<12>(std::stoi(instruction.imm)).to_string();

    instruction.label=instruction.imm+instruction.rs1+instruction.funct3+instruction.rd+instruction.opcode;

}

int convert_IType_Arithmetic_Shamt(std::string inst) {


}

int convert_IType_Load(std::string inst) {


}

int convert_IType_Jump(std::string inst) {


}

int main () {
    std::string filename = "add_shift.s";

    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::stringstream lineStream(line);

        std::string instruction;
        lineStream >> instruction; // get first token

        // i-type arithmetic imm
        if (instruction == "addi" || instruction == "slti" || instruction == "sltiu" || instruction == "xori" || instruction == "ori"|| instruction == "andi") {
            convert_IType_Arithmetic_Imm(line);

        } else if(instruction == "slli" || instruction == "srli" || instruction == "srai") {
            convert_IType_Arithmetic_Shamt(line);

        } else if(instruction == "lb" || instruction == "lh" || instruction == "lw" || instruction == "lbu" || instruction == "lhu") {
            convert_IType_Load(line);

        } else if(instruction == "jalr") {
            convert_IType_Jump(line);

        }

    }

    inputFile.close();

}