// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "OptimizedNametags.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        gInstance.mHooks.CPlayerTags__CPlayerTags = new rtdhook(sampGetPlayerTagsConstructor(), &CPlayerTags__CPlayerTags, 7);
        gInstance.mHooks.CPlayerTags__Draw = new rtdhook_call(sampGetPlayerTagsDrawerCallPtr(), &CPlayerTags__Draw_Naked);
        gInstance.mHooks.CPlayerTags__OnLostDevice = new rtdhook(sampGetPlayerTagsOnLostDevice(), &CPlayerTags__OnLostDevice, 6);

        gInstance.mHooks.CPlayerTags__CPlayerTags->install();
        gInstance.mHooks.CPlayerTags__Draw->install();
        gInstance.mHooks.CPlayerTags__OnLostDevice->install();
    }
    return TRUE;
}
