// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "OptimizedNametags.h"

rtdhook_call* mainloop_hook = nullptr;

void mainloop()
{
    static bool loaded = false;
#pragma warning(disable : 26812)
    if (!loaded && sampGetVersion() > SAMPVER::SAMP_UNKNOWN) // If SAMP loaded & is compatible version
#pragma warning(default : 26812)
    {
        gInstance.mHooks.CPlayerTags__Draw = new rtdhook_call(sampGetPlayerTagsDrawerCallPtr(), &CPlayerTags__Draw_Naked);
        gInstance.mHooks.CPlayerTags__OnLostDevice = new rtdhook(sampGetPlayerTagsOnLostDevice(), &CPlayerTags__OnLostDevice, 6);
        gInstance.mHooks.CEntity__GetDistanceToCamera = new rtdhook(sampGetPedDistanceGetterFuncPtr(), &CEntity__GetDistanceToCamera, 6);

        gInstance.mHooks.CPlayerTags__Draw->install();
        gInstance.mHooks.CPlayerTags__OnLostDevice->install();
        gInstance.mHooks.CEntity__GetDistanceToCamera->install();

        gInstance.mD3DDevice = *reinterpret_cast<IDirect3DDevice9**>(0xC97C28);

        loaded = true;
    }
    reinterpret_cast<void(*)()>(mainloop_hook->getHookedFunctionAddress())();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        mainloop_hook = new rtdhook_call(0x53E968, &mainloop);
        mainloop_hook->install();
    }
    return TRUE;
}
