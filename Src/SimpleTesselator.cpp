
#include "DoomWadPrivate.h"

struct F2DPolygon
{
	INT Verts[FPoly::MAX_VERTICES];
	INT NumVerts;
	const FVector2D* VertList;
	int Area;

	F2DPolygon(const INT* Points, const FVector2D* V)
		: NumVerts(3), VertList(V)
	{
		for (INT i = 0; i < 3; ++i)
			Verts[i] = Points[i];
		Update();
	}
	F2DPolygon(const F2DPolygon& A, const F2DPolygon& B, const INT MergeStartA, const INT MergeStartB, const INT MergeEndA, const INT MergeEndB)
		: NumVerts(0), VertList(A.VertList)
	{
		INT i = MergeEndA;
		Verts[NumVerts++] = A.Verts[i];
		for(;;)
		{
			++i;
			if (i == A.NumVerts)
				i = 0;
			if (i == MergeStartA)
				break;
			Verts[NumVerts++] = A.Verts[i];
		}
		Verts[NumVerts++] = A.Verts[MergeStartA];
		i = MergeStartB;
		for (;;)
		{
			++i;
			if (i == B.NumVerts)
				i = 0;
			if (i == MergeEndB)
				break;
			Verts[NumVerts++] = B.Verts[i];
		}
	}
	inline void Clone(const F2DPolygon& Other)
	{
		appMemcpy(this, &Other, sizeof(F2DPolygon));
	}

	inline UBOOL Validate(INT MinVerts) const
	{
		guard(F2DPolygon::Validate);
		if (NumVerts < MinVerts)
			return FALSE;
		FVector2D LastDir = (VertList[Verts[0]] - VertList[Verts[NumVerts - 1]]).SafeNormal();
		INT j;
		for (INT i = 0; i < NumVerts; ++i)
		{
			j = i + 1;
			if (j == NumVerts)
				j = 0;
			FVector2D Dir = (VertList[Verts[j]] - VertList[Verts[i]]).SafeNormal();
			if ((LastDir ^ Dir) < -DELTA)
				return FALSE;
			LastDir = Dir;
		}
		return TRUE;
		unguard;
	}
	inline void Update()
	{
		guard(F2DPolygon::Update);
		F2DBox Box(0);
		for (INT i = 0; i < NumVerts; ++i)
			Box += VertList[Verts[i]];
		Area = appRound(Box.Max.Distance(Box.Min) * 10.f);
		unguard;
	}
	UBOOL TryToMerge(const F2DPolygon& Other)
	{
		guardSlow(F2DPolygon::TryToMerge);
		if ((NumVerts + Other.NumVerts) >= FPoly::VERTEX_THRESHOLD)
			return FALSE;

		for (INT i = 0; i < NumVerts; ++i)
		{
			// Try to find merge start point.
			const INT iStart = Other.FindVertexIndex(Verts[i]);
			if (iStart == INDEX_NONE)
				continue;

			UBOOL bSkip = TRUE;
			for (INT j = (iStart + 1); ; ++j)
			{
				if (j == Other.NumVerts)
					j = 0;
				if (j == iStart)
					break;

				// Make sure there is at least 1 vertex inbetween.
				if (bSkip)
				{
					bSkip = FALSE;
					continue;
				}

				const INT iEnd = FindVertexIndex(Other.Verts[j]);
				if (iEnd == INDEX_NONE || (iEnd == i))
					continue;

				// Make sure there is at least 1 vertex inbetween.
				INT iPrev = i - 1;
				if (iPrev < 0)
					iPrev = NumVerts - 1;
				if (iEnd == iPrev)
					continue;

				F2DPolygon NewPoly(*this, Other, i, iStart, iEnd, j);
				if (NewPoly.Validate(Min<INT>(NumVerts, Other.NumVerts) + 1))
				{
					Clone(NewPoly);
					Update();
					return TRUE;
				}
			}
		}
		return FALSE;
		unguardSlow;
	}
	inline INT FindVertexIndex(const INT inVertex) const
	{
		guardSlow(F2DPolygon::FindVertexIndex);
		for (INT i = 0; i < NumVerts; ++i)
			if (Verts[i] == inVertex)
				return i;
		return INDEX_NONE;
		unguardSlow;
	}
};
inline INT Compare(const F2DPolygon* A, const F2DPolygon* B)
{
	return (B->Area - A->Area);
}

struct FLineCollection : FScopedMemMark
{
private:
	TArray<F2DPolygon*> Polys;
	const FVector2D* Verts;
	TArray<FSimpleTesselator::FOutputTris>& Output;

public:
	FLineCollection(const F2DTessellator& Tes, TArray<FSimpleTesselator::FOutputTris>& Out )
		: Verts(&Tes.Vertices(0)), Output(Out), FScopedMemMark(GMem)
	{
		guard(FLineCollection::FLineCollection);
		Polys.SetSize(Tes.Tris.Num());
		for (INT i = 0; i < Tes.Tris.Num(); ++i)
		{
			Polys(i) = new (GMem) F2DPolygon(Tes.Tris(i).Verts, Verts);
		}
		unguard;
	}
	void MergeLines()
	{
		guard(FLineCollection::MergeLines);
		UBOOL IsMerging = TRUE;
		INT i, j;
		INT NumMerged = 0;
		while (IsMerging)
		{
			IsMerging = FALSE;
			Sort(Polys);
			for (i = 0; i < Polys.Num(); ++i)
			{
				for (j = (i + 1); j < Polys.Num(); ++j)
				{
					if (Polys(i)->TryToMerge(*Polys(j)))
					{
						IsMerging = TRUE;
						Polys.Remove(j);
						++NumMerged;
						break;
					}
				}
				if (IsMerging)
					break;
			}
		}

		if (NumMerged)
			debugf(TEXT("Merged %i polys, from %i to %i polys!"), NumMerged, Polys.Num() + NumMerged, Polys.Num());
		Output.Empty(Polys.Num());
		for (i = 0; i < Polys.Num(); ++i)
		{
			const auto* P = Polys(i);
			auto* Out = new(Output) FSimpleTesselator::FOutputTris();
			Out->Verts.SetSize(P->NumVerts);
			for (j = 0; j < P->NumVerts; ++j)
				Out->Verts(j) = P->Verts[j];
		}
		unguard;
	}
};

void FSimpleTesselator::Tesselate()
{
	guard(FSimpleTesselator::Tesselate);
	const INT NumVerts = Input.Num();
	const FVector2D* Verts = &Input(0);
	F2DTessellator Tesselator;
	Tesselator.Vertices.SetSize(NumVerts);
	INT i;
	for (i = 0; i < NumVerts; ++i)
	{
		Tesselator.Vertices(i) = Verts[i];
		Tesselator.Vertices(i).Y *= -1.f;
	}
	Tesselator.Tesselate();

	FLineCollection Merger(Tesselator, Output);
	Merger.MergeLines();
	unguard;
}
