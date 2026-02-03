#include "rv32i.h"
#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>


void processRType(std::string line, const Instruction* instruction);

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput, const Instruction * instruction);
std::string convert_IType_Load_Jump(std::string instructionInput, const Instruction* instruction);

void processSType(std::string line, const Instruction* instruction);
void processBType(std::string line, const Instruction* instruction);
void processUType(std::string line, const Instruction* instruction);
void processJType(std::string line, const Instruction* instruction);


int main() {
  // The filename to read
  std::string filename = "test.s";

  // Open the file
  std::ifstream inputFile(filename);

  // Check if the file opened successfully
  if (!inputFile.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return 1;
  }

  // Read and print line by line
  std::string line;  // Create the base string object
  std::string token; // create base toekn
  int lineNum = 0;
  while (std::getline(inputFile, line)) {

    std::cout << lineNum << ": \t|";
    std::cout << line;
    // for (char c : line) {
    // 	std::cout << c << std::endl;
    // }
    lineNum++;

    // Here we should have it such that it check what type of line it is:
    // the types are:
    //	Function (ie main:)
    //	instruction (add)
    //	comment (#)
    //	and for now, unknown
    // Let start with creating
    // we should use the hader file for doing the following, checking if the
    // header variable exists, and if so we know it is an instruction, and from
    // there we should return back the struct of that object
    //		IE: for line 21, it is lui, which is an instruction, and should
    // return (U, 011 0111, 2) (type, op code, number of oprerands)
    //
    //		how do we do a lookup on the header file for checking to see if
    // it exists.

    // first things first, lets get the tokens for now

    std::stringstream ss(line);
    ss >> token;

    // std::cout << token<< '\t';

    if (token == "#") {
      // std::cout << "\tit was a comment"  << std::endl;

    } else {
      const Instruction *instruction = getInstructions(token);

      if (instruction != nullptr) {
        // std::cout << "Found the following: ";
        // std::cout << instruction->name << '\t';
        // std::cout << instruction->opcode<< '\t';
        // std::cout <<  typeToString(instruction->type)<< '\t';
        // std::cout << instruction->funct3 << '\t';
        // std::cout << instruction->funct7 << '\t';

        // this means we found a proper instructions

        switch (instruction->type) {

        case InstructionType::R:
          processRType(line, instruction);
          break;
        case InstructionType::I:
            if (instruction->name == "addi" || instruction->name == "slti" || instruction->name == "sltiu" || instruction->name == "xori" || instruction->name == "ori"|| instruction->name == "andi" || instruction->name == "slli" || instruction->name == "srli" || instruction->name == "srai") {
                convert_IType_Arithmetic_Imm_Shamt(line, instruction);

            } else if(instruction->name == "lb" || instruction->name == "lh" || instruction->name == "lw" || instruction->name == "lbu" || instruction->name == "lhu" || instruction->name == "jalr") {
                convert_IType_Load_Jump(line, instruction);

            }
            break;

        case InstructionType::S:
          processSType(line, instruction);
          break;
        case InstructionType::B:
          processBType(line, instruction);
          break;
        case InstructionType::U:
          processUType(line, instruction);
          break;
        case InstructionType::J:
          processJType(line, instruction);
          break;
        default:
          std::cout << "defaulting";
          break;
        }
      }
    }
    std::cout << std::endl;
    // now we do checks to see what type of thing it is
  }

  // Close the file (optional as destructor closes it, but good practice)
  inputFile.close();

  return 0;
}

std::string registerToBinary(const std::string registerName) {
  std::unordered_map<std::string, int> registerMap = {
      {"zero", 0}, {"ra", 1},  {"sp", 2},  {"gp", 3},  {"tp", 4},  {"t0", 5},
      {"t1", 6},   {"t2", 7},  {"s0", 8}, // s0 or fp
      {"s1", 9},   {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14},
      {"a5", 15},  {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20},
      {"s5", 21},  {"s6", 22}, {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26},
      {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31},

  };

  int binary = registerMap[registerName];
  return std::bitset<5>(binary).to_string();
}

void processRType(std::string line, const Instruction *instruction) {
  // R = funct 7 + rs2 + rs1 + funct3 + rd + opcode
  std::stringstream ss(line);
  std::string token;
  std::string rd;
  std::string rs1;
  std::string rs2;
  std::string funct7;
  std::string funct3;
  std::string opcode;

  // parse lin
  ss >> token; // gets instruction name
  ss >> rd;
  if (!rd.empty() && rd.back() == ',') // remove commas
    rd.pop_back();

  ss >> rs1;
  if (!rs1.empty() && rs1.back() == ',')
    rs1.pop_back();

  ss >> rs2;
  if (!rs2.empty() && rs2.back() == ',')
    rs2.pop_back();

  // get binary registers
  std::string binary_rd = registerToBinary(rd);
  std::string binary_rs1 = registerToBinary(rs1);
  std::string binary_rs2 = registerToBinary(rs2);

  // turn the functs into binary using bitset
  std::string binary_funct7 = std::bitset<7>(instruction->funct7).to_string();
  std::string binary_funct3 = std::bitset<3>(instruction->funct3).to_string();
  std::string binary_opcode = std::bitset<7>(instruction->opcode).to_string();

  // // result = funct7 + rs2 + rs1 + funct3 + rd + opcode;
  // std::cout << std::endl;
  // std::cout << "Printing important details for this line\n\t";
  // std::cout << rd + "\n\t" + rs1 + "\n\t" + rs2 << std::endl;

  // get final result
  std::string binaryResult = binary_funct3 + binary_rs2 + binary_rs1 +
                             binary_funct7 + binary_rd + binary_opcode;

}

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput, const Instruction * instruction) {
	std::istringstream sstream(instructionInput);
	std::string instructionToken;
	std::vector<std::string> instructionTokens;

	while(sstream >> instructionToken) {
		instructionTokens.push_back(instructionToken);
	}

	std::string rd = instructionTokens[1];
	int commaIndex = rd.find(",");
	rd.erase(commaIndex, 1);

	std::string rs1 = instructionTokens[2];
	commaIndex = rs1.find(",");
	rs1.erase(commaIndex, 1);

	std::string immshamt = instructionTokens[3];

	rd = std::bitset<5>(getRegister(rd)->address).to_string();
	rs1 = std::bitset<5>(getRegister(rs1)->address).to_string();
	immshamt = std::bitset<12>(std::stoi(immshamt) & 0xFFF).to_string();

	return (immshamt+rs1+std::bitset<3>((instruction->funct3) & 0xFFF).to_string()+rd+std::bitset<7>((instruction->opcode) & 0xFFF).to_string());
}

std::string convert_IType_Load_Jump(std::string instructionInput, const Instruction* instruction) {
	std::istringstream sstream(instructionInput);
	std::string instructionToken;
	std::vector<std::string> instructionTokens;

	while(sstream >> instructionToken) {
		instructionTokens.push_back(instructionToken);
	}

	std::string rd = instructionTokens[1];
	int commaIndex = rd.find(",");
	rd.erase(commaIndex, 1);

	std::string offsetrs1 = instructionTokens[2];

	int offset = std::stoi(offsetrs1.substr(0, offsetrs1.find('(')));
	
    std::string imm = std::bitset<12>(offset & 0xFFF).to_string();
    
	int leftparenthesis = offsetrs1.find('(');
	int rightparenthesis = offsetrs1.find(')');
	std::string rs1 = offsetrs1.substr(leftparenthesis+1, rightparenthesis-leftparenthesis-1);

	rd = std::bitset<5>(getRegister(rd)->address).to_string();
	rs1 = std::bitset<5>(getRegister(rs1)->address).to_string();

	return (imm+rs1+std::bitset<3>(instruction->funct3).to_string()+rd+std::bitset<7>(instruction->opcode).to_string());
}

void processSType(std::string line, const Instruction* instruction)
{
	// S =imm + rs2 + rs1 + funct3 + imm + opcode
	
}
void processBType(std::string line, const Instruction* instruction)
{
	// B = imm + rs2 + rs1 + funct3 + imm + opcode
}
void processUType(std::string line, const Instruction* instruction)
{
	// U = imm + rd + opcodew
}
void processJType(std::string line, const Instruction* instruction)
{
	// J = imm + rd + opcode	
}

