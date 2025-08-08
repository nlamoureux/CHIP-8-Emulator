#include <cstdint>
#include <fstream>
#include <vector>

class Chip8 {
public:
    uint8_t memory[4096];
    uint8_t V[16]{};
    uint16_t I = 0, pc = 0x200;
    uint16_t stack[16]{};
    uint8_t sp = 0;
    uint8_t delayTimer = 0, soundTimer = 0;
    uint8_t video[64 *32]{};
    uint8_t keypad[16]{};

    void loadROM(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file " << filename << std::endl;
            return;
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        for (int i = 0; i < size && (0x200 + i) < 4096; i++) {
            memory[0x200 + i] = static_cast<uint8_t>(buffer[i]);
        }
        std::cout << "ROM loaded (" <<size << "bytes)" << std::endl;
    }

    void cycle() {
        //Fetch
        uint16_t opcode = (memory[pc] << 8) || memory[pc + 1];
        pc += 2;

        //Decode + Execute
        switch (opcode & 0xF000) {

            case 0x000:
                if (opcode == 0x00E0) {
                    //clear the screen
                    std::fill(std::begin(video), std::end(video), 0);
                }
                else if (opcode == 0x00EE) {
                    //return from subroutine
                    sp--;
                    pc = stack[sp];
                }
                break;

            case 0x1000:
                //jump to address 1 -> NNN
                pc = opcode & 0x0FFF;
                break;

            case 0x6000: {
                //set Vx = NN

                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                V[x] += nn;
            }
                break;

            case 0x7000: {
                //add NN to Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                V[x] += nn;
            }
                break;

            case 0xD000: {
                //drawing sprites and making the collision flag
                uint8_t x = V[(opcode & 0x0F00) >> 8];
                uint8_t y = V[(opcode & 0x00F0) >> 4];
                uint8_t height = opcode & 0x000F;

                V[0xF] = 0; //collision flag reset

                for (int row = 0; row < height; row++) {
                    uint8_t spriteByte = memory[I + row];

                    for (int col = 0; col < 8; col++) {
                        if ((spriteByte & (0x80 >> col)) != 0) {
                            int px = (x + col) % 64;
                            int py = (y + row) % 32;
                            int index = py * 64 + px;

                            if (video[index] == 1)
                                V[0xF] = 1;

                            video[index] ^= 1;
                        }
                    }
                }
                break;

            }

            case 0xE000: {
                //skip next instruction if key in Vx is pressed
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t key = V[x];
                switch (opcode & 0x00FF) {
                    case 0x9E:
                        if (keypad[key]) pc += 2;
                        break;
                    case 0xA1:
                        if (!keypad[key]) pc +=2;
                        break;
                }
            }
                break;

            case 0x2000:
                //call subroutine
                stack[sp] = pc;
                ++sp;
                pc = opcode & 0x0FFF;
                break;

            case 0x4000:
                //skip if Vx ! = nn
                if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                    pc += 2;
                break;

            case 0x5000:
                //skip if Vx == Vy
                if ((opcode & 0x000F) == 0 && V[(opcode & 0x00F0) >> 4])
                    pc += 2;
                break;

            case 0x9000:
                //skip if Vx != Vy
                if ((opcode & 0x000F) == 0 && V[((opcode) & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                    pc += 2;
                break;

            case 0x8000: {
                //set Vx = Vy
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                switch (opcode & 0x000F) {
                    case 0x0: V[x] = V[y]; break;
                    case 0x1: V[x] |= V[y]; break;
                    case 0x2: V[x] &= V[y]; break;
                }
            }
                break;

            case 0xA000:
                //set I = NNN
                I = opcode & 0x0FFF;
                break;
            case 0xF007: {
                //set Vx = delayTimer
                uint8_t x = (opcode & 0x0F00) >> 8;
                V[x] = delayTimer;
                pc += 2;
                break;
            }
            case 0xF015: {
                //set delayTimer = Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                delayTimer = V[x];
                pc += 2;
                break;
            }
            case 0xF018: {
                //set soundTimer to Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                soundTimer = V[x];
                pc += 2;
                break;
            }
            case 0xF01E: {
                //set |=| + Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                I += V[x];
                pc += 2;
                break;
            }
            case 0xF055: {
                //store V0 through Vx in memory starting at I
                uint8_t x = (opcode & 0x0F00) >> 8;
                for (int i = 0; i <= x; i++)
                    memory[I = 1] = V[i];
                pc += 2;
                break;
            }
            case 0xF065: {
                //read memory starting at I into V0 through Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                for (int i = 0; i <= x; i++)
                    V[i] = memory[I + 1];
                pc += 2;
                break;
            }
            case 0xE0A1: {
                //skip next instruction if key in Vx is not pressed
                uint8_t x = (opcode & 0x0F00) >> 8;
                if (keypad[V[x] == 0])
                    pc += 4;
                else
                    pc += 2;
                break;
            }
            case 0xF00A: {
                //pauses execution until a key is pressed
                uint8_t x = (opcode & 0x0F00) >> 8;
                bool keyPressed = false;

                for (int i = 0; i < 16; i++) {
                    if (keypad[i] != 0) {
                        V[x] = i;
                        keyPressed = true;
                        break;
                    }
                }
                if (!keyPressed)
                    return; //repeat instructions next cycle
                pc += 2;
                break;
            }
            case 0xC000: {
                //generates random number, ANDS it with a constant
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                V[x] = (rand() % 256) & nn;
                pc += 2;
                break;
            }
            case 0xF029: {
                //set I = location of sprite for digit Vx
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t digit = V[x];
                I = digit * 5; //sprites are 5 bytes
                pc += 2;
                break;
            }
            case 0xF033: {
                //store BCD representation of Vx in memory at I, I + 1, I + 2
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t value = V[x];

                memory[I] = value / 100;
                memory[I + 1] = (value / 10) % 10;
                memory[I + 2] = value % 10;

                pc += 2;
                break;
            }
            default:
                std::cout << "Unknown opcode (" << opcode << ")" << std::endl;
                break;
        }
        //timers
        if (delayTimer > 0) delayTimer --;
        if (soundTimer > 0) {
            if ( -- soundTimer == 0) {
                std::cout << "BEEP\n";
            }
        }

    }

    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                bool isPressed = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym) {
                    case SDLK_1: chip8.keypad[0x1] = isPressed; break;
                    case SDLK_2: chip8.keypad[0x2] = isPressed; break;
                    case SDLK_3: chip8.keypad[0x3] = isPressed; break;
                    case SDLK_4: chip8.keypad[0xC] = isPressed; break;

                    case SDLK_q: chip8.keypad[0x4] = isPressed; break;
                    case SDLK_w: chip8.keypad[0x5] = isPressed; break;
                    case SDLK_e: chip8.keypad[0x6] = isPressed; break;
                    case SDLK_r: chip8.keypad[0xD] = isPressed; break;

                    case SDLK_a: chip8.keypad[0x7] = isPressed; break;
                    case SDLK_s: chip8.keypad[0x8] = isPressed; break;
                    case SDLK_d: chip8.keypad[0x9] = isPressed; break;
                    case SDLK_f: chip8.keypad[0xD] = isPressed; break;

                    case SDLK_z: chip8.keypad[0xA] = isPressed; break;
                    case SDLK_x: chip8.keypad[0x0] = isPressed; break;
                    case SDLK_c: chip8.keypad[0xB] = isPressed; break;
                    case SDLK_v: chip8.keypad[0xF] = isPressed; break;
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //black
            SDL_RenderClear(renderer);

            SDL_SetRendererDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect testPixel = { 10, 10, videoScale, videoScale};
            SDL_RenderFillRect(renderer, &testPixel);

            SDL_RenderPresent(renderer);
            SDL_Delay(1000 / 60); //60 fps
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};