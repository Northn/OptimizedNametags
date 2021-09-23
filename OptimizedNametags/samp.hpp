#pragma once

enum SAMPVER {
    SAMP_NOT_LOADED,
    SAMP_UNKNOWN,
    SAMP_037_R1,
    SAMP_037_R3_1
};

const uintptr_t samp_addressess[][14]
{
    // CPlayerTags::CPlayerTags <- UNUSED, Drawer -> call CPlayerTags::DrawLabel, CPlayerTags::OnLostDevice, CPlayerTags::OnResetDevice <- UNUSED, CFonts, CFonts::DrawLittleText, CFonts::GetLittleTextScreenSize, D3DXMATRIX Projection, D3DXMATRIX View, CDeathWindow, CDeathWindow::CreateAuxFonts, CEntity::GetDistanceToCamera(), ReturnAddress from DrawLabel, ReturnAddress from DrawHealth
    {0x68610, 0x70F96, 0x68F70, 0x68FA0, 0x21A0FC, 0x66E00, 0x66BD0, 0x12C980, 0x12C940, 0x21A0EC, 0x65F10, 0x9A7D0, 0x70E10, 0x6FD00},
    {0x6C580, 0x74E8A, 0x6CEE0, 0x6CF10, 0x26E8E4, 0x6AD70, 0x6AB40, 0x140B00, 0x140AC0, 0x26E8D0, 0x69440, 0x9EA80, 0x74CFC, 0x73BEC}
};

inline uintptr_t sampGetBase()
{
    static uintptr_t sampBase = SAMPVER::SAMP_NOT_LOADED;
    if (sampBase == SAMPVER::SAMP_NOT_LOADED)
        sampBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("samp.dll"));
    return sampBase;
}

#pragma warning(disable : 26812)
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
#pragma warning(default : 26812)

#define SAMP_OFFSET samp_addressess[sampGetVersion() - 2]

// UNUSED
//inline uintptr_t sampGetPlayerTagsConstructor()
//{
//    return sampGetBase() + SAMP_OFFSET[0];
//}

inline uintptr_t sampGetPlayerTagsDrawerCallPtr()
{
    return sampGetBase() + SAMP_OFFSET[1];
}

inline uintptr_t sampGetPlayerTagsOnLostDevice()
{
    return sampGetBase() + SAMP_OFFSET[2];
}

// UNUSED
//inline uintptr_t sampGetPlayerTagsOnResetDevice()
//{
//    return sampGetBase() + SAMP_OFFSET[3];
//}

inline void sampDrawText(ID3DXSprite* sprite, const char* text, RECT& rect, DWORD color, BOOL shadow)
{
    reinterpret_cast<void(__thiscall*)(void*, ID3DXSprite*, const char*, RECT, int, DWORD, BOOL)>(sampGetBase() + SAMP_OFFSET[5])
        (*reinterpret_cast<void**>(sampGetBase() + SAMP_OFFSET[4]), sprite, text, rect, DT_LEFT | DT_NOCLIP, color, shadow);
}

inline SIZE sampGetMeasuredTextSize(const char* text)
{
    SIZE size;
    reinterpret_cast<void(__thiscall*)(void*, SIZE*, const char*, int)>(sampGetBase() + SAMP_OFFSET[6])
        (*reinterpret_cast<void**>(sampGetBase() + SAMP_OFFSET[4]), &size, text, DT_LEFT | DT_NOCLIP);
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

inline ID3DXFont* sampGetAuxFont()
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

inline uintptr_t sampGetPedDistanceGetterFuncPtr()
{
    return sampGetBase() + SAMP_OFFSET[11];
}

inline std::tuple<void*, void*> sampGetPedDistanceGetterReturnAddresses()
{
    static void *label = nullptr, *health = nullptr;
    if (label && health)
        goto ret;

    label = reinterpret_cast<void*>(sampGetBase() + SAMP_OFFSET[12]);
    health = reinterpret_cast<void*>(sampGetBase() + SAMP_OFFSET[13]);

ret:
    return std::make_tuple(label, health);
}

#undef SAMP_OFFSET
