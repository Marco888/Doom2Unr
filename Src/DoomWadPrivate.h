#pragma once

#include "Engine.h"
#include "Editor.h"
#include "WADContents.h"

class UDoomWadExp : public UCommandlet
{
public:
	DECLARE_CLASS(UDoomWadExp, UCommandlet, 0, DoomWadExp);

	void StaticConstructor();
	INT Main(const TCHAR* Parms);
};
