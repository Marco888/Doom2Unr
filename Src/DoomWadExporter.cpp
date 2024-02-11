
#include "DoomWadPrivate.h"

IMPLEMENT_CLASS(UDoomWadExp);
IMPLEMENT_PACKAGE(DoomWadExp);

static const TCHAR SkyTexName[] = TEXT("ShaneDay.Daysky");
static FVector2D* GVertexList = NULL;

#define DEBUG_SECTOR_INDEX -1
#define DUMP_DEBUG_SHAPES (DEBUG_SECTOR_INDEX>=0)
#if DUMP_DEBUG_SHAPES
static UBOOL GDebugOutput = FALSE;
#define DEBUG_MSG(...) if(GDebugOutput) debugf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

void UDoomWadExp::StaticConstructor()
{
	guard(UDoomWadExp::StaticConstructor);
	HelpCmd = TEXT("doomWADexp");
	HelpOneLiner = TEXT("Convert DoomWad into Unreal packages");
	HelpUsage = TEXT("doomWADexp doom.wad");
	HelpParm[0] = TEXT("WAD file");
	HelpDesc[0] = TEXT("Filename of the WAD file");
	IsServer = FALSE;
	IsClient = FALSE;
	IsEditor = TRUE;
	LazyLoad = FALSE;
	unguard;
}

struct FWADHeaderContents
{
	static FWADHeaderContents* GContents;

	FArchive* FileAr;
	WADHeader Header;
	INT GameVersion, GameNumber;
	TArray<WADFileEntry> Lumps;
	FString TexturePackageName, AssetPackageName, ClassesDir, LevelsPrefix, WadFileName;

	FWADHeaderContents(FArchive* Ar, const TCHAR* WADName, const TCHAR* EntsPackage)
		: FileAr(Ar), Header(Ar), GameVersion(0), ClassesDir(EntsPackage)
	{
		GContents = this;
		FString Str = FString::GetFilenameOnlyStr(WADName);
		WadFileName = Str;
		if (Str == TEXT("doom2"))
			GameNumber = 1;
		else if (Str == TEXT("plutonia"))
			GameNumber = 2;
		else if (Str == TEXT("tnt"))
			GameNumber = 3;
		else GameNumber = 0;
		TexturePackageName = FString::Printf(TEXT("D_%ls_rc"), *Str);
		AssetPackageName = FString::Printf(TEXT("D_%ls_Assets"), *Str);
		LevelsPrefix = FString::Printf(TEXT("M_%ls_"), *Str);
	}
	inline void FlagTextures()
	{
		guard(FWADHeaderContents::FlagTextures);
		{
			TCHAR TexStr[16], EndTexStr[16], TempName[16];
			for (INT i = 0; animdefs[i].istexture >= 0; ++i)
			{
				appStrncpy(TempName, ByteToStr(animdefs[i].startname, 9), 16);
				FInstTexture* StartTex = FInstTexture::GTextureMap->FindRef(FName(TempName));
				if (!StartTex)
					appStrcpy(TexStr, TempName);
				if (!StartTex)
				{
					appSprintf(TexStr, TEXT("B_%ls"), TempName);
					StartTex = FInstTexture::GTextureMap->FindRef(FName(TexStr));
					if (!StartTex)
						continue;
				}

				appStrncpy(EndTexStr, ByteToStr(animdefs[i].endname, 9), 12);
				TCHAR* ChrStr = &TexStr[appStrlen(TexStr) - 1];
				const TCHAR TargetChr = EndTexStr[appStrlen(EndTexStr) - 1];

				for (INT j = 0; j < 4; ++j)
				{
					++(*ChrStr);
					FInstTexture* CurTex = FInstTexture::GTextureMap->FindRef(FName(TexStr));
					if (CurTex)
						CurTex->FirstFrame = StartTex;
					if (*ChrStr == TargetChr)
						break;
				}
			}
		}
		{
			static const TCHAR* WaterList[] = { TEXT("B_FWATER1") , TEXT("B_SWATER1"), TEXT("B_BLOOD1"), TEXT("B_NUKAGE1"), TEXT("B_SLIME01"), TEXT("B_SLIME05") };
			for (INT i = 0; i < ARRAY_COUNT(WaterList); ++i)
			{
				FInstTexture* Tex = FInstTexture::GTextureMap->FindRef(FName(WaterList[i]));
				if (Tex)
					Tex->bIsWater = TRUE;
			}
		}
		{
			TCHAR TexStr[16];

			for (INT i = 0;; ++i)
			{
				if (!*alphSwitchList[i].name1)
					break;
				appFromAnsiInPlace(TexStr, alphSwitchList[i].name1, 16);
				FInstTexture* TexA = FInstTexture::GTextureMap->FindRef(FName(TexStr));
				appFromAnsiInPlace(TexStr, alphSwitchList[i].name2, 16);
				FInstTexture* TexB = FInstTexture::GTextureMap->FindRef(FName(TexStr));

				if (TexA)
				{
					TexA->bIsSwitch = TRUE;
					TexA->FirstFrame = TexB;
				}
				if (TexB)
				{
					TexB->bIsSwitch = TRUE;
					TexB->FirstFrame = TexA;
				}
			}
		}
		unguard;
	}
	inline UBOOL IsValidHeader() const
	{
		return (!appStrcmp(Header.USignature, TEXT("IWAD")) || !appStrcmp(Header.USignature, TEXT("PWAD")));
	}
	inline UBOOL LoadLumps()
	{
		Lumps.SetSize(Header.numFiles);
		FileAr->Seek(Header.offFAT);
		for (INT iFile = 0; iFile < Header.numFiles; ++iFile)
		{
			Lumps(iFile).Load(FileAr);
			//debugf(TEXT(" [%i] '%ls' Offset %i Size %i"), iFile, Lumps(iFile).UName, Lumps(iFile).offData, Lumps(iFile).lenData);
			if (!appStrcmp(Lumps(iFile).UName, TEXT("MAP01")))
				GameVersion |= 2;
			if (!appStrcmp(Lumps(iFile).UName, TEXT("E1M1")))
				GameVersion |= 1;
		}
		return (GameVersion == 1 || GameVersion == 2);
	}
	inline INT FindLump(const TCHAR* Name) const
	{
		for (INT i = 0; i < Header.numFiles; ++i)
			if (!appStrcmp(Lumps(i).UName, Name))
				return i;
		return INDEX_NONE;
	}
	inline UBOOL IsEpisodic() const
	{
		return (GameVersion == 1);
	}
	inline const TCHAR* GetGameName() const
	{
		switch (GameNumber)
		{
		case 0:
			return TEXT("Doom");
		case 1:
			return TEXT("Doom 2: Hell on Earth");
		case 2:
			return TEXT("The Plutonia Experiment");
		default:
			return TEXT("TNT: Evilution");
		}
	}
};
FWADHeaderContents* FWADHeaderContents::GContents = NULL;

const TCHAR* FInstTexture::GetFileName() const
{
	switch (Type)
	{
	case TT_Unknown:
		return TEXT("Engine.DefaultTexture");
	case TT_Sky:
		return TEXT("ShaneDay.Daysky");
	case TT_Switch:
	{
		TCHAR* Result = appStaticString1024();
		appStrcpy(Result, TEXT("MyLevel."));
		appStrcat(Result, *TextureName);
		return Result;
	}
	default:
	{
		TCHAR* Result = appStaticString1024();
		appSprintf(Result, TEXT("%ls.%ls.%ls"), *FWADHeaderContents::GContents->TexturePackageName, (Type == TT_Flat) ? TEXT("Base") : TEXT("World"), GetSaferName(*((FirstFrame && !bIsSwitch) ? FirstFrame->TextureName : TextureName)));
		return Result;
	}
	}
}

struct FWallTextures
{
	enum EWallTexMode : BYTE
	{
		WALLTEX_Default,
		WALLTEX_TopTex,
		WALLTEX_BottomTex,
		WALLTEX_MidTex,
		WALLTEX_ForcedSky,
	};
private:
	const side_t* Primary, * Secondary;

	inline FInstTexture* GetBottom() const
	{
		if (Primary->SwitchIndex == SWITCHTEX_Bottom)
			return Primary->SwitchTex;
		FInstTexture* Result = Primary->bottomtexture;
		if (!Result->IsValid() && Secondary)
			Result = Secondary->bottomtexture;
		return Result;
	}
	inline FInstTexture* GetMid() const
	{
		if (Primary->SwitchIndex == SWITCHTEX_Mid)
			return Primary->SwitchTex;
		FInstTexture* Result = Primary->midtexture;
		if (!Result->IsValid() && Secondary)
			Result = Secondary->midtexture;
		return Result;
	}
	inline FInstTexture* GetTop() const
	{
		if (Primary->SwitchIndex == SWITCHTEX_Top)
			return Primary->SwitchTex;
		FInstTexture* Result = Primary->toptexture;
		if (!Result->IsValid() && Secondary)
			Result = Secondary->toptexture;
		return Result;
	}

public:
	UBOOL IsScrollingTex;

	FWallTextures() = delete;
	FWallTextures(const line_t& LineDef, const side_t* SideDef, const side_t* SideDefB)
		: Primary(SideDef), Secondary(SideDefB), IsScrollingTex(LineDef.special == 48)
	{}
	FWallTextures(const FWallTextures& Other)
		: Primary(Other.Primary), Secondary(Other.Secondary), IsScrollingTex(Other.IsScrollingTex)
	{}

	inline FInstTexture* GetTexture(const EWallTexMode Mode = WALLTEX_Default) const
	{
		switch (Mode)
		{
		case WALLTEX_Default:
		case WALLTEX_MidTex:
			return GetMid();
		case WALLTEX_TopTex:
			return GetTop();
		case WALLTEX_ForcedSky:
			return &FInstTexture::GSkyTexture;
		default:
			return GetBottom();
		}
	}
	DWORD GetTextureFlags(const EWallTexMode WantedMode = WALLTEX_Default) const
	{
		switch (WantedMode)
		{
		case WALLTEX_ForcedSky:
			return PF_FakeBackdrop;
		default:
			return (IsScrollingTex ? PF_AutoUPan : PF_None);
		}
	}
};

struct FMapInfo
{
	TArray<FThingEntry> Things;
	TArray<line_t> Lines;
	TArray<side_t> SideDefs;
	TArray<vertex_t> Vertices;
	TArray<mapseg_t> MapSegs;
	TArray<subsector_s> SubSectors;
	TArray<node_t> Nodes;
	TArray<sector_t> Sectors;
	TArray<BYTE> RejectMatrix;
	TArray<WORD> BlockMap;
	FString MapTitle, MapFilename;
	INT iMap, iEpisode;

private:
	SWORD LastFreeTag;

public:
	FMapInfo()
		: iMap(0), iEpisode(0), LastFreeTag(0)
	{}

	inline SWORD GetUniqueTag()
	{
		SWORD Result = LastFreeTag;
		UBOOL bOccupied = TRUE;
		const INT NumSectors = Sectors.Num();
		while (bOccupied)
		{
			++Result;
			bOccupied = FALSE;
			for (INT i = 0; i < NumSectors; ++i)
				if (Sectors(i).tag == Result)
				{
					bOccupied = TRUE;
					break;
				}
		}
		LastFreeTag = Result;
		return Result;
	}
	const TCHAR* GetSkyTexture(const FWADHeaderContents& C) const
	{
		INT episode = iEpisode;
		INT map = iMap;

		// This was quite messy with SPECIAL and commented parts.
		// Supposedly hacks to make the latest edition work.
		// It might not work properly.
		if (episode < 1)
			episode = 1;

		if (C.GameNumber==0) // Doom 1
		{
			if (episode > 4)
				episode = 4;
			if (map > 9)
				map = 9;
		}
		else // All other.
		{
			if (episode > 3)
				episode = 3;
		}
		if (map < 1)
			map = 1;

		// set the sky map for the episode
		if (C.GameNumber != 0)
		{
			if (map < 12)
				return TEXT("SKY1");
			else if (map < 21)
				return TEXT("SKY2");
			return TEXT("SKY3");
		}
		switch (episode)
		{
		case 1:
			return TEXT("SKY1");
			break;
		case 2:
			return TEXT("SKY2");
			break;
		case 3:
			return TEXT("SKY3");
			break;
		default:	// Special Edition sky
			return TEXT("SKY4");
			break;
		}
	}
	void PostLoadMap()
	{
		guard(PostLoadMap);
		INT i;
		for (i = 0; i < Sectors.Num(); ++i)
			Sectors(i).SectorIndex = i;

		for (i = 0; i < MapSegs.Num(); ++i)
		{
			mapseg_t& S = MapSegs(i);
			line_t& L = Lines(S.line);

			S.linedef = &L;
			S.sidedef = &SideDefs(L.sidenum[S.side]);
			S.frontsector = &Sectors(SideDefs(L.sidenum[S.side]).sector);
			if (L.flags & ML_TWOSIDED)
			{
				S.othersidedef = &SideDefs(L.sidenum[S.side ^ 1]);
				S.backsector = &Sectors(SideDefs(L.sidenum[S.side ^ 1]).sector);
			}
		}
		for (i = 0; i < SubSectors.Num(); i++)
		{
			subsector_s& SS = SubSectors(i);
			const mapseg_t& S = MapSegs(SS.firstseg);
			const line_t& L = Lines(S.line);
			SS.sector = &Sectors(SideDefs(L.sidenum[S.side]).sector);
		}
		unguard;
	}

	inline UBOOL IsValid() const
	{
		return (Things.Num() && BlockMap.Num() && Lines.Num() && MapSegs.Num() && Nodes.Num() && RejectMatrix.Num() && Sectors.Num() && SideDefs.Num());
	}

	sector_t* GetPointSector(const FVector2D& Point) const
	{
		INT iNode = Nodes.Num() - 1;
		INT side, nextchild;
		for (;;)
		{
			const node_t& n = Nodes(iNode);
			side = n.R_PointOnSide(Point);
			nextchild = n.children[side];

			if (nextchild & NF_SUBSECTOR)
			{
				const subsector_s& SS = SubSectors(nextchild & ~NF_SUBSECTOR);
				return SS.sector;
			}
			if (iNode == nextchild)
				break;
			iNode = nextchild;
		}
		return nullptr;
	}
};

inline const TCHAR* DoomLocToStrRZ(const INT X, const INT Y, const FLOAT Z)
{
	TCHAR* Result = appStaticString1024();
	const FVector Pos = FVector(X * SCALE_FACTOR, Y * -SCALE_FACTOR, Z);
	appSprintf(Result, TEXT("X=%f,Y=%f,Z=%f"), Pos.X, Pos.Y, Pos.Z);
	return Result;
}
inline const TCHAR* DoomLocToStr(const INT X, const INT Y, const INT Z = 0)
{
	TCHAR* Result = appStaticString1024();
	const FVector Pos = FVector(X, -Y, Z) * SCALE_FACTOR;
	appSprintf(Result, TEXT("X=%f,Y=%f,Z=%f"), Pos.X, Pos.Y, Pos.Z);
	return Result;
}
inline const TCHAR* Doom2DLocToStr(const INT X, const INT Y)
{
	TCHAR* Result = appStaticString1024();
	appSprintf(Result, TEXT("X=%i,Y=%i"), X, -Y);
	return Result;
}
inline const TCHAR* Doom2DNormalToStr(const FVector2D& V)
{
	TCHAR* Result = appStaticString1024();
	appSprintf(Result, TEXT("X=%f,Y=%f,Z=0"), V.X, -V.Y);
	return Result;
}

inline FVector2D GetFwdDir(const FVector2D& A, const FVector2D& B)
{
	return (B - A).SafeNormal();
}

static UBOOL SurfIsInverse(const TArray<FVector2D>& Verts)
{
	const FVector2D BaseDir = GetFwdDir(Verts(0), Verts.Last());
	FVector2D PrevDir = BaseDir;
	FLOAT TotalCross = 0.f;
	for (INT i = 1; i < Verts.Num(); ++i)
	{
		const FVector2D Dir = GetFwdDir(Verts(i), Verts(i - 1));
		TotalCross += Dir ^ PrevDir;
		PrevDir = Dir;
	}
	TotalCross += BaseDir ^ PrevDir;
	return (TotalCross < 0.f);
}
static UBOOL SurfIsConvex(const TArray<FVector2D>& Verts)
{
	SurfIsInverse(Verts);
	const FVector2D BaseDir = GetFwdDir(Verts(0), Verts.Last());
	FVector2D PrevDir = BaseDir;
	for (INT i = 1; i < Verts.Num(); ++i)
	{
		const FVector2D Dir = GetFwdDir(Verts(i), Verts(i - 1));
		if ((Dir ^ PrevDir) < -SLERP_DELTA)
			return FALSE;
		PrevDir = Dir;
	}
	if ((BaseDir ^ PrevDir) < -SLERP_DELTA)
		return FALSE;
	return TRUE;
}

inline DWORD GetTypeHash(const sector_t* s)
{
	return reinterpret_cast<DWORD>(s);
}

static DWORD TagCounter = 1;
constexpr INT MaxNextRefs = 5;

struct FWallLine
{
	INT Start, End, NumNext, RefDests;
	INT WallAngle;
	sector_t* OtherSector;
	mapseg_t* seq;
	FWallLine* NextPiece[MaxNextRefs];

	DWORD Tag{};

	FWallLine(INT S, INT E, INT Ang, sector_t* OT, mapseg_t* sq)
		: Start(S), End(E), NumNext(0), RefDests(0), WallAngle(Ang & 0xFFFF), OtherSector(OT), seq(sq)
	{
		appMemzero(NextPiece, sizeof(NextPiece));
	}
	inline void AddEnd(FWallLine* L)
	{
		if (NumNext == MaxNextRefs)
			return;
		NextPiece[NumNext++] = L;
		++L->RefDests;
	}
	inline FWallLine* GetNext() const
	{
		return NextPiece[0];
	}
	inline void Reverse()
	{
		Exchange(Start, End);
		WallAngle = (0xFFFF - WallAngle);
	}

	inline INT CheckCircularRef()
	{
		++TagCounter;
		return HasRefToInner(this);
	}
	inline void SetSingleLink(FWallLine* Dest, TArray<FWallLine*>& ClearArray)
	{
		++TagCounter;
		SetSingleLinkInner(Dest, ClearArray);
	}
	inline UBOOL WallMatches(const FWallLine& Other) const
	{
		if ((WallAngle != Other.WallAngle) || OtherSector != Other.OtherSector)
			return FALSE;
		if (OtherSector && seq->sidedef->bottomtexture != Other.seq->sidedef->bottomtexture)
			return FALSE;
		return (seq->sidedef->midtexture == Other.seq->sidedef->midtexture);
	}
private:
	INT HasRefToInner(FWallLine* Other)
	{
		if (Tag == TagCounter)
			return 0;
		Tag = TagCounter;
		for (INT i = 0; i < NumNext; ++i)
		{
			if (NextPiece[i] == Other)
				return 1;
			INT Result = NextPiece[i]->HasRefToInner(Other);
			if (Result)
				return Result + 1;
		}
		return 0;
	}
	UBOOL SetSingleLinkInner(FWallLine* Dest, TArray<FWallLine*>& ClearArray)
	{
		if (Tag == TagCounter)
			return FALSE;
		Tag = TagCounter;
		for (INT i = 0; i < NumNext; ++i)
			if (NextPiece[i] == Dest || NextPiece[i]->SetSingleLinkInner(Dest, ClearArray))
			{
				ClearArray.RemoveItem(this);
				for (INT j = 0; j < NumNext; ++j)
				{
					if (i != j)
						--NextPiece[j]->RefDests;
				}
				if (i)
					NextPiece[0] = NextPiece[i];
				NumNext = 1;
				return TRUE;
			}
		return FALSE;
	}
};

enum ESectorSolidStatus : DWORD
{
	SECSTATUS_Unverifier,
	SECSTATUS_NonSolid,
	SECSTATUS_Solid,
};

struct FSectorShape
{
	friend struct FLineConnector;
public:
	INT NumLines, NumFloorLines;
	FWallLine** Lines;
	INT* FloorLines;
	const sector_t& sector;
	FVector2D BoundsMin, BoundsMax;
	INT SortScore;
	INT FloorHeight, CeilingHeight;
	FSectorShape* LinkedShape;
	ESectorSolidStatus SolidStatus;
	BITFIELD IsConvex : 1;
	BITFIELD IsCCW : 1;
	BITFIELD HasBaked : 1;
	BITFIELD CanSkipBSP : 1;
	BITFIELD CreateTopBottom : 1;

private:
	FSectorShape(const INT nLines, sector_t* sec)
		: NumLines(nLines), NumFloorLines(0), Lines(New<FWallLine*>(GMem, nLines)), FloorLines(nullptr), sector(*sec), FloorHeight(sec->floorheight), CeilingHeight(sec->ceilingheight), LinkedShape(nullptr), SolidStatus(SECSTATUS_Unverifier)
		, IsConvex(TRUE), IsCCW(FALSE), HasBaked(FALSE), CanSkipBSP(FALSE), CreateTopBottom(FALSE)
	{
	}
	inline void MergeLines()
	{
		guard(MergeLines);
		FWallLine* PrevLine = Lines[0];
		INT NumRemoved = 0;
		INT iSetOffset = 1;
		for (INT i = 1; i < NumLines; ++i, ++iSetOffset)
		{
			if (iSetOffset != i)
				Lines[iSetOffset] = Lines[i];
			if (PrevLine->WallMatches(*Lines[i]))
			{
				PrevLine->End = Lines[i]->End;
				++NumRemoved;
				--iSetOffset;
				continue;
			}
			PrevLine = Lines[i];
		}
		NumLines -= NumRemoved;
		unguard;
	}
	inline void CheckFlags()
	{
		guard(CheckFlags);
		const FVector2D BaseDir = GetFwdDir(GVertexList[Lines[0]->Start], GVertexList[Lines[NumLines - 1]->Start]);
		FVector2D PrevDir = BaseDir;
		FLOAT TotalCross = 0.f;
		FLOAT ResultCross;
		UBOOL AnyCW = FALSE;
		UBOOL AnyCCW = FALSE;
		INT i;
		for (i = 1; i < NumLines; ++i)
		{
			const FVector2D Dir = GetFwdDir(GVertexList[Lines[i]->Start], GVertexList[Lines[i - 1]->Start]);
			ResultCross = Dir ^ PrevDir;
			if (ResultCross < -SLERP_DELTA)
				AnyCCW = TRUE;
			else if (ResultCross > SLERP_DELTA)
				AnyCW = TRUE;
			TotalCross += ResultCross;
			PrevDir = Dir;
		}
		ResultCross = BaseDir ^ PrevDir;
		if (ResultCross < -SLERP_DELTA)
			AnyCCW = TRUE;
		else if (ResultCross > SLERP_DELTA)
			AnyCW = TRUE;
		TotalCross += ResultCross;

		if (TotalCross > 0.f)
		{
			IsCCW = TRUE;
			IsConvex = !AnyCCW;
		}
		else
		{
			IsCCW = FALSE;
			IsConvex = !AnyCW;
		}
		unguard;
	}
	inline void CreateFloorPlan() // Strip out all straight connector lines.
	{
		guard(CreateFloorPlan);
#if DUMP_DEBUG_SHAPES
		if (GDebugOutput)
		{
			debugf(TEXT("CreateFloorPlan %i lines"), NumLines);
			for (INT i = 0; i < NumLines; ++i)
				debugf(TEXT(" V[%i] Verts %i %i Ang %i"), i, Lines[i]->Start, Lines[i]->End, Lines[i]->WallAngle);
		}
#endif

		TArray<INT> flLines;
		flLines.Reserve(NumLines);
		INT i;
		INT PrevDir = Lines[0]->WallAngle;
		for (i = 1; i < NumLines; ++i)
		{
			if (PrevDir != Lines[i]->WallAngle)
				break;
		}
		if (i == NumLines) // Single line?
		{
			debugf(NAME_Warning, TEXT("Zero area shape!"));
		}
		else
		{
			const INT iStart = i;
			flLines.AddItem(iStart);
			PrevDir = Lines[iStart]->WallAngle;
			for (i = (iStart + 1); ; ++i)
			{
				if (i == NumLines)
					i = 0;
				if (i == iStart)
					break;
				if (PrevDir != Lines[i]->WallAngle)
				{
					PrevDir = Lines[i]->WallAngle;
					flLines.AddItem(i);
				}
			}
			for (i = 0; i < flLines.Num(); ++i)
				flLines(i) = Lines[flLines(i)]->Start;
		}

#if DUMP_DEBUG_SHAPES
		if (GDebugOutput)
		{
			debugf(TEXT("Created %i lines"), flLines.Num());
			for (i = 0; i < flLines.Num(); ++i)
				debugf(TEXT(" P[%i] %i"), i, flLines(i));
		}
#endif

		NumFloorLines = flLines.Num();
		if (NumFloorLines)
		{
			FloorLines = New<INT>(GMem, NumFloorLines);
			appMemcpy(FloorLines, &flLines(0), sizeof(INT) * NumFloorLines);
		}
		unguard;
	}
	inline void BuildBounds()
	{
		guard(BuildBounds);
		BoundsMin = BoundsMax = GVertexList[Lines[0]->Start];
		for (INT i = 1; i < NumLines; ++i)
		{
			BoundsMin.X = Min(BoundsMin.X, GVertexList[Lines[i]->Start].X);
			BoundsMin.Y = Min(BoundsMin.Y, GVertexList[Lines[i]->Start].Y);
			BoundsMax.X = Max(BoundsMax.X, GVertexList[Lines[i]->Start].X);
			BoundsMax.Y = Max(BoundsMax.Y, GVertexList[Lines[i]->Start].Y);
		}
		SortScore = appCeil((BoundsMax.X - BoundsMin.X) * (BoundsMax.Y - BoundsMin.Y));
		if (IsCCW)
			SortScore += 5;
		unguard;
	}
	void CalcVerts()
	{
		guard(FLineSegment::CalcVerts);
		MergeLines();
		CheckFlags();
		CreateFloorPlan();
		BuildBounds();
		if (!IsCCW)
			ReverseOrder();
		unguard;
	}

	// Check if is valid and should keep it around.
	inline UBOOL IsValidShape() const
	{
		return (NumLines >= 3);
	}

	// Merge this shape with another.
	void MergeWith(const FSectorShape& Other)
	{
		guard(FLineSegment::MergeWith);
		{
			FWallLine** NewLines = New<FWallLine*>(GMem, NumLines + Other.NumLines);
			appMemcpy(&NewLines[0], Lines, sizeof(void*) * NumLines);
			appMemcpy(&NewLines[NumLines], Other.Lines, sizeof(void*) * Other.NumLines);
			Lines = NewLines;
			NumLines += Other.NumLines;
		}
		if (NumFloorLines && Other.NumFloorLines)
		{
			INT* NewFlLines = New<INT>(GMem, NumFloorLines + Other.NumFloorLines + 2);
			// Find 2 closest points between these 2 shapes and add merging line there.
			INT i, j;
			INT BestPts[2] = { INDEX_NONE,INDEX_NONE };
			FLOAT Score, BestScore;
			for (i = 0; i < NumFloorLines; ++i)
			{
				const FVector2D& A = GVertexList[FloorLines[i]];
				for (j = 0; j < Other.NumFloorLines; ++j)
				{
					Score = A.DistSquared(GVertexList[Other.FloorLines[j]]);
					if (BestPts[0] == INDEX_NONE || Score < BestScore)
					{
						BestPts[0] = i;
						BestPts[1] = j;
						BestScore = Score;
					}
				}
			}
			INT* t = NewFlLines;
			*t++ = FloorLines[BestPts[0]];
			for (i = BestPts[0] + 1; ; ++i)
			{
				if (i == NumFloorLines)
					i = 0;
				if (i == BestPts[0])
					break;
				*t++ = FloorLines[i];
			}
			*t++ = FloorLines[BestPts[0]];
			*t++ = Other.FloorLines[BestPts[1]];
			for (i = BestPts[1] + 1; ; ++i)
			{
				if (i == Other.NumFloorLines)
					i = 0;
				if (i == BestPts[1])
					break;
				*t++ = Other.FloorLines[i];
			}
			*t++ = Other.FloorLines[BestPts[1]];
			
			FloorLines = NewFlLines;
			NumFloorLines += (Other.NumFloorLines + 2);
		}
		unguard;
	}

public:
	// Check if inner segment has contact with any other sector (or is this a pillar?).
	inline UBOOL HasOtherInnerSectors(const sector_t* TestSector) const
	{
		guardSlow(FLineSegment::HasOtherInnerSectors);
		for (INT i = 0; i < NumLines; ++i)
			if (Lines[i]->OtherSector != TestSector)
				return FALSE;
		return TRUE;
		unguardSlow;
	}

	// Test if this enclosed space has multiple sectors connecting to it.
	inline UBOOL HasSingularInnerSector() const
	{
		guardSlow(FLineSegment::HasSingularInnerSector);
		const sector_t* TestSector = Lines[0]->OtherSector;
		for (INT i = 1; i < NumLines; ++i)
			if (Lines[i]->OtherSector != TestSector)
				return FALSE;
		return TRUE;
		unguardSlow;
	}

	inline const TCHAR* GetConnectingSectors() const
	{
		guardSlow(FLineSegment::GetConnectingSectors);
		TSingleMap<const sector_t*> Mapped;
		TCHAR* Result = appStaticString1024();
		TCHAR TmpStr[8];
		INT num = 0;
		for (INT i = 0; i < NumLines; ++i)
			if (Lines[i]->OtherSector && Mapped.Set(Lines[i]->OtherSector))
			{
				if (!num)
					appSprintf(Result, TEXT("%i"), Lines[i]->OtherSector->SectorIndex);
				else if (num > 5)
				{
					appStrcat(Result, TEXT("..."));
					break;
				}
				else
				{
					appSnprintf(TmpStr, 8, TEXT(",%i"), Lines[i]->OtherSector->SectorIndex);
					appStrcat(Result, TmpStr);
				}
				num++;
			}
		if (!num)
			appStrcpy(Result, TEXT("-"));
		return Result;
		unguardSlow;
	}

	// Get highest neighbouring ceiling.
	inline SWORD GetTopCeiling() const
	{
		guardSlow(FLineSegment::GetTopCeiling);
		SWORD Result = sector.ceilingheight;
		for (INT i = 0; i < NumLines; ++i)
			if (Lines[i]->OtherSector)
				Result = Max(Result, Lines[i]->OtherSector->ceilingheight);
		return Result;
		unguardSlow;
	}

	// Get lowest neighbouring floor.
	inline SWORD GetBottomFloor() const
	{
		guardSlow(FLineSegment::GetBottomFloor);
		SWORD Result = sector.floorheight;
		for (INT i = 0; i < NumLines; ++i)
			if (Lines[i]->OtherSector)
				Result = Min(Result, Lines[i]->OtherSector->floorheight);
		return Result;
		unguardSlow;
	}

	// This shape fits inside another shape.
	inline UBOOL IsInsideSegment(const FSectorShape& Other) const
	{
		return (BoundsMin.X >= Other.BoundsMin.X) && (BoundsMin.Y >= Other.BoundsMin.Y) && (BoundsMax.X <= Other.BoundsMax.X) && (BoundsMax.Y <= Other.BoundsMax.Y);
	}

	// Shape is valid.
	inline UBOOL IsValid() const
	{
		return (NumLines > 2);
	}

	// Check if this sector is fully solid.
	inline UBOOL SectorIsSolid()
	{
		if (SolidStatus == SECSTATUS_Unverifier)
		{
			if (FloorHeight == CeilingHeight || IsCCW)
				SolidStatus = SECSTATUS_NonSolid;
			else
			{
				SolidStatus = SECSTATUS_Solid;
				for (INT i = 0; i < NumLines; ++i)
				{
					if (Lines[i]->OtherSector != NULL && (Lines[i]->seq->linedef->flags & (ML_TWOSIDED | ML_BLOCKING)) != (ML_TWOSIDED | ML_BLOCKING))
					{
						SolidStatus = SECSTATUS_NonSolid;
						break;
					}
				}
			}
		}
		return (SolidStatus == SECSTATUS_Solid);
	}

	// Shape can be merged with outer.
	inline UBOOL IsIdentical(const FSectorShape& Other) const
	{
		return (FloorHeight == Other.FloorHeight) && (CeilingHeight == Other.CeilingHeight) && (sector.floorpic == Other.sector.floorpic) && (sector.ceilingpic == Other.sector.ceilingpic);
	}

	// Dump shape as 2D shape editor file.
	void Dump2DShape(FOutputDevice& Out, const FVector& Offset)
	{
		Out.Log(TEXT("BEGIN Model\r\n"));
		Out.Log(TEXT("\tOrigin=(0,0)\r\n"));
		Out.Log(TEXT("\tCamera=(0,0)\r\n"));
		Out.Log(TEXT("\tZoom=1\r\n"));
		Out.Log(TEXT("\tGridSize=16\r\n"));
		Out.Log(TEXT("\tBEGIN Shape\r\n"));
		INT Dir = (IsCCW != 0) ? -1 : 1;
		INT iPrev = -Dir;
		if (iPrev < 0)
			iPrev = NumFloorLines - 1;
		INT XOffset = appRound(Offset.X);
		INT YOffset = appRound(Offset.Y);
		INT j = 0;
		for (INT i = 0; i < NumFloorLines; ++i)
		{
			const FVector2D& A = GVertexList[FloorLines[j]];
			const FVector2D& B = GVertexList[FloorLines[iPrev]];
			Out.Log(TEXT("\t\tBEGIN Segment\r\n"));
			Out.Logf(TEXT("\t\t\tVertex1=(%i,%i)\r\n"), INT(A.X) - XOffset, INT(-A.Y) + YOffset);
			Out.Logf(TEXT("\t\t\tVertex2=(%i,%i)\r\n"), INT(B.X) - XOffset, INT(-B.Y) + YOffset);
			Out.Log(TEXT("\t\t\tControlPoint1=(0,0)\r\n"));
			Out.Log(TEXT("\t\t\tControlPoint2=(0,0)\r\n"));
			Out.Log(TEXT("\t\t\tType=0\r\n"));
			Out.Log(TEXT("\t\t\tDetail=10\r\n"));
			Out.Log(TEXT("\t\tEND Segment\r\n"));
			iPrev = j;
			j += Dir;
			if (j < 0)
				j = NumFloorLines - 1;
		}
		Out.Log(TEXT("\tEND Shape\r\n"));
		Out.Log(TEXT("END Model\r\n"));
	}

	// Find a co-shape of this sector that shares edges with this shape.
	inline FSectorShape* GetNeighbouringShape(const FSectorShape& Other)
	{
		guardSlow(FLineSegment::GetNeighbouringShape);
		if (!LinkedShape)
			return this;
		FSectorShape* BestShape = nullptr;
		INT i, Score, BestScore;
		const auto* OtherSide = &Other.sector;

		for (FSectorShape* S = this; S; S = S->LinkedShape)
		{
			Score = Max<INT>(2 - Abs<INT>(S->NumLines - Other.NumLines), 0);
			for (i = 0; i < S->NumLines; ++i)
				if (S->Lines[i]->OtherSector == OtherSide)
					++Score;
			if (!BestShape || Score > BestScore)
			{
				BestShape = S;
				BestScore = Score;
			}
		}
		return BestShape;
		unguardSlow;
	}

	// Reverse shape order.
	void ReverseOrder()
	{
		guardSlow(FLineSegment::ReverseOrder);
		INT i;
		for (i = 0; i < NumLines; ++i)
			Lines[i]->Reverse();
		if (NumFloorLines)
		{
			INT j;
			for (i = 0, j = (NumFloorLines - 1); i < j; ++i, --j)
				Exchange(FloorLines[i], FloorLines[j]);
		}
		unguardSlow;
	}
};

struct FLineConnector : private FScopedMemMark
{
private:
	TArray<FWallLine*> LineList;

public:
	TArray<FSectorShape*> ShapeList;

	FLineConnector(const vertex_t* verts, const INT NumVerts)
		: FScopedMemMark(GMem)
	{
		GVertexList = New<FVector2D>(GMem, NumVerts);
		for (INT i = 0; i < NumVerts; ++i)
			GVertexList[i] = verts[i].GetVector2DFlip();
	}
	~FLineConnector()
	{
		GVertexList = nullptr;
	}

	inline void AddLineSegment(mapseg_t& sq, sector_t* OtherSideSector, const UBOOL bReverse = FALSE)
	{
		guardSlow(FLineConnector::AddLineSegment);
		FWallLine* Line = new (GMem) FWallLine(sq.v1, sq.v2, sq.angle, OtherSideSector, &sq);
		if (bReverse)
			Line->Reverse();
		LineList.AddItem(Line);
		unguardSlow;
	}
	void FinishSector(sector_t* sec)
	{
		guard(FLineConnector::FinishSector);
		if (!LineList.Num())
			return;

		INT i, j;
		FWallLine* T;
		for (i = 0; i < LineList.Num(); ++i)
		{
			T = LineList(i);
			for (j = 0; j < LineList.Num(); ++j)
			{
				if ((i != j) && T->End == LineList(j)->Start)
					T->AddEnd(LineList(j));
			}
		}

		TArray<FSectorShape*> NewShapes;
		for (i = 0; i < LineList.Num(); ++i)
		{
			T = LineList(i);
			INT NumRefs = T->CheckCircularRef();
			if (NumRefs > 2)
			{
				T->SetSingleLink(T, LineList);
				i = INDEX_NONE;

				FWallLine* First = T;
				INT ListSize = 1;
				for (T = First->GetNext(); T != First; T = T->GetNext())
					++ListSize;

				FSectorShape* seq = new (GMem) FSectorShape(ListSize, sec);
				seq->Lines[0] = First;
				ListSize = 0;
				for (T = First->GetNext(); T != First; T = T->GetNext())
					seq->Lines[++ListSize] = T;
				seq->CalcVerts();

				if (seq->IsValidShape())
					NewShapes.AddItem(seq);
			}
		}
		LineList.EmptyNoRealloc();

		// Must merge mover into one.
		if (NewShapes.Num() > 0 && sec->TriggerData && sec->TriggerData->IsMover())
		{
			for (i = 0; i < NewShapes.Num(); ++i)
			{
				if (!NewShapes(i)->IsCCW)
					continue;
				FSectorShape* sh = NewShapes(i);
				for(j=0; j< NewShapes.Num(); ++j)
					if (!NewShapes(j)->IsCCW && sh->IsInsideSegment(*NewShapes(j)))
					{
						sh->ReverseOrder();
						NewShapes(j)->MergeWith(*sh);
						break;
					}
				NewShapes.Remove(i--, 1);
			}
		}

		for (i = 0; i < NewShapes.Num(); ++i)
		{
			NewShapes(i)->LinkedShape = sec->Shape;
			sec->Shape = NewShapes(i);
			ShapeList.AddItem(NewShapes(i));
		}
		unguard;
	}
	void SortShapes();
};

inline INT Compare(const FSectorShape* A, const FSectorShape* B)
{
	return B->SortScore - A->SortScore;
}
void FLineConnector::SortShapes()
{
	guard(FLineConnector::SortShapes);
	// Sort by biggest shapes first, then smaller shapes last.
	// This ensures correct brush subtraction order.
	Sort(ShapeList);
	unguard;
}

struct FBSPOutput
{
	enum ECSGWriteMode : BYTE
	{
		CSG_Volume,
		CSG_Mover,
		CSG_Additive,
		CSG_Subtractive,
		CSG_SemiSolid,
		CSG_SemiSolidTop,
		CSG_SemiSolidBtm,
	};
	static inline const TCHAR* GetCSGWriteStr(const ECSGWriteMode M)
	{
		switch (M)
		{
		case CSG_Volume:
			return TEXT("Volume");
		case CSG_Mover:
			return TEXT("Mover");
		case CSG_Additive:
			return TEXT("Add");
		case CSG_Subtractive:
			return TEXT("Sub");
		case CSG_SemiSolid:
			return TEXT("Semi");
		case CSG_SemiSolidTop:
			return TEXT("SemiTop");
		case CSG_SemiSolidBtm:
			return TEXT("SemiBtm");
		default:
			return TEXT("<INVALID>");
		}
	}
	enum ECSGOutputDest : BYTE
	{
		CSGW_Brush,
		CSGW_Mover,
		CSGW_Top,
		CSGW_Bottom,
	};
	struct FBrushOutput
	{
		FStringOutputDevice Out;
		FInstTexture* FloorTex, *CeilTex;
		FLOAT FloorHeight, CeilingHeight;
		FVector BasePosition;
		BITFIELD bStandardWalls : 1, bAllowSplitter : 1, bFlipTextureU : 1;

		inline void BeginPolygon(FInstTexture* Texture, const DWORD PolyFlags, const FVector& Origin, const FVector& Normal, const FVector& TexU, const FVector& TexV)
		{
			guard(FBrushOutput::BeginPolygon);
			if (bWriteSurfs)
			{
				if (bWriteCSG && Texture->Type == TT_Sky)
					Out.Logf(TEXT("\t\t\t\tBegin Polygon Texture=%ls Flags=%u\r\n"), Texture->GetFileName(), PF_FakeBackdrop);
				else Out.Logf(TEXT("\t\t\t\tBegin Polygon Texture=%ls Flags=%u\r\n"), Texture->GetFileName(), PolyFlags);
			}
			else Out.Log(TEXT("\t\t\t\tBegin Polygon\r\n"));

			Out.Logf(TEXT("\t\t\t\t\tOrigin    %f,%f,%f\r\n"), Origin.X * SCALE_FACTOR, Origin.Y * SCALE_FACTOR, Origin.Z * SCALE_FACTOR);
			Out.Logf(TEXT("\t\t\t\t\tNormal    %f,%f,%f\r\n"), Normal.X, Normal.Y, Normal.Z);
			Out.Logf(TEXT("\t\t\t\t\tTextureU  %f,%f,%f\r\n"), TexU.X / SCALE_FACTOR, TexU.Y / SCALE_FACTOR, TexU.Z / SCALE_FACTOR);
			Out.Logf(TEXT("\t\t\t\t\tTextureV  %f,%f,%f\r\n"), TexV.X / SCALE_FACTOR, TexV.Y / SCALE_FACTOR, TexV.Z / SCALE_FACTOR);
			Out.Log(TEXT("\t\t\t\t\tPan       U=0 V=0\r\n"));
			unguard;
		}
		inline void WriteVertex(const FVector& V)
		{
			Out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), V.X * SCALE_FACTOR, V.Y * SCALE_FACTOR, V.Z * SCALE_FACTOR);
		}
		inline void EndPolygon()
		{
			Out.Log(TEXT("\t\t\t\tEnd Polygon\r\n"));
		}
		inline void SetBounds(const FLOAT FlrHeight, const FLOAT CeilHeight, const FLOAT HeightMod = 0.f)
		{
			FloorHeight = FlrHeight;
			CeilingHeight = CeilHeight;
			BasePosition.Z = (FlrHeight + CeilHeight) * 0.5f + HeightMod;
		}
		inline void AddSide(const FVector2D& PointA, const FVector2D& PointB, const FWallTextures& WallTex, const FWallTextures::EWallTexMode DesiredTex, const side_t& SideDef, FLOAT AlignZ)
		{
			guard(FBrushOutput::AddSide);
			FInstTexture* Tex = WallTex.GetTexture(DesiredTex);
			const FVector A = FVector(PointA, CeilingHeight) - BasePosition;
			const FVector B = FVector(PointB, CeilingHeight) - BasePosition;
			const FVector C = FVector(PointB, FloorHeight) - BasePosition;
			const FVector D = FVector(PointA, FloorHeight) - BasePosition;
			const FVector Normal = ((A - B) ^ FVector(0.f, 0.f, 1.f)).SafeNormalSlow();
			const FVector TexU = (bFlipTextureU ? (B - A) : (A - B)).SafeNormalSlow() * Tex->ScaleU;
			const FVector TexV = FVector(0.f, 0.f, -Tex->ScaleV);
			const FVector TexOrigin = FVector(bFlipTextureU ? PointA : PointB, AlignZ) - BasePosition
				- TexU * FLOAT(SideDef.textureoffset)
				- TexV * FLOAT(SideDef.rowoffset);
			BeginPolygon(Tex, WallTex.GetTextureFlags(DesiredTex), TexOrigin, Normal, TexU, TexV);
			WriteVertex(A);
			WriteVertex(B);
			WriteVertex(C);
			WriteVertex(D);
			EndPolygon();
			unguard;
		}
		inline void AddSideSplitted(const FVector2D& PointA, const FVector2D& PointB, const FLOAT SplitHeight, const FWallTextures& WallTex, const side_t& SideDef, FLOAT AlignZTop, FLOAT AlignZBtm)
		{
			guard(FBrushOutput::AddSideSplitted);
			if (!bAllowSplitter)
			{
				AddSide(PointA, PointB, WallTex, FWallTextures::EWallTexMode::WALLTEX_Default, SideDef, AlignZBtm);
				return;
			}

			const FVector A = FVector(PointA, CeilingHeight) - BasePosition;
			const FVector B = FVector(PointB, CeilingHeight) - BasePosition;
			const FVector C = FVector(PointB, FloorHeight) - BasePosition;
			const FVector D = FVector(PointA, FloorHeight) - BasePosition;
			const FVector SplitA = FVector(PointA, SplitHeight) - BasePosition;
			const FVector SplitB = FVector(PointB, SplitHeight) - BasePosition;
			const FVector Normal = ((A - B) ^ FVector(0.f, 0.f, 1.f)).SafeNormalSlow();
			const FVector bTexU = (bFlipTextureU ? (B - A) : (A - B)).SafeNormalSlow();
			const FVector bTexV = FVector(0.f, 0.f, -1.f);

			{
				FInstTexture* Tex = WallTex.GetTexture(FWallTextures::EWallTexMode::WALLTEX_BottomTex);
				const FVector TexU = bTexU * Tex->ScaleU;
				const FVector TexV = bTexV * Tex->ScaleV;
				const FVector TexOrigin = FVector(bFlipTextureU ? PointA : PointB, AlignZBtm) - BasePosition
					- TexU * FLOAT(SideDef.textureoffset)
					- TexV * FLOAT(SideDef.rowoffset);
				BeginPolygon(Tex, WallTex.GetTextureFlags(FWallTextures::EWallTexMode::WALLTEX_BottomTex), TexOrigin, Normal, TexU, TexV);
			}
			WriteVertex(SplitA);
			WriteVertex(SplitB);
			WriteVertex(C);
			WriteVertex(D);
			EndPolygon();

			{
				FInstTexture* Tex = WallTex.GetTexture(FWallTextures::EWallTexMode::WALLTEX_TopTex);
				const FVector TexU = bTexU * Tex->ScaleU;
				const FVector TexV = bTexV * Tex->ScaleV;
				const FVector TexOrigin = FVector(bFlipTextureU ? PointA : PointB, AlignZTop) - BasePosition
					- TexU * FLOAT(SideDef.textureoffset)
					- TexV * FLOAT(SideDef.rowoffset);
				BeginPolygon(Tex, WallTex.GetTextureFlags(FWallTextures::EWallTexMode::WALLTEX_TopTex), TexOrigin, Normal, TexU, TexV);
			}
			WriteVertex(A);
			WriteVertex(B);
			WriteVertex(SplitB);
			WriteVertex(SplitA);
			EndPolygon();
			unguard;
		}
		inline void SwapFloorCeilTex()
		{
			guardSlow(FBrushOutput::SwapFloorCeilTex);
			Exchange(FloorTex, CeilTex);
			unguardSlow;
		}
		inline void WriteBase(const TArray<FVector2D>& Verts, UBOOL bReverse, FLOAT ZHeight)
		{
			if (bReverse)
			{
				for ( INT i = (Verts.Num() - 1); i >= 0; --i)
					WriteVertex(FVector(Verts(i), ZHeight) - BasePosition);
			}
			else
			{
				for (INT i = 0; i < Verts.Num(); ++i)
					WriteVertex(FVector(Verts(i), ZHeight) - BasePosition);
			}
		}
	private:
		BITFIELD bWriteCSG : 1, bWriteSurfs : 1;
		const INT iModel;
		const ECSGWriteMode WriteMode;

		inline UBOOL WriteToOutput(ECSGOutputDest Mode)
		{
			if (!bStandardWalls)
				return FALSE;
			switch (Mode)
			{
			case CSGW_Brush:
				return (WriteMode != CSG_Mover) && (WriteMode != CSG_SemiSolidTop) && (WriteMode != CSG_SemiSolidBtm);
			case CSGW_Mover:
				return (WriteMode == CSG_Mover);
			case CSGW_Top:
				return (WriteMode == CSG_SemiSolidTop);
			case CSGW_Bottom:
				return (WriteMode == CSG_SemiSolidBtm);
			default:
				return FALSE;
			}
		}

		FBrushOutput(const INT iM, FInstTexture* FlrTex, FInstTexture* CelTex, const FLOAT FH, const FLOAT CH, const FVector& BasePos, ECSGWriteMode wM)
			: iModel(iM), FloorTex(FlrTex), CeilTex(CelTex), FloorHeight(FH), CeilingHeight(CH), bStandardWalls(TRUE), bAllowSplitter(TRUE), bWriteCSG(TRUE), bFlipTextureU(FALSE), bWriteSurfs(TRUE), BasePosition(BasePos)
			, WriteMode(wM)
		{}
		FBrushOutput() = delete;
		friend struct FBSPOutput;
	};

private:
	TArray<FBrushOutput*> OutList;
	TArray< TArray<FVector2D>* > Tris;
	UBOOL bBegunSurfs;
	const FLOAT BaseFlr, BaseCel;
	FInstTexture *BaseFloorTex, *BaseCeilTex;
	const FVector BasePosition;
	TArray<FVector2D>* PendingPoly;

	inline void VerifyBuffer()
	{
		guardSlow(VerifyBuffer);
		if (!bBegunSurfs)
		{
			bBegunSurfs = TRUE;
			for (INT i = 0; i < OutList.Num(); ++i)
			{
				OutList(i)->Out.Logf(TEXT("\t\tBegin Brush Name=Model%i\r\n"), OutList(i)->iModel);
				OutList(i)->Out.Log(TEXT("\t\t\tBegin PolyList\r\n"));
			}
		}
		unguardSlow;
	}
public:
	FBSPOutput(const FLOAT FloorHeight, const FLOAT CeilingHeight, FInstTexture* FloorTex, FInstTexture* CeilTex, const FVector& BasePos)
		: bBegunSurfs(FALSE), BaseFlr(FloorHeight), BaseCel(CeilingHeight), BaseFloorTex(FloorTex), BaseCeilTex(CeilTex), BasePosition(BasePos), PendingPoly(nullptr)
	{}
	FBSPOutput() = delete;
	~FBSPOutput() noexcept(false)
	{
		guard(FBSPOutput::~FBSPOutput);
		INT i;
		for (i = 0; i < OutList.Num(); ++i)
			delete OutList(i);
		for (i = 0; i < Tris.Num(); ++i)
			delete Tris(i);
		unguard;
	}

	inline FLOAT GetZHeightDiff(const FLOAT Ceiling, const FLOAT Floor) const
	{
		return (Ceiling + Floor) * 0.5f - BasePosition.Z;
	}
	inline UBOOL HasOutput() const
	{
		return (OutList.Num() > 0);
	}
	FBrushOutput* AppendBrush(const FVector& Offset, const ECSGWriteMode CSG, const TCHAR* ClassName, const INT ObjIndex, const INT ModelIndex, const INT iTag = INDEX_NONE)
	{
		guard(FBSPOutput::AppendBrush);
		DEBUG_MSG(TEXT("AddBrush %ls%i Model%i CSG %ls"), ClassName, ObjIndex, ModelIndex, GetCSGWriteStr(CSG));
		FBrushOutput* Ent = new FBrushOutput(ModelIndex, BaseFloorTex, BaseCeilTex, BaseFlr, BaseCel, BasePosition, CSG);
		OutList.AddItem(Ent);
		Ent->Out.Logf(TEXT("\tBegin Actor Class=%ls Name=%ls%i\r\n"), ClassName, ClassName, ObjIndex);

		if(iTag>=0)
			Ent->Out.Logf(TEXT("\t\tTag=TG%i\r\n"), iTag);
		switch (CSG)
		{
		case CSG_Volume:
			Ent->bWriteCSG = FALSE;
			Ent->bWriteSurfs = FALSE;
			Ent->bAllowSplitter = FALSE;
			break;
		case CSG_Mover:
			Ent->bWriteCSG = FALSE;
			Ent->bAllowSplitter = FALSE;
			Ent->bFlipTextureU = TRUE;
			break;
		case CSG_SemiSolid:
		case CSG_SemiSolidTop:
		case CSG_SemiSolidBtm:
			Ent->Out.Logf(TEXT("\t\tPolyFlags=%d\r\n"), PF_Semisolid);
		case CSG_Additive:
			Ent->Out.Log(TEXT("\t\tCsgOper=CSG_Add\r\n"));
			Ent->bFlipTextureU = TRUE;
			break;
		case CSG_Subtractive:
			Ent->Out.Log(TEXT("\t\tCsgOper=CSG_Subtract\r\n"));
			break;
		}
		const FVector ScLoc = (BasePosition + Offset) * SCALE_FACTOR;
		Ent->Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), ScLoc.X, ScLoc.Y, ScLoc.Z);
		if (CSG == CSG_Mover)
			Ent->Out.Logf(TEXT("\t\tBasePos=(X=%f,Y=%f,Z=%f)\r\n"), ScLoc.X, ScLoc.Y, ScLoc.Z);
		return Ent;
		unguard;
	}
	inline void BeginTris()
	{
		PendingPoly = new TArray<FVector2D>;
		Tris.AddItem(PendingPoly);
	}
	inline void AddVertex(const FVector2D& Vert)
	{
		PendingPoly->AddItem(Vert);
	}
	void BuildBase(const FVector2D& Origin) // Begin floor buffer.
	{
		guard(FBSPOutput::BeginFloor);
		VerifyBuffer();
		FVector2D BaseUV = Origin;
		BaseUV.X = FSnap(BasePosition.X + BaseUV.X, FlatResolution) - BasePosition.X;
		BaseUV.Y = FSnap(BasePosition.Y + BaseUV.Y, FlatResolution) - BasePosition.Y;
		const FVector Normal(0.f, 0.f, 1.f);
		const FVector TexU(1.f, 0.f, 0.f);
		const FVector TexV(0.f, 1.f, 0.f);

		// Floor.
		INT i, j;
		for (i = 0; i < OutList.Num(); ++i)
		{
			const FLOAT Flr = OutList(i)->FloorHeight;
			const FVector PolyBase(BaseUV.X, BaseUV.Y, Flr - BasePosition.Z);
			FInstTexture* Tex = OutList(i)->FloorTex;
			for (j = 0; j < Tris.Num(); ++j)
			{
				OutList(i)->BeginPolygon(Tex, PF_None, PolyBase, Normal, TexU, TexV);
				OutList(i)->WriteBase(*Tris(j), TRUE, Flr);
				OutList(i)->EndPolygon();
			}
		}

		// Ceil.
		for (i = 0; i < OutList.Num(); ++i)
		{
			const FLOAT Cel = OutList(i)->CeilingHeight;
			const FVector PolyBase(BaseUV.X, BaseUV.Y, Cel - BasePosition.Z);
			FInstTexture* Tex = OutList(i)->CeilTex;
			for (j = 0; j < Tris.Num(); ++j)
			{
				OutList(i)->BeginPolygon(Tex, PF_None, PolyBase, Normal, TexU, TexV);
				OutList(i)->WriteBase(*Tris(j), FALSE, Cel);
				OutList(i)->EndPolygon();
			}
		}
		unguard;
	}
	inline void FinishBrush(FOutputDevice* Output)
	{
		guard(FBSPOutput::FinishBrush);
		for (INT i = 0; i < OutList.Num(); ++i)
		{
			OutList(i)->Out.Log(TEXT("\t\t\tEnd PolyList\r\n"));
			OutList(i)->Out.Log(TEXT("\t\tEnd Brush\r\n"));
			OutList(i)->Out.Logf(TEXT("\t\tBrush=Model'MyLevel.Model%i'\r\n"), OutList(i)->iModel);
			OutList(i)->Out.Log(TEXT("\tEnd Actor\r\n"));

			Output->Log(OutList(i)->Out);
		}
		unguard;
	}
	void AddSide(const FVector2D& PointA, const FVector2D& PointB, const FWallTextures& WallTex, const FWallTextures::EWallTexMode DesiredTex, const side_t& SideDef, FLOAT AlignZ, ECSGOutputDest WriteDest)
	{
		guard(FBSPOutput::AddSide);
		for (INT i = 0; i < OutList.Num(); ++i)
			if (OutList(i)->WriteToOutput(WriteDest))
				OutList(i)->AddSide(PointA, PointB, WallTex, DesiredTex, SideDef, AlignZ);
		unguard;
	}
	void AddSideSplitted(const FVector2D& PointA, const FVector2D& PointB, const FLOAT SplitHeight, const FWallTextures& WallTex, const side_t& SideDef, FLOAT AlignZTop, FLOAT AlignZBtm)
	{
		guard(FBSPOutput::AddSideSplitted);
		for (INT i = 0; i < OutList.Num(); ++i)
			if (OutList(i)->bStandardWalls)
				OutList(i)->AddSideSplitted(PointA, PointB, SplitHeight, WallTex, SideDef, AlignZTop, AlignZBtm);
		unguard;
	}
};

struct FMapLoader
{
	FArchive* FileAr;
	FWADHeaderContents& Contents;
	TArray<FMapInfo*> Maps;
	FMapInfo* PendingMap;

private:
	sector_t* ExitSector;
	sector_t* SecretSector;
	FMapInfo* EditMap;
	INT iModel, iBrush, iVolume, iMover, iTeleporter;

public:

	FMapLoader(FArchive* A, FWADHeaderContents& FE)
		: FileAr(A), Contents(FE), PendingMap(NULL)
	{}
	UBOOL LoadMaps()
	{
		guard(FMapLoader::LoadMaps);
		Maps.Empty();
		PendingMap = NULL;
		TCHAR TempName[16];
		if (Contents.IsEpisodic())
		{
			INT iMap = 0, iEpisode = 0;

			while (1)
			{
				UBOOL AnyMap = FALSE;
				iMap = 0;
				++iEpisode;
				while (1)
				{
					++iMap;
					appSprintf(TempName, TEXT("E%iM%i"), iEpisode, iMap);
					if (!TryLoad(TempName))
						break;
					Maps.Last()->iMap = iMap;
					Maps.Last()->iEpisode = iEpisode;
					AnyMap = TRUE;
				}
				if (!AnyMap)
					break;
			}

			for (INT i = 0; i < Maps.Num(); ++i)
				Maps(i)->MapFilename = FString::Printf(TEXT("%lsE%iM%i"), *Contents.LevelsPrefix, Maps(i)->iEpisode, Maps(i)->iMap);
		}
		else
		{
			INT iMap = 0;

			while (1)
			{
				++iMap;
				if (iMap < 10)
					appSprintf(TempName, TEXT("MAP0%i"), iMap);
				else appSprintf(TempName, TEXT("MAP%i"), iMap);
				if (!TryLoad(TempName))
					break;
				Maps.Last()->iMap = iMap;
			}

			for (INT i = 0; i < Maps.Num(); ++i)
				Maps(i)->MapFilename = FString::Printf(TEXT("%lsMap%02i"), *Contents.LevelsPrefix, Maps(i)->iMap);
		}
		return (Maps.Num() > 0);
		unguard;
	}
	FMapInfo* FindMap(const INT iEpisode, const INT iMap) const
	{
		guard(FindMap);
		if (Contents.IsEpisodic())
		{
			for (INT i = 0; i < Maps.Num(); ++i)
				if (Maps(i)->iEpisode == iEpisode && Maps(i)->iMap == iMap)
					return Maps(i);
		}
		else
		{
			for (INT i = 0; i < Maps.Num(); ++i)
				if (Maps(i)->iMap == iMap)
					return Maps(i);
		}
		return nullptr;
		unguard;
	}
	void WriteT3DFile(FOutputDevice& Out, const INT iMapNum, const INT iBrushIndex = INDEX_NONE)
	{
		guard(WriteT3DFile);
		verifyf(Maps.IsValidIndex(iMapNum), TEXT("Invalid index %i/%i"), iMapNum, Maps.Num());
		FMapInfo& M = *Maps(iMapNum);
		EditMap = Maps(iMapNum);

		if (iBrushIndex == INDEX_NONE)
		{
			guard(PrepareMap);
			Out.Log(TEXT("Begin Map\r\n"));
			Out.Log(TEXT("\tBegin Actor Class=LevelInfo Name=LevelInfo0\r\n"));
			Out.Logf(TEXT("\t\tTitle=\"%ls\"\r\n"), Contents.GetGameName());
			Out.Log(TEXT("\t\tAuthor=\"ID-Software\"\r\n"));
			Out.Log(TEXT("\t\tbDisableRbPhysics=True\r\n"));
			Out.Log(TEXT("\t\tAmbientBrightness=255\r\n"));
			Out.Log(TEXT("\t\tTexUPanSpeed=-0.25\r\n"));
			Out.Log(TEXT("\t\tDefaultGameType=Class'DoomEngine.DoomGameInfo'\r\n"));
			{
				INT mapIndex = M.iMap;
				INT mapEp = INDEX_NONE;
				if (Contents.IsEpisodic())
					mapEp = M.iEpisode;
				FString Song = GetMapMusic(mapIndex, mapEp);
				Out.Logf(TEXT("\t\tSong=Music'%ls'\r\n"), *Song);
			}
			Out.Log(TEXT("\tEnd Actor\r\n"));
			WriteBuilderBrush(Out);
			unguard;
		}

		iModel = iBrush = 1;
		iVolume = iMover = iTeleporter = 0;
		INT iMatIndex = 0;
		
		const INT NumSectors = M.Sectors.Num();
		const INT NumSubSectors = M.SubSectors.Num();
		const INT NumSegs = M.MapSegs.Num();
		const INT NumLineDefs = M.Lines.Num();
		const vertex_t* Verts = &M.Vertices(0);
		const side_t* sidedefs = &M.SideDefs(0);

		ExitSector = SecretSector = NULL;
		INT issec, iseg;

		guard(CheckTriggers);
		for (INT idef = 0; idef < NumLineDefs; ++idef)
		{
			line_t& line = M.Lines(idef);
			if (!line.TriggerData)
				continue;

			if (line.TriggerData->TriggerFlags & (SPLT_FloorMover | SPLT_CeilMover))
			{
				if (line.tag)
				{
					for (INT isec = 0; isec < NumSectors; ++isec)
					{
						sector_t& S = M.Sectors(isec);
						if (S.tag == line.tag)
							S.TriggerData = line.TriggerData;
					}
				}
				else if (line.sidenum[1] != INDEX_NONE)
				{
					sector_t& S = M.Sectors(M.SideDefs(line.sidenum[1]).sector);
					S.TriggerData = line.TriggerData;
					if (!S.tag)
						S.tag = line.tag = M.GetUniqueTag();
					else line.tag = S.tag;
				}
			}
			else if (line.TriggerData->TriggerFlags & SPLT_Teleport)
			{
				for (INT isec = 0; isec < NumSectors; ++isec)
				{
					sector_t& S = M.Sectors(isec);
					if (S.tag == line.tag)
						S.bTeleportSector = TRUE;
				}
			}
			if (iBrushIndex != INDEX_NONE)
				continue;

			const sector_t& ThisSide = M.Sectors(M.SideDefs(line.sidenum[0]).sector);
			FLOAT Bottom = ThisSide.floorheight;
			FLOAT Top = ThisSide.ceilingheight;

			if (line.sidenum[1] != INDEX_NONE && !(line.TriggerData->TriggerFlags & (SPLT_SwitchTrigger | SPLT_ProjectileTrigger)))
			{
				const sector_t& OtherSide = M.Sectors(M.SideDefs(line.sidenum[1]).sector);
				Bottom = Min<FLOAT>(OtherSide.floorheight, Bottom);
				Top = Max<FLOAT>(OtherSide.ceilingheight, Top);

				Out.Logf(TEXT("\tBegin Actor Class=DoomLineTrigger Name=DoomLineTrigger%i\r\n"), iVolume++);
				if (line.TriggerData->TriggerFlags & SPLT_Teleport)
					Out.Log(TEXT("\t\tbSingleSided=true\r\n"));
			}
			else
			{
				if (line.TriggerData->TriggerFlags & SPLT_ProjectileTrigger)
					Out.Logf(TEXT("\tBegin Actor Class=DoomShootTrigger Name=DoomShootTrigger%i\r\n"), iVolume++);
				else Out.Logf(TEXT("\tBegin Actor Class=DoomUseTrigger Name=DoomUseTrigger%i\r\n"), iVolume++);

				// Figure out the switch texture to replace.
				side_t& sdef = M.SideDefs(line.sidenum[0]);

				FInstTexture* SwTex = NULL;
				if (sdef.toptexture->IsASwitch())
				{
					SwTex = sdef.toptexture;
					sdef.SwitchIndex = SWITCHTEX_Top;
				}
				else if (sdef.bottomtexture->IsASwitch())
				{
					SwTex = sdef.bottomtexture;
					sdef.SwitchIndex = SWITCHTEX_Bottom;
				}
				else if (sdef.midtexture->IsASwitch())
				{
					SwTex = sdef.midtexture;
					sdef.SwitchIndex = SWITCHTEX_Mid;
				}

				if (SwTex)
				{
					Out.Logf(TEXT("\t\tSwitchSound=Sound'%ls.DSSWTCHN'\r\n"), *Contents.TexturePackageName);
					sdef.SwitchTex = SwTex->CreateSwitch(iMatIndex);
					Out.Logf(TEXT("\t\tBegin Object Class=MaterialSequence Name=%ls\r\n"), *sdef.SwitchTex->TextureName);
					Out.Logf(TEXT("\t\t\tSequenceItems(0)=(Material=Texture'%ls',DisplayTime=1.0)\r\n"), SwTex->GetFileName());
					Out.Logf(TEXT("\t\t\tSequenceItems(1)=(Material=Texture'%ls',DisplayTime=1.0)\r\n"), (SwTex->FirstFrame ? SwTex->FirstFrame : SwTex)->GetFileName());
					Out.Log(TEXT("\t\t\tLoop=False\r\n"));
					Out.Log(TEXT("\t\t\tPaused=True\r\n"));
					Out.Logf(TEXT("\t\t\tMatUSize=%i\r\n"), SwTex->FinalU);
					Out.Logf(TEXT("\t\t\tMatVSize=%i\r\n"), SwTex->FinalV);
					Out.Log(TEXT("\t\tEnd Object\r\n"));
					Out.Logf(TEXT("\t\tSwitchMaterial=MaterialSequence'%ls'\r\n"), *sdef.SwitchTex->TextureName);
					++iMatIndex;
				}

				if (line.sidenum[1] != -1)
				{
					const sector_t& OtherSector = M.Sectors(M.SideDefs(line.sidenum[1]).sector);
					if (OtherSector.floorheight > ThisSide.floorheight)
						Top = OtherSector.floorheight;
				}
			}

			{
				if (line.TriggerData->TriggerFlags & (SPLT_FloorMover | SPLT_CeilMover))
					Out.Logf(TEXT("\t\tEvent=TG%i\r\n"), INT(line.tag));
				else if (line.TriggerData->TriggerFlags & SPLT_Teleport)
					Out.Logf(TEXT("\t\tEvent=TP%i\r\n"), INT(line.tag));

				const FVector PointA = FVector(Verts[line.v1].GetVector2DFlip(), Bottom) * SCALE_FACTOR;
				const FVector PointB = FVector(Verts[line.v2].GetVector2DFlip(), Top) * SCALE_FACTOR;
				const FVector MidPoint = (PointA + PointB) * 0.5f;
				const FVector LineDir = (PointA - PointB).SafeNormal2D();
				const FVector SideDir = LineDir ^ FVector(0.f, 0.f, 1.f);
				const FLOAT SideLength = (PointA - PointB).Size2D();

				Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), MidPoint.X, MidPoint.Y, MidPoint.Z);
				FRotator SideRot = SideDir.Rotation();
				Out.Logf(TEXT("\t\tRotation=(Yaw=%i,Pitch=0,Roll=0)\r\n"), SideRot.Yaw);
				Out.Logf(TEXT("\t\tDrawScale3D=(X=1.0,Y=%f,Z=%f)\r\n"), Max(SideLength - 8.f, 1.f) / 100.f, (Top - Bottom) / (100.f / SCALE_FACTOR));
			}
			if(line.TriggerData->TriggerFlags & SPLT_MonsterOnly)
				Out.Log(TEXT("\t\tbMonstersOnly=true\r\n"));
			if (line.TriggerData->TriggerFlags & SPLT_PlayerOnly)
				Out.Log(TEXT("\t\tbPlayersOnly=true\r\n"));
			if (line.TriggerData->TriggerFlags & SPLT_TriggerOnce)
				Out.Log(TEXT("\t\tbTriggerOnceOnly=true\r\n"));

			switch (line.TriggerData->LockType)
			{
			case LOCKTYPE_Blue:
				Out.Logf(TEXT("\t\tRequiredItem=class'%ls'\r\n"), EntTypeToStr(MT_MISC4));
				break;
			case LOCKTYPE_Yellow:
				Out.Logf(TEXT("\t\tRequiredItem=class'%ls'\r\n"), EntTypeToStr(MT_MISC6));
				break;
			case LOCKTYPE_Red:
				Out.Logf(TEXT("\t\tRequiredItem=class'%ls'\r\n"), EntTypeToStr(MT_MISC5));
				break;
			case LOCKTYPE_None:
				break;
			}

			if (line.TriggerData->TriggerFlags & SPLT_Exit)
			{
				if (line.TriggerData->TriggerFlags & SPLT_SecretExit)
				{
					Out.Log(TEXT("\t\tEvent=SecretExit\r\n"));
					SecretSector = &M.Sectors(M.SideDefs(line.sidenum[0]).sector);
				}
				else
				{
					Out.Log(TEXT("\t\tEvent=LevelExit\r\n"));
					ExitSector = &M.Sectors(M.SideDefs(line.sidenum[0]).sector);
				}
			}
			Out.Log(TEXT("\tEnd Actor\r\n"));
		}

		if (iBrushIndex == INDEX_NONE)
		{
			// Hardcoded map specific kill events.
			mobjtype_t KillEvent = NUMMOBJTYPES;
			mobjtype_t SecKillEvent = NUMMOBJTYPES;
			INT TriggerTag = 666, SecTriggerTag = 667;
			DWORD MoverType, SecMoverType;
			EMoverFlags MoveType, SecMoveType;
			if (Contents.IsEpisodic())
			{
				switch (M.iEpisode)
				{
				case 1:
					if (M.iMap == 8)
					{
						KillEvent = MT_BRUISER;
						MoverType = SPLT_FloorMover;
						MoveType = MFT_LowestFloor;
					}
					break;

				case 2:
					if (M.iMap == 8)
						KillEvent = MT_CYBORG;
					TriggerTag = INDEX_NONE;
					break;

				case 3:
					if (M.iMap == 8)
						KillEvent = MT_SPIDER;
					TriggerTag = INDEX_NONE;
					break;

				case 4:
					if (M.iMap == 6)
					{
						KillEvent = MT_CYBORG;
						MoverType = SPLT_CeilMover;
						MoveType = MFT_HighestCeiling;
					}
					else if (M.iMap == 8)
					{
						KillEvent = MT_SPIDER;
						MoverType = SPLT_FloorMover;
						MoveType = MFT_LowestFloor;
					}
					break;
				}
			}
			else
			{
				if (M.iMap == 7)
				{
					KillEvent = MT_FATSO;
					SecKillEvent = MT_BABY;
					MoverType = SPLT_FloorMover;
					MoveType = MFT_LowestFloor;
					SecMoverType = SPLT_FloorMover;
					SecMoveType = MFT_LowestFloor;
				}
				else
				{
					// Check if has map 32 keens.
					const INT NumEnts = M.Things.Num();
					for (INT j = 0; j < NumEnts; ++j)
					{
						if (!M.Things(j).IsSpawnPoint() && M.Things(j).GetEntType() == MT_KEEN)
						{
							KillEvent = MT_KEEN;
							MoverType = SPLT_CeilMover;
							MoveType = MFT_HighestCeiling;
							break;
						}
					}
				}
			}
			if (KillEvent != NUMMOBJTYPES)
			{
				Out.Log(TEXT("\tBegin Actor Class=DoomKillAllEvent Name=DoomKillAllEvent0\r\n"));
				Out.Log(TEXT("\t\tLocation=(X=0,Y=0,Z=0)\r\n"));
				Out.Logf(TEXT("\t\tPawnType=class'%ls'\r\n"), EntTypeToStr(KillEvent));
				if (TriggerTag == INDEX_NONE)
				{
					Out.Log(TEXT("\t\tEvent=LevelExit\r\n"));
					if (!ExitSector)
						ExitSector = &M.Sectors(1);
				}
				else
				{
					Out.Logf(TEXT("\t\tEvent=TG%i\r\n"), TriggerTag);
					for (INT isec = 0; isec < NumSectors; ++isec)
					{
						sector_t& S = M.Sectors(isec);
						if (S.tag == TriggerTag)
							S.TriggerData = new (GMem) FTriggerData(MoverType, MoveType);
					}
				}
				Out.Log(TEXT("\tEnd Actor\r\n"));

				if (SecKillEvent != NUMMOBJTYPES)
				{
					Out.Log(TEXT("\tBegin Actor Class=DoomKillAllEvent Name=DoomKillAllEvent1\r\n"));
					Out.Log(TEXT("\t\tLocation=(X=0,Y=0,Z=16)\r\n"));
					Out.Logf(TEXT("\t\tPawnType=class'%ls'\r\n"), EntTypeToStr(SecKillEvent));
					Out.Logf(TEXT("\t\tEvent=TG%i\r\n"), SecTriggerTag);
					for (INT isec = 0; isec < NumSectors; ++isec)
					{
						sector_t& S = M.Sectors(isec);
						if (S.tag == SecTriggerTag)
							S.TriggerData = new (GMem) FTriggerData(SecMoverType, SecMoveType);
					}
					Out.Log(TEXT("\tEnd Actor\r\n"));
				}
			}
		}
		unguard;

		{
			FLineConnector Connector(Verts, M.Vertices.Num());
			guard(InitSectors);
			for (INT isec = 0; isec < NumSectors; ++isec)
			{
				if (!M.Sectors(isec).CanMapSector())
					continue;

				sector_t* S = &M.Sectors(isec);
				for (issec = 0; issec < NumSubSectors; ++issec)
				{
					const subsector_s& SS = M.SubSectors(issec);
					if (SS.sector != S)
						continue;

					mapseg_t* segl = &M.MapSegs(SS.firstseg);
					for (iseg = 0; iseg < SS.numsegs; ++iseg)
					{
						const line_t& linedef = *segl[iseg].linedef;
						const side_t& sdef = *segl[iseg].sidedef;

#if DEBUG_SECTOR_INDEX>=0
						GDebugOutput = ((S->SectorIndex == DEBUG_SECTOR_INDEX) && ((segl[iseg].frontsector == S) || (segl[iseg].backsector == S)));
						if (GDebugOutput)
						{
							debugf(TEXT("AddLine Front %i Back %i"), segl[iseg].frontsector->SectorIndex, segl[iseg].backsector ? segl[iseg].backsector->SectorIndex : INDEX_NONE);
							const FVector2D A = Verts[segl[iseg].v1].GetVector2DFlip() * SCALE_FACTOR;
							const FVector2D B = Verts[segl[iseg].v2].GetVector2DFlip() * SCALE_FACTOR;
							debugf(TEXT(" Verts %f,%f - %f,%f (%i - %i Angle %i)"), A.X, A.Y, B.X, B.Y, INT(segl[iseg].v1), INT(segl[iseg].v2), INT(segl[iseg].angle));
							FWallTextures WT(linedef, &sdef, nullptr);
							debugf(TEXT("  Tex %ls,%ls,%ls"), WT.GetTexture(FWallTextures::WALLTEX_TopTex)->Describe(), WT.GetTexture(FWallTextures::WALLTEX_MidTex)->Describe(), WT.GetTexture(FWallTextures::WALLTEX_BottomTex)->Describe());
							if (segl[iseg].othersidedef)
							{
								WT = FWallTextures(linedef, segl[iseg].othersidedef, nullptr);
								debugf(TEXT("  OTex %ls,%ls,%ls"), WT.GetTexture(FWallTextures::WALLTEX_TopTex)->Describe(), WT.GetTexture(FWallTextures::WALLTEX_MidTex)->Describe(), WT.GetTexture(FWallTextures::WALLTEX_BottomTex)->Describe());
							}
						}
#endif
						if (segl[iseg].frontsector == S)
						{
							if (segl[iseg].backsector != S)
								Connector.AddLineSegment(segl[iseg], segl[iseg].backsector);
						}
						else Connector.AddLineSegment(segl[iseg], segl[iseg].frontsector, TRUE);
					}
				}
				Connector.FinishSector(S);
			}
			Connector.SortShapes();
			unguard;

			guard(FormBSP);
			FStringOutputDevice LateBSP;
			for (INT z = 0; z < Connector.ShapeList.Num(); ++z)
			{
				FormBSPOf(Out, LateBSP, z, Connector, M, iBrushIndex);
			}
			Out.Log(LateBSP);
			unguard;
		}

		if (iBrushIndex == INDEX_NONE)
		{
			guard(GatherEnts);
			const INT NumEnts = M.Things.Num();
			INT j;
			INT psnum = 0;
			//TMap<INT, INT> EntCounter;
			INT EntCounter = 0;
			const INT SpawnHeight = appRound(GetDefault<APlayerStart>()->CollisionHeight / SCALE_FACTOR);
			for (j = 0; j < NumEnts; ++j)
			{
				const FThingEntry& T = M.Things(j);
				FVector2D Point(T.x, T.y);
				sector_t* sector = M.GetPointSector(Point);
				if (!sector)
					continue;

				const INT YawAng = -appRound(FLOAT(T.angle) * (65536.f / 360.f));
				if (T.IsSpawnPoint())
				{
					if (T.IsDMSpawn())
						continue;
					Out.Logf(TEXT("\tBegin Actor Class=PlayerStart Name=PlayerStart%i\r\n"), psnum++);
					Out.Logf(TEXT("\t\tLocation=(%ls)\r\n"), DoomLocToStr(T.x, T.y, sector->floorheight + SpawnHeight));
					Out.Logf(TEXT("\t\tRotation=(Yaw=%i)\r\n"), YawAng);
					/*if (T.IsDMSpawn())
					{
						Out.Log(TEXT("\t\tbCoopStart=false\r\n"));
						Out.Log(TEXT("\t\tbSinglePlayerStart=false\r\n"));
					}
					else
					{
						Out.Log(TEXT("\t\tbCoopStart=true\r\n"));
						Out.Log(TEXT("\t\tbSinglePlayerStart=true\r\n"));
					}*/
					Out.Log(TEXT("\tEnd Actor\r\n"));
				}
				else if (!T.IsDMEntity())
				{
					INT eType = T.GetEntType();
					if (eType == INDEX_NONE)
						continue;

					INT num = 0;
#if 0
					INT* count = EntCounter.Find(eType);
					if (!count)
						EntCounter.Set(eType, 1);
					else num = *count++;
#else
					num = EntCounter++;
#endif

					if (eType == MT_TELEPORTMAN)
					{
						if (!sector->bTeleportSector)
							continue;
						Out.Logf(TEXT("\tBegin Actor Class=DoomTeleportDest Name=DoomTeleportDest%i\r\n"), num);
						Out.Logf(TEXT("\t\tLocation=(%ls)\r\n"), DoomLocToStrRZ(T.x, T.y, FLOAT(sector->floorheight) * SCALE_FACTOR + 16.f));
						Out.Logf(TEXT("\t\tRotation=(Yaw=%i)\r\n"), YawAng);
						const TCHAR* EntName = EntTypeToStr(MT_TFOG);
						Out.Logf(TEXT("\t\tTeleportFX=Class'%ls'\r\n"), EntName);
						Out.Logf(TEXT("\t\tTag=TP%i\r\n"), INT(sector->tag));
					}
					else if ((eType == MT_BOSSSPIT) || (eType == MT_BOSSTARGET))
					{
						Out.Logf(TEXT("\tBegin Actor Class=DoomBossSpawn Name=DoomBossSpawn%i\r\n"), num);
						Out.Logf(TEXT("\t\tLocation=(%ls)\r\n"), DoomLocToStrRZ(T.x, T.y, FLOAT(sector->floorheight) * SCALE_FACTOR + 16.f));
						Out.Logf(TEXT("\t\tTag=\"%ls\"\r\n"), (eType == MT_BOSSTARGET) ? TEXT("SpitTarget") : TEXT("SpawnPos"));
					}
					else
					{
						const TCHAR* EntName = EntTypeToStr(eType);
						const mobjinfo_t& mob = mobjinfo[eType];

						Out.Logf(TEXT("\tBegin Actor Class=%ls Name=%ls%i\r\n"), EntName, EntName, num);
						Out.Logf(TEXT("\t\tLocation=(%ls)\r\n"), DoomLocToStrRZ(T.x, T.y, (FLOAT(sector->floorheight) + FLOAT(mob.height >> FRACBITS) * 0.5f) * SCALE_FACTOR));

						Out.Logf(TEXT("\t\tRotation=(Yaw=%i)\r\n"), YawAng);
						if (!(T.options & EOPTS_Easy))
							Out.Log(TEXT("\t\tbDifficulty0=false\r\n"));
						if (!(T.options & EOPTS_Medium))
							Out.Log(TEXT("\t\tbDifficulty1=false\r\n"));
						if (!(T.options & EOPTS_Hard))
						{
							Out.Log(TEXT("\t\tbDifficulty2=false\r\n"));
							Out.Log(TEXT("\t\tbDifficulty3=false\r\n"));
						}
						//if (T.options & EOPTS_Net)
						//	Out.Log(TEXT("\t\tbSinglePlayer=false\r\n"));
					}
					Out.Log(TEXT("\tEnd Actor\r\n"));
				}
			}
			unguard;
		}
		if (iBrushIndex == INDEX_NONE)
			Out.Log(TEXT("End Map\r\n"));
		unguard;
	}
private:
	void FormBSPOf(FOutputDevice& Out, FOutputDevice& LateBSP, const INT iSegment, FLineConnector& Connector, const FMapInfo& Map, const INT iBrushIndex)
	{
		guard(FMapLoader::FormBSPOf);
		FSectorShape& LSegment = *Connector.ShapeList(iSegment);
		if (LSegment.HasBaked)
			return;

#if DUMP_DEBUG_SHAPES
		GDebugOutput = (LSegment.sector.SectorIndex == DEBUG_SECTOR_INDEX);
		debugf(TEXT("LineSegment Sector %i (%ls) NumLines %i Mover %ls CCW %i Skip %i"), LSegment.sector.SectorIndex, LSegment.GetConnectingSectors(), LSegment.NumLines, (LSegment.sector.TriggerData && LSegment.sector.TriggerData->IsMover()) ? GTrue : GFalse, INT(LSegment.IsCCW), INT(LSegment.CanSkipBSP));
#endif

		LSegment.HasBaked = TRUE;
		INT iSurf;
		const INT NumLines = LSegment.NumLines;
		FWallLine** Lines = LSegment.Lines;

		FLOAT FlrHeight = LSegment.FloorHeight;
		FLOAT CeilHeight = LSegment.CeilingHeight;
		const FLOAT HeightDif = (CeilHeight - FlrHeight);
		const sector_t& Sector = LSegment.sector;

		guard(CheckTriggerWalls);
		if (Sector.TriggerData)
		{
			if (Sector.TriggerData->TriggerFlags & SPLT_CeilMover)
			{
				for (iSurf = 0; iSurf < NumLines; ++iSurf)
				{
					if (Lines[iSurf]->OtherSector)
						CeilHeight = Max<FLOAT>(CeilHeight, Lines[iSurf]->OtherSector->ceilingheight);
				}
			}
			if (Sector.TriggerData->TriggerFlags & SPLT_FloorMover)
			{
				for (iSurf = 0; iSurf < NumLines; ++iSurf)
				{
					if (Lines[iSurf]->OtherSector)
						FlrHeight = Min<FLOAT>(FlrHeight, Lines[iSurf]->OtherSector->floorheight);
				}
			}
		}
		unguard;

		FBSPOutput::ECSGWriteMode WriteBSPMode = FBSPOutput::ECSGWriteMode::CSG_Subtractive;
		guard(CheckCSG);
		if (LSegment.IsCCW)
		{
			WriteBSPMode = FBSPOutput::ECSGWriteMode::CSG_Additive;
			if (LSegment.HasSingularInnerSector())
			{
				sector_t* OtherSector = LSegment.Lines[0]->OtherSector;
				if (!OtherSector)
					WriteBSPMode = FBSPOutput::ECSGWriteMode::CSG_SemiSolid;
				else
				{
					FSectorShape* OtherSegment = OtherSector->Shape->GetNeighbouringShape(LSegment);
					if (!OtherSegment || !OtherSegment->IsValid())
						WriteBSPMode = FBSPOutput::ECSGWriteMode::CSG_SemiSolid;
					else
					{
						if (OtherSegment->FloorHeight <= LSegment.FloorHeight && OtherSegment->CeilingHeight >= LSegment.CeilingHeight) // Do not bother adding solid shape in between if the inner shape is the same or bigger size.
						{
							if (OtherSector->TriggerData && OtherSector->TriggerData->IsMover()) // Uh, an elevator, bail out.
								return;

							// Doesn't need to create this inner sector unless its something special...
							if (LSegment.IsIdentical(*OtherSegment))
								OtherSegment->CanSkipBSP = TRUE;
							// Check if could clip the side of the inner sector if it only extrudes extra floor OR ceiling space.
							else if (OtherSegment->FloorHeight == LSegment.FloorHeight)
								OtherSegment->FloorHeight = LSegment.CeilingHeight;
							else OtherSegment->CeilingHeight = LSegment.FloorHeight;
							return;
						}
						else if (OtherSegment->FloorHeight >= LSegment.FloorHeight && OtherSegment->CeilingHeight <= LSegment.CeilingHeight) // Can create one or two solid pillars inwards.
						{
							OtherSegment->CreateTopBottom = TRUE;
							return;
						}
					}
				}
			}
		}
		unguard;

		FVector BasePos(0.f, 0.f, 0.f);

		// Get sector bounding box center for brush position.
		{
			FBox BoundBox(FVector(LSegment.BoundsMin, 0.f), FVector(LSegment.BoundsMax, 0.f));
			BasePos = BoundBox.GetCentroid();
			BasePos.Z = FlrHeight + (CeilHeight - FlrHeight) * 0.5f;
			BasePos = BasePos.GridSnap(FVector(16.f, 16.f, 16.f));
		}

		FBSPOutput BSPOut(FlrHeight, CeilHeight, Sector.floorpic, Sector.ceilingpic, BasePos);

		const UBOOL IsFloorMv = Sector.TriggerData && (Sector.TriggerData->TriggerFlags & SPLT_FloorMover) != 0;
		const UBOOL IsCeilMv = Sector.TriggerData && (Sector.TriggerData->TriggerFlags & SPLT_CeilMover) != 0;
		const UBOOL IsWaterSector = !IsFloorMv && !IsCeilMv && Sector.floorpic->IsWater();
		UBOOL bHasTopBtm = FALSE;

		// Init brush
		guard(InitMainBrush);
		if (!LSegment.CanSkipBSP)
		{
			if (iBrushIndex != INDEX_NONE)
			{
				if (iBrush == iBrushIndex)
					LSegment.Dump2DShape(Out, BasePos);
				if (LSegment.CreateTopBottom)
				{
					INT Res = LSegment.GetTopCeiling();
					if (Res > LSegment.CeilingHeight)
						++iBrush;
					Res = LSegment.GetBottomFloor();
					if (Res < LSegment.FloorHeight)
						++iBrush;
				}
				else ++iBrush;
				return;
			}

			if (LSegment.CreateTopBottom)
			{
				INT Res = LSegment.GetTopCeiling();
				if (Res > LSegment.CeilingHeight && !IsCeilMv)
				{
					const FLOAT ZDiff = BSPOut.GetZHeightDiff(LSegment.CeilingHeight, Res);
					FBSPOutput::FBrushOutput* BSP = BSPOut.AppendBrush(FVector(0.f, 0.f, ZDiff), FBSPOutput::ECSGWriteMode::CSG_SemiSolidTop, TEXT("Brush"), iBrush++, iModel++);
					BSP->SetBounds(LSegment.CeilingHeight, Res);
					BSP->bAllowSplitter = FALSE;
					BSP->SwapFloorCeilTex();
					bHasTopBtm = TRUE;
				}
				Res = LSegment.GetBottomFloor();
				if (Res < LSegment.FloorHeight && !IsFloorMv)
				{
					const FLOAT ZDiff = BSPOut.GetZHeightDiff(Res, LSegment.FloorHeight);
					FBSPOutput::FBrushOutput* BSP = BSPOut.AppendBrush(FVector(0.f, 0.f, ZDiff), FBSPOutput::ECSGWriteMode::CSG_SemiSolidBtm, TEXT("Brush"), iBrush++, iModel++);
					BSP->SetBounds(Res, LSegment.FloorHeight);
					BSP->bAllowSplitter = FALSE;
					BSP->SwapFloorCeilTex();
					bHasTopBtm = TRUE;
				}
			}
			else
			{
				//bDebugOutput = (iBrush == 4);
				BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), WriteBSPMode, TEXT("Brush"), iBrush++, iModel++);
			}
		}
		unguard;

		if (Sector.IsSecretArea())
			BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), FBSPOutput::ECSGWriteMode::CSG_Volume, TEXT("DoomSecretVolume"), iVolume++, iModel++);

		guard(CreateDamageVolumes);
		const INT ZoneDPS = Sector.GetDamagePerSec();
		if (ZoneDPS == INDEX_NONE)
		{
			FBSPOutput::FBrushOutput* Res = BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), FBSPOutput::ECSGWriteMode::CSG_Volume, TEXT("DoomKillExit"), iVolume++, iModel++);
			Res->Out.Log(TEXT("\t\tZoneAreaType=DZONE_Shape\r\n"));
		}
		else if (IsWaterSector)
		{
			FBSPOutput::FBrushOutput* Res = BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), FBSPOutput::ECSGWriteMode::CSG_Volume, TEXT("DynamicWaterZoneInfo"), iVolume++, iModel++);
			Res->Out.Log(TEXT("\t\tZoneAreaType=DZONE_Shape\r\n"));
			if (ZoneDPS)
			{
				Res->Out.Log(TEXT("\t\tbPainZone=True\r\n"));
				Res->Out.Logf(TEXT("\t\tDamageType=\"%ls\"\r\n"), Sector.GetDamageName());
				Res->Out.Logf(TEXT("\t\tDamagePerSec=%i\r\n"), ZoneDPS);
			}
			Res->CeilingHeight = FlrHeight + 8.f;
		}
		else if (ZoneDPS)
		{
			FBSPOutput::FBrushOutput* Res = BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), FBSPOutput::ECSGWriteMode::CSG_Volume, TEXT("DynamicZoneInfo"), iVolume++, iModel++);
			Res->Out.Log(TEXT("\t\tZoneAreaType=DZONE_Shape\r\n"));
			Res->Out.Log(TEXT("\t\tbPainZone=True\r\n"));
			Res->Out.Logf(TEXT("\t\tDamageType=\"%ls\"\r\n"), Sector.GetDamageName());
			Res->Out.Logf(TEXT("\t\tDamagePerSec=%i\r\n"), ZoneDPS);
			Res->CeilingHeight = FlrHeight + 8.f;
		}
		unguard;

		guard(CreateMovers);
		const INT MoverSize = appRound(HeightDif);
		if (IsFloorMv)
		{
			FBSPOutput::FBrushOutput* Res = BSPOut.AppendBrush(FVector(0.f, 0.f, -HeightDif), FBSPOutput::ECSGWriteMode::CSG_Mover, TEXT("DoomMover"), iMover++, iModel++, Sector.tag);
			Res->SwapFloorCeilTex();
			SetupSectorMover(Res->Out, LSegment);
		}
		if (IsCeilMv)
		{
			FBSPOutput::FBrushOutput* Res = BSPOut.AppendBrush(FVector(0.f, 0.f, HeightDif), FBSPOutput::ECSGWriteMode::CSG_Mover, TEXT("DoomMover"), iMover++, iModel++, Sector.tag);
			Res->SwapFloorCeilTex();
			SetupSectorMover(Res->Out, LSegment);
		}
		unguard;

		UBOOL bWasSolidSector = FALSE;
		guard(CreateBlockingSectors);
		if (LSegment.SectorIsSolid())
		{
			bWasSolidSector = TRUE;
			BSPOut.AppendBrush(FVector(0.f, 0.f, 0.f), FBSPOutput::ECSGWriteMode::CSG_Volume, TEXT("BlockingVolume"), iVolume++, iModel++);
		}
		unguard;

		if (BSPOut.HasOutput())
		{
			guard(CreateWalls);
			if (LSegment.NumFloorLines)
			{
				const INT FloorLines = LSegment.NumFloorLines;
				const INT* FlLinesPtr = LSegment.FloorLines;

				if (LSegment.IsConvex)
				{
					BSPOut.BeginTris();
					for (iSurf = (FloorLines - 1); iSurf >= 0; --iSurf)
						BSPOut.AddVertex(GVertexList[FlLinesPtr[iSurf]]);
				}
				else
				{
					// Concave, so must tesselate it!
					debugf(TEXT("Tesselate Brush%i with %i lines!"), iBrush - 1, FloorLines);

					FSimpleTesselator Tesselator;
					Tesselator.Input.SetSize(FloorLines);
					for (iSurf = 0; iSurf < FloorLines; ++iSurf)
						Tesselator.Input(iSurf) = GVertexList[FlLinesPtr[iSurf]];
					Tesselator.Tesselate();
					const FVector2D* gVerts = &Tesselator.Input(0);

					for (iSurf = 0; iSurf < Tesselator.Output.Num(); ++iSurf)
					{
						BSPOut.BeginTris();
						const INT nVerts = Tesselator.Output(iSurf).Verts.Num();
						const INT* iVerts = &Tesselator.Output(iSurf).Verts(0);
						for (INT i = (nVerts - 1); i >= 0; --i)
							BSPOut.AddVertex(gVerts[iVerts[i]]);
					}
				}
			}
			BSPOut.BuildBase(GVertexList[LSegment.FloorLines[0]]);

			// Walls
			for (iSurf = 0; iSurf < NumLines; ++iSurf)
			{
				const FWallLine& Wall = *Lines[iSurf];
				const mapseg_t& seq = *Wall.seq;
				const sector_t* OtherSide = Wall.OtherSector;
				side_t* WallSideDef = (LSegment.IsCCW && seq.othersidedef) ? seq.othersidedef : seq.sidedef;
				side_t* OtherSideDef = (seq.othersidedef) ? seq.othersidedef : seq.sidedef;
				FWallTextures WallTex(*seq.linedef, seq.sidedef, seq.othersidedef);
				FWallTextures InvTex = WallTex;
				const FVector2D& A = GVertexList[Lines[iSurf]->Start];
				const FVector2D& B = GVertexList[Lines[iSurf]->End];
				if (seq.othersidedef)
					InvTex = FWallTextures(*seq.linedef, seq.othersidedef, seq.sidedef);
				if (LSegment.IsCCW)
					WallTex = InvTex;

				FLOAT TexAlignTop = (Wall.seq->linedef->IsUnpegFloor() ? Sector.floorheight : Sector.ceilingheight);
				FLOAT TexAlignBtm = TexAlignTop;
				if (seq.linedef->IsTwoSided() && OtherSide)
				{
					TexAlignTop = (!seq.linedef->IsUnpegCeil() ? Max(Sector.ceilingheight, OtherSide->ceilingheight) : Min(Sector.ceilingheight, OtherSide->ceilingheight));
					TexAlignBtm = (seq.linedef->IsUnpegFloor() ? Min(Sector.floorheight, OtherSide->floorheight) : Max(Sector.floorheight, OtherSide->floorheight));
				}

				DEBUG_MSG(TEXT("Line %i (sectors %i/%i) - Height %i - %i WallTex %ls,%ls,%ls (UpUnPeg %i BtmUnPeg %i)"), iSurf, LSegment.sector.SectorIndex, OtherSide ? OtherSide->SectorIndex : INDEX_NONE,
					OtherSide ? INT(OtherSide->floorheight) : INDEX_NONE, OtherSide ? INT(OtherSide->ceilingheight) : INDEX_NONE, seq.sidedef->GetBottomTex()->Describe(), seq.sidedef->GetMidTex()->Describe(), seq.sidedef->GetTopTex()->Describe(),
					seq.linedef->IsUnpegCeil(), seq.linedef->IsUnpegFloor());

#if DUMP_DEBUG_SHAPES
				if (GDebugOutput)
				{
					const FVector2D rA = A * SCALE_FACTOR;
					const FVector2D rB = B * SCALE_FACTOR;
					debugf(TEXT(" Verts %f,%f - %f,%f"), rA.X, rA.Y, rB.X, rB.Y);
				}
#endif

				if (bHasTopBtm)
				{
					BSPOut.AddSide(A, B, InvTex, FWallTextures::WALLTEX_TopTex, *OtherSideDef, TexAlignTop, FBSPOutput::ECSGOutputDest::CSGW_Top);
					BSPOut.AddSide(A, B, InvTex, FWallTextures::WALLTEX_BottomTex, *OtherSideDef, TexAlignBtm, FBSPOutput::ECSGOutputDest::CSGW_Bottom);
				}
				if (IsFloorMv || IsCeilMv)
				{
					if (IsFloorMv)
						BSPOut.AddSide(A, B, InvTex, FWallTextures::WALLTEX_BottomTex, *OtherSideDef, TexAlignBtm, FBSPOutput::ECSGOutputDest::CSGW_Mover);
					else BSPOut.AddSide(A, B, InvTex, FWallTextures::WALLTEX_TopTex, *OtherSideDef, TexAlignTop, FBSPOutput::ECSGOutputDest::CSGW_Mover);
				}
				if (!OtherSide) // Ordinary wall...
				{
					DEBUG_MSG(TEXT("  NormalWall"));
					BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_Default, *WallSideDef, TexAlignTop, FBSPOutput::ECSGOutputDest::CSGW_Brush);
					continue;
				}
				if (OtherSide->floorheight == OtherSide->ceilingheight && OtherSide->floorheight == Sector.floorheight && (OtherSide->ceilingpic->Type == TT_Sky)) // Skybox wall.
				{
					DEBUG_MSG(TEXT("  SkyWall"));
					BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_ForcedSky, *WallSideDef, 0.f, FBSPOutput::ECSGOutputDest::CSGW_Brush);
					continue;
				}
				if (OtherSide->floorheight == OtherSide->ceilingheight && !OtherSide->TriggerData && LSegment.sector.floorheight > OtherSide->floorheight && LSegment.sector.ceilingheight < OtherSide->ceilingheight) // Nothing but a texture splitter on a wall.
				{
					DEBUG_MSG(TEXT("  SplitTopBtm"));
					//const UBOOL bOtherSky = (OtherSide->ceilingpic->Type == TT_Sky);
					BSPOut.AddSideSplitted(A, B, OtherSide->floorheight, WallTex, *WallSideDef, TexAlignTop, TexAlignBtm);
					continue;
				}
				if (!OtherSide->TriggerData || !OtherSide->TriggerData->IsMover())
				{
					if (OtherSide->floorheight <= Sector.floorheight)
					{
						DEBUG_MSG(TEXT("  Top"));
						BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_TopTex, *WallSideDef, TexAlignTop, FBSPOutput::ECSGOutputDest::CSGW_Brush);
					}
					else if (OtherSide->ceilingheight >= Sector.ceilingheight)
					{
						DEBUG_MSG(TEXT("  Bottom"));
						BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_BottomTex, *WallSideDef, TexAlignBtm, FBSPOutput::ECSGOutputDest::CSGW_Brush);
					}
					else
					{
						DEBUG_MSG(TEXT("  SplitTopBtm2"));
						BSPOut.AddSideSplitted(A, B, OtherSide->floorheight, WallTex, *WallSideDef, TexAlignTop, TexAlignBtm);
					}
				}
				else if (OtherSide->TriggerData->TriggerFlags & SPLT_CeilMover)
				{
					DEBUG_MSG(TEXT("  Bottom (top mover)"));
					BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_BottomTex, *WallSideDef, TexAlignBtm, FBSPOutput::ECSGOutputDest::CSGW_Brush);
				}
				else
				{
					DEBUG_MSG(TEXT("  Top (floor mover)"));
					BSPOut.AddSide(A, B, WallTex, FWallTextures::WALLTEX_TopTex, *WallSideDef, TexAlignTop, FBSPOutput::ECSGOutputDest::CSGW_Brush);
				}
			}
			unguard;

			// Exit brush
			BSPOut.FinishBrush(&Out);
		}

#define WRITE_VERTEX(out,vert) out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), vert.X * SCALE_FACTOR, vert.Y * SCALE_FACTOR, vert.Z * SCALE_FACTOR);

		guard(CreateSheets);
		if (LSegment.sector.floorheight != LSegment.sector.ceilingheight)
		{
			FlrHeight = LSegment.sector.floorheight;
			CeilHeight = LSegment.sector.ceilingheight;

			// Check if brush contain two-sided lines.
			for (iSurf = 0; iSurf < NumLines; ++iSurf)
			{
				const FWallLine& Wall = *Lines[iSurf];
				if (!Wall.OtherSector || !LSegment.sector.IsSmallerThan(*Wall.OtherSector))
					continue;

				const FVector2D& StartPoint = GVertexList[Lines[iSurf]->Start];
				const FVector2D& EndPoint = GVertexList[Lines[iSurf]->End];
				if ((Wall.seq->linedef->flags & (ML_TWOSIDED | ML_BLOCKING | ML_DONTDRAW)) == (ML_TWOSIDED | ML_BLOCKING))
				{
					FWallTextures WallTex(*Wall.seq->linedef, Wall.seq->sidedef, Wall.seq->othersidedef);
					const side_t* SideDef = Wall.seq->sidedef;
					FInstTexture* WallTexName = WallTex.GetTexture();

					if (WallTexName->IsValid())
					{
						FLOAT SheetFloor = Max(LSegment.sector.floorheight, Wall.OtherSector->floorheight);
						FLOAT SheetCeil = Min(LSegment.sector.ceilingheight, Wall.OtherSector->ceilingheight);
						if (Wall.seq->linedef->flags & ML_DONTPEGBOTTOM)
							SheetCeil = Min(SheetFloor + WallTexName->V + SideDef->rowoffset, SheetCeil);
						else SheetFloor = Max(SheetCeil - WallTexName->V + SideDef->rowoffset, SheetFloor);

						const FVector BasePos = FVector(StartPoint, SheetCeil);
						const FVector A = FVector(0.f, 0.f, 0.f);
						const FVector B = FVector(EndPoint, SheetCeil) - BasePos;
						const FVector C = FVector(EndPoint, SheetFloor) - BasePos;
						const FVector D = FVector(StartPoint, SheetFloor) - BasePos;

						LateBSP.Logf(TEXT("\tBegin Actor Class=Brush Name=Brush%i\r\n"), iBrush++);
						LateBSP.Logf(TEXT("\t\tPolyFlags=%i\r\n"), (PF_TwoSided | PF_NotSolid));
						LateBSP.Log(TEXT("\t\tCsgOper=CSG_Add\r\n"));
						LateBSP.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), BasePos.X * SCALE_FACTOR, BasePos.Y * SCALE_FACTOR, BasePos.Z * SCALE_FACTOR);
						LateBSP.Logf(TEXT("\t\tBegin Brush Name=Model%i\r\n"), iModel);
						LateBSP.Log(TEXT("\t\t\tBegin PolyList\r\n"));

						const FVector Normal = ((-B) ^ FVector(0.f, 0.f, 1.f)).SafeNormalSlow();
						const FVector TexU = B.SafeNormalSlow() * WallTexName->ScaleU;
						const FVector TexV = FVector(0.f, 0.f, -WallTexName->ScaleV);
						const FVector Origin(0.f, 0.f, 0.f);

						LateBSP.Logf(TEXT("\t\t\t\tBegin Polygon Texture=%ls Flags=%u\r\n"), WallTexName->GetFileName(), WallTex.GetTextureFlags());
						LateBSP.Logf(TEXT("\t\t\t\t\tOrigin    %f,%f,%f\r\n"), Origin.X * SCALE_FACTOR, Origin.Y * SCALE_FACTOR, Origin.Z * SCALE_FACTOR);
						LateBSP.Logf(TEXT("\t\t\t\t\tNormal    %f,%f,%f\r\n"), Normal.X, Normal.Y, Normal.Z);
						LateBSP.Logf(TEXT("\t\t\t\t\tTextureU  %f,%f,%f\r\n"), TexU.X / SCALE_FACTOR, TexU.Y / SCALE_FACTOR, TexU.Z / SCALE_FACTOR);
						LateBSP.Logf(TEXT("\t\t\t\t\tTextureV  %f,%f,%f\r\n"), TexV.X / SCALE_FACTOR, TexV.Y / SCALE_FACTOR, TexV.Z / SCALE_FACTOR);
						LateBSP.Log(TEXT("\t\t\t\t\tPan       U=0 V=0\r\n"));

						WRITE_VERTEX(LateBSP, A);
						WRITE_VERTEX(LateBSP, B);
						WRITE_VERTEX(LateBSP, C);
						WRITE_VERTEX(LateBSP, D);

						LateBSP.Log(TEXT("\t\t\t\tEnd Polygon\r\n"));

						LateBSP.Log(TEXT("\t\t\tEnd PolyList\r\n"));
						LateBSP.Log(TEXT("\t\tEnd Brush\r\n"));
						LateBSP.Logf(TEXT("\t\tBrush=Model'MyLevel.Model%i'\r\n"), iModel++);
						LateBSP.Log(TEXT("\tEnd Actor\r\n"));
					}
				}
				if ((Wall.seq->linedef->flags & (ML_TWOSIDED | ML_BLOCKING)) == (ML_TWOSIDED | ML_BLOCKING) && !Wall.OtherSector->Shape->SectorIsSolid() && !bWasSolidSector)
				{
					const INT iPrevSurf = iSurf ? (iSurf - 1) : (NumLines - 1);
					const FVector BasePos = FVector(StartPoint, CeilHeight);
					const FVector A = FVector(0.f, 0.f, 0.f);
					const FVector B = FVector(EndPoint, CeilHeight) - BasePos;
					const FVector C = FVector(EndPoint, FlrHeight) - BasePos;
					const FVector D = FVector(StartPoint, FlrHeight) - BasePos;

					LateBSP.Logf(TEXT("\tBegin Actor Class=BlockingVolume Name=BlockingVolume%i\r\n"), iVolume++);
					LateBSP.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), BasePos.X* SCALE_FACTOR, BasePos.Y* SCALE_FACTOR, BasePos.Z* SCALE_FACTOR);
					LateBSP.Logf(TEXT("\t\tBegin Brush Name=Model%i\r\n"), iModel);
					LateBSP.Log(TEXT("\t\t\tBegin PolyList\r\n"));

					const FVector sideAxis = (-B).SafeNormalSlow();
					const FVector fwdAxis = ((-B) ^ FVector(0.f, 0.f, 1.f)).SafeNormalSlow();
					const FVector Origin(0.f, 0.f, 0.f);
					FVector TexU, TexV;

					const FVector nOffset = fwdAxis * 2.f;
					const FVector Pts[8] = { A + nOffset,B + nOffset,C + nOffset,D + nOffset,A - nOffset,B - nOffset,C - nOffset,D - nOffset };
					const INT PtList[6][4] = { {0,1,2,3}, {7,6,5,4}, {4,0,3,7}, {1,5,6,2}, {3,2,6,7}, {4,5,1,0} };
					const FVector sNormals[6] = { fwdAxis, -fwdAxis, sideAxis, -sideAxis, FVector(0,0,-1), FVector(0,0,1) };

					for (INT iSide = 0; iSide < 6; ++iSide)
					{
						const FVector& Normal = sNormals[iSide];
						Normal.FindBestAxisVectors(TexU, TexV);
						LateBSP.Log(TEXT("\t\t\t\tBegin Polygon\r\n"));
						LateBSP.Logf(TEXT("\t\t\t\t\tOrigin    %f,%f,%f\r\n"), Origin.X, Origin.Y, Origin.Z);
						LateBSP.Logf(TEXT("\t\t\t\t\tNormal    %f,%f,%f\r\n"), Normal.X, Normal.Y, Normal.Z);
						LateBSP.Logf(TEXT("\t\t\t\t\tTextureU  %f,%f,%f\r\n"), TexU.X, TexU.Y, TexU.Z);
						LateBSP.Logf(TEXT("\t\t\t\t\tTextureV  %f,%f,%f\r\n"), TexV.X, TexV.Y, TexV.Z);
						LateBSP.Log(TEXT("\t\t\t\t\tPan       U=0 V=0\r\n"));

						for (INT vt = 0; vt < 4; ++vt)
							WRITE_VERTEX(LateBSP, Pts[PtList[iSide][vt]]);

						LateBSP.Log(TEXT("\t\t\t\tEnd Polygon\r\n"));
					}

					LateBSP.Log(TEXT("\t\t\tEnd PolyList\r\n"));
					LateBSP.Log(TEXT("\t\tEnd Brush\r\n"));
					LateBSP.Logf(TEXT("\t\tBrush=Model'MyLevel.Model%i'\r\n"), iModel++);
					LateBSP.Log(TEXT("\tEnd Actor\r\n"));
				}
			}
		}
		unguard;

		guard(SetupExitSecrets);
		if (ExitSector && ExitSector == &LSegment.sector)
		{
			ExitSector = NULL;

			Out.Logf(TEXT("\tBegin Actor Class=DoomEndLevelTrigger Name=DoomEndLevelTrigger%i\r\n"), iTeleporter++);
			Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), BasePos.X * SCALE_FACTOR, BasePos.Y * SCALE_FACTOR, BasePos.Z * SCALE_FACTOR);
			Out.Log(TEXT("\t\tTag=LevelExit\r\n"));

			FMapInfo* NextMap = NULL;
			if (Contents.IsEpisodic())
			{
				if (EditMap->iMap == 9)
				{
					INT iNext = 1;
					switch (EditMap->iEpisode)
					{
					case 1:
						iNext = 3;
						break;
					case 2:
						iNext = 5;
						break;
					case 3:
						iNext = 6;
						break;
					case 4:
						iNext = 2;
						break;
					}
					NextMap = FindMap(EditMap->iEpisode, iNext);
				}
				else if (EditMap->iMap == 8)
					NextMap = FindMap(EditMap->iEpisode + 1, 1);
				else NextMap = FindMap(EditMap->iEpisode, EditMap->iMap + 1);
			}
			else if (EditMap->iMap >= 31)
				NextMap = FindMap(0, 15);
			else NextMap = FindMap(0, EditMap->iMap + 1);

			if (NextMap)
				Out.Logf(TEXT("\t\tURL=%ls\r\n"), *NextMap->MapFilename);
			Out.Log(TEXT("\tEnd Actor\r\n"));
		}
		if (SecretSector && SecretSector == &LSegment.sector)
		{
			SecretSector = NULL;

			Out.Logf(TEXT("\tBegin Actor Class=DoomEndLevelTrigger Name=DoomEndLevelTrigger%i\r\n"), iTeleporter++);
			Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), BasePos.X * SCALE_FACTOR, BasePos.Y * SCALE_FACTOR, BasePos.Z * SCALE_FACTOR + 16.f);
			Out.Log(TEXT("\t\tTag=SecretExit\r\n"));

			FMapInfo* NextMap = NULL;
			if (Contents.IsEpisodic())
				NextMap = FindMap(EditMap->iEpisode, 9);
			else if (EditMap->iMap == 31)
				NextMap = FindMap(0, 32);
			else NextMap = FindMap(0, 31);

			if (NextMap)
				Out.Logf(TEXT("\t\tURL=%ls\r\n"), *NextMap->MapFilename);
			Out.Log(TEXT("\tEnd Actor\r\n"));
		}
		unguard;
		unguardf((TEXT("(Shape %i/%i)"), iSegment, Connector.ShapeList.Num()));
	}
	INT GetMoverTravelHeight(const FSectorShape& LSegment)
	{
		TArray<const sector_t*> NeighBors;
		{
			const INT NumLines = LSegment.NumLines;
			FWallLine** Lines = LSegment.Lines;
			for (INT i = 0; i < NumLines; ++i)
				if (Lines[i]->OtherSector)
					NeighBors.AddUniqueItem(Lines[i]->OtherSector);
		}
		if (NeighBors.Num() == 0)
			return 0;
		const INT BaseCeiling = LSegment.sector.ceilingheight;
		const INT BaseFloor = LSegment.sector.floorheight;
		UBOOL bTargetCeil = TRUE;
		INT TargetHeight = 0;
		switch (LSegment.sector.TriggerData->MoveType)
		{
		case MFT_LowestCeiling:
		{
			TargetHeight = 10000;
			for (INT i = 0; i < NeighBors.Num(); ++i)
			{
				if (NeighBors(i)->ceilingheight > BaseCeiling)
					TargetHeight = Min<INT>(TargetHeight, NeighBors(i)->ceilingheight);
			}
			TargetHeight -= 8;
			break;
		}
		case MFT_CloseToFloor:
			TargetHeight = LSegment.sector.floorheight;
			bTargetCeil = FALSE;
			break;
		case MFT_CrushToFloor:
			TargetHeight = LSegment.sector.floorheight + 8;
			bTargetCeil = FALSE;
			break;
		case MFT_HighestCeiling:
		{
			INT LCeil = LSegment.sector.ceilingheight;
			for (INT i = 0; i < NeighBors.Num(); ++i)
				LCeil = Max<INT>(LCeil, NeighBors(i)->ceilingheight);
			TargetHeight = LCeil;
			break;
		}
		case MFT_NearestFloorUp:
		{
			INT NFlr = LSegment.sector.floorheight + 10000;
			for (INT i = 0; i < NeighBors.Num(); ++i)
			{
				if (NeighBors(i)->floorheight > LSegment.sector.floorheight)
					NFlr = Min<INT>(NFlr, NeighBors(i)->floorheight);
			}
			TargetHeight = NFlr;
			bTargetCeil = FALSE;
			break;
		}
		case MFT_NearestFloorDown:
		{
			INT NFlr = LSegment.sector.floorheight - 10000;
			for (INT i = 0; i < NeighBors.Num(); ++i)
			{
				if (NeighBors(i)->floorheight < LSegment.sector.floorheight)
					NFlr = Max<INT>(NFlr, NeighBors(i)->floorheight);
			}
			TargetHeight = NFlr;
			bTargetCeil = FALSE;
			break;
		}
		case MFT_LowestFloor:
		{
			INT NFlr = LSegment.sector.floorheight;
			for (INT i = 0; i < NeighBors.Num(); ++i)
				NFlr = Min<INT>(NFlr, NeighBors(i)->floorheight);
			TargetHeight = NFlr;
			bTargetCeil = FALSE;
			break;
		}
		case MFT_HighestFloor:
		{
			TargetHeight = -10000;
			for (INT i = 0; i < NeighBors.Num(); ++i)
			{
				if (NeighBors(i)->floorheight < BaseFloor)
					TargetHeight = Max<INT>(TargetHeight, NeighBors(i)->floorheight);
			}
			bTargetCeil = FALSE;
			break;
		}
		case MFT_CrushToCeiling:
			TargetHeight = LSegment.sector.ceilingheight - 8;
			break;
		}
		if (bTargetCeil)
			TargetHeight = TargetHeight - LSegment.sector.ceilingheight;
		else TargetHeight = TargetHeight - LSegment.sector.floorheight;
		return TargetHeight;
	}
	void SetupSectorMover(FOutputDevice& Out, const FSectorShape& LSegment)
	{
		FLOAT TravelDist = FLOAT(GetMoverTravelHeight(LSegment)) * SCALE_FACTOR;
		FLOAT MoveSpeed = GAME_FPS * SCALE_FACTOR;
		switch (LSegment.sector.TriggerData->MoverSpeed)
		{
		case DMS_NormalDoor:
			Out.Logf(TEXT("\t\tOpeningSound=Sound'%ls.DSDOROPN'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosingSound=Sound'%ls.DSDORCLS'\r\n"), *Contents.TexturePackageName);
			MoveSpeed *= 2.f;
			break;
		case DMS_BlazingDoor:
			MoveSpeed *= 8.f;
			Out.Logf(TEXT("\t\tOpeningSound=Sound'%ls.DSBDOPN'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosingSound=Sound'%ls.DSBDCLS'\r\n"), *Contents.TexturePackageName);
			break;
		case DMS_NormalFloor:
			Out.Logf(TEXT("\t\tMoveAmbientSound=Sound'%ls.DSSTNMOV'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tOpenedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			break;
		case DMS_NormalPlatform:
			MoveSpeed *= 4.f;
			Out.Logf(TEXT("\t\tOpeningSound=Sound'%ls.DSPSTART'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosingSound=Sound'%ls.DSPSTART'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tOpenedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			break;
		case DMS_BlazingPlatform:
			MoveSpeed *= 16.f;
			Out.Logf(TEXT("\t\tOpeningSound=Sound'%ls.DSPSTART'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosingSound=Sound'%ls.DSPSTART'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tOpenedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			Out.Logf(TEXT("\t\tClosedSound=Sound'%ls.DSPSTOP'\r\n"), *Contents.TexturePackageName);
			break;
		}
		Out.Logf(TEXT("\t\tMoveTime=%f\r\n"), Abs(TravelDist / MoveSpeed));
		Out.Logf(TEXT("\t\tKeyPos(1)=(X=0.0,Y=0.0,Z=%f)\r\n"), TravelDist);
		if (LSegment.sector.TriggerData->TriggerFlags & SPLT_TriggerOnce)
			Out.Log(TEXT("\t\tbTriggerOnceOnly=True\r\n"));
	}
	static void WriteBuilderBrush(FOutputDevice& Out)
	{
		guard(WriteBuilderBrush);
		Out.Log(TEXT("\tBegin Actor Class=Brush Name=Brush0\r\n"));
		Out.Log(TEXT("\t\tMainScale=(SheerAxis=SHEER_ZX)\r\n"));
		Out.Log(TEXT("\t\tPostScale=(SheerAxis=SHEER_ZX)\r\n"));
		Out.Log(TEXT("\t\tBegin Brush Name=Brush\r\n"));
		WriteCubeBrush(FVector(0.f, 0.f, 0.f), Out);
		Out.Log(TEXT("\t\tEnd Brush\r\n"));
		Out.Log(TEXT("\t\tBrush=Model'MyLevel.Brush'\r\n"));
		Out.Log(TEXT("\tEnd Actor\r\n"));
		unguard;
	}
	static void WriteLevelBrush(const FVector& Offset, FOutputDevice& Out, INT Index)
	{
		guard(WriteLevelBrush);
		Out.Logf(TEXT("\tBegin Actor Class=Brush Name=Brush%i\r\n"), Index+1);
		Out.Log(TEXT("\t\tMainScale=(SheerAxis=SHEER_ZX)\r\n"));
		Out.Log(TEXT("\t\tPostScale=(SheerAxis=SHEER_ZX)\r\n"));
		Out.Log(TEXT("\t\tCsgOper=CSG_Subtract\r\n"));
		Out.Logf(TEXT("\t\tBegin Brush Name=Model%i\r\n"), Index + 2);
		WriteCubeBrush(Offset, Out);
		Out.Log(TEXT("\t\tEnd Brush\r\n"));
		Out.Logf(TEXT("\t\tBrush=Model'MyLevel.Model%i'\r\n"), Index + 2);
		Out.Log(TEXT("\tEnd Actor\r\n"));
		Out.Logf(TEXT("\tBegin Actor Class=ZoneInfo Name=ZoneInfo%i\r\n"), Index);
		Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), Offset.X, Offset.Y, Offset.Z);
		Out.Log(TEXT("\t\tRemoteRole=ROLE_None\r\n"));
		Out.Log(TEXT("\tEnd Actor\r\n"));
		unguard;
	}
	inline static void WriteFace(FOutputDevice& Out, const FVector& A, const FVector& B, const FVector& C, const FVector& D)
	{
		const FVector Normal = ((B - A) ^ (C - A)).SafeNormalSlow();
		const FVector AxisA = (B - A).SafeNormalSlow();
		const FVector AxisB = (AxisA ^ Normal);

		Out.Log(TEXT("\t\t\t\tBegin Polygon\r\n"));
		Out.Logf(TEXT("\t\t\t\t\tOrigin    %f,%f,%f\r\n"), A.X * SCALE_FACTOR, A.Y * SCALE_FACTOR, A.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\t\t\t\tNormal    %f,%f,%f\r\n"), Normal.X, Normal.Y, Normal.Z);
		Out.Logf(TEXT("\t\t\t\t\tTextureU  %f,%f,%f\r\n"), AxisA.X * SCALE_FACTOR, AxisA.Y * SCALE_FACTOR, AxisA.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\t\t\t\tTextureV  %f,%f,%f\r\n"), AxisB.X * SCALE_FACTOR, AxisB.Y * SCALE_FACTOR, AxisB.Z * SCALE_FACTOR);
		Out.Log(TEXT("\t\t\t\t\tPan       U=0 V=0\r\n"));
		Out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), A.X * SCALE_FACTOR, A.Y * SCALE_FACTOR, A.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), B.X * SCALE_FACTOR, B.Y * SCALE_FACTOR, B.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), C.X * SCALE_FACTOR, C.Y * SCALE_FACTOR, C.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\t\t\t\tVertex    %f,%f,%f\r\n"), D.X * SCALE_FACTOR, D.Y * SCALE_FACTOR, D.Z * SCALE_FACTOR);
		Out.Log(TEXT("\t\t\t\tEnd Polygon\r\n"));
	}
	static void WriteLineBrush(FOutputDevice& Out, const FVector2D& PointA, const FVector2D& PointB, FLOAT Bottom, FLOAT Top, const INT BrushIndex)
	{
		FVector MidPoint = (FVector(PointA, Bottom) + FVector(PointB, Top)) * 0.5f;

		Out.Logf(TEXT("\t\tLocation=(X=%f,Y=%f,Z=%f)\r\n"), MidPoint.X * SCALE_FACTOR, MidPoint.Y * SCALE_FACTOR, MidPoint.Z * SCALE_FACTOR);
		Out.Logf(TEXT("\t\tBegin Brush Name=Model%i\r\n"), BrushIndex);
		Out.Log(TEXT("\t\t\tBegin PolyList\r\n"));

		FVector2D Dir(PointB - PointA);
		Dir.Normalize();
		constexpr FLOAT Depth = 4.f;
		const FVector2D OutAxis = FVector2D(-Dir.Y, Dir.X);
		
		const FVector2D A = PointA + (OutAxis * Depth) - FVector2D(MidPoint);
		const FVector2D B = PointB + (OutAxis * Depth) - FVector2D(MidPoint);
		const FVector2D C = PointB - (OutAxis * Depth) - FVector2D(MidPoint);
		const FVector2D D = PointA - (OutAxis * Depth) - FVector2D(MidPoint);

		Bottom -= MidPoint.Z;
		Top -= MidPoint.Z;
		WriteFace(Out, FVector(A, Bottom), FVector(B, Bottom), FVector(B, Top), FVector(A, Top));
		WriteFace(Out, FVector(D, Bottom), FVector(C, Bottom), FVector(C, Top), FVector(D, Top));
		WriteFace(Out, FVector(A, Bottom), FVector(D, Bottom), FVector(D, Top), FVector(A, Top));
		WriteFace(Out, FVector(B, Bottom), FVector(C, Bottom), FVector(C, Top), FVector(B, Top));
		WriteFace(Out, FVector(A, Bottom), FVector(B, Bottom), FVector(C, Bottom), FVector(D, Bottom));
		WriteFace(Out, FVector(D, Top), FVector(C, Top), FVector(B, Top), FVector(A, Top));

		Out.Log(TEXT("\t\t\tEnd PolyList\r\n"));
		Out.Log(TEXT("\t\tEnd Brush\r\n"));
		Out.Logf(TEXT("\t\tBrush=Model'MyLevel.Model%i'\r\n"), BrushIndex);
	}
	static void WriteCubeBrush(const FVector& Offset, FOutputDevice& Out)
	{
		Out.Log(TEXT("\t\t\tBegin PolyList\r\n"));

		constexpr FLOAT brExtent = 128.f;
		constexpr FLOAT Sides[6][3] = { {-1.f,0.f,0.f},{0.f,1.f,0.f},{1.f,0.f,0.f},{0.f,-1.f,0.f},{0.f,0.f,1.f},{0.f,0.f,-1.f} };
		for (INT iSide = 0; iSide < 6; ++iSide)
		{
			Out.Logf(TEXT("\t\t\t\tBegin Polygon Texture=Engine.DefaultTexture Flags=%i\r\n"), INT(PF_OccluderPoly));
			const FVector Normal(Sides[iSide][0], Sides[iSide][1], Sides[iSide][2]);
			const FVector MidPoint = Offset + (Normal * brExtent);
			FVector YAxis, ZAxis;
			if (Abs(Normal.X) > 0.5f)
				YAxis = FVector(0.f, -1.f, 0.f);
			else YAxis = FVector(-1.f, 0.f, 0.f);
			ZAxis = Normal ^ YAxis;

			const FVector Edges[4] = { MidPoint + (YAxis * brExtent) + (ZAxis * brExtent),
										MidPoint - (YAxis * brExtent) + (ZAxis * brExtent),
										MidPoint - (YAxis * brExtent) - (ZAxis * brExtent),
										MidPoint + (YAxis * brExtent) - (ZAxis * brExtent) };

			Out.Logf(TEXT("\t\t\t\t\tOrigin    %i,%i,%i\r\n"), appRound(Edges[0].X), appRound(Edges[0].Y), appRound(Edges[0].Z));
			Out.Logf(TEXT("\t\t\t\t\tNormal    %i,%i,%i\r\n"), appRound(Normal.X), appRound(Normal.Y), appRound(Normal.Z));
			Out.Logf(TEXT("\t\t\t\t\tTextureU  %i,%i,%i\r\n"), appRound(YAxis.X), appRound(YAxis.Y), appRound(YAxis.Z));
			Out.Logf(TEXT("\t\t\t\t\tTextureV  %i,%i,%i\r\n"), appRound(ZAxis.X), appRound(ZAxis.Y), appRound(ZAxis.Z));
			Out.Log(TEXT("\t\t\t\t\tPan       U=0 V=0\r\n"));
			for(INT i=0; i<4; ++i)
				Out.Logf(TEXT("\t\t\t\t\tVertex    %i,%i,%i\r\n"), appRound(Edges[i].X), appRound(Edges[i].Y), appRound(Edges[i].Z));
			Out.Log(TEXT("\t\t\t\tEnd Polygon\r\n"));
		}
		Out.Log(TEXT("\t\t\tEnd PolyList\r\n"));
	}
	static inline FVector GetLevelLocation(INT iLevel)
	{
		guard(GetLevelLocation);
		FVector Result(0.f, 0.f, 0.f);
		if (iLevel > 16)
		{
			Result.Y += 1024.f;
			Result.X = -2048.f;
			iLevel -= 16;
		}
		Result.X += (iLevel * 512.f);
		return Result;
		unguard;
	}
	UBOOL TryLoad(const TCHAR* MapName)
	{
		guardSlow(TryLoad);
		INT i = Contents.FindLump(MapName);
		if(i>=0)
		{
			PendingMap = new FMapInfo;
			if (!ProcessMap(i + 1))
			{
				delete PendingMap;
				GWarn->Logf(TEXT("Invalid mapdata for '%ls'"), MapName);
			}
			else
			{
				PendingMap->MapTitle = GetLevelName(Contents.GameNumber, Maps.Num());
				Maps.AddItem(PendingMap);
			}
			return TRUE;
		}
		return FALSE;
		unguardSlow;
	}
	UBOOL ProcessMap(INT LumpIndex)
	{
		guard(ProcessMap);
		while (LumpIndex < Contents.Lumps.Num())
		{
			const WADFileEntry& E = Contents.Lumps(LumpIndex);
			if (!appStrcmp(E.UName, TEXT("THINGS")))
				LoadThings(E);
			else if (!appStrcmp(E.UName, TEXT("LINEDEFS")))
				LoadLines(E);
			else if (!appStrcmp(E.UName, TEXT("SIDEDEFS")))
				LoadSides(E);
			else if (!appStrcmp(E.UName, TEXT("VERTEXES")))
				LoadVertices(E);
			else if (!appStrcmp(E.UName, TEXT("SEGS")))
				LoadSegments(E);
			else if (!appStrcmp(E.UName, TEXT("SSECTORS")))
				LoadSubSectors(E);
			else if (!appStrcmp(E.UName, TEXT("NODES")))
				LoadNodes(E);
			else if (!appStrcmp(E.UName, TEXT("SECTORS")))
				LoadSectors(E);
			else if (!appStrcmp(E.UName, TEXT("REJECT")))
				LoadReject(E);
			else if (!appStrcmp(E.UName, TEXT("BLOCKMAP")))
				LoadBlockmap(E);
			else break;
			++LumpIndex;
		}
		if (PendingMap->IsValid())
		{
			PendingMap->PostLoadMap();
			return TRUE;
		}
		return FALSE;
		unguard;
	}
	
#define LOAD_DATA(dataStride,dataAr,dataType) \
	const INT num = W.lenData / dataStride; \
	FileAr->Seek(W.offData); \
	PendingMap->Things.Reserve(num); \
	for (INT i = 0; i < num; i++) \
		new (PendingMap->dataAr) dataType(FileAr); \

	void LoadThings(const WADFileEntry& W)
	{
		guard(LoadThings);
		LOAD_DATA(THING_SIZE, Things, FThingEntry);
		unguard;
	}
	void LoadLines(const WADFileEntry& W)
	{
		guard(LoadLines);
		LOAD_DATA(LINEDEF_SIZE, Lines, line_t);
		unguard;
	}
	void LoadSides(const WADFileEntry& W)
	{
		guard(LoadSides);
		LOAD_DATA(SIDEDEF_SIZE, SideDefs, side_t);
		unguard;
	}
	void LoadVertices(const WADFileEntry& W)
	{
		guard(LoadVertices);
		LOAD_DATA(VERTEX_SIZE, Vertices, vertex_t);
		unguard;
	}
	void LoadSegments(const WADFileEntry& W)
	{
		guard(LoadSegments);
		LOAD_DATA(SEGMENT_SIZE, MapSegs, mapseg_t);
		unguard;
	}
	void LoadSubSectors(const WADFileEntry& W)
	{
		guard(LoadSubSectors);
		LOAD_DATA(SSECTOR_SIZE, SubSectors, subsector_s);
		unguard;
	}
	void LoadNodes(const WADFileEntry& W)
	{
		guard(LoadNodes);
		LOAD_DATA(WNODE_SIZE, Nodes, node_t);
		unguard;
	}
	void LoadSectors(const WADFileEntry& W)
	{
		guard(LoadSectors);
		LOAD_DATA(SECTOR_SIZE, Sectors, sector_t);
		unguard;
	}
	void LoadReject(const WADFileEntry& W)
	{
		guard(LoadReject);
		FileAr->Seek(W.offData);
		PendingMap->RejectMatrix.SetSize(W.lenData);
		FileAr->Serialize(&PendingMap->RejectMatrix(0), W.lenData);
		unguard;
	}
	void LoadBlockmap(const WADFileEntry& W)
	{
		guard(LoadBlockmap);
		FileAr->Seek(W.offData);
		PendingMap->BlockMap.SetSize(W.lenData / sizeof(WORD));
		FileAr->Serialize(&PendingMap->BlockMap(0), W.lenData);
		unguard;
	}
};

struct FPaletteMap
{
	FColor Palette[NUM_PAL_COLORS];
};
struct FColorMap
{
	BYTE C[NUM_PAL_COLORS];
};

struct FDoomTexture
{
	FString Name, OrgName;
	INT USize, VSize;
	const UBOOL bMasked;
	UBOOL IsAnimated, IsSprite, IsFlat;
	TArray<BYTE> Pix;
	UTexture* RefObject;
	FLOAT AnimRate;

	FDoomTexture() = delete;

	FDoomTexture(const maptexture_t& I)
		: USize(I.width), VSize(I.height), bMasked(I.masked != 0), IsAnimated(FALSE), IsSprite(FALSE), IsFlat(FALSE), RefObject(nullptr)
	{
		Name = ByteToStr(I.name, 8);
		OrgName = Name;
		Pix.SetSize(USize * VSize);
		appMemzero(&Pix(0), Pix.Num());
	}
	FDoomTexture(const TCHAR* InName) // Flat
		: Name(FString::Printf(TEXT("B_%ls"), InName)), OrgName(InName), USize(FlatResolution), VSize(FlatResolution), bMasked(FALSE), IsAnimated(FALSE), IsSprite(FALSE), IsFlat(TRUE), RefObject(nullptr)
	{
		Pix.SetSize(FlatTotalSize);
		appMemzero(&Pix(0), Pix.Num());
	}
	FDoomTexture(const TCHAR* InName, const patch_t& patch) // Sprite
		: Name(InName), OrgName(InName), USize(patch.width), VSize(patch.height), bMasked(FALSE), IsAnimated(FALSE), IsSprite(TRUE), IsFlat(FALSE), RefObject(nullptr)
	{
		Pix.SetSize(USize * VSize);
		appMemzero(&Pix(0), Pix.Num());
	}
	~FDoomTexture() noexcept(false)
	{
	}

	void MergeTextureAt(FDoomTexture* Other, const INT XOffset, const INT YOffset)
	{
		guard(FDoomTexture::MergeTextureAt);
		BYTE* SrcPix = &Other->Pix(0);
		BYTE* DestPix = &Pix(XOffset + (YOffset * USize));

		const INT SrcX = Other->USize;
		const INT DestX = USize;
		const INT SrcY = Other->VSize;

		for (INT y = 0; y < SrcY; ++y, SrcPix += SrcX, DestPix += DestX)
		{
			appMemcpy(DestPix, SrcPix, SrcX);
		}
		unguard;
	}
	void RescaleTexture(const INT NewX, const INT NewY)
	{
		guard(FDoomTexture::RescaleTexture);
		if (NewX == USize && NewY == VSize)
			return;

		TArray<BYTE> NPix(NewX * NewY);
		appMemzero(&NPix(0), NPix.Num());
		BYTE* Bit = &NPix(0);

		for (INT y = 0; y < VSize; ++y, Bit += NewX)
		{
			appMemcpy(Bit, &Pix(y * USize), USize);
		}
		NPix.ExchangeArray(&Pix);
		USize = NewX;
		VSize = NewY;
		unguardf((TEXT("(%ls)"), *Name));
	}
	UBOOL UpscaleToFit()
	{
		guardSlow(FDoomTexture::UpscaleToFit);
		const INT nx = FNextPowerOfTwo(USize);
		const INT ny = FNextPowerOfTwo(VSize);
		if (nx == USize && ny == VSize)
			return FALSE;

		RescaleTexture(nx, ny);
		return TRUE;
		unguardSlow;
	}
	UBOOL ResizeToFit(const FColor* Palette)
	{
		guard(FDoomTexture::ResizeToFit);
		const INT nx = FNextPowerOfTwo(USize);
		const INT ny = FNextPowerOfTwo(VSize);
		if (nx == USize && ny == VSize)
			return FALSE;
		const FLOAT xs = FLOAT(USize) / FLOAT(nx);
		const FLOAT ys = FLOAT(VSize) / FLOAT(ny);
		//debugf(TEXT("Texture '%ls' wasn't a power of two size (%ix%i), resizing..."), Name, USize, VSize);

		INT i, x, y;
		FPlane* PalPlanes = new FPlane[NUM_PAL_COLORS];
		guard(GetPalette);
		for (i = 0; i < NUM_PAL_COLORS; ++i)
			PalPlanes[i] = Palette[i].Plane();
		unguard;

		TArray<BYTE> NPix(nx * ny);

		FPlane* CPlanes = new FPlane[Pix.Num()];
		const BYTE* OPix = &Pix(0);
		guard(GetPixels);
		for (i = 0; i < Pix.Num(); ++i)
		{
			if (!OPix[i])
				CPlanes[i] = FPlane(0.f, 0.f, 0.f, 0.f);
			else CPlanes[i] = PalPlanes[OPix[i]];
		}
		unguard;

		BYTE* wPix = &NPix(0);
		FPlane NColor;
		FLOAT remainx, remainy;
		FLOAT fx = 0.f, fy = 0.f;
		guard(BestPixels);
		for (y = 0; y < ny; ++y, fy += ys)
		{
			INT yBase = appFloor(fy);
			remainy = fy - FLOAT(yBase);
			yBase *= USize;
			const INT nYBase = y * nx;
			for (x = 0, fx = 0.f; x < nx; ++x, fx += xs)
			{
				INT xBase = appFloor(fx);
				remainx = fx - FLOAT(xBase);
				if (remainx < 0.01f && remainy < 0.01f)
				{
					wPix[x + nYBase] = OPix[xBase + yBase];
					continue;
				}
				const FPlane& A = CPlanes[xBase + yBase];
				if (remainy < 0.01f)
				{
					const FPlane& B = CPlanes[xBase + yBase + 1];
					NColor = (A * (1.f - remainx)) + (B * remainx);
				}
				else if (remainx < 0.01f)
				{
					const FPlane& B = CPlanes[xBase + yBase + USize];
					NColor = (A * (1.f - remainy)) + (B * remainy);
				}
				else
				{
					const FPlane& B = CPlanes[xBase + yBase + 1];
					const FPlane& C = CPlanes[xBase + yBase + USize];
					const FPlane& D = CPlanes[xBase + yBase + USize + 1];
					const FPlane SideX = (A * (1.f - remainx)) + (B * remainx);
					const FPlane SideY = (C * (1.f - remainx)) + (D * remainx);
					NColor = (SideX * (1.f - remainy)) + (SideY * remainy);
				}
				if (NColor.W < 0.5f)
					wPix[x + nYBase] = 0;
				else
				{
					FLOAT BestScore = 0.f;
					for (i = 1; i < NUM_PAL_COLORS; ++i)
					{
						const FLOAT Score = PalPlanes[i].DistSquared(NColor);
						if (i == 1 || BestScore > Score)
						{
							BestScore = Score;
							wPix[x + nYBase] = i;
						}
					}
				}
			}
		}
		unguard;
		delete[] PalPlanes;
		delete[] CPlanes;
		NPix.ExchangeArray(&Pix);
		USize = nx;
		VSize = ny;
		return TRUE;
		unguardf((TEXT("(%ls)"), *Name));
	}
	inline void SetPixel(const INT x, const INT y, const BYTE Target)
	{
		const INT px = x + (y * USize);
		if (Pix.IsValidIndex(px))
			Pix(px) = Target==0 ? 1 : Target;
		else GWarn->Logf(TEXT("Pixel out of range %i/%i vs %i/%i"), x, y, USize, VSize);
	}
	inline UBOOL IsMasked() const
	{
		if (bMasked)
			return TRUE;
		for (INT i = 0; i < Pix.Num(); ++i)
			if (!Pix(i))
				return TRUE;
		return FALSE;
	}
};

constexpr INT IconRes = 32;
constexpr INT MaxSpriteRes = 1024;
struct FSpriteEntry
{
	FString Name;
	INT XSize, YSize, OffsetX, OffsetY, StartX, StartY;
	FDoomTexture* Texture;
	FSpriteEntry* TopSprite;
	UBOOL MirrorSprite;

private:
	INT WriteX, WriteY, RowY;
public:

	FSpriteEntry(const patch_t& p, FDoomTexture* tex)
		: Name(tex->Name), XSize(p.width), YSize(p.height), OffsetX(p.leftoffset), OffsetY(p.topoffset), StartX(0), StartY(0), Texture(tex), TopSprite(nullptr), MirrorSprite(FALSE), WriteX(p.width + 1), WriteY(0), RowY(p.height)
	{}
	~FSpriteEntry() noexcept(false)
	{
	}
	FSpriteEntry* CreateMirror() const
	{
		FSpriteEntry* NewSprite = new FSpriteEntry(*this);
		NewSprite->MirrorSprite = TRUE;
		NewSprite->Name += TEXT("_");
		return NewSprite;
	}
	UBOOL MergeSprites(FSpriteEntry* Other)
	{
		guard(FSpriteEntry::MergeSprites);
		if ((WriteY + Other->YSize) > MaxSpriteRes) // Impossible to fit more in this sprite sheet.
			return FALSE;

		if ((WriteX + Other->XSize + 1) > MaxSpriteRes)
		{
			if ((WriteY + RowY + Other->YSize + 1) > MaxSpriteRes) // Impossible to fit more in this sprite sheet.
				return FALSE;

			WriteX = 0;
			WriteY += (RowY + 1);
			RowY = Other->YSize;
		}
		else RowY = Max(RowY, Other->YSize);

		Other->StartX = WriteX;
		Other->StartY = WriteY;

		Texture->RescaleTexture(Max(Texture->USize, WriteX + Other->XSize + 1), Max(Texture->VSize, WriteY + Other->YSize));
		Texture->MergeTextureAt(Other->Texture, WriteX, WriteY);
		WriteX += (Other->XSize + 1);
		Other->SetParent(this);
		
		return TRUE;
		unguard;
	}
	inline UBOOL ReadyToMerge() const
	{
		return (Texture != NULL) && (TopSprite == NULL);
	}
	inline void SetParent(FSpriteEntry* Other)
	{
		guard(FSpriteEntry::SetParent);
		delete Texture;
		Texture = nullptr;
		TopSprite = Other;
		unguard;
	}
	inline UBOOL NameMatches(const TCHAR* InName) const
	{
		const TCHAR* S = *Name;
		for (INT i = 0; i < 4; ++i)
		{
			if (S[i] != InName[i] || !S[i])
				return FALSE;
		}
		return TRUE;
	}
	inline UBOOL IsDirectionalSprite() const
	{
		return Name.Len() >= 6;
	}
	inline UBOOL IsMirrorSprite() const
	{
		return Name.Len() >= 8;
	}

	inline INT GetFrameNum() const
	{
		return Name[4] - 'A';
	}
	inline INT GetDirNum() const
	{
		return Name[5] - '1';
	}
	inline INT GetMirrorFrameNum() const
	{
		return Name[6] - 'A';
	}
	inline INT GetMirrorDirNum() const
	{
		return Name[7] - '1';
	}

	inline UTexture* GetTexture() const
	{
		return TopSprite ? TopSprite->Texture->RefObject : Texture->RefObject;
	}
	inline UBOOL Identical(const FSpriteEntry& Other) const
	{
		return GetTexture() == Other.GetTexture() && XSize == Other.XSize && YSize == Other.YSize && StartX == Other.StartX && StartY == Other.StartY;
	}

	void MakeSafeName()
	{
		guard(FSpriteEntry::MakeSafeName);
		INT CharCnt = 0;
		{
			TCHAR* S = &Name[0];
			while (*S)
			{
				if (!appIsAlnum(*S) && *S != '_')
				{
					CharCnt += (*S - 'Z');
					*S = '_';
				}
				++S;
			}
		}
		if (CharCnt)
		{
			TCHAR NewStr[] = { '_',0,0,0 };
			NewStr[1] = ToHexDigit(CharCnt & 0xF);
			if (CharCnt > 0xF)
			{
				NewStr[2] = ToHexDigit(CharCnt >> 4);
				NewStr[3] = 0;
			}
			Name += NewStr;
		}
		unguard;
	}
	void GenerateIcon(TArray<BYTE>& Pix) const
	{
		guard(FSpriteEntry::GenerateIcon);
		Pix.SetSize(IconRes * IconRes);
		appMemzero(&Pix(0), Pix.Num());
		const FDoomTexture& BaseTex = TopSprite ? *TopSprite->Texture : *Texture;
		const BYTE* Src = &BaseTex.Pix(0);
		const FLOAT XDelta = FLOAT(XSize) / FLOAT(IconRes);
		const FLOAT YDelta = FLOAT(YSize) / FLOAT(IconRes);
		BYTE* Dest = &Pix(0);
		const FLOAT XPos = FLOAT(StartX);
		FLOAT YPos = FLOAT(StartY);
		
		for (INT y = 0; y < IconRes; ++y, YPos += YDelta)
		{
			FLOAT xf = XPos;
			const INT RowPos = appFloor(YPos) * BaseTex.USize;
			for (INT x = 0; x < IconRes; ++x, xf += XDelta)
				*Dest++ = Src[RowPos + appFloor(xf)];
		}
		unguard;
	}
};

INT Compare(const FSpriteEntry* A, const FSpriteEntry* B)
{
	return (A->YSize == B->YSize) ? (A->XSize - B->XSize) : (A->YSize - B->YSize);
}

struct FSoundFX
{
	FString Name;
	TArray<BYTE> Data;
	UBOOL IsValid;
	ESfxType sfxType;

	FSoundFX(const TCHAR* InName, const INT DataSize, FArchive* Ar, ESfxType t)
		: Name(InName), IsValid(FALSE), sfxType(t)
	{
		WORD FormatNumber; // Format number (must be 3) 
		WORD SampleRate; // Sample rate (usually, but not necessarily, 11025)
		INT NumSamples; // Number of samples + 32 pad bytes

		*Ar << FormatNumber << SampleRate << NumSamples;
		if (FormatNumber != 3)
			return;

		Ar->Seek(Ar->Tell() + 16); // 16 pad bytes
		NumSamples -= 32; // Strip 16 byte padding from start and end.

		{
			INT sz = sizeof(FRiffWaveHeader) + sizeof(FRiffChunkOld) + sizeof(FFormatChunk) + sizeof(FRiffChunkOld) + NumSamples;
			Data.SetSize(sz);
		}

		BYTE* Dest = &Data(0);

		guard(InitHeader);
		{
			FRiffWaveHeader* H = (FRiffWaveHeader*)Dest;
			H->rID = mmioFOURCC('R', 'I', 'F', 'F');
			H->ChunkLen = Data.Num() - (sizeof(DWORD) * 2);
			H->wID = mmioFOURCC('W', 'A', 'V', 'E');
			Dest += sizeof(FRiffWaveHeader);
		}
		{
			FRiffChunkOld* R = (FRiffChunkOld*)Dest;
			R->ChunkID = mmioFOURCC('f', 'm', 't', ' ');
			R->ChunkLen = sizeof(FFormatChunk);
			Dest += sizeof(FRiffChunkOld);
		}
		{
			FFormatChunk* F = (FFormatChunk*)Dest;
			F->wFormatTag = 1;
			F->nChannels = 1;
			F->nSamplesPerSec = SampleRate;
			F->nAvgBytesPerSec = NumSamples;
			F->nBlockAlign = sizeof(BYTE);
			F->wBitsPerSample = sizeof(BYTE) << 3;
			F->cbSize = 0;
			Dest += sizeof(FFormatChunk);
		}
		{
			FRiffChunkOld* R = (FRiffChunkOld*)Dest;
			R->ChunkID = mmioFOURCC('d', 'a', 't', 'a');
			R->ChunkLen = NumSamples;
			Dest += sizeof(FRiffChunkOld);
		}
		unguard;

		Ar->Serialize(Dest, NumSamples);
		IsValid = TRUE;
	}
};
struct FMusicFX
{
	FString Name;
	TArray<BYTE> Midi;

	FMusicFX(const TCHAR* InName, const INT DataSize, FArchive* Ar)
		: Name(InName), Midi(DataSize)
	{
		Ar->Serialize(&Midi(0), DataSize);
	}
};

struct FDirSprite
{
	FSpriteEntry* DirSprites[8];
};
struct FSpriteGroup
{
	TArray<FDirSprite> Frames;

	inline void SetSpriteInfo(const INT iDir, const INT iFrame, FSpriteEntry* Sprite)
	{
		if (Frames.Num() <= iFrame)
			Frames.AddZeroed(iFrame + 1 - Frames.Num());
		Frames(iFrame).DirSprites[iDir] = Sprite;
	}
};

static TCHAR* TexLumpNames[] = { TEXT("TEXTURE1"),TEXT("TEXTURE2") };

struct FTextureLoader
{
private:
	FWADHeaderContents& Contents;
	TArray<INT> TexLookup;
	TArray<FDoomTexture*> TexList;
	TArray<FPaletteMap> PaletteList;
	TArray<FColorMap> ColorMaps;
	TArray<FSpriteEntry*> Sprites;
	TArray<FSoundFX*> Sounds;
	TArray<FMusicFX*> Musics;
	INT MaskReplacement{};

	TMap<FName, INT> TexNameMap;
	TMap<INT, FSpriteGroup*> SpriteMap;
	TMap<INT, FSoundFX*> SoundMap;
	TMap<INT, FString> IconMap;
	UBOOL bLoadedPreData{};

	UPackage* DoomPck;
	UPackage* IconsPck;
	UPackage* AssetPck;
	UPalette* AsPalette;

	inline INT FindTextureIndex(const TCHAR* InName) const
	{
		FName FindName(InName, FNAME_Find);
		if (FindName == NAME_None)
			return INDEX_NONE;
		const INT* Result = TexNameMap.Find(FindName);
		return (Result ? *Result : INDEX_NONE);
	}

public:
	const TCHAR* GetSoundByIndex(const INT index) const
	{
		if (!index)
			return nullptr;
		FSoundFX* S = SoundMap.FindRef(index);
		if (!S)
			return nullptr;
		TCHAR* Result = appStaticString1024();
		appSprintf(Result, TEXT("%ls.%ls"), (S->sfxType == SFX_EntitySfx) ? *Contents.AssetPackageName : *Contents.TexturePackageName, *S->Name);
		return Result;
	}
	const FSpriteGroup* GetSpriteByIndex(const INT index) const
	{
		return SpriteMap.FindRef(index);
	}
	const FSpriteEntry* FindUnmirroredSprite(const FSpriteEntry& SP) const
	{
		guard(FindUnmirroredSprite);
		for (INT i = 0; i < Sprites.Num(); ++i)
		{
			if (!Sprites(i)->MirrorSprite && Sprites(i)->Identical(SP))
				return Sprites(i);
		}
		return nullptr;
		unguard;
	}

	// Generate an icon for sprite.
	const TCHAR* GetSpriteIcon(const INT EntIndex)
	{
		guard(FTextureLoader::GetSpriteIcon);
		const mobjinfo_t& mob = mobjinfo[EntIndex];
		const state_t& st = states[mob.spawnstate];
		{
			FString* Res = IconMap.Find(st.sprite);
			if (Res)
				return **Res;
		}
		FSpriteGroup* G = SpriteMap.FindRef(st.sprite);
		if (!G || !G->Frames.Num())
			return nullptr;

		FDirSprite& Dir = G->Frames(0);
		FSpriteEntry* S = Dir.DirSprites[1] ? Dir.DirSprites[1] : Dir.DirSprites[0];
		if (!S)
			return nullptr;

		TArray<BYTE> Pixels;
		S->GenerateIcon(Pixels);

		FString IconName = FString::Printf(TEXT("I_%ls"), *S->Name);
		UTexture* T = new(IconsPck, FName(*IconName), RF_Public | RF_Standalone) UTexture(TEXF_P8, IconRes, IconRes);
		T->Palette = AsPalette;
		T->LODSet = LODSET_None;
		FMipmap& M = T->Mips(0);
		T->PolyFlags |= PF_Masked;
		appMemcpy(&M.DataArray(0), &Pixels(0), Pixels.Num());

		FString NewEntry = IconMap.Set(st.sprite, *IconName);
		return *NewEntry;
		unguard;
	}

	FTextureLoader(FWADHeaderContents& C)
		: Contents(C)
	{}
	~FTextureLoader() noexcept(false)
	{
		guard(FTextureLoader::~FTextureLoader);
		INT i;
		for (i = 0; i < TexList.Num(); ++i)
			delete TexList(i);
		for (i = 0; i < Sprites.Num(); ++i)
			delete Sprites(i);
		for (i = 0; i < Sounds.Num(); ++i)
			delete Sounds(i);
		for (i = 0; i < Musics.Num(); ++i)
			delete Musics(i);
		for (TMap<INT, FSpriteGroup*>::TConstIterator It(SpriteMap); It; ++It)
			delete It.Value();
		unguard;
	}
	void LoadAssets()
	{
		guard(FTextureLoader::LoadAssets);
		if (!TexList.Num())
			return;
		LoadObject<UClass>(NULL, TEXT("Engine.Texture"), NULL, LOAD_NoFail, NULL); // Load UnrealScript properties!

		INT i;
		for (i = 0; animdefs[i].istexture>=0; ++i)
		{
			INT iStart = FindTextureIndex(ByteToStr(animdefs[i].startname, 9));
			INT iEnd = FindTextureIndex(ByteToStr(animdefs[i].endname, 9));
			if (iStart == INDEX_NONE || iEnd == INDEX_NONE || iStart > iEnd)
				continue;

			const FLOAT AnimRate = 60.f / FLOAT(animdefs[i].speed);
			for (INT j = iStart; j < iEnd; ++j)
			{
				TexList(j)->IsAnimated = TRUE;
				TexList(j)->AnimRate = AnimRate;
			}
		}

		DoomPck = UObject::CreatePackage(NULL, *Contents.TexturePackageName);
		AssetPck = UObject::CreatePackage(NULL, *Contents.AssetPackageName);
		UPackage* WorldGrp = UObject::CreatePackage(DoomPck, TEXT("World"));
		UPackage* BaseGrp = UObject::CreatePackage(DoomPck, TEXT("Base"));
		UPackage* SpriteGrp = UObject::CreatePackage(AssetPck, TEXT("Sprite"));
		IconsPck = UObject::CreatePackage(AssetPck, TEXT("Icons"));
		UPalette* Palette = new(DoomPck, FName(TEXT("DoomPalette")), RF_Public)UPalette();
		AsPalette = new(AssetPck, FName(TEXT("AssetPalette")), RF_Public)UPalette();
		
		guard(SavePalette);
		Palette->Colors.Add(NUM_PAL_COLORS);
		FColor* Dest = &Palette->Colors(0);
		const FColor* Src = PaletteList(0).Palette;
		for (i = 0; i < NUM_PAL_COLORS; ++i)
			Dest[i] = Src[i];
		AsPalette->Colors = Palette->Colors;
		unguard;

		guard(SaveTextures);
		for (i = 0; i < TexList.Num(); ++i)
		{
			FDoomTexture& D = *TexList(i);
			D.ResizeToFit(PaletteList(0).Palette);
			UTexture* T = new(D.IsSprite ? SpriteGrp : D.IsFlat ? BaseGrp : WorldGrp, FName(GetSaferName(*D.Name)), RF_Public | RF_Standalone) UTexture(TEXF_P8, D.USize, D.VSize);
			T->Palette = D.IsSprite ? AsPalette : Palette;
			T->LODSet = D.IsSprite ? LODSET_Skin : LODSET_World;
			FMipmap& M = T->Mips(0);
			if (D.IsMasked())
				T->PolyFlags |= PF_Masked;

#if 0
			if (D.IsSprite)
			{
				T->UClampMode = EUClampMode::UClamp;
				T->VClampMode = EVClampMode::VClamp;
			}
#endif

			appMemcpy(&M.DataArray(0), &D.Pix(0), D.Pix.Num());
			if ((D.USize > 64 || D.VSize > 64) && !D.IsSprite)
				T->CreateMips(TRUE, TRUE);
			D.RefObject = T;
		}
		unguard;

		guard(GenAnimations);
		// Setup animations.
		for (i = 0; i < TexList.Num(); ++i)
		{
			if (TexList(i)->IsAnimated && TexList(i)->RefObject && (i + 1) < TexList.Num())
			{
				TexList(i)->RefObject->AnimNext = TexList(i + 1)->RefObject;
				TexList(i)->RefObject->MinFrameRate = TexList(i)->RefObject->MaxFrameRate = TexList(i)->AnimRate;
			}
		}
		unguard;

		guard(SaveSprites);
		// Create sprite sheet objects.
#if 0
		UClass* SheetClass = LoadObject<UClass>(NULL, TEXT("DoomEngine.dmSprite"), NULL, LOAD_NoFail, NULL);
		if (SheetClass)
#endif
		{
#if 0
			UObjectProperty* TexProp = FindField<UObjectProperty>(SheetClass, TEXT("Texture"));
			UIntProperty* SpriteX = FindField<UIntProperty>(SheetClass, TEXT("SpriteX"));
			UIntProperty* SpriteY = FindField<UIntProperty>(SheetClass, TEXT("SpriteY"));
			UIntProperty* SizeX = FindField<UIntProperty>(SheetClass, TEXT("SizeX"));
			UIntProperty* SizeY = FindField<UIntProperty>(SheetClass, TEXT("SizeY"));
			UIntProperty* OffsetX = FindField<UIntProperty>(SheetClass, TEXT("OffsetX"));
			UIntProperty* OffsetY = FindField<UIntProperty>(SheetClass, TEXT("OffsetY"));
			UBoolProperty* Mirror = FindField<UBoolProperty>(SheetClass, TEXT("bMirror"));
#endif

			for (i = 0; i < Sprites.Num(); ++i)
			{
				FSpriteEntry& S = *Sprites(i);
#if 0
				UObject* ResObj = UObject::StaticConstructObject(SheetClass, AssetPck, FName(*S.Name), (RF_Public | RF_Standalone));
				BYTE* Data = reinterpret_cast<BYTE*>(ResObj);
				*reinterpret_cast<UObject**>(Data + TexProp->Offset) = S.TopSprite ? S.TopSprite->Texture->RefObject : S.Texture->RefObject;
				*reinterpret_cast<INT*>(Data + SpriteX->Offset) = S.StartX;
				*reinterpret_cast<INT*>(Data + SpriteY->Offset) = S.StartY;
				*reinterpret_cast<INT*>(Data + SizeX->Offset) = S.XSize;
				*reinterpret_cast<INT*>(Data + SizeY->Offset) = S.YSize;
				*reinterpret_cast<INT*>(Data + OffsetX->Offset) = S.OffsetX;
				*reinterpret_cast<INT*>(Data + OffsetY->Offset) = S.OffsetY;
				if (S.MirrorSprite)
					*reinterpret_cast<BITFIELD*>(Data + Mirror->Offset) |= Mirror->BitMask;
#endif
				if (!S.MirrorSprite || !FindUnmirroredSprite(S))
				{
					UTexture* Tex = S.GetTexture();
					UStaticMesh* SM = new (AssetPck, FName(*S.Name), (RF_Public | RF_Standalone)) UStaticMesh();
					SM->Textures.SetSize(1);
					SM->Textures(0) = Tex;
					{
						FStaticMeshTexGroup* G = new(SM->SMGroups) FStaticMeshTexGroup;
						G->PolyFlags = (SMFL_Masked | SMFL_TwoSided | SMFL_NoCollision);
						G->SmoothGroup = 0;
						G->Texture = 0;
					}

					const FVector2D TexSize(Tex->USize, Tex->VSize);
					const FVector2D uvBase(FLOAT(S.StartX) / TexSize.X, FLOAT(S.StartY) / TexSize.Y);
					const FVector2D uvEnd(FLOAT(S.StartX + S.XSize) / TexSize.X, FLOAT(S.StartY + S.YSize) / TexSize.Y);
					{
						FStaticMeshTri* T = new (SM->SMTris) FStaticMeshTri;
						T->iVertex[0] = 0;
						T->iVertex[1] = 1;
						T->iVertex[2] = 2;
						T->Tex[0].Set(uvBase.X, uvBase.Y);
						T->Tex[1].Set(uvEnd.X, uvBase.Y);
						T->Tex[2].Set(uvBase.X, uvEnd.Y);
						T->GroupIndex = 0;

						T = new (SM->SMTris) FStaticMeshTri;
						T->iVertex[0] = 1;
						T->iVertex[1] = 3;
						T->iVertex[2] = 2;
						T->Tex[0].Set(uvEnd.X, uvBase.Y);
						T->Tex[1].Set(uvEnd.X, uvEnd.Y);
						T->Tex[2].Set(uvBase.X, uvEnd.Y);
						T->GroupIndex = 0;
					}
					{
						const FVector2D Sz(FLOAT(S.XSize), FLOAT(S.YSize));
						const FVector Of(0.f, FLOAT(S.XSize) - (FLOAT(S.OffsetX) * 2.f), FLOAT(S.OffsetY));

						SM->SMVerts.AddItem((FVector(0.f, -Sz.X, Sz.Y) + Of) * 0.5f * SCALE_FACTOR);
						SM->SMVerts.AddItem((FVector(0.f, Sz.X, Sz.Y) + Of) * 0.5f * SCALE_FACTOR);
						SM->SMVerts.AddItem((FVector(0.f, -Sz.X, -Sz.Y) + Of) * 0.5f * SCALE_FACTOR);
						SM->SMVerts.AddItem((FVector(0.f, Sz.X, -Sz.Y) + Of) * 0.5f * SCALE_FACTOR);

						if (!S.MirrorSprite)
						{
							for (INT i = 0; i < 4; ++i)
								SM->SMVerts(i).Y *= -1.f;
						}
					}
					SM->FrameVerts = SM->SMVerts.Num();
					SM->SetupConnects();
				}
			}
		}
		unguard;

		guard(SaveSounds);
		// Sound FX.
		FName WavName(TEXT("OGG"));
		for (i = 0; i < Sounds.Num(); ++i)
		{
			USound* Sound = new((Sounds(i)->sfxType == SFX_EntitySfx) ? AssetPck : DoomPck, FName(*Sounds(i)->Name), RF_Public | RF_Standalone) USound();
			Sound->Data.SetSize(Sounds(i)->Data.Num());
			appMemcpy(Sound->Data.GetData(), Sounds(i)->Data.GetData(), Sounds(i)->Data.Num());
			Sound->FileType = WavName;
			Sound->Data.Compress(3);
		}
		unguard;

#if 0
		guard(SaveMusic);
		// Music
		FName MidiName(TEXT("MIDI"));
		for (i = 0; i < Musics.Num(); ++i)
		{
			UMusic* Music = new(AssetPck, FName(*Musics(i)->Name), RF_Public | RF_Standalone) UMusic();
			Music->Data.SetSize(Musics(i)->Midi.Num());
			appMemcpy(Music->Data.GetData(), Musics(i)->Midi.GetData(), Musics(i)->Midi.Num());
			Music->FileType = MidiName;
		}
		unguard;
#endif
		unguard;
	}
	void SaveAssets()
	{
		guard(FTextureLoader::SaveAssets);
		FString Filename = FString(TEXT("../Textures/")) + Contents.TexturePackageName + TEXT(".utx");
		UObject::SavePackage(DoomPck, NULL, RF_Standalone, *Filename);
		Filename = FString(TEXT("../System/")) + Contents.AssetPackageName + TEXT(".u");
		UObject::SavePackage(AssetPck, NULL, RF_Standalone, *Filename);
		unguard;
	}
	void LoadTexDimensions()
	{
		guard(LoadTexDimensions);
		INT iPatch = Contents.FindLump(TEXT("PNAMES"));
		if (iPatch == INDEX_NONE)
		{
			GWarn->Log(TEXT("Failed to find texture names header"));
			return;
		}
		FArchive* A = Contents.FileAr;
		A->Seek(Contents.Lumps(iPatch).offData);
		INT Count, i, j;
		*A << Count;
		//debugf(TEXT("NumTexPatches %i"), Count);
		TexLookup.SetSize(Count);

		BYTE shName[8];
		TCHAR pName[16];
		for (i = 0; i < Count; ++i)
		{
			A->Serialize(shName, sizeof(shName));
			for (j = 0; j < 8; ++j)
				pName[j] = shName[j];
			pName[8] = 0;
			TexLookup(i) = Contents.FindLump(pName);
		}

		for (i = 0; i < ARRAY_COUNT(TexLumpNames); ++i)
		{
			INT iTex = Contents.FindLump(TexLumpNames[i]);
			if (iTex == INDEX_NONE)
				continue;

			const INT BaseOffset = Contents.Lumps(iTex).offData;
			A->Seek(BaseOffset);
			INT TexCount;
			*A << TexCount;

			for (INT directory = BaseOffset + sizeof(INT), j = 0; j < TexCount; ++j, directory += sizeof(INT))
			{
				A->Seek(directory);
				INT Offset;
				*A << Offset;
				A->Seek(BaseOffset + Offset);

				maptexture_t texInfo(A);
				const TCHAR* TexName = ByteToStr(texInfo.name, 8);
				FInstTexture* Tex = FInstTexture::RegisterTexture(TexName);
				Tex->SetDimensions(texInfo.width, texInfo.height);
				Tex->Type = TT_Walls;
			}
		}

		guard(AssignFlats);
		const INT iStartFlat = Contents.FindLump(TEXT("F_START")) + 1;
		const INT iEndFlat = Contents.FindLump(TEXT("F_END"));

		if (iStartFlat == INDEX_NONE || iEndFlat == INDEX_NONE)
		{
			GWarn->Log(TEXT("WAD missing flat indices!"));
			return;
		}

		for (i = iStartFlat; i < iEndFlat; ++i)
		{
			if (appStricmp(Contents.Lumps(i).UName, SkyTextureName) == NULL)
				continue;
			appSprintf(pName, TEXT("B_%ls"), Contents.Lumps(i).UName);
			FInstTexture* Tex = FInstTexture::RegisterTexture(pName);
			if (Tex->Type != TT_Sky)
				Tex->Type = TT_Flat;
		}
		unguard;
		bLoadedPreData = TRUE;
		unguard;
	}
	void LoadTex()
	{
		guard(LoadTex);
		if (!bLoadedPreData)
			return;

		FArchive* A = Contents.FileAr;
		INT i, j;

		// Load palette.
		INT iPalette = Contents.FindLump(TEXT("PLAYPAL"));
		if (iPalette == INDEX_NONE)
		{
			GWarn->Log(TEXT("Failed to find texture playpalette header"));
			return;
		}
		PaletteList.SetSize(1); // We don't need rest...
		//PaletteList.SetSize(Contents.Lumps(iPalette).lenData / NUM_PAL_COLORS / 3);
		A->Seek(Contents.Lumps(iPalette).offData);
		for (i = 0; i < PaletteList.Num(); ++i)
		{
			BYTE R, G, B;
			for (j = 0; j < NUM_PAL_COLORS; ++j)
			{
				*A << R << G << B;
				PaletteList(i).Palette[j] = FColor(R, G, B, 255);
			}
		}

		// Get replacement mask color offset for color 0
		{
			const FVector BaseVec = PaletteList(0).Palette[0].Vector();
			MaskReplacement = 0;
			FLOAT BestScore = 0.f;
			for (i = 1; i < NUM_PAL_COLORS; ++i)
			{
				const FLOAT Score = BaseVec.DistSquared(PaletteList(0).Palette[i].Vector());
				if (!MaskReplacement || Score < BestScore)
				{
					BestScore = Score;
					MaskReplacement = i;
				}
			}
			PaletteList(0).Palette[0] = FColor(0, 255, 255, 255);
		}

		// Load color remapping.
		iPalette = Contents.FindLump(TEXT("COLORMAP"));
		if (iPalette == INDEX_NONE)
		{
			GWarn->Log(TEXT("Failed to find texture colormap header"));
			return;
		}
		ColorMaps.SetSize(Contents.Lumps(iPalette).lenData / NUM_PAL_COLORS);
		A->Seek(Contents.Lumps(iPalette).offData);
		for (i = 0; i < ColorMaps.Num(); ++i)
			A->Serialize(ColorMaps(i).C, NUM_PAL_COLORS);

		// Load the map texture definitions from textures.lmp.
		// The data is contained in one or two lumps,
		//  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
		for (i = 0; i < ARRAY_COUNT(TexLumpNames); ++i)
			LoadTexLump(TexLumpNames[i]);

		// Load flats (floor/ceiling textures)
		// NOTE: They are always 64x64 resolution!
		const BYTE* ColorMap = ColorMaps(0).C;
		guard(LoadFlats);
		const INT iStartFlat = Contents.FindLump(TEXT("F_START"))+1;
		const INT iEndFlat = Contents.FindLump(TEXT("F_END"));

		BYTE* FlatBits = new BYTE[FlatTotalSize];
		for (i = iStartFlat; i < iEndFlat; ++i)
		{
			const WADFileEntry& Lump = Contents.Lumps(i);
			if (Lump.lenData != FlatTotalSize || appStricmp(Lump.UName, SkyTextureName) == NULL) // Start/End lumps, skip em!
				continue;

			A->Seek(Lump.offData);
			A->Serialize(FlatBits, FlatTotalSize);

			FDoomTexture* NewTexture = new FDoomTexture(Lump.UName);
			TexNameMap.Set(FName(*NewTexture->OrgName), TexList.Num());
			TexList.AddItem(NewTexture);

			for (INT j = 0; j < FlatTotalSize; ++j)
				NewTexture->Pix(j) = ColorMap[FlatBits[j]] ? ColorMap[FlatBits[j]] : MaskReplacement;
		}
		delete[] FlatBits;
		unguard;

		guard(LoadSprites);
		const INT iStartSprite = Contents.FindLump(TEXT("S_START")) + 1;
		const INT iEndSprite = Contents.FindLump(TEXT("S_END"));

		if (iStartSprite == INDEX_NONE || iEndSprite == INDEX_NONE)
		{
			GWarn->Log(TEXT("WAD missing sprite indices!"));
			return;
		}

		for (i = iStartSprite; i < iEndSprite; ++i)
		{
			const WADFileEntry& Lump = Contents.Lumps(i);
			if (Lump.lenData == 0) // Start/End lumps, skip em!
				continue;

			const INT PatchOffset = Lump.offData;
			A->Seek(Lump.offData);
			patch_t SpritePatch(A);
			const INT TexHeight = SpritePatch.height;
			const INT x2 = SpritePatch.width;
			INT y;
			BYTE* source = new BYTE[TexHeight];

			FDoomTexture* NewTexture = new FDoomTexture(Lump.UName, SpritePatch);
			FSpriteEntry* NewSprite = new FSpriteEntry(SpritePatch, NewTexture);
			Sprites.AddItem(NewSprite);

			for (INT x = 0; x < x2; x++)
			{
				A->Seek(PatchOffset + SpritePatch.columnofs[x]);
				column_t patchcol;
				patchcol.Load(A);

				while (patchcol.topdelta != 0xff)
				{
					INT count = patchcol.length;
					const INT NextOffset = A->Tell() + count + 1;
					INT position = patchcol.topdelta;

					if (position < 0)
					{
						count += position;
						position = 0;
					}

					if (position + count > TexHeight)
						count = TexHeight - position;

					if (count > 0)
					{
						A->Serialize(source, count);
						for (y = 0; y < count; ++y)
							NewTexture->SetPixel(x, position + y, ColorMap[source[y]] ? ColorMap[source[y]] : MaskReplacement);
					}
					A->Seek(NextOffset);
					patchcol.Load(A);
				}
			}
			delete[] source;
		}
		unguard;

		guard(MergeSprites);
		Sort(Sprites);
		INT j;
		for (i = 0; i < Sprites.Num(); ++i)
		{
			if (!Sprites(i)->ReadyToMerge())
				continue;
			for (j = (i + 1); j < Sprites.Num(); ++j)
			{
				if (Sprites(j)->ReadyToMerge())
					Sprites(i)->MergeSprites(Sprites(j));
			}
		}

		INT SheetNum = 0;
		for (i = 0; i < Sprites.Num(); ++i)
		{
			if (Sprites(i)->Texture)
			{
				Sprites(i)->Texture->Name = FString::Printf(TEXT("SpriteSheet_%i"), SheetNum);
				++SheetNum;
				Sprites(i)->Texture->UpscaleToFit();
				TexList.AddItem(Sprites(i)->Texture);
			}
		}
		unguard;

		guard(GetSpriteAnimations);
		TCHAR SpriteName[8];
		const INT iOrgSprites = Sprites.Num(); // Don't loop over mirror duplicates.
		for (i = 0; i < spritenum_t::NUMSPRITES; ++i)
		{
			appFromAnsiInPlace(SpriteName, sprnames[i], 8);
			FSpriteGroup* Group = NULL;

			for (j = 0; j < iOrgSprites; ++j)
			{
				FSpriteEntry* S = Sprites(j);
				if (!S->NameMatches(SpriteName))
					continue;
				
				if (!Group)
				{
					Group = new FSpriteGroup;
					SpriteMap.Set(i, Group);
				}
				if (S->IsDirectionalSprite() && S->GetDirNum() >= 0)
				{
					Group->SetSpriteInfo(S->GetDirNum(), S->GetFrameNum(), S);
					if (S->IsMirrorSprite())
					{
						FSpriteEntry* MS = S->CreateMirror();
						Sprites.AddItem(MS);
						Group->SetSpriteInfo(S->GetMirrorDirNum(), S->GetMirrorFrameNum(), MS);
					}
				}
				else Group->SetSpriteInfo(0, S->GetFrameNum(), S);
			}
		}

		for (i = 0; i < Sprites.Num(); ++i) // Arch-Viles have so many frames that they include illegal name chars.
			Sprites(i)->MakeSafeName();
		unguard;
		unguard;
	}
	void LoadSfx()
	{
		guard(LoadSfx);
		FArchive* A = Contents.FileAr;
		INT i;
		TCHAR tempName[32];
		TCHAR ansName[32];
		for (i = 1; i< sfxenum_t::NUMSFX; ++i)
		{
			S_sfx[i].lumpnum = i;
			if (S_sfx[i].link)
				continue;
			ESfxType sfxType = GetSfxType((sfxenum_t)i);
			if (sfxType == SFX_GameSfx)
				continue;
			appFromAnsiInPlace(ansName, S_sfx[i].name, 32);
			appStrupr(ansName);
			appSnprintf(tempName, 32, TEXT("DS%ls"), ansName);
			const INT iSndIndex = Contents.FindLump(tempName);
			if (iSndIndex == INDEX_NONE)
			{
				GWarn->Logf(TEXT("Failed to find sfx '%ls'"), tempName);
				continue;
			}
			const WADFileEntry& Lump = Contents.Lumps(iSndIndex);
			A->Seek(Lump.offData);
			FSoundFX* NewSound = new FSoundFX(tempName, Lump.lenData, A, sfxType);
			if (NewSound->IsValid)
			{
				Sounds.AddItem(NewSound);
				SoundMap.Set(i, NewSound);
			}
			else delete NewSound;
		}

#if 0 // TODO - Convert from MUS format?
		for (i = 1; ; ++i)
		{
			if (S_music[i].name == nullptr)
				break;

			appFromAnsiInPlace(ansName, S_music[i].name, 32);
			appStrupr(ansName);
			appSnprintf(tempName, 32, TEXT("D_%ls"), ansName);
			const INT iMusicIndex = Contents.FindLump(tempName);
			if (iMusicIndex == INDEX_NONE)
			{
				GWarn->Logf(TEXT("Failed to find music '%ls'"), tempName);
				continue;
			}

			const WADFileEntry& Lump = Contents.Lumps(iMusicIndex);
			A->Seek(Lump.offData);
			FMusicFX* NewMusic = new FMusicFX(tempName, Lump.lenData, A);
			Musics.AddItem(NewMusic);
		}
#endif
		unguard;
	}

private:
	void LoadTexLump(const TCHAR* Name)
	{
		guard(LoadTexLump);
		INT iTex = Contents.FindLump(Name);
		if (iTex == INDEX_NONE)
			return;

		FArchive* A = Contents.FileAr;
		const INT BaseOffset = Contents.Lumps(iTex).offData;
		A->Seek(BaseOffset);
		INT TexCount;
		*A << TexCount;
		INT x, x1, x2, j, y;

		//debugf(TEXT("LoadTex '%ls' count %i"), Name, TexCount);
		
		for (INT directory = BaseOffset + sizeof(INT), i = 0; i < TexCount; ++i, directory += sizeof(INT))
		{
			A->Seek(directory);
			INT Offset;
			*A << Offset;
			A->Seek(BaseOffset + Offset);

			maptexture_t texInfo(A);
			//debugf(TEXT("  Info[%i] '%ls' Mask %i Res %i/%i Patches %i"), i, ByteToStr(texInfo.name, 8), INT(texInfo.masked), INT(texInfo.width), INT(texInfo.height), INT(texInfo.patchcount));

			FDoomTexture* NewTexture = new FDoomTexture(texInfo);
			TexNameMap.Set(FName(*NewTexture->OrgName), TexList.Num());
			TexList.AddItem(NewTexture);

			const INT numPatches = texInfo.patchcount;
			texpatch_t* patches = reinterpret_cast<texpatch_t*>(appAlloca(sizeof(texpatch_t) * numPatches));
			for (j = 0; j < numPatches; ++j)
			{
				mappatch_t patch;
				patch.Load(A);
				const INT RealPatch = TexLookup.IsValidIndex(patch.patch) ? TexLookup(patch.patch) : INDEX_NONE;
				if (RealPatch == INDEX_NONE)
					GWarn->Logf(TEXT("Failed to load '%ls' invalid patch(%i/%i) = %i"), NewTexture->Name, patch.patch, TexLookup.Num(), RealPatch);
				patches[j].Set(patch, RealPatch);
			}

			const INT TexWidth = texInfo.width;
			const INT TexHeight = texInfo.height;
			BYTE* source = new BYTE[TexHeight];

			{
				for (j = 0; j < numPatches; j++)
				{
					if (patches[j].patch == INDEX_NONE)
						continue;
					const INT PatchOffset = Contents.Lumps(patches[j].patch).offData;
					A->Seek(PatchOffset);

					patch_t patch(A);

					x1 = patches[j].originx;
					x2 = x1 + patch.width;

					x = Max(0, x1);
					x2 = Min(TexWidth, x2);
					const BYTE* ColorMap = ColorMaps(patches[j].iColor).C;

					for (; x < x2; x++)
					{
						A->Seek(PatchOffset + patch.columnofs[x - x1]);
						column_t patchcol;
						patchcol.Load(A);

						while (patchcol.topdelta != 0xff)
						{
							INT count = patchcol.length;
							const INT NextOffset = A->Tell() + count + 1;
							INT position = patches[j].originy + patchcol.topdelta;

							if (position < 0)
							{
								count += position;
								position = 0;
							}

							if (position + count > TexHeight)
								count = TexHeight - position;

							if (count > 0)
							{
								A->Serialize(source, count);
								for (y = 0; y < count; ++y)
									NewTexture->SetPixel(x, position + y, ColorMap[source[y]] ? ColorMap[source[y]] : MaskReplacement);
							}
							A->Seek(NextOffset);
							patchcol.Load(A);
						}
					}
				}
			}

			delete[] source;
		}
		unguardf((TEXT("(%ls)"), Name));
	}
};

static const TCHAR* GetParentClass(INT MobIndex)
{
	const mobjinfo_t& mob = mobjinfo[MobIndex];
	if (IsMonster(MobIndex))
		return TEXT("DoomMonster");
	if (IsProjectile(MobIndex))
		return TEXT("DoomProjectile");
	return TEXT("DoomDeco");
}
inline UBOOL IsValidState(const INT Index)
{
	return (Index > statenum_t::S_NULL && Index < statenum_t::NUMSTATES);
}
static const TCHAR* GetSafeName(const TCHAR* FuncName)
{
	static TCHAR Result[32];
	if (FuncName[1] == '_')
	{
		appStrcpy(Result, TEXT("Func"));
		appStrcat(Result, (FuncName + 2));
	}
	else appStrcpy(Result, FuncName);
	return Result;
}

struct FEntityCreator
{
private:
	const FWADHeaderContents& Contents;
	FTextureLoader& Loader;
	const TCHAR* PendingEntName;
	TMap<INT, INT> ExtractedMap;
	INT StateNumber;
	UBOOL bExplodingProjectile{};

public:
	FEntityCreator(const FWADHeaderContents& C, FTextureLoader& L)
		: Contents(C), Loader(L)
	{}
	~FEntityCreator() noexcept(false)
	{
	}

	void GetSound(const INT Index, const TCHAR* Prop, FStringOutputDevice& Out) const
	{
		const sfxinfo_t& s = S_sfx[Index];
		const TCHAR* Res = Loader.GetSoundByIndex(s.link ? s.link->lumpnum : Index);
		if (Res)
		{
			Out.Logf(TEXT("\t%ls=Sound'%ls'\r\n"), Prop, Res);
			if (s.pitch > 0)
				Out.Logf(TEXT("\t%lsPitch=%f\r\n"), Prop, FLOAT(s.pitch) / 100.f);
		}
	}
	void GetState(const INT Index, const TCHAR* Prop, FStringOutputDevice& Out)
	{
		if (!IsValidState(Index))
			return;
		INT MappedIndex = StateNumber;
		const INT* ExItem = ExtractedMap.Find(Index);
		if (!ExItem)
		{
			Out.Logf(TEXT("\r\n\tBEGIN OBJECT CLASS=dmDoomState NAME=%ls_%i\r\n"), PendingEntName, MappedIndex);
			TMap<INT, INT> ExpFrames;
			ExpFrames.Set(Index, 0);
			INT i = Index;
			INT j;
			INT iLoopCount = 0;
			UBOOL KeepLoop = TRUE;
			while (KeepLoop)
			{
				const state_t& st = states[i];
				// Get sprite name.
				FStringOutputDevice SpriteName;
				const FSpriteGroup* SpriteGroup = Loader.GetSpriteByIndex(st.sprite);
				if (SpriteGroup)
				{
					const INT FrameNum = st.frame & FF_FRAMEMASK;
					if (SpriteGroup->Frames.IsValidIndex(FrameNum))
					{
						const FDirSprite& F = SpriteGroup->Frames(FrameNum);
						FSpriteEntry* AnyEntry = NULL;
						for(j=0; j<8; ++j)
							if (F.DirSprites[j])
							{
								AnyEntry = F.DirSprites[j];
								break;
							}

						if (AnyEntry)
						{
							for (j = 0; j < 8; ++j)
							{
								if (j)
									SpriteName.Log(TEXT(","));
								const FSpriteEntry* S = (F.DirSprites[j] ? F.DirSprites[j] : AnyEntry);
								if (S->MirrorSprite)
								{
									const FSpriteEntry* M = Loader.FindUnmirroredSprite(*S);
									if (M)
										SpriteName.Logf(TEXT("Sprite[%i]=StaticMesh'%ls.%ls',Mirror[%i]=1"), j, *Contents.AssetPackageName, *M->Name, j);
									else SpriteName.Logf(TEXT("Sprite[%i]=StaticMesh'%ls.%ls'"), j, *Contents.AssetPackageName, *S->Name);
								}
								else SpriteName.Logf(TEXT("Sprite[%i]=StaticMesh'%ls.%ls'"), j, *Contents.AssetPackageName, *S->Name);
							}
						}
					}
					if (SpriteName.Len())
						SpriteName.Log(TEXT(","));
				}

				// Get next frame (relative to local state frame counter).
				const INT NextState = st.nextstate;
				INT iNext = NextState;
				if (!IsValidState(iNext))
				{
					iNext = INDEX_NONE;
					KeepLoop = FALSE;
				}
				else if (iNext< Index || iNext>i + 1) // Assume we should leave this loop now if we make odd jumps.
				{
					iNext = INDEX_NONE;
					KeepLoop = FALSE;
				}
				else
				{
					INT* iRefNext = ExpFrames.Find(NextState);
					if (iRefNext)
					{
						iNext = *iRefNext;
						KeepLoop = FALSE;
					}
					else
					{
						++iLoopCount;
						iNext = iLoopCount;
						ExpFrames.Set(NextState, iLoopCount);
					}
				}
				FLOAT Duration = FLOAT(st.tics) / GAME_FPS;
				const TCHAR* Action = st.action.acv ? st.action.acv() : NULL;
				if (Action)
				{
					if (!appStricmp(Action, TEXT("A_Explode")))
					{
						bExplodingProjectile = TRUE;
						Action = NULL;
					}
					else Action = GetSafeName(Action);
				}
				Out.Logf(TEXT("\t\tFrame.Add((%lsDuration=%f,Next=%i,Event=%ls,bUnlit=%ls))\r\n"), *SpriteName, Duration, iNext, Action ? Action : TEXT("None"), (st.frame & FF_FULLBRIGHT) ? TEXT("True") : TEXT("False"));
				i = NextState;
			}
			Out.Log(TEXT("\tEND OBJECT\r\n"));

			ExItem = &Index;
			ExtractedMap.Set(Index, StateNumber);
			++StateNumber;
		}
		else MappedIndex = *ExItem;

		Out.Logf(TEXT("\t%ls=%ls_%i\r\n"), Prop, PendingEntName, MappedIndex);
	}
	void SaveEnts()
	{
		guard(FEntityCreator::SaveEnts);
		FString FilePath = FString(TEXT("..") PATH_SEPARATOR) + Contents.ClassesDir;
		GFileManager->MakeDirectory(*FilePath);
		FilePath *= TEXT("Classes");
		GFileManager->MakeDirectory(*FilePath);
		FilePath += PATH_SEPARATOR;
		warnf(TEXT("Dumping .uc files to %ls"), *FilePath);

		INT Num = 0;
		for (INT i = 0; i < mobjtype_t::NUMMOBJTYPES; ++i)
		{
			const TCHAR* EntName = EntTypeToStr(i);
			if (!EntName)
				continue;

			bExplodingProjectile = FALSE;
			++Num;
			StateNumber = 0;
			ExtractedMap.Empty();
			PendingEntName = EntName;
			const FString EFileName = FilePath + EntName + TEXT(".uc");
			FStringOutputDevice MobData;

			const mobjinfo_t& mob = mobjinfo[i];
			MobData.Log(TEXT("//<BEGIN_AUTOGEN_DATA>\r\n"));
			if (IsMonster(i))
			{
				MobData.Logf(TEXT("\tBloodFX=Class'%ls'\r\n"), EntTypeToStr(MT_BLOOD));
				MobData.Logf(TEXT("\tHealth=%i\r\n"), mob.spawnhealth);
				MobData.Logf(TEXT("\tReactionTime=%f\r\n"), FLOAT(mob.reactiontime) / GAME_FPS);
				if (!mob.speed)
					MobData.Log(TEXT("\tGroundSpeed=0.01\r\n"));
				else MobData.Logf(TEXT("\tGroundSpeed=%f\r\n"), FLOAT(mob.speed * GAME_FPS) * SPEED_TO_UE);
				MobData.Logf(TEXT("\tAtkDamage=%i\r\n"), mob.damage);

				switch (mob.seesound)
				{
				case sfx_posit1:
				case sfx_posit2:
				case sfx_posit3:
					GetSound(sfx_posit1, TEXT("Acquire"), MobData);
					GetSound(sfx_posit2, TEXT("AcquireB"), MobData);
					GetSound(sfx_posit3, TEXT("AcquireC"), MobData);
					break;
				case sfx_bgsit1:
				case sfx_bgsit2:
					GetSound(sfx_bgsit1, TEXT("Acquire"), MobData);
					GetSound(sfx_bgsit2, TEXT("AcquireB"), MobData);
					break;
				default:
					GetSound(mob.seesound, TEXT("Acquire"), MobData);
				}

				switch (mob.deathsound)
				{
				case sfx_podth1:
				case sfx_podth2:
				case sfx_podth3:
					GetSound(sfx_podth1, TEXT("Die"), MobData);
					GetSound(sfx_podth2, TEXT("DieB"), MobData);
					GetSound(sfx_podth3, TEXT("DieC"), MobData);
					break;
				case sfx_bgdth1:
				case sfx_bgdth2:
					GetSound(sfx_bgdth1, TEXT("Die"), MobData);
					GetSound(sfx_bgdth2, TEXT("DieB"), MobData);
					break;
				default:
					GetSound(mob.deathsound, TEXT("Die"), MobData);
				}
				
				GetSound(mob.painsound, TEXT("HitSound1"), MobData);
				GetSound(mob.activesound, TEXT("Roam"), MobData);
				GetSound(mob.attacksound, TEXT("AttackSound"), MobData);
				GetSound(sfx_slop, TEXT("GibbedSound"), MobData);

				GetState(mob.spawnstate, TEXT("SpawnState"), MobData);
				GetState(mob.seestate, TEXT("AcquireState"), MobData);
				GetState(mob.painstate, TEXT("PainState"), MobData);
				GetState(mob.meleestate, TEXT("MeleeState"), MobData);
				GetState(mob.missilestate, TEXT("RangedState"), MobData);
				GetState(mob.deathstate, TEXT("DeathState"), MobData);
				GetState(mob.xdeathstate, TEXT("GibbedState"), MobData);
				GetState(mob.raisestate, TEXT("ResurrectState"), MobData);

				if (mob.flags & MF_FLOAT)
				{
					MobData.Log(TEXT("\tbCanFly=true\r\n"));
					MobData.Logf(TEXT("\tAirSpeed=%f\r\n"), FLOAT(mob.speed * GAME_FPS) * SPEED_TO_UE);
				}
			}
			else if (IsProjectile(i))
			{
				MobData.Logf(TEXT("\tSpeed=%f\r\n"), FLOAT(mob.speed >> FRACBITS) * SCALE_FACTOR * GAME_FPS);
				MobData.Logf(TEXT("\tDamage=%i\r\n"), mob.damage);
				GetSound(mob.seesound, TEXT("SpawnSound"), MobData);
				GetSound(mob.deathsound, TEXT("ImpactSound"), MobData);

				GetState(mob.spawnstate, TEXT("SpawnState"), MobData);
				GetState(mob.deathstate, TEXT("ImpactState"), MobData);

				if (bExplodingProjectile)
					MobData.Log(TEXT("\tbExplodingProjectile=True\r\n"));
			}
			else
			{
				GetState(mob.spawnstate, TEXT("SpawnState"), MobData);
			}

			if (IsValidState(mob.spawnstate))
			{
				const TCHAR* IconName = Loader.GetSpriteIcon(i);
				if (IconName)
					MobData.Logf(TEXT("\tTexture=Texture'%ls.%ls'\r\n"), *Contents.AssetPackageName, IconName);
			}

			MobData.Logf(TEXT("\tCollisionRadius=%f\r\n"), FLOAT(mob.radius >> FRACBITS) * 0.8f * SCALE_FACTOR);
			MobData.Logf(TEXT("\tCollisionHeight=%f\r\n"), FLOAT(mob.height >> FRACBITS) * 0.5f * SCALE_FACTOR);
			MobData.Logf(TEXT("\tMass=%i\r\n"), mob.mass);
			MobData.Logf(TEXT("\tEntType=%i\r\n"), i);

			FStringOutputDevice FileOut;
			FString OldData;
			if (appLoadFileToString(OldData, *EFileName))
			{
				{
					INT iSeek = 0;
					for (;;)
					{
						INT iSound = OldData.InStrOffset(TEXT("<AUTOSND:"), iSeek);
						if (iSound == INDEX_NONE)
							break;

						iSeek = iSound + 9;
						INT iSndEnd = OldData.InStrOffset(TEXT(">"), iSeek);
						if (iSndEnd == INDEX_NONE)
							break;

						const FString SndStr = OldData.Mid(iSeek, iSndEnd - iSeek);
						iSeek = iSndEnd + 1;
						FString SoundVar, SoundFX;
						if (!SndStr.Divide(TEXT("="), SoundVar, SoundFX))
							continue;

						sfxenum_t sx = LookupSfxName(*SoundFX);
						if (sx == sfx_None)
							continue;
						GetSound(sx, *SoundVar, MobData);
					}

					iSeek = 0;
					for (;;)
					{
						INT iSound = OldData.InStrOffset(TEXT("<AUTOSTATE:"), iSeek);
						if (iSound == INDEX_NONE)
							break;

						iSeek = iSound + 11;
						INT iSndEnd = OldData.InStrOffset(TEXT(">"), iSeek);
						if (iSndEnd == INDEX_NONE)
							break;

						const FString SndStr = OldData.Mid(iSeek, iSndEnd - iSeek);
						iSeek = iSndEnd + 1;
						FString StateVar, StateName;
						if (!SndStr.Divide(TEXT("="), StateVar, StateName))
							continue;

						statenum_t st = LookupStateName(*StateName);
						if (st == S_NULL)
							continue;
						GetState(st, *StateVar, MobData);
					}
				}
				INT iFoundStart = OldData.InStr(TEXT("//<BEGIN_AUTOGEN_DATA>"));
				INT iFoundEnd = OldData.InStr(TEXT("//<END_AUTOGEN_DATA>"));

				if (iFoundStart != INDEX_NONE)
					FileOut.Log(OldData.Left(iFoundStart));
				else FileOut.Log(OldData);

				FileOut.Log(MobData);

				if (iFoundEnd != INDEX_NONE)
					FileOut.Log(OldData.Mid(iFoundEnd));
				else FileOut.Log(TEXT("//<END_AUTOGEN_DATA>\r\n"));
			}
			else
			{
				FileOut.Logf(TEXT("Class %ls extends %ls;\r\n\r\ndefaultproperties\r\n{\r\n"), EntName, GetParentClass(i));
				FileOut.Log(MobData);
				FileOut.Log(TEXT("//<END_AUTOGEN_DATA>\r\n"));
				FileOut.Log(TEXT("}\r\n"));
			}

			appSaveStringToFile(*FileOut, *EFileName);
		}
		warnf(TEXT("Done, writing %i files!"), Num);
		unguard;
	}
};

INT UDoomWadExp::Main(const TCHAR* Parms)
{
	guard(UDoomWadExp::Main);
	FString Arg, WadFile, OutEnts;
	INT ExportMaps = INDEX_NONE;
	INT ExportShape = INDEX_NONE;
	UBOOL bEnts = FALSE;
	while (ParseToken(Parms, Arg, FALSE))
	{
		if (Arg.Left(1) == TEXT("-"))
		{
			const TCHAR* ArgS = *Arg + 1;
			if (!appStricmp(ArgS, TEXT("maps")))
				ExportMaps = 0;
			else if (Parse(ArgS, TEXT("map="), ExportMaps))
				continue;
			else if (Parse(ArgS, TEXT("shape="), ExportShape))
				continue;
			else if (Parse(ArgS, TEXT("ents="), OutEnts))
				bEnts = TRUE;
			else break;
		}
		else if (WadFile.Len()) // Should only have one WAD file.
		{
			WadFile.Empty();
			break;
		}
		else WadFile = Arg;
	}
	if (!WadFile.Len() || (ExportMaps == INDEX_NONE && !bEnts))
	{
		GWarn->Logf(NAME_Error, TEXT("Invalid parms, must have: <input wad filename> <options>"));
		GWarn->Logf(TEXT("Options:"));
		GWarn->Logf(TEXT("-maps - Export all maps."));
		GWarn->Logf(TEXT("-map=XXX - Export a single map."));
		GWarn->Logf(TEXT("-ents=<PACKAGENAME> - Export entity data along with sprites and sounds as resource file."));
		GWarn->Logf(TEXT("-shape=XXX - Must be done along with single map, export a brush sheet as 2D shape file (input brush object number)."));
		return 1;
	}
	FArchive* Ar = GFileManager->CreateFileReader(*WadFile);
	if (!Ar)
	{
		GWarn->Logf(TEXT("Failed to locate WAD: %ls"), *WadFile);
		return 1;
	}

	FWADHeaderContents Header(Ar, *WadFile, *OutEnts);
	if (!Header.IsValidHeader())
	{
		delete Ar;
		GWarn->Logf(TEXT("File had invalid header '%ls'"), Header.Header.USignature);
		return 1;
	}
	//GWarn->Logf(TEXT("Got '%ls' NumFiles %i FAToffset %i"), Header.USignature, Header.numFiles, Header.offFAT);

	if (!Header.LoadLumps())
	{
		GWarn->Log(TEXT("Invalid game version!"));
		delete Ar;
		return 1;
	}
	GWarn->Logf(TEXT("Detected %ls WAD"), Header.GetGameName());

	FTextureLoader TLoader(Header);
	TLoader.LoadTexDimensions();
	Header.FlagTextures();
	if (bEnts)
	{
		TLoader.LoadTex();
		TLoader.LoadSfx();
		TLoader.LoadAssets();
		FEntityCreator EntC(Header, TLoader);
		EntC.SaveEnts();
		TLoader.SaveAssets();
	}

	if (ExportMaps != INDEX_NONE)
	{
		GWarn->Log(TEXT("Loading maps..."));
		FMapLoader MapLoad(Ar, Header);
		if (!MapLoad.LoadMaps())
			GWarn->Log(TEXT("WAD contained no maps???"));
		else if (ExportMaps > 0 && !MapLoad.Maps.IsValidIndex(ExportMaps - 1))
			GWarn->Logf(TEXT("Trying to export map out of range %i/%i"), ExportMaps, MapLoad.Maps.Num());
		else
		{
			LoadObject<UClass>(NULL, TEXT("Engine.PlayerStart"), NULL, LOAD_NoFail, NULL); // Make sure to load UScript properties.
			GWarn->Logf(TEXT("Loaded %i maps!"), MapLoad.Maps.Num());

			if (ExportShape > 0)
			{
				GWarn->Logf(TEXT("Exporting Brush%i..."), ExportShape);
				if (ExportMaps == 0)
					GWarn->Log(TEXT("You must specify single map!"));
				else
				{
					const FMapInfo& Map = *MapLoad.Maps(ExportMaps - 1);
					FStringOutputDevice Str;
					MapLoad.WriteT3DFile(Str, (ExportMaps - 1), ExportShape);
					FString Filename = FString(TEXT("..") PATH_SEPARATOR) + Map.MapFilename + TEXT(".2ds");
					appSaveStringToFile(Str, *Filename);
					GWarn->Logf(TEXT("Wrote brush file: %ls"), *Filename);
				}
			}
			else
			{
				for (INT iMap = 0; iMap < MapLoad.Maps.Num(); ++iMap)
				{
					if (ExportMaps != 0 && ExportMaps != (iMap + 1))
						continue;
					const FMapInfo& Map = *MapLoad.Maps(iMap);
					FStringOutputDevice Str;
					MapLoad.WriteT3DFile(Str, iMap);

					FString Filename = FString(TEXT("..") PATH_SEPARATOR) + Map.MapFilename + TEXT(".t3d");
					appSaveStringToFile(Str, *Filename);
					GWarn->Logf(TEXT("Wrote map file: %ls"), *Filename);
				}
			}
		}
	}
	delete Ar;
	return 0;
	unguard;
}
