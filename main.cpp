#include <io/pad.h>
#include <sys/timer.h>
#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <stdint.h>
#include <string.h>

SYS_MODULE_INFO(SR2_Mega_Trainer, 0, 1, 1);
SYS_MODULE_START(Trainer_Main_Start);
SYS_MODULE_STOP(Trainer_Main_Stop);

// Navigation & Category States
bool Is_Menu_Visible = false;
int Active_Category = 0; // 0=Weapons, 1=Weather, 2=World, 3=Outfits, 4=Characters
int Sub_Selection_Indices[5] = {0, 0, 0, 0, 0};

// Static Constants for Data Sizes
const int WEAPON_COUNT = 12;
const int WEATHER_COUNT = 6;
const int WORLD_COUNT = 7;
const int CLOTHES_COUNT = 7;
const int CHAR_COUNT = 15;

// Target Engine Offsets for Saints Row 2 (PS3 Retail)
#define WEAPON_ADD_TRIGGER_ADDR   0x002E5C10 
#define WEATHER_SET_TRIGGER_ADDR  0x004B2A80 
#define WORLD_PHYSICS_FLAG_ADDR   0x003B2A10 
#define OUTFIT_SWAP_POINTER_ADDR  0x01E8D910 
#define MODEL_SWAP_POINTER_ADDR   0x01E8D910 

// Data Structures
struct ModOption {
    const char* Label;
    uint32_t ValuePayload;
};

// --- EXTENDED DATA ARRAYS ---
ModOption WeaponList[WEAPON_COUNT] = {
    {"GDHC .50 Pistol",    932},
    {"Vice 9 Pistol",      931},
    {"K6 Krukov Rifle",    935},
    {"AR-50 Launcher",     936},
    {"AS12 Riot Shotgun",  934},
    {"Pimp Cane Shotgun",  949},
    {"GAL 43 Submachine",  933},
    {"McManus Sniper 2010",938},
    {"Mini-Gun Heavy",     939},
    {"RPG Rocket Launcher",946},
    {"Samurai Sword",      948},
    {"Stun Gun",           941}
};

ModOption WeatherList[WEATHER_COUNT] = {
    {"Clear Skies",        78669},
    {"Thunderstorm",       666},
    {"Overcast Night",     78665},
    {"Heavy Rain Fall",    78666},
    {"Foggy Morning",      78667},
    {"Normal Ambience",    78670}
};

ModOption WorldList[WORLD_COUNT] = {
    {"Low Gravity Mode",       18},
    {"Super Explosions",        7},
    {"Civilian Pedestrian War",19},
    {"Raining Pedestrians",    20},
    {"Drunk Pedestrians",      15},
    {"Evil Cars Mode",         12},
    {"Super Bad Cop Aggr",     11}
};

ModOption ClothingOutfits[CLOTHES_COUNT] = {
    {"Full Pimp Suit Set",     0x0A},
    {"Fireman Hazard Gear",    0x1F},
    {"Prison Inmate Rags",     0x02},
    {"Saints Gang Colors",     0x44},
    {"Police Uniform",         0x12},
    {"Ninja Robes",            0x21},
    {"Hot Dog Mascot Suit",    0x57}
};

ModOption CharacterMeshes[CHAR_COUNT] = {
    {"Johnny Gat",             0x10A},
    {"Shaundi",                0x10B},
    {"Pierce Washington",      0x10C},
    {"Carlos Mendoza",         0x10D},
    {"Aisha",                  0x10E},
    {"Maero (Brotherhood)",    0x20A},
    {"Matt Rhodes",            0x20C},
    {"Jessica",                0x20B},
    {"The General (Samedi)",   0x30A},
    {"Mr. Sunshine",           0x3A1},
    {"Veteran Child",          0x30C},
    {"Shogo Akuji (Ronin)",    0x25C},
    {"Jyunichi",               0x25E},
    {"Dane Vogel (Ultor)",     0x0D1},
    {"Ultor SWAT Guard",       0x0D4}
};

// --- FUNCTION EXECUTION MODULES ---
void Poke_Memory_Byte(uint32_t address, uint32_t data) {
    uint32_t* ptr = (uint32_t*)address;
    if (ptr != nullptr) *ptr = data;
}

void Execute_Trainer_Modification() {
    int currentIdx = Sub_Selection_Indices[Active_Category];
    
    switch (Active_Category) {
        case 0: // Weapon Injection
            Poke_Memory_Byte(WEAPON_ADD_TRIGGER_ADDR, WeaponList[currentIdx].ValuePayload);
            break;
            
        case 1: // Weather Shift
            Poke_Memory_Byte(WEATHER_SET_TRIGGER_ADDR, WeatherList[currentIdx].ValuePayload);
            break;
            
        case 2: // World Rule Toggles
            Poke_Memory_Byte(WORLD_PHYSICS_FLAG_ADDR, WorldList[currentIdx].ValuePayload);
            break;
            
        case 3: { // Change Clothing Outfits
            uint32_t playerPtr = *(uint32_t*)OUTFIT_SWAP_POINTER_ADDR;
            if (playerPtr != 0) {
                Poke_Memory_Byte(playerPtr + 0x2B4, ClothingOutfits[currentIdx].ValuePayload);
            }
            break;
        }
            
        case 4: { // Character Model Swapping
            uint32_t playerPtr = *(uint32_t*)MODEL_SWAP_POINTER_ADDR;
            if (playerPtr != 0) {
                Poke_Memory_Byte(playerPtr + 0x1C, CharacterMeshes[currentIdx].ValuePayload);
            }
            break;
        }
    }
}

// --- DUALSHOCK INPUT CHECK LOOP ---
void SR2_Advanced_Input_Thread(uint64_t arg) {
    PadInfo padInfo;
    PadData padData;
    ioPadInit(7);

    while (true) {
        ioPadGetInfo(&padInfo);
        if (padInfo.status == PAD_STATUS_CONNECTED) {
            ioPadGetData(0, &padData);

            // Toggle Visibility: L3 + R3
            if (padData.BTN_L3 && padData.BTN_R3) {
                Is_Menu_Visible = !Is_Menu_Visible;
                sys_timer_usleep(300000);
            }

            if (Is_Menu_Visible) {
                // Category Paging: L1 (Previous Section), R1 (Next Section)
                if (padData.BTN_L1) {
                    Active_Category = (Active_Category == 0) ? 4 : Active_Category - 1;
                    sys_timer_usleep(200000);
                }
                if (padData.BTN_R1) {
                    Active_Category = (Active_Category == 4) ? 0 : Active_Category + 1;
                    sys_timer_usleep(200000);
                }

                // Get array boundary dynamically based on current page
                int maxElements = 0;
                if (Active_Category == 0) maxElements = WEAPON_COUNT;
                else if (Active_Category == 1) maxElements = WEATHER_COUNT;
                else if (Active_Category == 2) maxElements = WORLD_COUNT;
                else if (Active_Category == 3) maxElements = CLOTHES_COUNT;
                else if (Active_Category == 4) maxElements = CHAR_COUNT;

                if (padData.BTN_UP) {
                    Sub_Selection_Indices[Active_Category]--;
                    if (Sub_Selection_Indices[Active_Category] < 0) {
                        Sub_Selection_Indices[Active_Category] = maxElements - 1;
                    }
                    sys_timer_usleep(150000);
                }
                if (padData.BTN_DOWN) {
                    Sub_Selection_Indices[Active_Category]++;
                    if (Sub_Selection_Indices[Active_Category] >= maxElements) {
                        Sub_Selection_Indices[Active_Category] = 0;
                    }
                    sys_timer_usleep(150000);
                }

                // Action Execution: Press CROSS (X) to apply modification instantly
                if (padData.BTN_CROSS) {
                    Execute_Trainer_Modification();
                    sys_timer_usleep(300000);
                }
            }
        }
        sys_timer_usleep(16666); 
    }
    sys_ppu_thread_exit(0);
}

extern "C" int Trainer_Main_Start(size_t args, void *argp) {
    sys_ppu_thread_t id;
    sys_ppu_thread_create(&id, SR2_Advanced_Input_Thread, 0, 1001, 0x3000, SYS_PPU_THREAD_CREATE_JOINABLE, "SR2AdvancedThread");
    return SYS_PRX_RESIDENT;
}

extern "C" int Trainer_Main_Stop(void) {
    return SYS_PRX_RESIDENT;
}
