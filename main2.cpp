
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
    std::string inst;
    std::string rd;
    std::string rs1;
    std::string shamt;
    std::string opcode;
    std::string funct3;
    std::string funct7;

    std::string label;

} itype_arithmetic_shamt;

typedef struct ITypeInstruction_Load {
    std::string inst;
    std::string rd;
    std::string imm;
    std::string rs1;
    std::string opcode;
    std::string funct3;

    std::string label;

} itype_load;

typedef struct ITypeInstruction_Jump {
    std::string inst;
    std::string rd;
    std::string imm;
    std::string rs1;
    std::string opcode;
    std::string funct3;

    std::string label;

} itype_jump;

std::vector<std::string> binaryInstructions; // holds all binary reps

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


int convert_IType_Arithmetic_Shamt(std::string instructionInput) {
    // tokenize to make instruction
    std:std::istringstream sstream(instructionInput);
    std::string instructionToken;
    std::vector<std::string> instructionTokens;

    while(sstream >> instructionToken) { // while sep by whitespace
        instructionTokens.push_back(instructionToken);

    }

    itype_arithmetic_shamt instruction;
    
    instruction.inst=instructionTokens[0];

    instruction.rd=instructionTokens[1];
    int commaIndex = instruction.rd.find(",");
    instruction.rd.erase(commaIndex, 1);

    instruction.rs1=instructionTokens[2];
    commaIndex = instruction.rs1.find(",");
    instruction.rs1.erase(commaIndex, 1);

    instruction.shamt=instructionTokens[3];
    instruction.opcode="0010011";

    if(instruction.inst=="slli") {
        instruction.funct3="001";
        instruction.funct7="0000000";

    } else if(instruction.inst=="srli") {
        instruction.funct3="101";
        instruction.funct7="0000000";

    } else if(instruction.inst=="srai") {
        instruction.funct3="101";
        instruction.funct7="0100000";
        
    } 

    instruction.rd = registerToBinary(instruction.rd);
    instruction.rs1 = registerToBinary(instruction.rs1);
    instruction.shamt = std::bitset<12>(std::stoi(instruction.shamt) & 0xFFF).to_string();

    instruction.label=instruction.shamt+instruction.rs1+instruction.funct3+instruction.rd+instruction.opcode; // if we need the string representation

    binaryInstructions.push_back(instruction.label); // add to a binary instructions vector

    return(std::stoi(instruction.label, nullptr, 2)); // if we need the int representation

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

    instruction.rd=instructionTokens[1];
    int commaIndex = instruction.rd.find(",");
    instruction.rd.erase(commaIndex, 1);

    instruction.rs1=instructionTokens[2];
    commaIndex = instruction.rs1.find(",");
    instruction.rs1.erase(commaIndex, 1);

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
    instruction.imm = std::bitset<12>(std::stoi(instruction.imm) & 0xFFF).to_string();

    instruction.label=instruction.imm+instruction.rs1+instruction.funct3+instruction.rd+instruction.opcode; // if we need the string representation

    binaryInstructions.push_back(instruction.label); // add to a binary instructions vector

    return(std::stoi(instruction.label, nullptr, 2)); // if we need the int representation

}

int convert_IType_Load(std::string instructionInput) {
    // tokenize to make instruction
    std:std::istringstream sstream(instructionInput);
    std::string instructionToken;
    std::vector<std::string> instructionTokens;

    while(sstream >> instructionToken) { // while sep by whitespace
        instructionTokens.push_back(instructionToken);

    }

    itype_load instruction;
    
    instruction.inst=instructionTokens[0];

    instruction.rd=instructionTokens[1];
    int commaIndex = instruction.rd.find(",");
    instruction.rd.erase(commaIndex, 1);

    int offset = std::stoi(instructionTokens[2].substr(0, instructionTokens[2].find('(')));
    instruction.imm = std::bitset<12>(offset & 0xFFF).to_string(); // & 0xFFF handles neg values

    int leftParenthesisIndex = instructionTokens[2].find('(');
    int rightParenthesisIndex = instructionTokens[2].find(')');
    std::string rs1 = instructionTokens[2].substr(leftParenthesisIndex+1, rightParenthesisIndex-leftParenthesisIndex-1);

    instruction.opcode="0000011";

    if(instruction.inst=="lb") {
        instruction.funct3="000";

    } else if(instruction.inst=="lh") {
        instruction.funct3="001";

    } else if(instruction.inst=="lw") {
        instruction.funct3="010";
        
    } else if(instruction.inst=="lbu") {
        instruction.funct3="100";
        
    } else if(instruction.inst=="lhu") {
        instruction.funct3="101";
        
    } 

    instruction.rd = registerToBinary(instruction.rd);
    instruction.rs1 = registerToBinary(rs1);

    instruction.label=instruction.imm+instruction.rs1+instruction.funct3+instruction.rd+instruction.opcode; // if we need the string representation

    binaryInstructions.push_back(instruction.label); // add to a binary instructions vector

    return(std::stoi(instruction.label, nullptr, 2)); // if we need the int representation

}

int convert_IType_Jump(std::string instructionInput) {
    // tokenize to make instruction
    std:std::istringstream sstream(instructionInput);
    std::string instructionToken;
    std::vector<std::string> instructionTokens;

    while(sstream >> instructionToken) { // while sep by whitespace
        instructionTokens.push_back(instructionToken);

    }

    itype_jump instruction;
    
    instruction.inst=instructionTokens[0];

    instruction.rd=instructionTokens[1];
    int commaIndex = instruction.rd.find(",");
    instruction.rd.erase(commaIndex, 1);

    int offset = std::stoi(instructionTokens[2].substr(0, instructionTokens[2].find('(')));
    instruction.imm = std::bitset<12>(offset & 0xFFF).to_string(); // & 0xFFF handles neg values

    int leftParenthesisIndex = instructionTokens[2].find('(');
    int rightParenthesisIndex = instructionTokens[2].find(')');
    std::string rs1 = instructionTokens[2].substr(leftParenthesisIndex+1, rightParenthesisIndex-leftParenthesisIndex-1);

    instruction.opcode="1100111";

    instruction.funct3="000";

    instruction.rd = registerToBinary(instruction.rd);
    instruction.rs1 = registerToBinary(rs1);

    instruction.label=instruction.imm+instruction.rs1+instruction.funct3+instruction.rd+instruction.opcode; // if we need the string representation

    binaryInstructions.push_back(instruction.label); // add to a binary instructions vector

    return(std::stoi(instruction.label, nullptr, 2)); // if we need the int representation

}

int main () {
    std::string filename = "test.s";

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

    for(int i=0; i<binaryInstructions.size(); i++) {
        printf("%s\n", binaryInstructions[i].c_str());

    }

    inputFile.close();

}