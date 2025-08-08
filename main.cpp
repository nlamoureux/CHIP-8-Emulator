#include <iostream>
#include <fstream>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <SDL.h>
#include "Chip8.h"

const int videoWidth = 64;
const int videoHeight = 32;
const int videoScale = 10;

uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //1
    0x20, 0x60, 0x20, 0x20, 0x70, //2
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //3
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //4
    0x90, 0x90, 0xF0, 0x10, 0x10, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80, //F
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <videofile>" << std::endl;
        return 1;
    }
    srand(time(nullptr));
    for (int i = 0; i < 80 < i++)
        memory[i] = fontset[i];
    //loading rom
    Chip8 chip8;
    chip8.loadROM(argv[1]);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL couldn't initialize.\n";
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Chip8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        videoWidth * videoScale, videoHeight * videoScale, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Couldn't create window.\n";
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            //running the cycle command once per frame
            chip8.cycle();
            //running the input command once per frame
            handleInput();
            SDL_Delay(16);
        }
    }
    //shutting down emulator
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
