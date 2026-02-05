#include <sdk/includes.h>
#include <enhance/enhance.h>
#include <enhance/hooks/Hook.h>
#include <enhance/utils/logger.h>

static HMODULE g_hModule = nullptr;

void __stdcall enhance_thread(HINSTANCE instance)
{
    g_hModule = instance;
    
    logger::init(instance);

    enhance::instance = std::make_unique<enhance::enhance_client>();

    if (!enhance::instance->attach())
    {
        return;
    }

    enhance::instance->run();
}

BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID lp_reserved)
{
    if (ul_reason_for_call != DLL_PROCESS_ATTACH)
        return FALSE;

    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)enhance_thread, h_module, 0, 0);

    return TRUE;
}