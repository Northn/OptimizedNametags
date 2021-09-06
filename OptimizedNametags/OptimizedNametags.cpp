#include "pch.h"
#include "OptimizedNametags.h"

bool OptimizedNametags::shouldRedrawNametag(NameTag& nt, const char* name, D3DCOLOR color, bool isAfk) {
	bool ret = (!nt.sprite || !nt.texture || !nt.surface || !nt.renderToSurface) || nt.redraw || nt.isAfk != isAfk || nt.color != color;
	if (nt.redraw) nt.redraw = false;
	return ret;
}

bool OptimizedNametags::createElements(NameTag& nt, SIZE& size)
{
	static D3DSURFACE_DESC desc;

	if (!nt.sprite || !nt.texture || !nt.surface || !nt.renderToSurface)
	{
		DX_SAFE_RELEASE(nt.sprite);
		DX_SAFE_RELEASE(nt.texture);
		DX_SAFE_RELEASE(nt.renderToSurface);
		DX_SAFE_RELEASE(nt.surface);

		if (FAILED(D3DXCreateSprite(mD3DDevice, &nt.sprite))) return false;
		if (FAILED(D3DXCreateTexture(mD3DDevice, size.cx + 1, size.cy + 3 + 48, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &nt.texture)))
			return false;
		if (FAILED(nt.texture->GetSurfaceLevel(0, &nt.surface))) return false;
		if (FAILED(nt.surface->GetDesc(&desc))) return false;
		if (FAILED(D3DXCreateRenderToSurface(mD3DDevice, desc.Width,
			desc.Height, desc.Format, TRUE, D3DFMT_D16, &nt.renderToSurface))) return false;
	}
	return true;
}

void __fastcall CPlayerTags__Draw(uintptr_t self, int id, CVector* playerPos, const char* szText,
	D3DCOLOR color, float fDistanceToCamera, bool bDrawStatus, int nStatus)
{
	auto& nt = gInstance.mNametags[id];

	auto& pDevice = gInstance.mD3DDevice;
	auto isAfk = bDrawStatus && nStatus == 2;

	if (gInstance.shouldRedrawNametag(nt, szText, color, isAfk))
	{
		auto textSize = sampGetMeasuredTextSize(szText);
		if (!gInstance.createElements(nt, textSize)) return;

		RECT rect{ 0, 0, textSize.cx + 1, textSize.cy + 1 };

		if (SUCCEEDED(nt.renderToSurface->BeginScene(nt.surface, NULL)))
		{
			pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0);
			if (SUCCEEDED(nt.sprite->Begin(D3DXSPRITE_ALPHABLEND)))
			{
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BLENDFACTOR);
				nt.center = static_cast<float>(textSize.cx) / 2.f;
				sampDrawText(nt.sprite, szText, rect, color, true);

				if (isAfk)
				{
					auto auxFont = sampGetDeathWindowFont();

					if (auxFont)
					{
						rect.top = textSize.cy + 3;
						rect.left = static_cast<long>(nt.center / 2.6f);
						auxFont->DrawTextA(nt.sprite, "C", 1, &rect, DT_NOCLIP | DT_LEFT, D3DCOLOR_XRGB(0, 0, 0));
						rect.left += 3;
						rect.top -= 1;
						auxFont->DrawTextA(nt.sprite, "E", 1, &rect, DT_NOCLIP | DT_LEFT, -1);
					}
				}
				nt.sprite->End();
			}
			nt.renderToSurface->EndScene(0);
		}

		nt.color = color;
		nt.isAfk = isAfk;
	}

	D3DXVECTOR3 TagPos(playerPos->x, playerPos->y, playerPos->z);
	TagPos.z += 0.25f + (fDistanceToCamera * 0.047f);

	D3DVIEWPORT9 Viewport;
	pDevice->GetViewport(&Viewport);

	D3DXVECTOR3 Out;
	D3DXMATRIX matIdent;
	D3DXMatrixIdentity(&matIdent);

	D3DXVec3Project(&Out, &TagPos, &Viewport, sampGetProjectionMatrix(), sampGetViewMatrix(), &matIdent);

	if (Out.z > 1.f) return;

	if (SUCCEEDED(nt.sprite->Begin(D3DXSPRITE_ALPHABLEND)))
	{
		D3DXVECTOR3 center{ nt.center, 0.f, 0.f };
		nt.sprite->Draw(nt.texture, NULL, &center, &Out, 0xFFFFFFFF);
		nt.sprite->End();
	}

	//if (bDrawStatus and nStatus == 2)
	/*{
		auto CDeathWindow = *reinterpret_cast<uintptr_t*>(sampGetBase() + 0x26E8D0);
		if (!*reinterpret_cast<BOOL*>(CDeathWindow + 331))
		{
			reinterpret_cast<void(__thiscall*)(uintptr_t)>(sampGetBase() + 0x69440)(CDeathWindow);
		}
		ID3DXFont* auxFont = *reinterpret_cast<ID3DXFont**>(CDeathWindow + 339);
		ID3DXFont* weapFont = *reinterpret_cast<ID3DXFont**>(CDeathWindow + 319);
		if (auxFont && weapFont)
		{
			std::cout << reinterpret_cast<char*>(sampGetBase() + 0xE62F8) << std::endl;
			RECT rect{ (int)Out.x - 33, (int)Out.y + 32, (int)Out.x + 1 - 33, (int)Out.y + 33 };
			rect.left -= 2;
			rect.right -= 2;
			auxFont->DrawTextA(NULL, "C", 1, &rect, 261, D3DCOLOR_XRGB(0, 0, 0));
			rect.left -= 2;
			rect.right -= 2;
			auxFont->DrawTextA(NULL, "E", 1, &rect, 261, -1);
		}
	}*/

	/*reinterpret_cast<void(__thiscall*)(uintptr_t, CVector * playerPos, const char* szText,
		D3DCOLOR color, float fDistanceToCamera, bool bDrawStatus, int nStatus)>(gInstance.mHooks.CPlayerTags__Draw->getHookedFunctionAddress())
		(self, playerPos, szText, color, fDistanceToCamera, bDrawStatus, nStatus);*/
}

__declspec(naked) void CPlayerTags__Draw_Naked()
{
	/*static CVector* coords;
	static const char* text;*/
	__asm {
		mov     edx, edi          // id: save edi, push id to drawer function
		/*mov     edi, [esp + 4]     // coords
		mov     coords, edi
		mov     edi, [esp + 8]     // text
		mov     text, edi

		mov     edi, edx           // return id to edi */
		jmp     CPlayerTags__Draw
	}
}

void __fastcall CPlayerTags__OnLostDevice(void* self)
{
	for (auto& nt : gInstance.mNametags)
	{
		DX_SAFE_RELEASE(nt.sprite);
		DX_SAFE_RELEASE(nt.texture);
		DX_SAFE_RELEASE(nt.renderToSurface);
		DX_SAFE_RELEASE(nt.surface);
	}

	reinterpret_cast<decltype(CPlayerTags__OnLostDevice)*>(gInstance.mHooks.CPlayerTags__OnLostDevice->getTrampoline())
		(self);
}
