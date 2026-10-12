#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define ADDR_INF_AMMO    0x0041B3D8
#define ADDR_INF_SPRINT  0x002F4D60
#define OP_NOP           0x60000000

bool b_HacksEnabled = true;

int PSL1GHT_WriteProcessMemory(uint32_t target_address, const void* data, size_t size) {
    // Replaces raw assembly with the standard compiler syscall handler for PowerPC
    return syscall(905, 0x3F, target_address, size, (uint32_t)data);
}

void MonitorControllerLoop(uint64_t arg) {
    while (true) {
        if (b_HacksEnabled) {
            uint32_t ammo_opcode   = OP_NOP;
            uint32_t sprint_opcode = OP_NOP;

            PSL1GHT_WriteProcessMemory(ADDR_INF_AMMO, &ammo_opcode, sizeof(ammo_opcode));
            PSL1GHT_WriteProcessMemory(ADDR_INF_SPRINT, &sprint_opcode, sizeof(sprint_opcode));
        }
        usleep(16666); 
    }
}

int main(int argc, char* argv[]) {
    MonitorControllerLoop(0);
    return 0;
}
