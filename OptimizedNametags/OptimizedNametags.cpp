#include "pch.h"
#include "OptimizedNametags.h"

#pragma intrinsic(_ReturnAddress)

bool OptimizedNametags::shouldRedrawNametag(NameTag& nt, D3DCOLOR color, bool isAfk) {
	bool ret = (!nt.sprite || !nt.texture || !nt.surface || !nt.renderToSurface) || nt.redraw || nt.isAfk != isAfk || nt.color != color;
	if (nt.redraw) nt.redraw = false;
	return ret;
}

bool OptimizedNametags::createElements(NameTag& nt, SIZE& textureSize)
{
	static D3DSURFACE_DESC desc;

	if (!nt.sprite || !nt.texture || !nt.surface || !nt.renderToSurface)
	{
		DX_SAFE_RELEASE(nt.sprite);
		DX_SAFE_RELEASE(nt.texture);
		DX_SAFE_RELEASE(nt.renderToSurface);
		DX_SAFE_RELEASE(nt.surface);

		/* Reserved due to some unknown reasons: nicknames sometimes gets cropped and can't be fully rendered.
		 * I have no idea why it's happening */
		textureSize.cx += 1 /*outline*/ + 48 /*reserved*/ + 1;
		textureSize.cy += 1 /*outline*/ + 4 /*AFK padding*/ + 48 /*AFK icon*/;
		if ((textureSize.cx % 2) != 0) textureSize.cx++;
		if ((textureSize.cy % 2) != 0) textureSize.cy++;

		if (FAILED(D3DXCreateSprite(mD3DDevice, &nt.sprite))) return false;
		if (FAILED(D3DXCreateTexture(mD3DDevice, textureSize.cx, textureSize.cy, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &nt.texture)))
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

	if (gInstance.shouldRedrawNametag(nt, color, isAfk))
	{
		auto textSize = sampGetMeasuredTextSize(szText);
		SIZE textureSize = textSize;
		if (!gInstance.createElements(nt, textureSize)) return;
		RECT rect{ 1, 1, textureSize.cx, textureSize.cy };

		if (SUCCEEDED(nt.renderToSurface->BeginScene(nt.surface, NULL)))
		{
			pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0);
			if (SUCCEEDED(nt.sprite->Begin(D3DXSPRITE_ALPHABLEND)))
			{
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BLENDFACTOR);
				nt.center = 1.f + static_cast<float>(textSize.cx) / 2.f;
				sampDrawText(nt.sprite, szText, rect, color, true);

				if (isAfk)
				{
					auto auxFont = sampGetAuxFont();

					if (auxFont)
					{
						rect.top += textSize.cy + 4;
						rect.left = static_cast<long>(nt.center / 2.6f);
						auxFont->DrawTextA(nt.sprite, "C", 1, &rect, DT_NOCLIP | DT_LEFT, D3DCOLOR_XRGB(0, 0, 0));
						rect.left += 3;
						rect.top -= 1;
						auxFont->DrawTextA(nt.sprite, "E", 1, &rect, DT_NOCLIP | DT_LEFT, -1);
					}
				}
				nt.sprite->End();

				nt.color = color;
				nt.isAfk = isAfk;
			}
			nt.renderToSurface->EndScene(0);
		}
	}

	D3DXVECTOR3 TagPos{ playerPos->x, playerPos->y, playerPos->z + 0.21f + (fDistanceToCamera * 0.05f)};

	D3DVIEWPORT9 Viewport;
	pDevice->GetViewport(&Viewport);

	D3DXVECTOR3 Out;
	D3DXMATRIX matIdent;
	D3DXMatrixIdentity(&matIdent);

	D3DXVec3Project(&Out, &TagPos, &Viewport, sampGetProjectionMatrix(), sampGetViewMatrix(), &matIdent);
	Out.x -= nt.center;
	Out.x = static_cast<float>(static_cast<long>(Out.x));
	Out.y = static_cast<float>(static_cast<long>(Out.y));

	if (SUCCEEDED(nt.sprite->Begin(D3DXSPRITE_ALPHABLEND)))
	{
		nt.sprite->Draw(nt.texture, NULL, NULL, &Out, 0xFFFFFFFF);
		nt.sprite->End();
	}

	/*reinterpret_cast<void(__thiscall*)(uintptr_t, CVector * playerPos, const char* szText,
		D3DCOLOR color, float fDistanceToCamera, bool bDrawStatus, int nStatus)>(gInstance.mHooks.CPlayerTags__Draw->getHookedFunctionAddress())
		(self, playerPos, szText, color, fDistanceToCamera, bDrawStatus, nStatus);*/
}

__declspec(naked) void CPlayerTags__Draw_Naked()
{
	__asm {
		mov     edx, edi
		jmp     CPlayerTags__Draw
	}
}

double __fastcall CEntity__GetDistanceToCamera(uintptr_t self)
{
	static void *label, *health;
	uintptr_t CEntity = 0;

	std::tie(label, health) = sampGetPedDistanceGetterReturnAddresses();
	auto retAddress = _ReturnAddress();
	if (retAddress != label && retAddress != health)
		goto ret;
	
	CEntity = *reinterpret_cast<uintptr_t*>(self + 0x40);

	if (!reinterpret_cast<bool(__thiscall*)(uintptr_t)>(0x534540)(CEntity))
		return DBL_MAX;

ret:
	return reinterpret_cast<decltype(CEntity__GetDistanceToCamera)*>(gInstance.mHooks.CEntity__GetDistanceToCamera->getTrampoline())
		(self);
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
