#include "pch.h"
#include "AssetUtils.h"
#include "../engine/Engine.h"

extern engine* Engine;

void assetCallbacks::ImageLoadCallback(cTextureAsset* Asset)
{
	Engine->AssetRegistry.push_back(Asset);
	Engine->TextureRegistry.push_back(Asset);
	Engine->Renderer.RegisterTexture(Asset, false);
}

void assetCallbacks::FontLoadCallback(cFontAsset* Asset)
{
	Engine->AssetRegistry.push_back(Asset);
	Engine->Renderer.RegisterTexture(Asset, false);
}