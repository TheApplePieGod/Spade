#include "pch.h"
#include "../engine/Engine.h"

extern engine* Engine;

assetTypes::material_data material::Serialize()
{
	assetTypes::material_data Data = assetTypes::material_data();
	
	Data.DiffuseColor = DiffuseColor;
	Data.Reflectivity = Reflectivity;

	if (DiffuseTextureID != -1)
		strcpy(Data.DiffuseFileName, Engine->TextureRegistry[DiffuseTextureID]->Filename);
	if (NormalTextureID != -1)
		strcpy(Data.NormalFileName, Engine->TextureRegistry[NormalTextureID]->Filename);
	if (ReflectiveTextureID != -1)
		strcpy(Data.ReflectiveFileName, Engine->TextureRegistry[ReflectiveTextureID]->Filename);

	return Data;
}

void material::Deserialize(assetTypes::material_data Data)
{
	DiffuseColor = Data.DiffuseColor;
	Reflectivity = Data.Reflectivity;

	DiffuseTextureID = GetTextureIDFromName(Data.DiffuseFileName);
	NormalTextureID = GetTextureIDFromName(Data.NormalFileName);
	ReflectiveTextureID = GetTextureIDFromName(Data.ReflectiveFileName);
}