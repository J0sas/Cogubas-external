#include "memory.hpp"
#include <thread>



struct glow_t
{
    float r, g, b, a;
};

int main()
{
    // get csgo handle
    const auto mem = memory(L"csgo.exe");
    
    // get client.dll address
    const auto client = mem.get_module_address(L"client.dll");


    while (true) {
        // sleep so our pc does not die
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // get local player, mflags and crosshair(this crosshair will be used later)

        const auto local_player = mem.read<std::uintptr_t>(client + offsets::signatures::dwLocalPlayer);
        const auto local_player_flags = mem.read<std::uintptr_t>(local_player + offsets::netvars::m_fFlags);
        const auto local_player_team = mem.read<std::uintptr_t>(local_player + offsets::netvars::m_iTeamNum);
        
        const auto glow_object_manager = mem.read<std::uintptr_t>(client + offsets::signatures::dwGlowObjectManager);
        
        const auto crosshair_id = mem.read<std::uintptr_t>(local_player + offsets::netvars::m_iCrosshairId);
        
        const auto entity_for_triggerbot = mem.read<std::uintptr_t>(client + offsets::signatures::dwEntityList + ((crosshair_id - 1) * 0x10));
        
        const glow_t color = { 0.f, 0.5f, 1.f, 1.f };

        //triggerbot
        //todo: find a way to deal with recoil
        if (GetAsyncKeyState(VK_XBUTTON1))
        {
            if (crosshair_id > 0 && crosshair_id < 64)
            {

                // check if the player is on my team
                if ((mem.read<std::uintptr_t>(entity_for_triggerbot + offsets::netvars::m_iTeamNum) == local_player_team)) {
                    continue;
                }
                // shoots :D 
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                mem.write<std::uintptr_t>(client + offsets::signatures::dwForceAttack, 6); // +attack
            }
        }
        //glow
        for (auto i = 0; i < 64; i++)
        {
            const auto entity = mem.read<std::uintptr_t>(client + offsets::signatures::dwEntityList + (i * 0x10)); 
            // get the location of each entity, from 0 to 64.

            if (!entity)
                continue;

            // check if they are on my team

            if (mem.read<std::uintptr_t>(entity + offsets::netvars::m_iTeamNum) == local_player_team)
                continue;


            const auto glow_index = mem.read<std::int32_t>(entity + offsets::netvars::m_iGlowIndex);

            mem.write<glow_t>(glow_object_manager + (glow_index * 0x38) + 0x8, color); 
            // write our glow structure, so we dont have to make multiple WPM

            mem.write<bool>(glow_object_manager + (glow_index * 0x38) + 0x28, true); // render when occluded
            mem.write<bool>(glow_object_manager + (glow_index * 0x38) + 0x29, false); // render when unoccluded
        }
        
        // bhop
        if (GetAsyncKeyState(VK_SPACE))
        {
            if (local_player_flags == 257)
            {
                mem.write<std::uintptr_t>(client + offsets::signatures::dwForceJump, 5);
            }
            else
            {
                mem.write<std::uintptr_t>(client + offsets::signatures::dwForceJump, 4);
            }
        }
       

        if (GetAsyncKeyState(VK_DELETE))
        {
            return 0;
        }
    }

    return 0;
}