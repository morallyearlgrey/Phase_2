#include "rv32i.h"
#include <bitset>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <string>

std::string processRType(std::string line, const Instruction* instruction);

uint32_t currentAddress = 0x0;

void firstPass(std::string filename);

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput, const Instruction * instruction);
std::string convert_IType_Load_Jump(std::string instructionInput, const Instruction* instruction);

std::string processSType(std::string line, const Instruction* instruction);
std::string processBType(std::string line, const Instruction* instruction);
std::string processUType(std::string line, const Instruction* instruction);
std::string processJType(std::string line, const Instruction* instruction);
uint32_t parseImmediate(const std::string immediate);
int getRegisterAddressSafe(const std::string& registerName);
static std::unordered_map<std::string, int32_t> SymbolTable;
int PC;

int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        return 1;
    }
    
    std::string basePath = argv[1];
    std::string inputPath = argv[2]; 
    
    if (!basePath.empty() && basePath.back() != '/') {
        basePath += '/';
    }
    
    std::string filename = inputPath;
    size_t lastSlash = inputPath.rfind('/');
    if (lastSlash != std::string::npos) {
        filename = inputPath.substr(lastSlash + 1);
    }
    
    std::string baseName = filename;
    size_t dotPos = baseName.rfind('.');
    if (dotPos != std::string::npos) {
        baseName = baseName.substr(0, dotPos);
    }
    
    std::string outputBinPath = basePath + baseName + ".bin";
    std::string outputHexPath = basePath + baseName + ".hex.txt";
    
	firstPass(inputPath);

    std::ifstream inputFile(inputPath);

    std::ofstream binaryFile(outputBinPath);
    std::ofstream hexFile(outputHexPath);

    if (!inputFile.is_open()) {
        return 1;
    }

    std::string line;  
    int lineNum = 0;
    PC = 0; 
    while (std::getline(inputFile, line)) {
        std::cout << lineNum << ": \t|";
        std::cout << line;
        
        lineNum++;

        size_t commentPos = line.find('#');
        std::string cleanLine = line;
        if (commentPos != std::string::npos) {
            cleanLine = line.substr(0, commentPos);
        }

        std::stringstream ss(cleanLine);
        std::string token; 
        ss >> token;

        if (token.empty() || token == ".data" || token == ".text" || token == ".globl" || token == ".word") {
            std::cout << std::endl;
            continue;
        }

        if (token.find(':') != std::string::npos) {
            if (ss >> token) {
                //There's something after the label, continue processing
            } else {
                std::cout << std::endl;
                continue;
            }
        }

        const Instruction *instruction = getInstructions(token);

        std::string label = "";

        if (instruction != nullptr) {
            switch (instruction->type) {

            case InstructionType::R:
                label = processRType(cleanLine, instruction);
                break;
            case InstructionType::I:
                if (instruction->name == "addi" || instruction->name == "slti" || instruction->name == "sltiu" || instruction->name == "xori" || instruction->name == "ori"|| instruction->name == "andi" || instruction->name == "slli" || instruction->name == "srli" || instruction->name == "srai") {
                    label = convert_IType_Arithmetic_Imm_Shamt(cleanLine, instruction);

                } else if(instruction->name == "lb" || instruction->name == "lh" || instruction->name == "lw" || instruction->name == "lbu" || instruction->name == "lhu" || instruction->name == "jalr") {
                    label = convert_IType_Load_Jump(cleanLine, instruction);

                }
                break;

            case InstructionType::S:
                label = processSType(cleanLine, instruction);
                break;

            case InstructionType::B:
                label = processBType(cleanLine, instruction);
                break;

            case InstructionType::U:
                label = processUType(cleanLine, instruction);
                break;

            case InstructionType::J:
                label = processJType(cleanLine, instruction);
                break;

            default:
                std::cout << "defaulting";
                break;
            }
            
            PC += 4;
        }

        if(!label.empty() && label.length() == 32) {
            if(binaryFile.is_open()) {

                binaryFile << label << std::endl;

            }


            uint32_t binaryDecimalForm = std::bitset<32>(label).to_ulong();
            std::string hexForm = binaryToHex(binaryDecimalForm);

            if(hexFile.is_open()) {

                hexFile << "0x" + hexForm << std::endl;

            }

        }

    std::cout << std::endl;
  }


  binaryFile.close();
  hexFile.close();
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

std::string processRType(std::string line, const Instruction *instruction) {
  std::stringstream ss(line);
  std::string token;
  std::string rd;
  std::string rs1;
  std::string rs2;
  std::string funct7;
  std::string funct3;
  std::string opcode;

  ss >> token; 
  if (token.find(':') != std::string::npos) {
      ss >> token;
  }
  
  ss >> rd;
  if (!rd.empty() && rd.back() == ',') 
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

  std::string binary_funct7 = std::bitset<7>(instruction->funct7).to_string();
  std::string binary_funct3 = std::bitset<3>(instruction->funct3).to_string();
  std::string binary_opcode = std::bitset<7>(instruction->opcode).to_string();

  std::string binaryResult = binary_funct7 + binary_rs2 + binary_rs1 +
                           binary_funct3 + binary_rd + binary_opcode;

    return binaryResult;

}

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput, const Instruction * instruction) {
	std::istringstream sstream(instructionInput);
	std::string instructionToken;
	std::vector<std::string> instructionTokens;

	while(sstream >> instructionToken) {
		instructionTokens.push_back(instructionToken);
	}
	
	int offset = 0;
	if (!instructionTokens.empty() && instructionTokens[0].find(':') != std::string::npos) {
		offset = 1;
	}
	
    if(instructionTokens.size() < offset + 4) {
        return std::string(32, '0'); 
    }

	std::string rd = instructionTokens[offset + 1];
	int commaIndex = rd.find(",");
	if(commaIndex != std::string::npos) {
        rd.erase(commaIndex, 1);
    }

	std::string rs1 = instructionTokens[offset + 2];
	commaIndex = rs1.find(",");
	if(commaIndex != std::string::npos) {
        rs1.erase(commaIndex, 1);
    }

	std::string immshamt = instructionTokens[offset + 3];

    const Register* rd_reg = getRegister(rd);
    const Register* rs1_reg = getRegister(rs1);
    
    if (rd_reg == nullptr || rs1_reg == nullptr) {
        return std::string(32, '0');
    }

	rd = std::bitset<5>(getRegister(rd)->address).to_string();
	rs1 = std::bitset<5>(getRegister(rs1)->address).to_string();

    int immediateValue = parseImmediate(immshamt);
    immshamt = std::bitset<12>(immediateValue & 0xFFF).to_string();


	return (immshamt+rs1+std::bitset<3>((instruction->funct3) & 0xFFF).to_string()+rd+std::bitset<7>((instruction->opcode) & 0xFFF).to_string());
}

std::string convert_IType_Load_Jump(std::string instructionInput, const Instruction* instruction) {
	std::istringstream sstream(instructionInput);
	std::string instructionToken;
	std::vector<std::string> instructionTokens;

	while(sstream >> instructionToken) {
		instructionTokens.push_back(instructionToken);
	}

    if(instructionTokens.size() < 3) {
        return std::string(32, '0');
    }

	std::string rd = instructionTokens[1];
	int commaIndex = rd.find(",");
	if(commaIndex != std::string::npos) {  
        rd.erase(commaIndex, 1);
    }


	std::string offsetrs1 = instructionTokens[2];

	std::string offsetStr = (offsetrs1.substr(0, offsetrs1.find('(')));
	int offset = parseImmediate(offsetStr);

    std::string imm = std::bitset<12>(offset & 0xFFF).to_string();
    
	int leftparenthesis = offsetrs1.find('(');
	int rightparenthesis = offsetrs1.find(')');
	std::string rs1 = offsetrs1.substr(leftparenthesis+1, rightparenthesis-leftparenthesis-1);

    const Register* rd_reg = getRegister(rd);
    const Register* rs1_reg = getRegister(rs1);
    
    if (rd_reg == nullptr || rs1_reg == nullptr) {
        return std::string(32, '0');
    }

	rd = std::bitset<5>(getRegister(rd)->address).to_string();
	rs1 = std::bitset<5>(getRegister(rs1)->address).to_string();


	return (imm+rs1+std::bitset<3>(instruction->funct3).to_string()+rd+std::bitset<7>(instruction->opcode).to_string());
}

std::string processBType(std::string line, const Instruction *instruction) {
  
  std::stringstream ss(line);
  std::string token;
  std::string rs1;
  std::string rs2;
  std::string label;

  ss >> token; 
  ss >> rs1;
  if (!rs1.empty() && rs1.back() == ',')
    rs1.pop_back();

  ss >> rs2;
  if (!rs2.empty() && rs2.back() == ',')
    rs2.pop_back();

  ss >> label;
  const Register* reg1 = getRegister(rs1);
  uint32_t rs1_b = 0;
  if (reg1 != nullptr) {
    rs1_b = reg1->address;
  } else {
    std::cout << "Register not found: " << rs1 << "\n";
  }
  const Register* reg2 = getRegister(rs2);
  uint32_t rs2_b = 0;
  if (reg2 != nullptr) {
    rs2_b = reg2->address;
  } else {
    std::cout << "Register not found: " << rs2 << "\n";
  }

  uint32_t funct3_b = instruction->funct3;

  auto it = SymbolTable.find(label);
  uint32_t label_b = 0;
  if (it != SymbolTable.end()) {
    label_b = it->second;
  } else {
    std::cout << "Label not found: " << label << "\n";
  }

  int32_t offset = label_b - PC;
  int32_t imm = offset >> 1;

  uint32_t imm12   = (imm >> 12) & 0x1;
  uint32_t imm11   = (imm >> 11) & 0x1;
  uint32_t imm10_5 = (imm >> 5)  & 0x3F;
  uint32_t imm4_1  = (imm >> 1)  & 0xF;

  uint32_t inst = 0;
  inst |= (imm12   << 31);
  inst |= (imm10_5 << 25);
  inst |= (rs2_b   << 20);
  inst |= (rs1_b   << 15);
  inst |= (funct3_b << 12);
  inst |= (imm4_1  << 8);
  inst |= (imm11   << 7);
  inst |= instruction->opcode; 

  return std::bitset<32>(inst).to_string();

}

std::string processSType(std::string line, const Instruction* instruction)
{
	std::stringstream ss(line);
	std::string token;
	std::string rs1;
	std::string rs2;
	std::string offset;
	std::string funct3;
	std::string opcode;

	ss >> token; 
	ss >> rs2; 
	rs2.pop_back();
	ss >> offset; 		
	

	int start = offset.find('(');
	int end = offset.find(')');

	rs1 = offset.substr(start+1, end - start -1);
	offset = offset.substr(0, start);

	int rs2_int = getRegisterAddressSafe(rs2);
    int rs1_int = getRegisterAddressSafe(rs1);

	std::string imm_40 = std::bitset<12>(offset).to_string().substr(7);
	std::string imm_115 = std::bitset<12>(offset).to_string().substr(0,7);

	std::string binary = imm_115 + std::bitset<5>(rs2_int).to_string()+std::bitset<5>(rs1_int).to_string()+ std::bitset<3>(instruction->funct3).to_string()+imm_40 + std::bitset<7>(instruction->opcode).to_string();

  return binary;

}
std::string processUType(std::string line, const Instruction* instruction)
{
	std::stringstream ss(line);
	std::string token;
	std::string rd;
	std::string imm;
	std::string opcode;
	int rd_int;

	ss >> token; 	
	ss >> rd;
	if (!rd.empty() && rd.back() == ',')
		rd.pop_back();
	ss >> imm; 	
	
	const Register* reg = getRegister(rd);
	if (reg == nullptr) {
		std::cerr << "Error: Invalid register '" << rd << "' in U-type instruction" << std::endl;
		return std::string(32, '0');
	}
	rd_int = reg->address;

	uint32_t imm_value = parseImmediate(imm);

	std::string binary;
	binary = std::bitset<20>(imm_value).to_string() + std::bitset<5>(rd_int).to_string() + std::bitset<7>(instruction->opcode).to_string();
	return binary;
}
std::string processJType(std::string line, const Instruction* instruction)
{

	std::stringstream ss(line);
	std::string token;
	std::string rd;
	std::string label;
	std::string opcode;
	int rd_int;

	ss >> token; 	
	if (token.find(':') != std::string::npos) {
		ss >> token; 
	}
	
	ss >> rd;
	if (!rd.empty() && rd.back() == ',')
		rd.pop_back();
	ss >> label; 
	
	if (rd.empty() || label.empty()) {
		std::cerr << "Error: Not enough tokens in JAL instruction: '" << line << "'" << std::endl;
		return std::string(32, '0');
	}
	
	const Register* reg = getRegister(rd);
	if (reg == nullptr) {
		std::cerr << "Error: Invalid register '" << rd << "' in JAL instruction" << std::endl;
		return std::string(32, '0');
	}
	rd_int = reg->address;

	int32_t offset = 0;
	if (SymbolTable.find(label) != SymbolTable.end()) {
		int32_t target_addr = SymbolTable[label];
		offset = target_addr - PC;
	} else {
		std::cerr << "Error: Label '" << label << "' not found in symbol table" << std::endl;
		return std::string(32, '0');
	}

	uint32_t imm = (uint32_t)offset;
	
	uint32_t imm20 = (imm >> 20) & 0x1;     
	uint32_t imm10_1 = (imm >> 1) & 0x3FF;  
	uint32_t imm11 = (imm >> 11) & 0x1;      
	uint32_t imm19_12 = (imm >> 12) & 0xFF;  

	uint32_t j_imm = 0;
	j_imm |= (imm20 << 19);      
	j_imm |= (imm19_12 << 11);   
	j_imm |= (imm11 << 10);      
	j_imm |= imm10_1;     

	uint32_t inst = 0;
	inst |= (j_imm << 12);
	inst |= (rd_int << 7);
	inst |= instruction->opcode;

	std::string binary = std::bitset<32>(inst).to_string();
	return binary;
}


uint32_t parseImmediate(const std::string immediate) {
	if(immediate.find("%hi")==0) {
		int startIndex = immediate.find('(')+1;
		int endIndex = immediate.find(')');
		std::string imm = immediate.substr(startIndex, endIndex-startIndex);

		if(SymbolTable.find(imm)!=SymbolTable.end()) {
            int address = SymbolTable[imm];
            return(address>>12)&0xFFFFF;

        } else {
            return -1;

        }

	}

    if(immediate.find("%lo")==0) {
        int startIndex = immediate.find('(')+1;
		int endIndex = immediate.find(')');
		std::string imm = immediate.substr(startIndex, endIndex-startIndex);

        if(SymbolTable.find(imm)!=SymbolTable.end()) {
            int address = SymbolTable[imm];
            return address & 0xFFF;

        } else {
            return -1;

        }

    }

    return std::stoi(immediate); 

}

void firstPass(std::string filename) {
	std::ifstream inputFile(filename);
	if (!inputFile.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}

	std::string line;
	currentAddress = 0x0; 

	while (std::getline(inputFile, line)) {
		size_t commentPos = line.find('#');
		if (commentPos != std::string::npos) {
			line = line.substr(0, commentPos);
		}

		std::stringstream ss(line);
		std::string token;
		if (!(ss >> token)) continue; 

		if (token.back() == ':') {
			std::string labelName = token.substr(0, token.length() - 1);
		    SymbolTable[labelName] = currentAddress;
			
			if (!(ss >> token)) continue; 
		}

		if (token == ".data") {
			continue;
		}
		if (token == ".text") {
			continue;
		}
		if (token == ".word") {
			currentAddress += 4;
			continue;
		}
		if (token == ".globl") {
			continue;
		}
		
		if (getInstructions(token) != nullptr) {
			currentAddress += 4;
		}
	}
	inputFile.close();
}

int getRegisterAddressSafe(const std::string& registerName) {
    const Register* reg = getRegister(registerName);
    if (reg == nullptr) {
        std::cerr << "Error:'" << registerName << "'" << std::endl;
        return 0;
    }
    return reg->address;
}