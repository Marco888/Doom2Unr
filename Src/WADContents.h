#pragma once

#include "WadData.h"

struct sector_t;
struct line_t;
struct side_t;
struct subsector_s;
struct node_t;
struct vertex_t;

constexpr INT FlatResolution = 64;
constexpr INT FlatTotalSize = (FlatResolution * FlatResolution);
constexpr TCHAR SkyTextureName[] = TEXT("F_SKY1");

#define FF_FULLBRIGHT	0x8000	// flag in thing->frame
#define FF_FRAMEMASK	0x7fff

inline const TCHAR* ByteToStr(const BYTE* B, const INT Len)
{
	TCHAR* Res = appStaticString1024();
	for (INT i = 0; i < Len; ++i)
		Res[i] = B[i];
	Res[Len] = 0;
	return Res;
}

enum EDoomTextureType : BYTE
{
	TT_Unknown,
	TT_Flat,
	TT_Walls,
	TT_Sky,
	TT_Switch,
};
inline const TCHAR* GetTexTypeStr(EDoomTextureType T)
{
	switch (T)
	{
	case TT_Unknown:
		return TEXT("Unknown");
	case TT_Flat:
		return TEXT("Flat");
	case TT_Walls:
		return TEXT("Wall");
	case TT_Sky:
		return TEXT("Sky");
	case TT_Switch:
		return TEXT("Switch");
	default:
		return TEXT("Undefined");
	}
}

// Instanced doom texture.
struct FInstTexture
{
	FName TextureName;
	INT U, V, FinalU, FinalV;
	FLOAT ScaleU, ScaleV;
	FInstTexture* FirstFrame;
	EDoomTextureType Type;
	BITFIELD bIsWater : 1;
	BITFIELD bIsLava : 1;
	BITFIELD bIsSwitch : 1;

	FInstTexture(FName TexName, EDoomTextureType TT)
		: TextureName(TexName), U(FlatResolution), V(FlatResolution), FinalU(FlatResolution), FinalV(FlatResolution), ScaleU(1.f), ScaleV(1.f), FirstFrame(nullptr), Type(TT), bIsWater(FALSE), bIsLava(FALSE), bIsSwitch(FALSE)
	{}
	inline void SetDimensions(INT inU, INT inV)
	{
		U = inU;
		V = inV;
		FinalU = FNextPowerOfTwo(inU);
		FinalV = FNextPowerOfTwo(inV);

		if (U != FinalU)
			ScaleU = FLOAT(FinalU) / FLOAT(U);
		else ScaleU = 1.f;

		if (V != FinalV)
			ScaleV = FLOAT(FinalV) / FLOAT(V);
		else ScaleV = 1.f;
	}
	inline void CopyDimensions(const FInstTexture& Other)
	{
		U = Other.U;
		V = Other.V;
		FinalU = Other.FinalU;
		FinalV = Other.FinalV;
		ScaleU = Other.ScaleU;
		ScaleV = Other.ScaleV;
	}
	const TCHAR* GetFileName() const;

	inline UBOOL IsValid() const
	{
		return (Type != TT_Unknown);
	}
	inline UBOOL IsWater() const
	{
		return ((FirstFrame ? FirstFrame->bIsWater : bIsWater) != 0);
	}
	inline UBOOL IsLava() const
	{
		return ((FirstFrame ? FirstFrame->bIsLava : bIsLava) != 0);
	}
	inline UBOOL IsASwitch() const
	{
		return (bIsSwitch && (Type != TT_Unknown));
	}

	static FInstTexture GSkyTexture, GNullTexture;
	static TMap<FName, FInstTexture*>* GTextureMap;

	static FInstTexture* RegisterTexture(FName TexName);
	static FInstTexture* ReadTextureName(FArchive* Ar, const TCHAR* Prefix = NULL);

	FInstTexture* CreateSwitch(const INT SwitchIndex);

	inline const TCHAR* Describe() const
	{
		guardSlow(FInstTexture::Describe);
		TCHAR* Result = appStaticString1024();
		if (TextureName == NAME_None)
			appSprintf(Result, TEXT("<%ls>"), *TextureName);
		else appSprintf(Result, TEXT("<Name='%ls',Type=%ls,Parent='%ls',Res=(%i,%i)>"), *TextureName, GetTexTypeStr(Type), (FirstFrame ? *FirstFrame->TextureName : TEXT("NULL")), U, V);
		return Result;
		unguardSlow;
	}
};

struct WADHeader
{
	BYTE Signature[4]; // "IWAD" or "PWAD", not NULL-terminated
	INT numFiles; // Number of files 
	INT offFAT; // Offset of FAT (directory) from start of file
	TCHAR USignature[5];

	WADHeader(FArchive* Ar)
	{
		Ar->Serialize(Signature, sizeof(WADHeader::Signature));
		*Ar << numFiles << offFAT;

		for (INT i = 0; i < 4; ++i)
			USignature[i] = Signature[i];
		USignature[4] = 0;
	}
};
struct WADFileEntry
{
	INT offData; // Offset of lump's data (from start of WAD file)
	INT lenData; //	Size of lump, in bytes
	BYTE name[8]; // Name of lump, not NULL terminated but padded with 0x00 if necessary
	TCHAR UName[9];

	inline void Load(FArchive* Ar)
	{
		*Ar << offData << lenData;
		Ar->Serialize(name, sizeof(WADFileEntry::name));

		for (INT i = 0; i < 8; ++i)
			UName[i] = name[i];
		UName[8] = 0;
	}
};

enum EEntFlags
{
	EOPTS_Easy=1,
	EOPTS_Medium=2,
	EOPTS_Hard=4,
	EOPTS_Ambush=8,
	EOPTS_Net=16,
};

constexpr INT THING_SIZE = (sizeof(short) * 5);
struct FThingEntry
{
	SWORD x, y, angle, type, options;

	FThingEntry(FArchive* Ar)
	{
		*Ar << x << y << angle << type << options;
	}

	inline UBOOL IsSpawnPoint() const
	{
		return (type > 0 && type <= 4) || type == 11;
	}
	inline UBOOL IsDMSpawn() const
	{
		return type == 11;
	}
	inline UBOOL IsDMEntity() const
	{
		return (options & EOPTS_Net) != 0;
	}

	inline const TCHAR* GetTypeName() const
	{
		static TCHAR MiscStr[64];
		if (IsSpawnPoint())
		{
			if (IsDMSpawn())
				return TEXT("DM Spawn");
			appSprintf(MiscStr, TEXT("SP Spawn(%i)"), INT(type));
			return MiscStr;
		}

		// find which type to spawn
		for (INT i = 0; i < NUMMOBJTYPES; i++)
			if (type == mobjinfo[i].doomednum)
				return EntTypeToStr(i);
		appSprintf(MiscStr, TEXT("<Undefined %i>"), INT(type));
		return MiscStr;
	}
	inline INT GetEntType() const
	{
		for (INT i = 0; i < NUMMOBJTYPES; i++)
			if (type == mobjinfo[i].doomednum)
				return i;
		return INDEX_NONE;
	}
	inline const TCHAR* GetOpts() const
	{
		static TCHAR OptsStr[1024];
		OptsStr[0] = 0;

		if (options & EOPTS_Easy)
			appStrcat(OptsStr, TEXT("<EASY>"));
		if (options & EOPTS_Medium)
			appStrcat(OptsStr, TEXT("<MEDIUM>"));
		if (options & EOPTS_Hard)
			appStrcat(OptsStr, TEXT("<HARD>"));
		if (options & EOPTS_Ambush)
			appStrcat(OptsStr, TEXT("<AMBUSH>"));
		if (options & EOPTS_Net)
			appStrcat(OptsStr, TEXT("<NET>"));
		return OptsStr;
	}
};

// A LineDef, as used for editing, and as input
// to the BSP builder.
constexpr INT LINEDEF_SIZE = (sizeof(short) * 7);

enum ESpecialLineType : DWORD
{
	SPLT_None = 0x00,
	SPLT_PassTrigger = (1 << 0),
	SPLT_ProjectileTrigger = (1 << 1),
	SPLT_SwitchTrigger = (1 << 2),
	SPLT_FloorMover = (1 << 3),
	SPLT_CeilMover = (1 << 4),
	SPLT_Light = (1 << 6),
	SPLT_Teleport = (1 << 7),
	SPLT_Exit = (1 << 8),
	SPLT_SecretExit = (1 << 9),
	SPLT_TriggerOnce = (1 << 10),
	SPLT_MonsterOnly = (1 << 11),
	SPLT_PlayerOnly = (1 << 12),
};

enum EMoverFlags : BYTE
{
	MFT_None,
	MFT_LowestCeiling, // lowest nearby ceiling.
	MFT_CloseToFloor, // floor height.
	MFT_CrushToFloor, // floor height + 8 units.
	MFT_HighestCeiling, // highest nearby ceiling.
	MFT_Stairs,
	MFT_NearestFloorUp, // up nearest floor.
	MFT_LowestFloor, // lowest floor.
	MFT_NearestFloorDown, // down nearest floor.
	MFT_HighestFloor, // highest nearby floor.
	MFT_CrushToCeiling, // ceiling height - 8 units.
};
enum EDoorMoveSpeed : BYTE
{
	DMS_NormalDoor,
	DMS_BlazingDoor,
	DMS_NormalPlatform,
	DMS_BlazingPlatform,
	DMS_NormalFloor,
};

enum ELockType : BYTE
{
	LOCKTYPE_None,
	LOCKTYPE_Blue,
	LOCKTYPE_Yellow,
	LOCKTYPE_Red,
};

struct FTriggerData
{
	DWORD TriggerFlags;
	EMoverFlags MoveType;
	EDoorMoveSpeed MoverSpeed;
	ELockType LockType;

	FTriggerData(DWORD TF, EMoverFlags MF, EDoorMoveSpeed MS = DMS_NormalDoor, ELockType LT = LOCKTYPE_None)
		: TriggerFlags(TF), MoveType(MF), MoverSpeed(MS), LockType(LT)
	{}
	inline UBOOL IsMover() const
	{
		return ((TriggerFlags & (SPLT_FloorMover | SPLT_CeilMover)) != 0);
	}
};

struct line_t
{
	SWORD v1,v2,flags,special,tag;
	// sidenum[1] will be -1 if one sided
	SWORD sidenum[2];

	FTriggerData* TriggerData;

	line_t(FArchive* Ar)
		: TriggerData(nullptr)
	{
		*Ar << v1 << v2 << flags << special << tag << sidenum[0] << sidenum[1];

		if (special)
			InitSpecial();
	}
	~line_t()
	{
		delete TriggerData;
	}

	void InitSpecial();

	inline UBOOL IsTwoSided() const
	{
		return (flags & ML_TWOSIDED) != 0;
	}
	inline UBOOL IsUnpegCeil() const
	{
		return (flags & ML_DONTPEGTOP) != 0;
	}
	inline UBOOL IsUnpegFloor() const
	{
		return (flags & ML_DONTPEGBOTTOM) != 0;
	}
	inline const TCHAR* GetFlags() const
	{
		static TCHAR FlagStr[1024];
		FlagStr[0] = 0;

		if (flags & ML_BLOCKING)
			appStrcat(FlagStr, TEXT("<BLOCK>"));
		if (flags & ML_BLOCKMONSTERS)
			appStrcat(FlagStr, TEXT("<BLOCKMONSTER>"));
		if (flags & ML_TWOSIDED)
			appStrcat(FlagStr, TEXT("<TWOSIDE>"));
		if (flags & ML_DONTPEGTOP)
			appStrcat(FlagStr, TEXT("<NOPEG TOP>"));
		if (flags & ML_DONTPEGBOTTOM)
			appStrcat(FlagStr, TEXT("<NOPEG BOTTOM>"));
		if (flags & ML_SECRET)
			appStrcat(FlagStr, TEXT("<SECRET>"));
		if (flags & ML_SOUNDBLOCK)
			appStrcat(FlagStr, TEXT("<BLOCKSOUND>"));
		if (flags & ML_DONTDRAW)
			appStrcat(FlagStr, TEXT("<NO AUTOMAP>"));
		return FlagStr;
	}
};

enum ESwitchTextureMode : BYTE
{
	SWITCHTEX_None,
	SWITCHTEX_Top,
	SWITCHTEX_Bottom,
	SWITCHTEX_Mid,
};

constexpr INT SIDEDEF_SIZE = ((sizeof(SWORD) * 3) + (sizeof(BYTE) * 8 * 3));
struct side_t
{
	SWORD		textureoffset;
	SWORD		rowoffset;
	FInstTexture *toptexture, *bottomtexture, *midtexture;
	SWORD		sector; // Front sector, towards viewer.

	// Switch texture handle
	FInstTexture*		SwitchTex;
	ESwitchTextureMode	SwitchIndex;

	side_t(FArchive* Ar)
		: SwitchTex(nullptr), SwitchIndex(SWITCHTEX_None)
	{
		*Ar << textureoffset << rowoffset;
		toptexture = FInstTexture::ReadTextureName(Ar);
		bottomtexture = FInstTexture::ReadTextureName(Ar);
		midtexture = FInstTexture::ReadTextureName(Ar);
		*Ar << sector;
	}
	inline FInstTexture* GetTopTex() const
	{
		if (SwitchIndex == SWITCHTEX_Top)
			return SwitchTex;
		return toptexture;
	}
	inline FInstTexture* GetMidTex() const
	{
		if (SwitchIndex == SWITCHTEX_Mid)
			return SwitchTex;
		return midtexture;
	}
	inline FInstTexture* GetBottomTex() const
	{
		if (SwitchIndex == SWITCHTEX_Bottom)
			return SwitchTex;
		return bottomtexture;
	}
	inline UBOOL HasTex(const FInstTexture* Txt) const
	{
		return (toptexture ==Txt) || (midtexture == Txt) || (bottomtexture == Txt) || (SwitchTex == Txt);
	}
};

// A single Vertex.
constexpr INT VERTEX_SIZE = (sizeof(SWORD) * 2);
struct vertex_t
{
	SWORD x, y;

	vertex_t(FArchive* Ar)
	{
		*Ar << x << y;
	}
	inline FVector GetVector() const
	{
		return FVector(x, y, 0.f);
	}
	inline FVector2D GetVector2D() const
	{
		return FVector2D(x, y);
	}
	inline FVector2D GetVector2DFlip() const
	{
		return FVector2D(x, -y);
	}
};

// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
constexpr INT SEGMENT_SIZE = (sizeof(SWORD) * 6);
struct mapseg_t
{
	SWORD v1,v2,angle,line,side,offset;

	line_t* linedef;
	side_t* sidedef;
	side_t* othersidedef;

	// Sector references.
	// Could be retrieved from linedef, too.
	// backsector is NULL for one sided lines
	sector_t* frontsector;
	sector_t* backsector;

	mapseg_t(FArchive* Ar)
		: linedef(nullptr), sidedef(nullptr), othersidedef(nullptr), frontsector(nullptr), backsector(nullptr)
	{
		*Ar << v1 << v2 << angle << line << side << offset;
	}

	INT GetFrontSector() const;
	INT GetBackSector() const;
};

// SubSector, as generated by BSP.
constexpr INT SSECTOR_SIZE = (sizeof(SWORD) * 2);
struct subsector_s
{
	SWORD		numsegs;
	// Index of first one, segs are stored sequentially.
	SWORD		firstseg;

	sector_t* sector;

	subsector_s(FArchive* Ar)
		: sector(nullptr)
	{
		*Ar << numsegs << firstseg;
	}
};

// BSP node structure.
// Indicate a leaf.
#define	NF_SUBSECTOR	0x8000
constexpr INT WNODE_SIZE = (sizeof(WORD) * 14);
struct node_t
{
	// Partition line from (x,y) to x+dx,y+dy)
	SWORD		x,y,dx,dy;

	// Bounding box for each child,
	// clip against view frustum.
	SWORD		bbox[2][4];

	// If NF_SUBSECTOR its a subsector,
	// else it's a node of another subtree.
	WORD		children[2];

	node_t(FArchive* Ar)
	{
		*Ar << x << y << dx << dy;
		Ar->Serialize(bbox, sizeof(node_t::bbox));
		*Ar << children[0] << children[1];
	}
	inline FVector2D GetDirNormal() const
	{
		FVector2D Dir(dx, dy);
		Dir.Normalize();
		return Dir;
	}

	inline INT R_PointOnSide(const FVector2D& Point) const
	{
		if (!dx)
		{
			if (Point.X <= x)
				return INT(dy > 0);
			return INT(dy < 0);
		}
		if (!dy)
		{
			if (Point.Y <= y)
				return int(dx < 0.f);
			return int(dx > 0.f);
		}

		FLOAT deltaX = (Point.X - FLOAT(x));
		FLOAT deltaY = (Point.Y - FLOAT(y));

		if ((deltaY * FLOAT(dx)) < (FLOAT(dy) * deltaX))
		{
			// front side
			return 0;
		}
		// back side
		return 1;
	}
};

struct FSectorShape;

// Sector definition, from editing.
constexpr INT SECTOR_SIZE = ((sizeof(SWORD) * 5) + (sizeof(BYTE) * 8 * 2));
struct sector_t
{
	SWORD		floorheight;
	SWORD		ceilingheight;
	FInstTexture *floorpic, *ceilingpic;
	//BYTE		floorpic[8];
	//BYTE		ceilingpic[8];
	SWORD		lightlevel;
	SWORD		special;
	SWORD		tag;

	FSectorShape* Shape;
	INT SectorIndex;
	UBOOL bTeleportSector;
	FTriggerData* TriggerData;

	sector_t(FArchive* Ar)
		: Shape(nullptr), bTeleportSector(FALSE), TriggerData(nullptr)
	{
		*Ar << floorheight << ceilingheight;
		floorpic = FInstTexture::ReadTextureName(Ar, TEXT("B_"));
		ceilingpic = FInstTexture::ReadTextureName(Ar, TEXT("B_"));
		*Ar << lightlevel << special << tag;
	}

	inline UBOOL IsSmallerThan(const sector_t& Other) const
	{
		return (floorheight >= Other.floorheight && ceilingheight <= Other.ceilingheight);
	}
	inline UBOOL IsSecretArea() const
	{
		return (special == 9);
	}
	inline INT GetDamagePerSec() const
	{
		switch (special)
		{
		case 5:	// HELLSLIME DAMAGE
			return 10;
		case 7: // NUKAGE DAMAGE
			return 5;
		case 16: // SUPER HELLSLIME DAMAGE
		case 4: // STROBE HURT
			return 20;
		case 11: // EXIT SUPER DAMAGE! (for E1M8 finale)
			return -1;
		default:
			return 0;
		}
	}
	inline const TCHAR* GetDamageName() const
	{
		switch (special)
		{
		case 5:	// HELLSLIME DAMAGE
		case 7: // NUKAGE DAMAGE
		case 16: // SUPER HELLSLIME DAMAGE
			return TEXT("Corroded");
		default:
			return TEXT("Burned");
		}
	}
	inline UBOOL CanMapSector() const
	{
		return (floorheight != ceilingheight) || (TriggerData != NULL);
	}
};

inline INT mapseg_t::GetFrontSector() const
{
	return frontsector ? frontsector->SectorIndex : INDEX_NONE;
}
inline INT mapseg_t::GetBackSector() const
{
	return backsector ? backsector->SectorIndex : INDEX_NONE;
}

struct FSimpleTesselator
{
	struct FOutputTris
	{
		TArray<INT> Verts;
	};
	TArray<FVector2D> Input;
	TArray<FOutputTris> Output;

	void Tesselate();
};
