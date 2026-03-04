#include "rv32i.h"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>


std::string processRType(std::string line, const Instruction *instruction);

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput,
                                               const Instruction *instruction);
std::string convert_IType_Load_Jump(std::string instructionInput,
                                    const Instruction *instruction);

std::string processSType(std::string line, const Instruction *instruction);
std::string processBType(std::string line, const Instruction *instruction);
std::string processUType(std::string line, const Instruction *instruction);
std::string processJType(std::string line, const Instruction *instruction);
uint32_t parseImmediate(const std::string immediate);

// Global map to store label addresses
static std::unordered_map<std::string, int32_t> SymbolTable;
int PC;

uint32_t getBaseAddress() { return 0x0; }

// Returns true if successful, false otherwise
bool firstPass(std::string filename);

int main(int argc, char *argv[]) {
  std::string filename = std::string(argv[2]);
  std::string outputBase = filename;
  if (outputBase.length() > 2)
    outputBase.erase(outputBase.length() - 2, 2);

  std::string binFilename = outputBase + ".bin";
  std::string hexFilename = outputBase + ".hex.txt";

  if (!firstPass(filename)) {
    return 1;
  }

  // Open the file
  std::ifstream inputFile(filename);

  // Check if the file opened successfully
  if (!inputFile.is_open()) {
    std::cerr << "Error: Could not open file1 " << filename << std::endl;
    return 1;
  }

  std::ofstream binaryFile(binFilename);
  std::ofstream hexFile(hexFilename);

  // Read and print line by line
  std::string line;  // Create the base string object
  std::string token; // create base toekn
  int lineNum = 0;

  PC = getBaseAddress(); // Reset PC for second pass

  while (std::getline(inputFile, line)) {
    // std::cout << lineNum << ": \t|";
    // std::cout << line;

    lineNum++;

    std::stringstream ss(line);
    if (!(ss >> token))
      continue; // Empty line or whitespace

    // Handle Comments
    if (token == "#" || token[0] == '#') {
      // std::cout << std::endl;
      continue;
    }

    std::string instructionLine = line;

    // Handle Labels (skip, addressed in first pass)
    if (token.back() == ':') {
      if (!(ss >> token)) { // If nothing else on line
        // std::cout << std::endl;
        continue;
      }
      // Update instructionLine to point after the label
      size_t colonPos = line.find(':');
      if (colonPos != std::string::npos) {
        instructionLine = line.substr(colonPos + 1);
      }
    }

    // Handle Directives
    if (token == ".data" || token == ".text" || token == ".globl") {
      // std::cout << std::endl;
      continue;
    }

    // Handle .word logic
    if (token == ".word") {
      int32_t value;
      ss >> value;
      // Write to files - DISABLED output to match Gradescope requirements
      // Only PC is incremented.

      PC += 4;
      // std::cout << "\t[DATA: " << value << "]";
      // std::cout << std::endl;
      continue;
    }

    const Instruction *instruction = getInstructions(token);
    std::string label = "";

    if (instruction != nullptr) {
      switch (instruction->type) {
      case InstructionType::R:
        label = processRType(instructionLine, instruction);
        break;
      case InstructionType::I:
        if (instruction->name == "addi" || instruction->name == "slti" ||
            instruction->name == "sltiu" || instruction->name == "xori" ||
            instruction->name == "ori" || instruction->name == "andi" ||
            instruction->name == "slli" || instruction->name == "srli" ||
            instruction->name == "srai") {
          label =
              convert_IType_Arithmetic_Imm_Shamt(instructionLine, instruction);
        } else if (instruction->name == "lb" || instruction->name == "lh" ||
                   instruction->name == "lw" || instruction->name == "lbu" ||
                   instruction->name == "lhu" || instruction->name == "jalr") {
          label = convert_IType_Load_Jump(instructionLine, instruction);
        }
        break;
      case InstructionType::S:
        label = processSType(instructionLine, instruction);
        break;
      case InstructionType::B:
        label = processBType(instructionLine, instruction);
        break;
      case InstructionType::U:
        label = processUType(instructionLine, instruction);
        break;
      case InstructionType::J:
        label = processJType(instructionLine, instruction);
        break;
      default:
        std::cout << "defaulting";
        break;
      }

      // Instruction processed, increment PC
      PC += 4;
    }

    if (!label.empty() && label.length() == 32) {
      if (binaryFile.is_open()) {
        binaryFile << label << std::endl;
      }

      uint32_t binaryDecimalForm = std::bitset<32>(label).to_ulong();
      std::string hexForm = binaryToHex(binaryDecimalForm);

      if (hexFile.is_open()) {
        hexFile << "0x" + hexForm << std::endl;
      }
    }

    // std::cout << std::endl;
  }

  // Close the file (optional as destructor closes it, but good practice)

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
  std::string binaryResult = binary_funct7 + binary_rs2 + binary_rs1 +
                             binary_funct3 + binary_rd + binary_opcode;

  return binaryResult;
}

std::string convert_IType_Arithmetic_Imm_Shamt(std::string instructionInput,
                                               const Instruction *instruction) {
  std::istringstream sstream(instructionInput);
  std::string instructionToken;
  std::vector<std::string> instructionTokens;

  while (sstream >> instructionToken) {
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

  int immediateValue = parseImmediate(immshamt);
  immshamt = std::bitset<12>(immediateValue & 0xFFF).to_string();

  // std::cout << (immshamt+rs1+std::bitset<3>((instruction->funct3) &
  // 0xFFF).to_string()+rd+std::bitset<7>((instruction->opcode) &
  // 0xFFF).to_string());

  return (immshamt + rs1 +
          std::bitset<3>((instruction->funct3) & 0xFFF).to_string() + rd +
          std::bitset<7>((instruction->opcode) & 0xFFF).to_string());
}

std::string convert_IType_Load_Jump(std::string instructionInput,
                                    const Instruction *instruction) {
  std::istringstream sstream(instructionInput);
  std::string instructionToken;
  std::vector<std::string> instructionTokens;

  while (sstream >> instructionToken) {
    instructionTokens.push_back(instructionToken);
  }

  std::string rd = instructionTokens[1];
  int commaIndex = rd.find(",");
  rd.erase(commaIndex, 1);

  std::string offsetrs1 = instructionTokens[2];

  std::string offsetStr = (offsetrs1.substr(0, offsetrs1.find('(')));
  int offset = parseImmediate(offsetStr);

  std::string imm = std::bitset<12>(offset & 0xFFF).to_string();

  int leftparenthesis = offsetrs1.find('(');
  int rightparenthesis = offsetrs1.find(')');
  std::string rs1 = offsetrs1.substr(leftparenthesis + 1,
                                     rightparenthesis - leftparenthesis - 1);

  rd = std::bitset<5>(getRegister(rd)->address).to_string();
  rs1 = std::bitset<5>(getRegister(rs1)->address).to_string();

  // std::cout <<
  // (imm+rs1+std::bitset<3>(instruction->funct3).to_string()+rd+std::bitset<7>(instruction->opcode).to_string());

  return (imm + rs1 + std::bitset<3>(instruction->funct3).to_string() + rd +
          std::bitset<7>(instruction->opcode).to_string());
}

std::string processBType(std::string line, const Instruction *instruction) {

  // B = imm12 + imm10_5 + rs2 + rs1 + funct3 + imm4_1 + imm11 + opcode
  std::stringstream ss(line);
  std::string token;
  std::string rs1;
  std::string rs2;
  std::string label;

  // Parse line: beq rs1, rs2, label
  ss >> token; // gets instruction name (beq, bne, etc.)
  ss >> rs1;
  if (!rs1.empty() && rs1.back() == ',')
    rs1.pop_back();

  ss >> rs2;
  if (!rs2.empty() && rs2.back() == ',')
    rs2.pop_back();

  ss >> label;

  // Collect rs1 and rs2 register values
  const Register *reg1 = getRegister(rs1);
  uint32_t rs1_b = 0;
  if (reg1 != nullptr) {
    rs1_b = reg1->address;
  } else {
    // std::cout << "Register not found: " << rs1 << "\n";
  }
  const Register *reg2 = getRegister(rs2);
  uint32_t rs2_b = 0;
  if (reg2 != nullptr) {
    rs2_b = reg2->address;
  } else {
    // std::cout << "Register not found: " << rs2 << "\n";
  }

  // Get funct3 from instruction struct
  uint32_t funct3_b = instruction->funct3;

  // Get Label address from global map
  auto it = SymbolTable.find(label);
  uint32_t label_b = 0;
  if (it != SymbolTable.end()) {
    label_b = it->second;
  } else {
    // std::cout << "Label not found: " << label << "\n";
  }

  // Compute offset (label address - PC), then >> 1
  int32_t offset = label_b - PC;
  int32_t imm = offset;

  // Extract immediate fields
  uint32_t imm12 = (imm >> 12) & 0x1;
  uint32_t imm11 = (imm >> 11) & 0x1;
  uint32_t imm10_5 = (imm >> 5) & 0x3F;
  uint32_t imm4_1 = (imm >> 1) & 0xF;

  // Build instruction: imm12[31] + imm10_5[30:25] + rs2[24:20] + rs1[19:15] +
  // funct3[14:12] + imm4_1[11:8] + imm11[7] + opcode[6:0]
  uint32_t inst = 0;
  inst |= (imm12 << 31);
  inst |= (imm10_5 << 25);
  inst |= (rs2_b << 20);
  inst |= (rs1_b << 15);
  inst |= (funct3_b << 12);
  inst |= (imm4_1 << 8);
  inst |= (imm11 << 7);
  inst |= instruction->opcode; // B-type opcode is 0x63 (0b1100011)

  // Output results
  return std::bitset<32>(inst).to_string();
  //   std::string hexInst = binaryToHex(inst);
}

std::string processSType(std::string line, const Instruction *instruction) {
  // S =imm + rs2 + rs1 + funct3 + imm + opcode
  std::stringstream ss(line);
  std::string token;
  std::string rs1;
  std::string rs2;
  std::string offset;
  std::string funct3;
  std::string opcode;

  // parse lin
  ss >> token; // gets instruction name
  ss >> rs2;   // gets rd
  rs2.pop_back();
  ss >> offset; // gets offset and next rs1
  // funct3 = instruction->funct3;
  // opcode = instruction->opcode;

  int start = offset.find('(');
  int end = offset.find(')');

  rs1 = offset.substr(start + 1, end - start - 1);
  offset = offset.substr(0, start);

  int rs2_int = getRegister(rs2)->address;
  int rs1_int = getRegister(rs1)->address;

  std::string imm_40 = std::bitset<12>(offset).to_string().substr(7);
  std::string imm_115 = std::bitset<12>(offset).to_string().substr(0, 7);

  // std::cout << "\t\t|";
  // std::cout << token + "(instruction)" + rs2 + "(rs2)" + offset + "(offset)"
  // + rs1 + "(rs1)\n";

  // std::cout << imm_115 << std::endl;
  // std::cout << imm_40 << std::endl;

  // S =imm(11:5) + rs2 + rs1 + funct3 + imm(4:0) + opcode
  // std::cout <<"\t|"+ imm_115 + "\t|" + std::bitset<5>(rs2_int).to_string() +
  // "\t|"+std::bitset<5>(rs1_int).to_string()+ "\t|" +
  // std::bitset<3>(instruction->funct3).to_string() + "\t|"+imm_40 + "\t|" +
  // std::bitset<7>(instruction->opcode).to_string();

  std::string binary = imm_115 + std::bitset<5>(rs2_int).to_string() +
                       std::bitset<5>(rs1_int).to_string() +
                       std::bitset<3>(instruction->funct3).to_string() +
                       imm_40 + std::bitset<7>(instruction->opcode).to_string();

  return binary;

  // S =imm + rs2 + rs1 + funct3 + imm + opcode
}
std::string processUType(std::string line, const Instruction *instruction) {
  // U = imm[31:12] + rd + opcode
  std::stringstream ss(line);
  std::string token;
  std::string rd;
  std::string immStr;

  ss >> token; // instruction
  ss >> rd;    // rd
  if (rd.back() == ',')
    rd.pop_back();
  ss >> immStr; // immediate

  int rd_int = getRegister(rd)->address;
  uint32_t immVal = 0;

  // Check for %hi(symbol)
  if (immStr.find("%hi") == 0) {
    int startIndex = immStr.find('(') + 1;
    int endIndex = immStr.find(')');
    std::string sym = immStr.substr(startIndex, endIndex - startIndex);

    if (SymbolTable.find(sym) != SymbolTable.end()) {
      immVal = SymbolTable[sym];
      // For lui/auipc, the immediate is the upper 20 bits.
      // The instruction expects the 20 bits directly in the top 20 bits of the
      // 32-bit word? Actually, U-type format is: imm[31:12] | rd | opcode If we
      // write 'lui t0, 0x12345', the imm is 0x12345. %hi(addr) typically
      // extracts the upper 20 bits of addr. So if addr is 0x12345678, %hi gives
      // 0x12345. We need to shift it down correctly if it's not already. But
      // valid `imm` for U-type is 20 bits.

      // Standard behavior for %hi(sym): (sym + 0x800) >> 12.
      // But simpler logic: just (sym >> 12).
      immVal = (immVal >> 12) & 0xFFFFF;
    } else {
      // std::cerr << "Symbol not found for %hi: " << sym << std::endl;
    }
  } else {
    // Decimal or Hex
    immVal = std::stoi(immStr, nullptr, 0);
    immVal = immVal & 0xFFFFF; // Apply mask for safety
  }

  // Construct U-Type Binary
  // imm[31:12] (which is just the 20 bits we have) << 12
  // But wait, the standard U-type format puts the immediate at bits 31-12.
  // Our logic below constructs it part by part.

  std::string binary = std::bitset<20>(immVal).to_string() +
                       std::bitset<5>(rd_int).to_string() +
                       std::bitset<7>(instruction->opcode).to_string();
  return binary;
}
std::string processJType(std::string line, const Instruction *instruction) {
  // J = imm[20|10:1|11|19:12] + rd + opcode
  std::stringstream ss(line);
  std::string token;
  std::string
      rd; // for jal, rd is optional parsing-wise? Standard: jal rd, label OR
          // jal label (rd=ra) But usually explicit: jal x0, label
  std::string label;

  ss >> token;
  ss >> rd;

  // Check if rd is a register. If it doesn't end in comma and next token is
  // empty, maybe strict format "jal label"? But for this project, let's assume
  // "jal rd, label" format as typically seen. In add_shift.s: "jal x0, store"

  if (rd.back() == ',') {
    rd.pop_back();
    ss >> label;
  } else {
    // Handle case where specific assembler might use "jal label" (implies
    // rd=x1/ra) But the input file uses "jal x0, store". So rd is x0. We need
    // to be careful. If rd is just "store" (label), `getRegister` will return
    // null.
    if (getRegister(rd) == nullptr) {
      label = rd;
      rd = "ra"; // Default to ra (x1)
    } else {
      // It is a register, so label must be next
      ss >> label;
    }
  }

  int rd_int = getRegister(rd)->address;

  int32_t offset = 0;
  if (SymbolTable.find(label) != SymbolTable.end()) {
    int32_t targetAddr = SymbolTable[label];
    offset = targetAddr - PC;
  } else {
    // std::cerr << "Label not found for jal: " << label << std::endl;
  }

  // Offset is byte offset. J-type imm encodes offset >> 1
  offset >>= 1;

  // Extract bits
  uint32_t imm20 = (offset >> 19) & 0x1;
  uint32_t imm10_1 = (offset) & 0x3FF; // bits 1-10
  uint32_t imm11 = (offset >> 10) & 0x1;
  uint32_t imm19_12 = (offset >> 11) & 0xFF; // bits 12-19

  // Construct scrambled immediate
  // imm[20] | imm[10:1] | imm[11] | imm[19:12]
  std::string bin_imm20 = std::bitset<1>(imm20).to_string();
  std::string bin_imm10_1 = std::bitset<10>(imm10_1).to_string();
  std::string bin_imm11 = std::bitset<1>(imm11).to_string();
  std::string bin_imm19_12 = std::bitset<8>(imm19_12).to_string();

  std::string binary = bin_imm20 + bin_imm10_1 + bin_imm11 + bin_imm19_12 +
                       std::bitset<5>(rd_int).to_string() +
                       std::bitset<7>(instruction->opcode).to_string();

  return binary;
}

uint32_t parseImmediate(const std::string immediate) {
  if (immediate.find("%hi") == 0) {
    int startIndex = immediate.find('(') + 1;
    int endIndex = immediate.find(')');
    std::string imm = immediate.substr(startIndex, endIndex - startIndex);

    if (SymbolTable.find(imm) != SymbolTable.end()) {
      int address = SymbolTable[imm];
      return (address >> 12) & 0xFFFFF;

    } else {
      return -1;
    }
  }

  if (immediate.find("%lo") == 0) {
    int startIndex = immediate.find('(') + 1;
    int endIndex = immediate.find(')');
    std::string imm = immediate.substr(startIndex, endIndex - startIndex);

    if (SymbolTable.find(imm) != SymbolTable.end()) {
      int address = SymbolTable[imm];
      return address & 0xFFF;

    } else {
      return -1;
    }
  }

  return std::stoi(immediate); // return reg immediate lol
}

bool firstPass(std::string filename) {
  std::ifstream inputFile(filename);
  if (!inputFile.is_open()) {
    std::cerr << "Error: Could not open file " << filename << " for first pass."
              << std::endl;
    return false;
  }

  std::string line;
  std::string token;
  PC = getBaseAddress(); // Start PC at base address

  while (std::getline(inputFile, line)) {
    // Strip comments if any (simple check)
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos) {
      line = line.substr(0, commentPos);
    }

    std::stringstream ss(line);
    if (!(ss >> token))
      continue; // Empty line

    // Check for labels (ends with ':')
    if (token.back() == ':') {
      std::string labelName = token.substr(0, token.size() - 1);
      SymbolTable[labelName] = PC;

      // If there's more on the line (e.g., "label: instruction"), continue
      // processing
      if (!(ss >> token))
        continue; // Nothing else on this line
    }

    // Directives
    if (token == ".data" || token == ".text" || token == ".globl") {
      continue;
    }

    if (token == ".word") {
      PC += 4;
      continue;
    }

    // Instructions
    if (getInstructions(token) != nullptr) {
      PC += 4;
    }
  }
  inputFile.close();
  return true;
}
