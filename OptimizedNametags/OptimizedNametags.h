#pragma once
#pragma comment(lib, "d3dx9.lib")

typedef struct { float x, y, z; } CVector;

class OptimizedNametags
{
public:
	static constexpr int MAX_PLAYERS = 1004;

	struct NameTag {
		ID3DXSprite* sprite = nullptr;
		ID3DXRenderToSurface* renderToSurface = nullptr;
		IDirect3DSurface9* surface = nullptr;
		IDirect3DTexture9* texture = nullptr;

		D3DCOLOR color = -1;
		float center = 0.f;
		bool redraw = false,
			isAfk = false;
	};

	NameTag mNametags[MAX_PLAYERS];

	struct {
		rtdhook
			* CPlayerTags__OnLostDevice = nullptr,
			* CEntity__GetDistanceToCamera = nullptr;
		rtdhook_call* CPlayerTags__Draw = nullptr;
	} mHooks;

	IDirect3DDevice9* mD3DDevice = nullptr;

	bool     shouldRedrawNametag(NameTag& nt, const char* name, D3DCOLOR color, bool isAfk);
	bool     createElements(NameTag& nt, SIZE& textureSize);
};

#define DX_SAFE_RELEASE(p) if (p != nullptr) { p->Release(); p = nullptr; }

inline OptimizedNametags gInstance;

void __fastcall CPlayerTags__Draw(uintptr_t self, int id, CVector* playerPos, const char* szText,
	D3DCOLOR color, float fDistanceToCamera, bool bDrawStatus, int nStatus);
void CPlayerTags__Draw_Naked();

double __fastcall CEntity__GetDistanceToCamera(uintptr_t self);

void __fastcall CPlayerTags__OnLostDevice(void* self);
