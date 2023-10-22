#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <set>


using namespace std;

u_int8_t ROM[65536 - 4096];
u_int8_t getOpcode(const string& FW);

int main() {
    cout << "starting...\n";
    ifstream arq("../code.asm");
    if (!arq) {
        cout << "can't open: ../code.asm\n";
        abort();
    }
    unordered_map<string, u_int8_t> opcodeMap = {
        {"HLT", 0xff},
        {"NOP", 0x00},
        {"ADD", 0x01},
        {"SUB", 0x02},
        {"MUL", 0x03},
        {"DIV", 0x04},
        {"ETA", 0x05},
        {"ETB", 0x06},
        {"ETC", 0x07},
        {"ETD", 0x08},
        {"ATE", 0x09},
        {"BTE", 0x0A},
        {"CTE", 0x0B},
        {"DTE", 0x0C},
        {"PTE", 0x0F},
        {"CMP", 0x11},
        {"INE", 0x14},
        {"PIX", 0x15},
        {"CLR", 0x16}
    };
    cout << "compiling...\ntext block\n";
    string line;
    bool text=1;
    for(int i = 0, j = 1; getline(arq, line); ++i, ++j) {
        if(i > (65536 - 4096)) {
            cout << "your code passes (65536 - 4096) Bytes\n";
            abort();
        } 
        if(line == ".DATA") {text=0;i--;cout << "data block\n";continue;}
        if(!text) {
            if(line.size() < 2) {cout << "ERROR line " << j << ": wrong size\n"; abort();}
            ROM[i]=stoi(line.substr(0, 2), nullptr, 16);
            continue;
        }
        string FW = line.substr(0, 3);
        auto it = opcodeMap.find(FW);
        if (it != opcodeMap.end()) {
            ROM[i] = it->second;
            continue;
        }
        set<string> validOps = {"LDE", "SVE", "JMP", "JNE", "JPE"};
        if (validOps.find(FW) != validOps.end()) {
            if (line.size() < 7) {
                cout << "ERROR line " << j << ": wrong size\n";
                abort();
            }
            string LW = line.substr(4);
            try {
                ROM[i] = getOpcode(FW);
                ROM[i+1] = stoi(LW.substr(0, 2), nullptr, 16);
                ROM[i+2] = stoi(LW.substr(2, 2), nullptr, 16);
                i += 2;
            } catch (const exception& e) {
                cout << "ERROR line " << j << ": LW wrong formatted\n";
                abort();
            }
            continue;
        }
        cout << "ERROR line " << j << ": command not found\n";
        abort();
    }
    arq.close();
    cout << "writing...\n";
    ofstream outFile("../memory.rom", ios::out | ios::binary);
    if (!outFile) {
        cout << "can't open: ../memory.rom for writing\n";
        abort();
    }
    outFile.write(reinterpret_cast<char*>(ROM), sizeof(ROM));
    outFile.close();
    cout << "Well done.\n";
    return 0;
}

u_int8_t getOpcode(const string& FW) {
    if (FW == "LDE") return 0x0D;
    if (FW == "SVE") return 0x0E;
    if (FW == "JMP") return 0x10;
    if (FW == "JNE") return 0x12;
    if (FW == "JPE") return 0x13;
    return 0x00;
}