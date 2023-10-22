#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <SDL.h>

using namespace std;

u_int8_t RAM[65536];
u_int16_t Counter=0;
u_int16_t regA=0, regB=0, regC=0, regD=0, regE=0;
u_int16_t cache=0;
bool cmpS=0;
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 64;
const int PIXEL_SIZE = 10;

int LoadRom(string filePath);

int main(void) {
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Emulator Graphics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * PIXEL_SIZE, SCREEN_HEIGHT * PIXEL_SIZE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    screenSurface = SDL_GetWindowSurface(window);
    LoadRom("../memory.rom");
    bool clkok=1;
    SDL_Event e;
    cout << "initOK\n";
    while(clkok) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                clkok = 0;
            }
        }
        SDL_Rect rect;
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                rect.x = x * PIXEL_SIZE;
                rect.y = y * PIXEL_SIZE;
                rect.w = PIXEL_SIZE;
                rect.h = PIXEL_SIZE;
                if (RAM[(y * 64 + x) + (65536 - 4096)] == 1) {
                    SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
                } else {
                    SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
                }
            }
        }
        SDL_UpdateWindowSurface(window);
        cout << hex << setw(4) << setfill('0') << Counter << ": ";
        if(RAM[Counter] == 0xFF) {clkok=0; cout << "HLT\n"; continue;}
        if(RAM[Counter] == 0x00) {Counter++; cout << "NOP\n"; continue;}
        if(RAM[Counter] == 0x01) {regC = regA+regB; Counter++; cout << "ADD\n"; continue;}
        if(RAM[Counter] == 0x02) {regC = regA-regB; Counter++; cout << "SUB\n"; continue;}
        if(RAM[Counter] == 0x03) {regC = regA*regB; Counter++; cout << "MUL\n"; continue;}
        if(RAM[Counter] == 0x04) {regC = regA/regB; regD = regA%regB; Counter++; cout << "DIV\n"; continue;}
        if(RAM[Counter] == 0x05) {regA = regE; Counter++; cout << "ETA " << dec << regE << "\n"; continue;}
        if(RAM[Counter] == 0x06) {regB = regE; Counter++; cout << "ETB " << dec << regE << "\n"; continue;}
        if(RAM[Counter] == 0x07) {regD = regE; Counter++; cout << "ETC " << dec << regE << "\n"; continue;}
        if(RAM[Counter] == 0x08) {regD = regE; Counter++; cout << "ETD " << dec << regE << "\n"; continue;}
        if(RAM[Counter] == 0x09) {regE = regA; Counter++; cout << "ATE " << dec << regA << "\n"; continue;}
        if(RAM[Counter] == 0x0A) {regE = regB; Counter++; cout << "BTE " << dec << regB << "\n"; continue;}
        if(RAM[Counter] == 0x0B) {regE = regC; Counter++; cout << "CTE " << dec << regC << "\n"; continue;}
        if(RAM[Counter] == 0x0C) {regE = regD; Counter++; cout << "DTE " << dec << regD << "\n"; continue;}
        if(RAM[Counter] == 0x0D) {
            //LDE
            cout << "LDE 0x";
            cache = (RAM[Counter+1] << 8) | RAM[Counter+2];
            Counter+=3;
            cout << hex << setw(4) << setfill('0') << cache << " ";
            regE=RAM[cache];
            cout << dec << regE << "\n";
            continue;
        }
        if(RAM[Counter] == 0x0E) {
            //SVE
            cout << "SVE 0x";
            cache = (RAM[Counter+1] << 8) | RAM[Counter+2];
            Counter+=3;
            cout << hex << setw(4) << setfill('0') << cache << " ";
            RAM[cache]=regE;
            cout << dec << regE << "\n";
            continue;
        }
        if(RAM[Counter] == 0x0F) {cout << "PTE: " << dec << regE << "\n"; Counter++; continue;}
        if(RAM[Counter] == 0x10) {
            //JMP
            cout << "JMP 0x";
            cache = (RAM[Counter+1] << 8) | RAM[Counter+2];
            cout << hex << setw(4) << setfill('0') << cache << " ";
            Counter = cache;
            continue;
        }
        if(RAM[Counter] == 0x11) {cmpS = (regA==regB); Counter++; cout << "CMP\n"; continue;}
        if(RAM[Counter] == 0x12) {
            //JNE
            cout << "JNE 0x";
            cache = (RAM[Counter+1] << 8) | RAM[Counter+2];
            cout << hex << setw(4) << setfill('0') << cache << "\n";
            if(!cmpS) Counter = cache;
            else Counter+=3;
            continue;
        }
        if(RAM[Counter] == 0x13) {
            //JPE
            cout << "JPE 0x";
            cache = (RAM[Counter+1] << 8) | RAM[Counter+2];
            cout << hex << setw(4) << setfill('0') << cache << "\n";
            if(cmpS) Counter = cache;
            else Counter+=3;
            continue;
        }
        if(RAM[Counter] == 0x14) {
            string input;
            cout << "INE(4hex): ";
            cin >> input;
            if(!input.size() == 4) {cout<<"ERROR not 4 digits\n";abort();}
            if(!isxdigit(input[0]) || !isxdigit(input[1]) || !isxdigit(input[2]) || !isxdigit(input[3])) {cout<<"ERROR not hex digit.\n";abort();}
            regE = stoi(input, nullptr, 16);
            Counter++;
            continue;
        }
        if(RAM[Counter] == 0x15) {
            if(regA < 64 && regB < 64) {
                RAM[(regB * 64 + regA) + (65536 - 4096)] = regC;
                Counter++;
                continue;
            }
            cout << "Error, position out of range.\n";
            abort();
        }
        if(RAM[Counter] == 0x16) {
            for(int i = 65536 - 4096; i < 65536; i++) {
                RAM[i] = 0;
            }
            Counter++;
            continue;
        }
        else {cout << "ERROR\n"; return 1;}
    }
    cout << "HLTOK\n";
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int LoadRom(string filePath) {
    FILE *arq;
    arq = fopen(filePath.c_str(), "rb");
    int a=fread(RAM, sizeof(u_int8_t), 65536, arq);
    fclose(arq);
    return 0;
}