#include "Simulator.h"

Simulator::Simulator() : n(false), z(false), c(false), v(false) {
    for (int i = 0; i < 12; i++) {
        registerArray[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        memoryArray[i] = 0;
        memoryInitialized[i] = false;
    }
}

uint32_t Simulator::convertStringToInt(string op) {
    int registerNum;
    uint32_t newOperand = 0;
    if (op.empty()) {
        return newOperand; 
    }
    if (op[0] == 'R') {
        registerNum = static_cast<int>(stoul(op.substr(1)));
        newOperand = findValueForRegister(registerNum);
        return newOperand;
    } else if (op[0] == '#') {
        newOperand = static_cast<uint32_t>(stoul(op.substr(1), nullptr, 0));
        return newOperand;
    }
    return newOperand;
}

uint32_t Simulator::findValueForRegister(int index) {
    for (int i = 0; i < 12; i++) {
        if (index == i) {
            return registerArray[i];
        }
    }
    throw out_of_range("Register is out of range (R0-R11)");
}

void Simulator::assignValueForRegister(int index, uint32_t value) {
    if (index >= 12 || index < 0) {
        cout << "Too small/large of a register: " << index << endl;
        return;
    }
    registerArray[index] = value;
}

void Simulator::executeOperation(string operation, string result, string operand1, string operand2, size_t& labelIndex, bool& branched) {
    uint32_t newOperand1, newOperand2, newResult, compareLeft, compareRight;
    int num;
    string condition = "";

    if (operation[0] == 'B') {
        if (operation.size() == 3) {
            condition = operation.substr(1, 2);
        }
        if (findCondition(condition)) {
            for (size_t i = 0; i < labelNames.size(); i++) {
                if (labelNames[i] == result) {
                    labelIndex = i;
                    branched = true;
                    return;
                }
            }

        }
    }
    newOperand1 = convertStringToInt(operand1);
    newOperand2 = convertStringToInt(operand2);

    //newOperand2 = static_cast<uint32_t>(stoul(operand2));
    if (operation.size() >= 5) {
        if (findCondition(operation.substr(operation.size()-2))) {
            if (operation.substr(0, operation.size()-2) == "MOV" || operation.substr(0, operation.size()-2) == "MVN") {
            MOVorMVN(operation.substr(0, operation.size()-2), result, operand1);
            return;
            } else if (operation.substr(0, operation.size()-2) == "LDR" || operation.substr(0, operation.size()-2) == "STR") {
                executeLDRorSTR(operation.substr(0, operation.size()-2), result, operand1);
                return;
            }
            newResult = executeNormalOperations(operation.substr(0, operation.size() - 2), newOperand1, newOperand2);
            num = static_cast<int>(stoul(result.substr(1)));
            assignValueForRegister(num, newResult);
        }
    } else if (operation.size() == 3) {
        if (operation == "MOV" || operation == "MVN") {
            MOVorMVN(operation, result, operand1);
            return;
        } else if (operation == "CMP") {
            compareLeft = convertStringToInt(result);
            compareRight = convertStringToInt(operand1);
            //newOperand2 is empty in CMP because there is only one result R, and one operand
            newResult = compareLeft - compareRight;
            updateNZCV(newResult, operation, compareLeft, compareRight);
            return;
        } else if (operation == "LDR" || operation == "STR") {
            executeLDRorSTR(operation, result, operand1);
            return;
        }
        newResult = executeNormalOperations(operation, newOperand1, newOperand2);
        num = static_cast<int>(stoul(result.substr(1)));
        assignValueForRegister(num, newResult);
    } else if (operation.size() == 4 && operation[operation.size()-1] == 'S') {
        newResult = executeNormalOperations(operation.substr(0, operation.size()-1), newOperand1, newOperand2);
        num = static_cast<int>(stoul(result.substr(1)));
        assignValueForRegister(num, newResult);
        updateNZCV(newResult, operation, newOperand1, newOperand2);
    }

}
void Simulator::MOVorMVN(string operation, string result, string operand) {
    int registerNum = static_cast<int>(stoul(result.substr(1)));
    if (operand[0] == 'R') {
        int operandNum = static_cast<int>(stoul(operand.substr(1)));
        uint32_t newOperand = findValueForRegister(operandNum);
        //uint32_t newOperand = static_cast<uint32_t>(stoul(operand.substr(1)));
        if (operation == "MOV") {
            assignValueForRegister(registerNum, newOperand);
        } else if (operation == "MVN") {
            assignValueForRegister(registerNum, ~newOperand);
        }
        //assignValueForRegister(registerNum, newOperand);

    } else if (operand[0] == '#') {
        uint32_t newOperand = static_cast<uint32_t>(stoul(operand.substr(1), nullptr, 0));
        if (operation == "MOV") {
            assignValueForRegister(registerNum, newOperand);
        } else if (operation == "MVN") {
            assignValueForRegister(registerNum, ~newOperand);
        }
    }

    
}

void Simulator::executeLDRorSTR(string operation, string resultRegister, string operand1) {
    int registerIndex = static_cast<int>(stoul(resultRegister.substr(1)));
    uint32_t address = convertStringToInt(operand1);
    if (registerIndex < 0 || registerIndex >= 12) {
        throw out_of_range("Register is out of range (R0-R11)");
    }
    if (address < 0x100 || address > 0x110 || address % 4 != 0) {
        throw out_of_range("Memory address out of range or unaligned");
    }
    size_t memoryIndex = (address - 0x100) / 4;

    if (operation == "LDR") {
        registerArray[registerIndex] = memoryArray[memoryIndex];
    } else if (operation == "STR") {
        memoryArray[memoryIndex] = registerArray[registerIndex];
        memoryInitialized[memoryIndex] = true;
    }
}

void Simulator::updateNZCV(uint32_t result, string operation, uint32_t op1, uint32_t op2) {
    //adds, subs, orrs
    
    n = (result >> 31) & 1;
    if (result == 0) {
        z = true;
    } else {
        z = false;
    }

    if (operation == "ADDS") {
        c = (result < op1);
        v = (~(op1 ^ op2) & (op1 ^ result)) >> 31 != 0;
    } else if(operation == "SUBS") {
        c = (op1 >= op2);
        v = ((op1^op2) & (op1 ^ result)) >> 31;
    } else if (operation == "ORRS") {
        c = false;
        v = false;
    } else if (operation == "CMP") {
        c = (op1 >= op2);
        v = (((op1 ^ op2) & (op1 ^ result)) >> 31) & 1;
    }
    //cout << "NZCV: " << n << z << c << v << endl;

}

uint32_t Simulator::executeNormalOperations(string operation, uint32_t op1, uint32_t op2) {
    if (operation == "ADD") {
        return op1 + op2;
    } else if (operation == "SUB") {
        return op1 - op2;
    } else if (operation == "AND") {
        return op1 & op2;
    } else if (operation == "ORR") {
        return op1 | op2;
    } else if (operation == "EOR") {
        return op1 ^ op2;
    } else if (operation == "LSR") {
        int immediateValue = static_cast<int>(op2);
        return op1 >> immediateValue;
    } else if (operation == "LSL") {
        int immediateValue = static_cast<int>(op2);
        return op1 << immediateValue;
    } else {
        throw runtime_error("Unsupported operator"); 
    }
    
}

bool Simulator::findCondition(string condition) {
    if (condition == "GT") {
        return (z == 0 && n == v);
    } else if (condition == "LT") {
        return (n != v);
    } else if (condition == "GE") {
        return (n == v);
    } else if (condition == "LE") {
        return (z == 1 || n != v);
    } else if (condition == "EQ") {
        return (z == 1);
    } else if (condition == "NE") {
        return (z == 0);
    } else if (condition == "AL") {
        return true;
    }
    return false;
}

void Simulator::loadProgram(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Unable to open input file " << filename << endl;
        return;
    }
    string line, labelName;
    string operatorName, result, operand1, operand2;

    while (getline(file, line)) {
        string normalized = line;
        for (char& ch : normalized) {
            if (ch == ',' || ch == '[' || ch == ']') ch = ' ';
        }

        stringstream ss(normalized);
        operatorName.clear();
        result.clear();
        operand1.clear();
        operand2.clear();
        size_t labelIndex = 0;
        bool branch = false;

        if (!(ss >> operatorName)) continue; // skip blank lines
        ss >> result >> operand1 >> operand2;
        executeOperation(operatorName, result, operand1, operand2, labelIndex, branch);
        printInstruction(line);
        }
}

void Simulator::printInstruction(string line) {
    cout << line << endl;
    cout << "Register array:" << endl;
    for (int i = 0; i < 12; i++) {

        cout << dec << "R" << i << " =0x" << hex << registerArray[i] << " ";
    }
    cout << endl;
    cout << "NZCV: " << n << z << c << v << endl;
    cout << "Memory array:" << endl;
    for (int i = 0; i < 5; i++) {
        if (!memoryArray[i]) {
            cout << "___";
        } else {
            cout << "0x" << hex << memoryArray[i];
        }
        if (i < 4) {
            cout << ",";
        }
    }
    cout << endl;

}