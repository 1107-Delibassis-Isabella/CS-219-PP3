#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <vector>
using namespace std;



class Simulator {
    private:
        uint32_t memoryArray[5]; //represents the 5 memory locations (0x100 to 0x110)
        bool memoryInitialized[5]; //will check if a memory location is either ___ or hex value
        uint32_t registerArray[12]; //represents the registers R0 to R11
        bool n, z, c, v;
        vector <string> labelNames;
        vector<size_t> labelTargets;



    public:
        Simulator();
        uint32_t convertStringToInt(string op);
        uint32_t findValueForRegister(int index);
        
        void assignValueForRegister(int index, uint32_t value);
        bool findCondition(string condition);
        void executeOperation(string operation, string result, string operand1, string operand2, size_t& labelIndex, bool& branched);
        void MOVorMVN(string operation, string result, string operand);
        void executeLDRorSTR(string operation, string resultRegister, string operand1);
        uint32_t executeNormalOperations(string operation, uint32_t op1, uint32_t op2);
        void loadProgram(const string& filename);

        
        void updateNZCV(uint32_t result, string operation, uint32_t op1, uint32_t op2);
        void printInstruction(string line);
};
#endif