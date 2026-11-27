#include "Simulator.h"

Simulator::Simulator() : n(false), z(false), c(false), v(false) {
    //Initalizes all registers with 0
    for (int i = 0; i < 12; i++) {
        registerArray[i] = 0;
    }

    //Initializes all of the memoryArray with 0
    for (int i = 0; i < 5; i++) {
        memoryArray[i] = 0;
        memoryInitialized[i] = false;
    }
}

uint32_t Simulator::convertStringToInt(string op) {
    int registerNum;
    uint32_t newOperand = 0;
    //If the string is empty, then it will return the newOperand to be 0
    if (op.empty()) {
        return newOperand; 
    }

    //Checks if the string begins with R, such as R0 or R11
    if (op[0] == 'R') {
        //Converts the number of the op to an int data type disregarding the 'R'
        registerNum = static_cast<int>(stoul(op.substr(1)));
        newOperand = findValueForRegister(registerNum);
        return newOperand;
    } else if (op[0] == '#') {
        //Converts the immediate value of the op to uint32_t data type disregarding the #
        newOperand = static_cast<uint32_t>(stoul(op.substr(1), nullptr, 0));
        return newOperand;
    }
    return newOperand;
}

uint32_t Simulator::findValueForRegister(int index) {
    //Check throughout the entire registerArray
    for (int i = 0; i < 12; i++) {
        //If the index is equal to i, then returns the value within that specific position of registerArray
        if (index == i) {
            return registerArray[i];
        }
    }
    //If not found, then returns an error message that index is out of range
    throw out_of_range("Register is out of range (R0-R11)");
}

void Simulator::assignValueForRegister(int index, uint32_t value) {
    //First checks if the index from the parameter is out of range
    if (index >= 12 || index < 0) {
        cout << "Too small/large of a register: " << index << endl;
        return;
    }
    //If within range, then we will assign the variable value to the registerArray with position index
    registerArray[index] = value;
}

void Simulator::executeOperation(string operation, string result, string operand1, string operand2, size_t& labelIndex, bool& branched) {
    uint32_t newOperand1, newOperand2, newResult, compareLeft, compareRight;
    int num;
    string condition = "";

    //Checks if the operation begins with 'B', which could be "BAL", "BNE", "BLE", "BGT", "BGE", etc
    if (operation[0] == 'B') {
        //checks if the operation size is 3, then will set the condition variable with the next two characters from the operation variable
        if (operation.size() == 3) {
            condition = operation.substr(1, 2);
        }

        //Checks if the condition is returned true
        if (findCondition(condition)) {
            //Goes through the labelNames array to find if any of the array is equal to the result string, and if so then return and make branched true 
            for (size_t i = 0; i < labelNames.size(); i++) {
                if (labelNames[i] == result) {
                    labelIndex = i;
                    branched = true;
                    return;
                }
            }
        }
    }
    //Converts the operand1 and operand2 variables to be uint32_t values
    newOperand1 = convertStringToInt(operand1);
    newOperand2 = convertStringToInt(operand2);

    //Checks if the operation contains conditions as well
    if (operation.size() >= 5) {
        if (findCondition(operation.substr(operation.size()-2))) {
            //Checks if the operation contains MOV or MVN
            if (operation.substr(0, operation.size()-2) == "MOV" || operation.substr(0, operation.size()-2) == "MVN") {
            MOVorMVN(operation.substr(0, operation.size()-2), result, operand1);
            return;
            } //Checks if the operation contains LDR or STR
            else if (operation.substr(0, operation.size()-2) == "LDR" || operation.substr(0, operation.size()-2) == "STR") {
                executeLDRorSTR(operation.substr(0, operation.size()-2), result, operand1);
                return;
            } //If the past two conditions are not met, then it will execute one of the usual operations like: ADD, SUB, AND, ORR, EOR, LSR, LSL
            newResult = executeNormalOperations(operation.substr(0, operation.size() - 2), newOperand1, newOperand2);
            //Finds the resulting register number
            num = static_cast<int>(stoul(result.substr(1)));
            //Assigns the newResult from the two operands to the registerArray with position num
            assignValueForRegister(num, newResult);
        }
    } else if (operation.size() == 3) {
        //Checks if the operation is MOV or MVN
        if (operation == "MOV" || operation == "MVN") {
            MOVorMVN(operation, result, operand1);
            return;
        } //Checks if the operation is CMP
        else if (operation == "CMP") {
            //Gets the values from the two operand strings and convert them to uint32_t
            compareLeft = convertStringToInt(result);
            compareRight = convertStringToInt(operand1);
            //Find the result by subtracting the second operand from the first opreand
            newResult = compareLeft - compareRight;
            //Updates the NZCV because it is CMP operation
            updateNZCV(newResult, operation, compareLeft, compareRight);
            return;
        } //Checks if the operation is LDR or STR
        else if (operation == "LDR" || operation == "STR") {
            executeLDRorSTR(operation, result, operand1);
            return;
        } //If the past two conditions are not met, then it will execute one of the usual operations like: ADD, SUB, AND, ORR, EOR, LSR, LSL
        newResult = executeNormalOperations(operation, newOperand1, newOperand2);
        //Finds the resulting register number
        num = static_cast<int>(stoul(result.substr(1)));
        //Assigns the newResult from the two operands to the registerArray with position num
        assignValueForRegister(num, newResult);
    } //Checks if the operation contains a last character of 'S' which will update NZCV
    else if (operation.size() == 4 && operation[operation.size()-1] == 'S') {
        newResult = executeNormalOperations(operation.substr(0, operation.size()-1), newOperand1, newOperand2);
        num = static_cast<int>(stoul(result.substr(1)));
        assignValueForRegister(num, newResult);
        updateNZCV(newResult, operation, newOperand1, newOperand2);
    }

}
void Simulator::MOVorMVN(string operation, string result, string operand) {
    //Finds the register number from result variable
    int registerNum = static_cast<int>(stoul(result.substr(1)));
    //Checks if the operand begins with 'R'
    if (operand[0] == 'R') {
        int operandNum = static_cast<int>(stoul(operand.substr(1)));
        uint32_t newOperand = findValueForRegister(operandNum);
        //Checks if operation is MOV
        if (operation == "MOV") {
            assignValueForRegister(registerNum, newOperand);
        } //Checks if operation is MVN
        else if (operation == "MVN") {
            assignValueForRegister(registerNum, ~newOperand);
        }
    } //Checks if the operand begins with #, an immediate value
    else if (operand[0] == '#') {
        //Finds the operand value from string to uint32_t
        uint32_t newOperand = static_cast<uint32_t>(stoul(operand.substr(1), nullptr, 0));
        if (operation == "MOV") {
            assignValueForRegister(registerNum, newOperand);
        } else if (operation == "MVN") {
            assignValueForRegister(registerNum, ~newOperand);
        }
    }

    
}

void Simulator::executeLDRorSTR(string operation, string resultRegister, string operand1) {
    //Finds the result register position
    int registerIndex = static_cast<int>(stoul(resultRegister.substr(1)));
    //Initializes address to the uint32_t value of operand1
    uint32_t address = convertStringToInt(operand1);
    //Checks if registerIndex is out of range
    if (registerIndex < 0 || registerIndex >= 12) {
        throw out_of_range("Register is out of range (R0-R11)");
    }
    //Checks if address is out of range
    if (address < 0x100 || address > 0x110 || address % 4 != 0) {
        throw out_of_range("Memory address out of range or unaligned");
    }
    //Initializes memoryIndex by subtracting 0x100 from address and dividing it by 4
    size_t memoryIndex = (address - 0x100) / 4;

    //Checks if operation is LDR
    if (operation == "LDR") {
        registerArray[registerIndex] = memoryArray[memoryIndex];
    } //Checks if operation is STR
    else if (operation == "STR") {
        memoryArray[memoryIndex] = registerArray[registerIndex];
        memoryInitialized[memoryIndex] = true;
    }
}

void Simulator::updateNZCV(uint32_t result, string operation, uint32_t op1, uint32_t op2) {
    //adds, subs, orrs
    
    //N and Z will be set the same way regardless of what the operation is
    n = (result >> 31) & 1;
    if (result == 0) {
        z = true;
    } else {
        z = false;
    }

    //Assign C and V from NZCV 
    if (operation == "ADDS") {
        c = (result < op1);
        v = (~(op1 ^ op2) & (op1 ^ result)) >> 31 != 0;
    } else if(operation == "SUBS") {
        c = (op1 >= op2);
        v = ((op1^op2) & (op1 ^ result)) >> 31;
    } else if (operation == "ANDS") {
        
        //V is not affected
    } else if (operation == "CMP") {
        c = (op1 >= op2);
        v = (((op1 ^ op2) & (op1 ^ result)) >> 31) & 1;
    }
}

uint32_t Simulator::executeNormalOperations(string operation, uint32_t op1, uint32_t op2) {
    //Checks if operation is ADD
    if (operation == "ADD") {
        return op1 + op2;
    } else if (operation == "SUB") { //Checks if operation is SUB
        return op1 - op2;
    } else if (operation == "AND") { //Checks if operation is AND
        return op1 & op2;
    } else if (operation == "ORR") { //Checks if operation is ORR
        return op1 | op2;
    } else if (operation == "EOR") { //Checks if operation is EOR
        return op1 ^ op2;
    } else if (operation == "LSR") { //Checks if operation is SUB
        //Converts the second operand to be an immediate value, int
        int immediateValue = static_cast<int>(op2); 
        return op1 >> immediateValue;
    } else if (operation == "LSL") {
        //Converts the second operand to be an immediate value, int
        int immediateValue = static_cast<int>(op2);
        return op1 << immediateValue;
    } else {
        //If none of the conditions are met, then will throw an error as an unsupported operator
        throw runtime_error("Unsupported operator"); 
    }
    
}

bool Simulator::findCondition(string condition) {
    //Checks if the condition is Greater Than
    if (condition == "GT") {
        return (z == 0 && n == v);
    } else if (condition == "LT") { //Checks if the condition is Less Than
        return (n != v);
    } else if (condition == "GE") { //Checks if the conditions is Greater Than or Equal
        return (n == v);
    } else if (condition == "LE") { //Checks if the conditions is Less Than or Equal
        return (z == 1 || n != v);
    } else if (condition == "EQ") { //Checks if the condition is Equal to
        return (z == 1);
    } else if (condition == "NE") { //Checks if the condition is Not Equal to
        return (z == 0);
    } else if (condition == "AL") { //Checks if the condition is BAL
        return true; //Will always return true if BAL
    }
    return false; //Returns false if all of the conditions aren't met
}

void Simulator::loadProgram(const string& filename) {
    ifstream file(filename);
    //Executes error
    if (!file.is_open()) {
        cout << "Error: Unable to open input file " << filename << endl;
        return;
    }
    string line, labelName;
    string operatorName, result, operand1, operand2;

    //Reads each line one by one
    while (getline(file, line)) {
        string normalized = line;
        for (char& ch : normalized) { //Checks if any of the char is commas or brackets and replaces it with white space
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
        ss >> result >> operand1 >> operand2; //Assigns the values for result, operand1, and operand2
        executeOperation(operatorName, result, operand1, operand2, labelIndex, branch);
        printInstruction(line);
        }
}

void Simulator::printInstruction(string line) {
    cout << line << endl;
    cout << "Register array:" << endl;
    for (int i = 0; i < 12; i++) { //Prints out all 12 of the registerArray values
        //Prints out the decimal value of i and hexadecimal value of each of the values from the registerArray
        cout << dec << "R" << i << " =0x" << hex << registerArray[i] << " ";
    }
    cout << endl;
    cout << "NZCV: " << n << z << c << v << endl; //prints out the NZCV values
    cout << "Memory array:" << endl;
    for (int i = 0; i < 5; i++) { //Prints out all 5 of the memoryArray values
        if (!memoryArray[i]) {
            cout << "___"; //Prints out blank if memoryArray[i] has no value
        } else {
            cout << "0x" << hex << memoryArray[i]; //Prints out hexadecimal value of memoryArray[i] if not blank
        }
        if (i < 4) { //Checks if the position i is not the last item from the memoryArray list, and if not then will print out a comma
            cout << ",";
        }
    }
    cout << endl;

}