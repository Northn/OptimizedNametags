#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3dx9core.h>

enum SAMPVER {
    SAMP_NOT_LOADED,
    SAMP_UNKNOWN,
    SAMP_037_R1,
    SAMP_037_R3_1
};

const uintptr_t samp_addressess[][11]
{
    // CPlayerTags::CPlayerTags, Drawer -> call CPlayerTags::DrawLabel, CPlayerTags::OnLostDevice, CPlayerTags::OnResetDevice <- UNUSED, CFonts, CFonts::DrawText, CFonts::GetTextScreenSize, D3DXMATRIX Projection, D3DXMATRIX View, CDeathWindow, CDeathWindow::CreateAuxFonts
    {0x68610, 0x70F96, 0x68F70, 0x68FA0, 0x21A0FC, 0x66C80, 0x66B20, 0x12C980, 0x12C940, 0x21A0EC, 0x65F10},
    {0x6C580, 0x74E8A, 0x6CEE0, 0x6CF10, 0x26E8E4, 0x6ABF0, 0x6AA90, 0x140B00, 0x140AC0, 0x26E8D0, 0x69440}
};

inline uintptr_t sampGetBase()
{
    static uintptr_t sampBase = SAMPVER::SAMP_NOT_LOADED;
    if (sampBase == SAMPVER::SAMP_NOT_LOADED)
        sampBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("samp.dll"));
    return sampBase;
}

// https://github.com/imring/TimeFormat/blob/master/samp.hpp#L19

inline SAMPVER sampGetVersion()
{
    static SAMPVER sampVersion = SAMPVER::SAMP_NOT_LOADED;
    if (sampVersion <= SAMPVER::SAMP_UNKNOWN)
    {
        uintptr_t base = sampGetBase();
        if (base == SAMPVER::SAMP_NOT_LOADED) return SAMPVER::SAMP_NOT_LOADED;

        IMAGE_NT_HEADERS* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);

        DWORD ep = ntHeader->OptionalHeader.AddressOfEntryPoint;
        switch (ep)
        {
        case 0x31DF13:
            sampVersion = SAMPVER::SAMP_037_R1;
            break;
        case 0xCC4D0:
            sampVersion = SAMPVER::SAMP_037_R3_1;
            break;
        default:
            sampVersion = SAMPVER::SAMP_UNKNOWN;
        }
    }

    return sampVersion;
}

#define SAMP_OFFSET samp_addressess[sampGetVersion() - 2]

inline uintptr_t sampGetPlayerTagsConstructor()
{
    return sampGetBase() + SAMP_OFFSET[0];
}

inline uintptr_t sampGetPlayerTagsDrawerCallPtr()
{
    return sampGetBase() + SAMP_OFFSET[1];
}

inline uintptr_t sampGetPlayerTagsOnLostDevice()
{
    return sampGetBase() + SAMP_OFFSET[2];
}

inline uintptr_t sampGetPlayerTagsOnResetDevice()
{
    return sampGetBase() + SAMP_OFFSET[3];
}

inline void sampDrawText(ID3DXSprite* sprite, const char* text, RECT& rect, DWORD color, BOOL shadow)
{
    reinterpret_cast<void(__thiscall*)(void*, ID3DXSprite*, const char*, RECT, DWORD, BOOL)>(sampGetBase() + SAMP_OFFSET[5])
        (*reinterpret_cast<void**>(sampGetBase() + SAMP_OFFSET[4]), sprite, text, rect, color, shadow);
}

inline SIZE sampGetMeasuredTextSize(const char* text)
{
    static SIZE size;
    reinterpret_cast<void(__thiscall*)(void*, SIZE*, const char*, int)>(sampGetBase() + SAMP_OFFSET[6])
        (*reinterpret_cast<void**>(sampGetBase() + SAMP_OFFSET[4]), &size, text, DT_LEFT);
    return size;
}

inline D3DXMATRIX* sampGetProjectionMatrix()
{
    return reinterpret_cast<D3DXMATRIX*>(sampGetBase() + SAMP_OFFSET[7]);
}

inline D3DXMATRIX* sampGetViewMatrix()
{
    return reinterpret_cast<D3DXMATRIX*>(sampGetBase() + SAMP_OFFSET[8]);
}

inline ID3DXFont* sampGetDeathWindowFonts()
{
    static ID3DXFont* auxFont = nullptr;
    static uintptr_t CDeathWindow = 0;

    if (auxFont)
        goto ret;

    CDeathWindow = *reinterpret_cast<uintptr_t*>(sampGetBase() + SAMP_OFFSET[9]);
    if (CDeathWindow)
    {
        if (!*reinterpret_cast<BOOL*>(CDeathWindow + 0x14B))
            reinterpret_cast<void(__thiscall*)(uintptr_t)>(sampGetBase() + SAMP_OFFSET[10])(CDeathWindow);

        auxFont = *reinterpret_cast<ID3DXFont**>(CDeathWindow + 0x153);
    }

ret:
    return auxFont;
}

#undef SAMP_OFFSET
