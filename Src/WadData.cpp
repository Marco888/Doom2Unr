
#include "DoomWadPrivate.h"

FInstTexture FInstTexture::GSkyTexture(SkyTextureName, TT_Sky);
FInstTexture FInstTexture::GNullTexture(NAME_None, TT_Unknown);
TMap<FName, FInstTexture*>* FInstTexture::GTextureMap = nullptr;

FInstTexture* FInstTexture::RegisterTexture(FName TexName)
{
	guard(FInstTexture::RegisterTexture);
	if (!GTextureMap)
	{
		GTextureMap = new TMap<FName, FInstTexture*>;
		GTextureMap->Set(GSkyTexture.TextureName, &GSkyTexture);
		GTextureMap->Set(NAME_None, &GNullTexture);
	}
	FInstTexture* Result = GTextureMap->FindRef(TexName);
	if (!Result)
	{
		Result = new FInstTexture(TexName, TT_Unknown);
		GTextureMap->Set(TexName, Result);
	}
	return Result;
	unguard;
}

FInstTexture* FInstTexture::ReadTextureName(FArchive* Ar, const TCHAR* Prefix)
{
	guard(ReadTextureName);
	FInstTexture* Result = &FInstTexture::GNullTexture;
	BYTE TexName[8];
	Ar->Serialize(TexName, 8);
	const TCHAR* n = ByteToStr(TexName, 8);
	if (appStrcmp(n, TEXT("-")))
	{
		if (Prefix && appStricmp(n, SkyTextureName))
		{
			TCHAR Temp[16];
			appStrcpy(Temp, Prefix);
			appStrcat(Temp, n);
			Result = FInstTexture::RegisterTexture(FName(Temp));
		}
		else Result = FInstTexture::RegisterTexture(FName(n));
	}
	return Result;
	unguard;
}

FInstTexture* FInstTexture::CreateSwitch(const INT SwitchIndex)
{
	guard(FInstTexture::CreateSwitch);
	TCHAR* SwitchName = appStaticString1024();
	appSprintf(SwitchName, TEXT("DoomSwitchMat%i"), SwitchIndex);
	FInstTexture* SwTex = RegisterTexture(FName(SwitchName));
	SwTex->CopyDimensions(*this);
	SwTex->Type = TT_Switch;
	SwTex->FirstFrame = this;
	return SwTex;
	unguard;
}

mobjinfo_t mobjinfo[NUMMOBJTYPES] = {

	{		// MT_PLAYER
	-1,		// doomednum
	S_PLAY,		// spawnstate
	100,		// spawnhealth
	S_PLAY_RUN1,		// seestate
	sfx_None,		// seesound
	0,		// reactiontime
	sfx_None,		// attacksound
	S_PLAY_PAIN,		// painstate
	255,		// painchance
	sfx_plpain,		// painsound
	S_NULL,		// meleestate
	S_PLAY_ATK1,		// missilestate
	S_PLAY_DIE1,		// deathstate
	S_PLAY_XDIE1,		// xdeathstate
	sfx_pldeth,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_PICKUP | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_POSSESSED
	3004,		// doomednum
	S_POSS_STND,		// spawnstate
	20,		// spawnhealth
	S_POSS_RUN1,		// seestate
	sfx_posit1,		// seesound
	8,		// reactiontime
	sfx_pistol,		// attacksound
	S_POSS_PAIN,		// painstate
	200,		// painchance
	sfx_popain,		// painsound
	0,		// meleestate
	S_POSS_ATK1,		// missilestate
	S_POSS_DIE1,		// deathstate
	S_POSS_XDIE1,		// xdeathstate
	sfx_podth1,		// deathsound
	8,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_posact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_POSS_RAISE1		// raisestate
	},

	{		// MT_SHOTGUY
	9,		// doomednum
	S_SPOS_STND,		// spawnstate
	30,		// spawnhealth
	S_SPOS_RUN1,		// seestate
	sfx_posit2,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_SPOS_PAIN,		// painstate
	170,		// painchance
	sfx_popain,		// painsound
	0,		// meleestate
	S_SPOS_ATK1,		// missilestate
	S_SPOS_DIE1,		// deathstate
	S_SPOS_XDIE1,		// xdeathstate
	sfx_podth2,		// deathsound
	8,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_posact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_SPOS_RAISE1		// raisestate
	},

	{		// MT_VILE
	64,		// doomednum
	S_VILE_STND,		// spawnstate
	700,		// spawnhealth
	S_VILE_RUN1,		// seestate
	sfx_vilsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_VILE_PAIN,		// painstate
	10,		// painchance
	sfx_vipain,		// painsound
	0,		// meleestate
	S_VILE_ATK1,		// missilestate
	S_VILE_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_vildth,		// deathsound
	15,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	500,		// mass
	0,		// damage
	sfx_vilact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_FIRE
	-1,		// doomednum
	S_FIRE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_UNDEAD
	66,		// doomednum
	S_SKEL_STND,		// spawnstate
	300,		// spawnhealth
	S_SKEL_RUN1,		// seestate
	sfx_skesit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_SKEL_PAIN,		// painstate
	100,		// painchance
	sfx_popain,		// painsound
	S_SKEL_FIST1,		// meleestate
	S_SKEL_MISS1,		// missilestate
	S_SKEL_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_skedth,		// deathsound
	10,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	500,		// mass
	0,		// damage
	sfx_skeact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_SKEL_RAISE1		// raisestate
	},

	{		// MT_TRACER
	-1,		// doomednum
	S_TRACER,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_skeatk,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_TRACEEXP1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_barexp,		// deathsound
	10 * FRACUNIT,		// speed
	11 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	10,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SMOKE
	-1,		// doomednum
	S_SMOKE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_FATSO
	67,		// doomednum
	S_FATT_STND,		// spawnstate
	600,		// spawnhealth
	S_FATT_RUN1,		// seestate
	sfx_mansit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_FATT_PAIN,		// painstate
	80,		// painchance
	sfx_mnpain,		// painsound
	0,		// meleestate
	S_FATT_ATK1,		// missilestate
	S_FATT_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_mandth,		// deathsound
	8,		// speed
	48 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx_posact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_FATT_RAISE1		// raisestate
	},

	{		// MT_FATSHOT
	-1,		// doomednum
	S_FATSHOT1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_firsht,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_FATSHOTX1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	20 * FRACUNIT,		// speed
	6 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	8,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_CHAINGUY
	65,		// doomednum
	S_CPOS_STND,		// spawnstate
	70,		// spawnhealth
	S_CPOS_RUN1,		// seestate
	sfx_posit2,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_CPOS_PAIN,		// painstate
	170,		// painchance
	sfx_popain,		// painsound
	0,		// meleestate
	S_CPOS_ATK1,		// missilestate
	S_CPOS_DIE1,		// deathstate
	S_CPOS_XDIE1,		// xdeathstate
	sfx_podth2,		// deathsound
	8,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_posact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_CPOS_RAISE1		// raisestate
	},

	{		// MT_TROOP
	3001,		// doomednum
	S_TROO_STND,		// spawnstate
	60,		// spawnhealth
	S_TROO_RUN1,		// seestate
	sfx_bgsit1,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_TROO_PAIN,		// painstate
	200,		// painchance
	sfx_popain,		// painsound
	S_TROO_ATK1,		// meleestate
	S_TROO_ATK1,		// missilestate
	S_TROO_DIE1,		// deathstate
	S_TROO_XDIE1,		// xdeathstate
	sfx_bgdth1,		// deathsound
	8,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_bgact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_TROO_RAISE1		// raisestate
	},

	{		// MT_SERGEANT
	3002,		// doomednum
	S_SARG_STND,		// spawnstate
	150,		// spawnhealth
	S_SARG_RUN1,		// seestate
	sfx_sgtsit,		// seesound
	8,		// reactiontime
	sfx_sgtatk,		// attacksound
	S_SARG_PAIN,		// painstate
	180,		// painchance
	sfx_dmpain,		// painsound
	S_SARG_ATK1,		// meleestate
	0,		// missilestate
	S_SARG_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_sgtdth,		// deathsound
	10,		// speed
	30 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_SARG_RAISE1		// raisestate
	},

	{		// MT_SHADOWS
	58,		// doomednum
	S_SARG_STND,		// spawnstate
	150,		// spawnhealth
	S_SARG_RUN1,		// seestate
	sfx_sgtsit,		// seesound
	8,		// reactiontime
	sfx_sgtatk,		// attacksound
	S_SARG_PAIN,		// painstate
	180,		// painchance
	sfx_dmpain,		// painsound
	S_SARG_ATK1,		// meleestate
	0,		// missilestate
	S_SARG_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_sgtdth,		// deathsound
	10,		// speed
	30 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_SHADOW | MF_COUNTKILL,		// flags
	S_SARG_RAISE1		// raisestate
	},

	{		// MT_HEAD
	3005,		// doomednum
	S_HEAD_STND,		// spawnstate
	400,		// spawnhealth
	S_HEAD_RUN1,		// seestate
	sfx_cacsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_HEAD_PAIN,		// painstate
	128,		// painchance
	sfx_dmpain,		// painsound
	0,		// meleestate
	S_HEAD_ATK1,		// missilestate
	S_HEAD_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_cacdth,		// deathsound
	8,		// speed
	31 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,		// flags
	S_HEAD_RAISE1		// raisestate
	},

	{		// MT_BRUISER
	3003,		// doomednum
	S_BOSS_STND,		// spawnstate
	1000,		// spawnhealth
	S_BOSS_RUN1,		// seestate
	sfx_brssit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_BOSS_PAIN,		// painstate
	50,		// painchance
	sfx_dmpain,		// painsound
	S_BOSS_ATK1,		// meleestate
	S_BOSS_ATK1,		// missilestate
	S_BOSS_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_brsdth,		// deathsound
	8,		// speed
	24 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_BOSS_RAISE1		// raisestate
	},

	{		// MT_BRUISERSHOT
	-1,		// doomednum
	S_BRBALL1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_firsht,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BRBALLX1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	15 * FRACUNIT,		// speed
	6 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	8,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_KNIGHT
	69,		// doomednum
	S_BOS2_STND,		// spawnstate
	500,		// spawnhealth
	S_BOS2_RUN1,		// seestate
	sfx_kntsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_BOS2_PAIN,		// painstate
	50,		// painchance
	sfx_dmpain,		// painsound
	S_BOS2_ATK1,		// meleestate
	S_BOS2_ATK1,		// missilestate
	S_BOS2_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_kntdth,		// deathsound
	8,		// speed
	24 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_BOS2_RAISE1		// raisestate
	},

	{		// MT_SKULL
	3006,		// doomednum
	S_SKULL_STND,		// spawnstate
	100,		// spawnhealth
	S_SKULL_RUN1,		// seestate
	0,		// seesound
	8,		// reactiontime
	sfx_sklatk,		// attacksound
	S_SKULL_PAIN,		// painstate
	256,		// painchance
	sfx_dmpain,		// painsound
	0,		// meleestate
	S_SKULL_ATK1,		// missilestate
	S_SKULL_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	8,		// speed
	16 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	50,		// mass
	3,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SPIDER
	7,		// doomednum
	S_SPID_STND,		// spawnstate
	3000,		// spawnhealth
	S_SPID_RUN1,		// seestate
	sfx_spisit,		// seesound
	8,		// reactiontime
	sfx_shotgn,		// attacksound
	S_SPID_PAIN,		// painstate
	40,		// painchance
	sfx_dmpain,		// painsound
	0,		// meleestate
	S_SPID_ATK1,		// missilestate
	S_SPID_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_spidth,		// deathsound
	12,		// speed
	128 * FRACUNIT,		// radius
	100 * FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BABY
	68,		// doomednum
	S_BSPI_STND,		// spawnstate
	500,		// spawnhealth
	S_BSPI_SIGHT,		// seestate
	sfx_bspsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_BSPI_PAIN,		// painstate
	128,		// painchance
	sfx_dmpain,		// painsound
	0,		// meleestate
	S_BSPI_ATK1,		// missilestate
	S_BSPI_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_bspdth,		// deathsound
	12,		// speed
	64 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	600,		// mass
	0,		// damage
	sfx_bspact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_BSPI_RAISE1		// raisestate
	},

	{		// MT_CYBORG
	16,		// doomednum
	S_CYBER_STND,		// spawnstate
	4000,		// spawnhealth
	S_CYBER_RUN1,		// seestate
	sfx_cybsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_CYBER_PAIN,		// painstate
	20,		// painchance
	sfx_dmpain,		// painsound
	0,		// meleestate
	S_CYBER_ATK1,		// missilestate
	S_CYBER_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_cybdth,		// deathsound
	16,		// speed
	40 * FRACUNIT,		// radius
	110 * FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_PAIN
	71,		// doomednum
	S_PAIN_STND,		// spawnstate
	400,		// spawnhealth
	S_PAIN_RUN1,		// seestate
	sfx_pesit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_PAIN_PAIN,		// painstate
	128,		// painchance
	sfx_pepain,		// painsound
	0,		// meleestate
	S_PAIN_ATK1,		// missilestate
	S_PAIN_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_pedth,		// deathsound
	8,		// speed
	31 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx_dmact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,		// flags
	S_PAIN_RAISE1		// raisestate
	},

	{		// MT_WOLFSS
	84,		// doomednum
	S_SSWV_STND,		// spawnstate
	50,		// spawnhealth
	S_SSWV_RUN1,		// seestate
	sfx_sssit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S_SSWV_PAIN,		// painstate
	170,		// painchance
	sfx_popain,		// painsound
	0,		// meleestate
	S_SSWV_ATK1,		// missilestate
	S_SSWV_DIE1,		// deathstate
	S_SSWV_XDIE1,		// xdeathstate
	sfx_ssdth,		// deathsound
	8,		// speed
	20 * FRACUNIT,		// radius
	56 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_posact,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_SSWV_RAISE1		// raisestate
	},

	{		// MT_KEEN
	72,		// doomednum
	S_KEENSTND,		// spawnstate
	100,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_KEENPAIN,		// painstate
	256,		// painchance
	sfx_keenpn,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_COMMKEEN,		// deathstate
	S_NULL,		// xdeathstate
	sfx_keendt,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	72 * FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY | MF_SHOOTABLE | MF_COUNTKILL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BOSSBRAIN
	88,		// doomednum
	S_BRAIN,		// spawnstate
	250,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_BRAIN_PAIN,		// painstate
	255,		// painchance
	sfx_bospn,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BRAIN_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_bosdth,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SHOOTABLE,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BOSSSPIT
	89,		// doomednum
	S_BRAINEYE,		// spawnstate
	1000,		// spawnhealth
	S_BRAINEYESEE,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	32 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOSECTOR,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BOSSTARGET
	87,		// doomednum
	S_NULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	32 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOSECTOR,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SPAWNSHOT
	-1,		// doomednum
	S_SPAWN1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_bospit,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	10 * FRACUNIT,		// speed
	6 * FRACUNIT,		// radius
	32 * FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY | MF_NOCLIP,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SPAWNFIRE
	-1,		// doomednum
	S_SPAWNFIRE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BARREL
	2035,		// doomednum
	S_BAR1,		// spawnstate
	20,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BEXP,		// deathstate
	S_NULL,		// xdeathstate
	sfx_barexp,		// deathsound
	0,		// speed
	10 * FRACUNIT,		// radius
	42 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SHOOTABLE | MF_NOBLOOD,		// flags
	S_NULL		// raisestate
	},

	{		// MT_TROOPSHOT
	-1,		// doomednum
	S_TBALL1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_firsht,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_TBALLX1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	10 * FRACUNIT,		// speed
	6 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_HEADSHOT
	-1,		// doomednum
	S_RBALL1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_firsht,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_RBALLX1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	10 * FRACUNIT,		// speed
	6 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_ROCKET
	-1,		// doomednum
	S_ROCKET,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_rlaunc,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_EXPLODE1,		// deathstate
	S_NULL,		// xdeathstate
	sfx_barexp,		// deathsound
	20 * FRACUNIT,		// speed
	11 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	20,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_PLASMA
	-1,		// doomednum
	S_PLASBALL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_plasma,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_PLASEXP,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	25 * FRACUNIT,		// speed
	13 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BFG
	-1,		// doomednum
	S_BFGSHOT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	0,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BFGLAND,		// deathstate
	S_NULL,		// xdeathstate
	sfx_rxplod,		// deathsound
	25 * FRACUNIT,		// speed
	13 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	100,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_ARACHPLAZ
	-1,		// doomednum
	S_ARACH_PLAZ,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_plasma,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_ARACH_PLEX,		// deathstate
	S_NULL,		// xdeathstate
	sfx_firxpl,		// deathsound
	25 * FRACUNIT,		// speed
	13 * FRACUNIT,		// radius
	8 * FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_PUFF
	-1,		// doomednum
	S_PUFF1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_BLOOD
	-1,		// doomednum
	S_BLOOD1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP,		// flags
	S_NULL		// raisestate
	},

	{		// MT_TFOG
	-1,		// doomednum
	S_TFOG,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_IFOG
	-1,		// doomednum
	S_IFOG,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_TELEPORTMAN
	14,		// doomednum
	S_NULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOSECTOR,		// flags
	S_NULL		// raisestate
	},

	{		// MT_EXTRABFG
	-1,		// doomednum
	S_BFGEXP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC0
	2018,		// doomednum
	S_ARM1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC1
	2019,		// doomednum
	S_ARM2,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC2
	2014,		// doomednum
	S_BON1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC3
	2015,		// doomednum
	S_BON2,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC4
	5,		// doomednum
	S_BKEY,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC5
	13,		// doomednum
	S_RKEY,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC6
	6,		// doomednum
	S_YKEY,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC7
	39,		// doomednum
	S_YSKULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC8
	38,		// doomednum
	S_RSKULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC9
	40,		// doomednum
	S_BSKULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_NOTDMATCH,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC10
	2011,		// doomednum
	S_STIM,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC11
	2012,		// doomednum
	S_MEDI,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC12
	2013,		// doomednum
	S_SOUL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_INV
	2022,		// doomednum
	S_PINV,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC13
	2023,		// doomednum
	S_PSTR,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_INS
	2024,		// doomednum
	S_PINS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC14
	2025,		// doomednum
	S_SUIT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC15
	2026,		// doomednum
	S_PMAP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC16
	2045,		// doomednum
	S_PVIS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MEGA
	83,		// doomednum
	S_MEGA,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL | MF_COUNTITEM,		// flags
	S_NULL		// raisestate
	},

	{		// MT_CLIP
	2007,		// doomednum
	S_CLIP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC17
	2048,		// doomednum
	S_AMMO,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC18
	2010,		// doomednum
	S_ROCK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC19
	2046,		// doomednum
	S_BROK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC20
	2047,		// doomednum
	S_CELL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC21
	17,		// doomednum
	S_CELP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC22
	2008,		// doomednum
	S_SHEL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC23
	2049,		// doomednum
	S_SBOX,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC24
	8,		// doomednum
	S_BPAK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC25
	2006,		// doomednum
	S_BFUG,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_CHAINGUN
	2002,		// doomednum
	S_MGUN,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC26
	2005,		// doomednum
	S_CSAW,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC27
	2003,		// doomednum
	S_LAUN,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC28
	2004,		// doomednum
	S_PLAS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SHOTGUN
	2001,		// doomednum
	S_SHOT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_SUPERSHOTGUN
	82,		// doomednum
	S_SHOT2,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPECIAL,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC29
	85,		// doomednum
	S_TECHLAMP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC30
	86,		// doomednum
	S_TECH2LAMP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC31
	2028,		// doomednum
	S_COLU,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC32
	30,		// doomednum
	S_TALLGRNCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC33
	31,		// doomednum
	S_SHRTGRNCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC34
	32,		// doomednum
	S_TALLREDCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC35
	33,		// doomednum
	S_SHRTREDCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC36
	37,		// doomednum
	S_SKULLCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC37
	36,		// doomednum
	S_HEARTCOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC38
	41,		// doomednum
	S_EVILEYE,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC39
	42,		// doomednum
	S_FLOATSKULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC40
	43,		// doomednum
	S_TORCHTREE,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC41
	44,		// doomednum
	S_BLUETORCH,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC42
	45,		// doomednum
	S_GREENTORCH,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC43
	46,		// doomednum
	S_REDTORCH,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC44
	55,		// doomednum
	S_BTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC45
	56,		// doomednum
	S_GTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC46
	57,		// doomednum
	S_RTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC47
	47,		// doomednum
	S_STALAGTITE,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC48
	48,		// doomednum
	S_TECHPILLAR,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC49
	34,		// doomednum
	S_CANDLESTIK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC50
	35,		// doomednum
	S_CANDELABRA,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC51
	49,		// doomednum
	S_BLOODYTWITCH,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	68 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC52
	50,		// doomednum
	S_MEAT2,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	84 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC53
	51,		// doomednum
	S_MEAT3,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	84 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC54
	52,		// doomednum
	S_MEAT4,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	68 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC55
	53,		// doomednum
	S_MEAT5,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	52 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC56
	59,		// doomednum
	S_MEAT2,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	84 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC57
	60,		// doomednum
	S_MEAT4,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	68 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC58
	61,		// doomednum
	S_MEAT3,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	52 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC59
	62,		// doomednum
	S_MEAT5,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	52 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC60
	63,		// doomednum
	S_BLOODYTWITCH,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	68 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC61
	22,		// doomednum
	S_HEAD_DIE6,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC62
	15,		// doomednum
	S_PLAY_DIE7,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC63
	18,		// doomednum
	S_POSS_DIE5,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC64
	21,		// doomednum
	S_SARG_DIE6,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC65
	23,		// doomednum
	S_SKULL_DIE6,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC66
	20,		// doomednum
	S_TROO_DIE5,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC67
	19,		// doomednum
	S_SPOS_DIE5,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC68
	10,		// doomednum
	S_PLAY_XDIE9,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC69
	12,		// doomednum
	S_PLAY_XDIE9,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC70
	28,		// doomednum
	S_HEADSONSTICK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC71
	24,		// doomednum
	S_GIBS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	0,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC72
	27,		// doomednum
	S_HEADONASTICK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC73
	29,		// doomednum
	S_HEADCANDLES,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC74
	25,		// doomednum
	S_DEADSTICK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC75
	26,		// doomednum
	S_LIVESTICK,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC76
	54,		// doomednum
	S_BIGTREE,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	32 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC77
	70,		// doomednum
	S_BBAR1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC78
	73,		// doomednum
	S_HANGNOGUTS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	88 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC79
	74,		// doomednum
	S_HANGBNOBRAIN,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	88 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC80
	75,		// doomednum
	S_HANGTLOOKDN,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC81
	76,		// doomednum
	S_HANGTSKULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC82
	77,		// doomednum
	S_HANGTLOOKUP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC83
	78,		// doomednum
	S_HANGTNOBRAIN,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	16 * FRACUNIT,		// radius
	64 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC84
	79,		// doomednum
	S_COLONGIBS,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC85
	80,		// doomednum
	S_SMALLPOOL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP,		// flags
	S_NULL		// raisestate
	},

	{		// MT_MISC86
	81,		// doomednum
	S_BRAINSTEM,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20 * FRACUNIT,		// radius
	16 * FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
	MF_NOBLOCKMAP,		// flags
	S_NULL		// raisestate
	}
};

char* sprnames[spritenum_t::NUMSPRITES] = {
	"TROO","SHTG","PUNG","PISG","PISF","SHTF","SHT2","CHGG","CHGF","MISG",
	"MISF","SAWG","PLSG","PLSF","BFGG","BFGF","BLUD","PUFF","BAL1","BAL2",
	"PLSS","PLSE","MISL","BFS1","BFE1","BFE2","TFOG","IFOG","PLAY","POSS",
	"SPOS","VILE","FIRE","FATB","FBXP","SKEL","MANF","FATT","CPOS","SARG",
	"HEAD","BAL7","BOSS","BOS2","SKUL","SPID","BSPI","APLS","APBX","CYBR",
	"PAIN","SSWV","KEEN","BBRN","BOSF","ARM1","ARM2","BAR1","BEXP","FCAN",
	"BON1","BON2","BKEY","RKEY","YKEY","BSKU","RSKU","YSKU","STIM","MEDI",
	"SOUL","PINV","PSTR","PINS","MEGA","SUIT","PMAP","PVIS","CLIP","AMMO",
	"ROCK","BROK","CELL","CELP","SHEL","SBOX","BPAK","BFUG","MGUN","CSAW",
	"LAUN","PLAS","SHOT","SGN2","COLU","SMT2","GOR1","POL2","POL5","POL4",
	"POL3","POL1","POL6","GOR2","GOR3","GOR4","GOR5","SMIT","COL1","COL2",
	"COL3","COL4","CAND","CBRA","COL6","TRE1","TRE2","ELEC","CEYE","FSKU",
	"COL5","TBLU","TGRN","TRED","SMBT","SMGT","SMRT","HDB1","HDB2","HDB3",
	"HDB4","HDB5","HDB6","POB1","POB2","BRS1","TLMP","TLP2"
};

#define DECL_ACT(actname) const TCHAR* actname() { return TEXT(#actname); }
#define DECL_ACT_NIL(actname) const TCHAR* actname() { return nullptr; }

DECL_ACT(A_Light0);
DECL_ACT(A_WeaponReady);
DECL_ACT(A_Lower);
DECL_ACT(A_Raise);
DECL_ACT(A_Punch);
DECL_ACT(A_ReFire);
DECL_ACT(A_FirePistol);
DECL_ACT(A_Light1);
DECL_ACT(A_FireShotgun);
DECL_ACT(A_Light2);
DECL_ACT(A_FireShotgun2);
DECL_ACT(A_CheckReload);
DECL_ACT(A_OpenShotgun2);
DECL_ACT(A_LoadShotgun2);
DECL_ACT(A_CloseShotgun2);
DECL_ACT(A_FireCGun);
DECL_ACT(A_GunFlash);
DECL_ACT(A_FireMissile);
DECL_ACT(A_Saw);
DECL_ACT(A_FirePlasma);
DECL_ACT(A_BFGsound);
DECL_ACT(A_FireBFG);
DECL_ACT(A_BFGSpray);
DECL_ACT(A_Explode);
DECL_ACT(A_Pain);
DECL_ACT(A_PlayerScream);
DECL_ACT(A_Fall);
DECL_ACT(A_XScream);
DECL_ACT_NIL(A_Look);
DECL_ACT_NIL(A_Chase);
DECL_ACT(A_FaceTarget);
DECL_ACT(A_PosAttack);
DECL_ACT(A_Scream);
DECL_ACT(A_SPosAttack);
DECL_ACT(A_VileChase);
DECL_ACT(A_VileStart);
DECL_ACT(A_VileTarget);
DECL_ACT(A_VileAttack);
DECL_ACT_NIL(A_StartFire);
DECL_ACT_NIL(A_Fire);
DECL_ACT(A_FireCrackle);
DECL_ACT(A_Tracer);
DECL_ACT(A_SkelWhoosh);
DECL_ACT(A_SkelFist);
DECL_ACT(A_SkelMissile);
DECL_ACT(A_FatRaise);
DECL_ACT(A_FatAttack1);
DECL_ACT(A_FatAttack2);
DECL_ACT(A_FatAttack3);
DECL_ACT(A_BossDeath);
DECL_ACT(A_CPosAttack);
DECL_ACT(A_CPosRefire);
DECL_ACT(A_TroopAttack);
DECL_ACT(A_SargAttack);
DECL_ACT(A_HeadAttack);
DECL_ACT(A_BruisAttack);
DECL_ACT(A_SkullAttack);
DECL_ACT(A_Metal);
DECL_ACT(A_SpidRefire);
DECL_ACT(A_BabyMetal);
DECL_ACT(A_BspiAttack);
DECL_ACT(A_Hoof);
DECL_ACT(A_CyberAttack);
DECL_ACT(A_PainAttack);
DECL_ACT(A_PainDie);
DECL_ACT_NIL(A_KeenDie);
DECL_ACT(A_BrainPain);
DECL_ACT(A_BrainScream);
DECL_ACT(A_BrainDie);
DECL_ACT(A_BrainAwake);
DECL_ACT(A_BrainSpit);
DECL_ACT(A_SpawnSound);
DECL_ACT(A_SpawnFly);
DECL_ACT(A_BrainExplode);

state_t	states[NUMSTATES] = {
	{SPR_TROO,0,-1,{NULL},S_NULL,0,0},	// S_NULL
	{SPR_SHTG,4,0,{A_Light0},S_NULL,0,0},	// S_LIGHTDONE
	{SPR_PUNG,0,1,{A_WeaponReady},S_PUNCH,0,0},	// S_PUNCH
	{SPR_PUNG,0,1,{A_Lower},S_PUNCHDOWN,0,0},	// S_PUNCHDOWN
	{SPR_PUNG,0,1,{A_Raise},S_PUNCHUP,0,0},	// S_PUNCHUP
	{SPR_PUNG,1,4,{NULL},S_PUNCH2,0,0},		// S_PUNCH1
	{SPR_PUNG,2,4,{A_Punch},S_PUNCH3,0,0},	// S_PUNCH2
	{SPR_PUNG,3,5,{NULL},S_PUNCH4,0,0},		// S_PUNCH3
	{SPR_PUNG,2,4,{NULL},S_PUNCH5,0,0},		// S_PUNCH4
	{SPR_PUNG,1,5,{A_ReFire},S_PUNCH,0,0},	// S_PUNCH5
	{SPR_PISG,0,1,{A_WeaponReady},S_PISTOL,0,0},// S_PISTOL
	{SPR_PISG,0,1,{A_Lower},S_PISTOLDOWN,0,0},	// S_PISTOLDOWN
	{SPR_PISG,0,1,{A_Raise},S_PISTOLUP,0,0},	// S_PISTOLUP
	{SPR_PISG,0,4,{NULL},S_PISTOL2,0,0},	// S_PISTOL1
	{SPR_PISG,1,6,{A_FirePistol},S_PISTOL3,0,0},// S_PISTOL2
	{SPR_PISG,2,4,{NULL},S_PISTOL4,0,0},	// S_PISTOL3
	{SPR_PISG,1,5,{A_ReFire},S_PISTOL,0,0},	// S_PISTOL4
	{SPR_PISF,32768,7,{A_Light1},S_LIGHTDONE,0,0},	// S_PISTOLFLASH
	{SPR_SHTG,0,1,{A_WeaponReady},S_SGUN,0,0},	// S_SGUN
	{SPR_SHTG,0,1,{A_Lower},S_SGUNDOWN,0,0},	// S_SGUNDOWN
	{SPR_SHTG,0,1,{A_Raise},S_SGUNUP,0,0},	// S_SGUNUP
	{SPR_SHTG,0,3,{NULL},S_SGUN2,0,0},	// S_SGUN1
	{SPR_SHTG,0,7,{A_FireShotgun},S_SGUN3,0,0},	// S_SGUN2
	{SPR_SHTG,1,5,{NULL},S_SGUN4,0,0},	// S_SGUN3
	{SPR_SHTG,2,5,{NULL},S_SGUN5,0,0},	// S_SGUN4
	{SPR_SHTG,3,4,{NULL},S_SGUN6,0,0},	// S_SGUN5
	{SPR_SHTG,2,5,{NULL},S_SGUN7,0,0},	// S_SGUN6
	{SPR_SHTG,1,5,{NULL},S_SGUN8,0,0},	// S_SGUN7
	{SPR_SHTG,0,3,{NULL},S_SGUN9,0,0},	// S_SGUN8
	{SPR_SHTG,0,7,{A_ReFire},S_SGUN,0,0},	// S_SGUN9
	{SPR_SHTF,32768,4,{A_Light1},S_SGUNFLASH2,0,0},	// S_SGUNFLASH1
	{SPR_SHTF,32769,3,{A_Light2},S_LIGHTDONE,0,0},	// S_SGUNFLASH2
	{SPR_SHT2,0,1,{A_WeaponReady},S_DSGUN,0,0},	// S_DSGUN
	{SPR_SHT2,0,1,{A_Lower},S_DSGUNDOWN,0,0},	// S_DSGUNDOWN
	{SPR_SHT2,0,1,{A_Raise},S_DSGUNUP,0,0},	// S_DSGUNUP
	{SPR_SHT2,0,3,{NULL},S_DSGUN2,0,0},	// S_DSGUN1
	{SPR_SHT2,0,7,{A_FireShotgun2},S_DSGUN3,0,0},	// S_DSGUN2
	{SPR_SHT2,1,7,{NULL},S_DSGUN4,0,0},	// S_DSGUN3
	{SPR_SHT2,2,7,{A_CheckReload},S_DSGUN5,0,0},	// S_DSGUN4
	{SPR_SHT2,3,7,{A_OpenShotgun2},S_DSGUN6,0,0},	// S_DSGUN5
	{SPR_SHT2,4,7,{NULL},S_DSGUN7,0,0},	// S_DSGUN6
	{SPR_SHT2,5,7,{A_LoadShotgun2},S_DSGUN8,0,0},	// S_DSGUN7
	{SPR_SHT2,6,6,{NULL},S_DSGUN9,0,0},	// S_DSGUN8
	{SPR_SHT2,7,6,{A_CloseShotgun2},S_DSGUN10,0,0},	// S_DSGUN9
	{SPR_SHT2,0,5,{A_ReFire},S_DSGUN,0,0},	// S_DSGUN10
	{SPR_SHT2,1,7,{NULL},S_DSNR2,0,0},	// S_DSNR1
	{SPR_SHT2,0,3,{NULL},S_DSGUNDOWN,0,0},	// S_DSNR2
	{SPR_SHT2,32776,5,{A_Light1},S_DSGUNFLASH2,0,0},	// S_DSGUNFLASH1
	{SPR_SHT2,32777,4,{A_Light2},S_LIGHTDONE,0,0},	// S_DSGUNFLASH2
	{SPR_CHGG,0,1,{A_WeaponReady},S_CHAIN,0,0},	// S_CHAIN
	{SPR_CHGG,0,1,{A_Lower},S_CHAINDOWN,0,0},	// S_CHAINDOWN
	{SPR_CHGG,0,1,{A_Raise},S_CHAINUP,0,0},	// S_CHAINUP
	{SPR_CHGG,0,4,{A_FireCGun},S_CHAIN2,0,0},	// S_CHAIN1
	{SPR_CHGG,1,4,{A_FireCGun},S_CHAIN3,0,0},	// S_CHAIN2
	{SPR_CHGG,1,0,{A_ReFire},S_CHAIN,0,0},	// S_CHAIN3
	{SPR_CHGF,32768,5,{A_Light1},S_LIGHTDONE,0,0},	// S_CHAINFLASH1
	{SPR_CHGF,32769,5,{A_Light2},S_LIGHTDONE,0,0},	// S_CHAINFLASH2
	{SPR_MISG,0,1,{A_WeaponReady},S_MISSILE,0,0},	// S_MISSILE
	{SPR_MISG,0,1,{A_Lower},S_MISSILEDOWN,0,0},	// S_MISSILEDOWN
	{SPR_MISG,0,1,{A_Raise},S_MISSILEUP,0,0},	// S_MISSILEUP
	{SPR_MISG,1,8,{A_GunFlash},S_MISSILE2,0,0},	// S_MISSILE1
	{SPR_MISG,1,12,{A_FireMissile},S_MISSILE3,0,0},	// S_MISSILE2
	{SPR_MISG,1,0,{A_ReFire},S_MISSILE,0,0},	// S_MISSILE3
	{SPR_MISF,32768,3,{A_Light1},S_MISSILEFLASH2,0,0},	// S_MISSILEFLASH1
	{SPR_MISF,32769,4,{NULL},S_MISSILEFLASH3,0,0},	// S_MISSILEFLASH2
	{SPR_MISF,32770,4,{A_Light2},S_MISSILEFLASH4,0,0},	// S_MISSILEFLASH3
	{SPR_MISF,32771,4,{A_Light2},S_LIGHTDONE,0,0},	// S_MISSILEFLASH4
	{SPR_SAWG,2,4,{A_WeaponReady},S_SAWB,0,0},	// S_SAW
	{SPR_SAWG,3,4,{A_WeaponReady},S_SAW,0,0},	// S_SAWB
	{SPR_SAWG,2,1,{A_Lower},S_SAWDOWN,0,0},	// S_SAWDOWN
	{SPR_SAWG,2,1,{A_Raise},S_SAWUP,0,0},	// S_SAWUP
	{SPR_SAWG,0,4,{A_Saw},S_SAW2,0,0},	// S_SAW1
	{SPR_SAWG,1,4,{A_Saw},S_SAW3,0,0},	// S_SAW2
	{SPR_SAWG,1,0,{A_ReFire},S_SAW,0,0},	// S_SAW3
	{SPR_PLSG,0,1,{A_WeaponReady},S_PLASMA,0,0},	// S_PLASMA
	{SPR_PLSG,0,1,{A_Lower},S_PLASMADOWN,0,0},	// S_PLASMADOWN
	{SPR_PLSG,0,1,{A_Raise},S_PLASMAUP,0,0},	// S_PLASMAUP
	{SPR_PLSG,0,3,{A_FirePlasma},S_PLASMA2,0,0},	// S_PLASMA1
	{SPR_PLSG,1,20,{A_ReFire},S_PLASMA,0,0},	// S_PLASMA2
	{SPR_PLSF,32768,4,{A_Light1},S_LIGHTDONE,0,0},	// S_PLASMAFLASH1
	{SPR_PLSF,32769,4,{A_Light1},S_LIGHTDONE,0,0},	// S_PLASMAFLASH2
	{SPR_BFGG,0,1,{A_WeaponReady},S_BFG,0,0},	// S_BFG
	{SPR_BFGG,0,1,{A_Lower},S_BFGDOWN,0,0},	// S_BFGDOWN
	{SPR_BFGG,0,1,{A_Raise},S_BFGUP,0,0},	// S_BFGUP
	{SPR_BFGG,0,20,{A_BFGsound},S_BFG2,0,0},	// S_BFG1
	{SPR_BFGG,1,10,{A_GunFlash},S_BFG3,0,0},	// S_BFG2
	{SPR_BFGG,1,10,{A_FireBFG},S_BFG4,0,0},	// S_BFG3
	{SPR_BFGG,1,20,{A_ReFire},S_BFG,0,0},	// S_BFG4
	{SPR_BFGF,32768,11,{A_Light1},S_BFGFLASH2,0,0},	// S_BFGFLASH1
	{SPR_BFGF,32769,6,{A_Light2},S_LIGHTDONE,0,0},	// S_BFGFLASH2
	{SPR_BLUD,2,8,{NULL},S_BLOOD2,0,0},	// S_BLOOD1
	{SPR_BLUD,1,8,{NULL},S_BLOOD3,0,0},	// S_BLOOD2
	{SPR_BLUD,0,8,{NULL},S_NULL,0,0},	// S_BLOOD3
	{SPR_PUFF,32768,4,{NULL},S_PUFF2,0,0},	// S_PUFF1
	{SPR_PUFF,1,4,{NULL},S_PUFF3,0,0},	// S_PUFF2
	{SPR_PUFF,2,4,{NULL},S_PUFF4,0,0},	// S_PUFF3
	{SPR_PUFF,3,4,{NULL},S_NULL,0,0},	// S_PUFF4
	{SPR_BAL1,32768,4,{NULL},S_TBALL2,0,0},	// S_TBALL1
	{SPR_BAL1,32769,4,{NULL},S_TBALL1,0,0},	// S_TBALL2
	{SPR_BAL1,32770,6,{NULL},S_TBALLX2,0,0},	// S_TBALLX1
	{SPR_BAL1,32771,6,{NULL},S_TBALLX3,0,0},	// S_TBALLX2
	{SPR_BAL1,32772,6,{NULL},S_NULL,0,0},	// S_TBALLX3
	{SPR_BAL2,32768,4,{NULL},S_RBALL2,0,0},	// S_RBALL1
	{SPR_BAL2,32769,4,{NULL},S_RBALL1,0,0},	// S_RBALL2
	{SPR_BAL2,32770,6,{NULL},S_RBALLX2,0,0},	// S_RBALLX1
	{SPR_BAL2,32771,6,{NULL},S_RBALLX3,0,0},	// S_RBALLX2
	{SPR_BAL2,32772,6,{NULL},S_NULL,0,0},	// S_RBALLX3
	{SPR_PLSS,32768,6,{NULL},S_PLASBALL2,0,0},	// S_PLASBALL
	{SPR_PLSS,32769,6,{NULL},S_PLASBALL,0,0},	// S_PLASBALL2
	{SPR_PLSE,32768,4,{NULL},S_PLASEXP2,0,0},	// S_PLASEXP
	{SPR_PLSE,32769,4,{NULL},S_PLASEXP3,0,0},	// S_PLASEXP2
	{SPR_PLSE,32770,4,{NULL},S_PLASEXP4,0,0},	// S_PLASEXP3
	{SPR_PLSE,32771,4,{NULL},S_PLASEXP5,0,0},	// S_PLASEXP4
	{SPR_PLSE,32772,4,{NULL},S_NULL,0,0},	// S_PLASEXP5
	{SPR_MISL,32768,1,{NULL},S_ROCKET,0,0},	// S_ROCKET
	{SPR_BFS1,32768,4,{NULL},S_BFGSHOT2,0,0},	// S_BFGSHOT
	{SPR_BFS1,32769,4,{NULL},S_BFGSHOT,0,0},	// S_BFGSHOT2
	{SPR_BFE1,32768,8,{NULL},S_BFGLAND2,0,0},	// S_BFGLAND
	{SPR_BFE1,32769,8,{NULL},S_BFGLAND3,0,0},	// S_BFGLAND2
	{SPR_BFE1,32770,8,{A_BFGSpray},S_BFGLAND4,0,0},	// S_BFGLAND3
	{SPR_BFE1,32771,8,{NULL},S_BFGLAND5,0,0},	// S_BFGLAND4
	{SPR_BFE1,32772,8,{NULL},S_BFGLAND6,0,0},	// S_BFGLAND5
	{SPR_BFE1,32773,8,{NULL},S_NULL,0,0},	// S_BFGLAND6
	{SPR_BFE2,32768,8,{NULL},S_BFGEXP2,0,0},	// S_BFGEXP
	{SPR_BFE2,32769,8,{NULL},S_BFGEXP3,0,0},	// S_BFGEXP2
	{SPR_BFE2,32770,8,{NULL},S_BFGEXP4,0,0},	// S_BFGEXP3
	{SPR_BFE2,32771,8,{NULL},S_NULL,0,0},	// S_BFGEXP4
	{SPR_MISL,32769,8,{A_Explode},S_EXPLODE2,0,0},	// S_EXPLODE1
	{SPR_MISL,32770,6,{NULL},S_EXPLODE3,0,0},	// S_EXPLODE2
	{SPR_MISL,32771,4,{NULL},S_NULL,0,0},	// S_EXPLODE3
	{SPR_TFOG,32768,6,{NULL},S_TFOG01,0,0},	// S_TFOG
	{SPR_TFOG,32769,6,{NULL},S_TFOG02,0,0},	// S_TFOG01
	{SPR_TFOG,32768,6,{NULL},S_TFOG2,0,0},	// S_TFOG02
	{SPR_TFOG,32769,6,{NULL},S_TFOG3,0,0},	// S_TFOG2
	{SPR_TFOG,32770,6,{NULL},S_TFOG4,0,0},	// S_TFOG3
	{SPR_TFOG,32771,6,{NULL},S_TFOG5,0,0},	// S_TFOG4
	{SPR_TFOG,32772,6,{NULL},S_TFOG6,0,0},	// S_TFOG5
	{SPR_TFOG,32773,6,{NULL},S_TFOG7,0,0},	// S_TFOG6
	{SPR_TFOG,32774,6,{NULL},S_TFOG8,0,0},	// S_TFOG7
	{SPR_TFOG,32775,6,{NULL},S_TFOG9,0,0},	// S_TFOG8
	{SPR_TFOG,32776,6,{NULL},S_TFOG10,0,0},	// S_TFOG9
	{SPR_TFOG,32777,6,{NULL},S_NULL,0,0},	// S_TFOG10
	{SPR_IFOG,32768,6,{NULL},S_IFOG01,0,0},	// S_IFOG
	{SPR_IFOG,32769,6,{NULL},S_IFOG02,0,0},	// S_IFOG01
	{SPR_IFOG,32768,6,{NULL},S_IFOG2,0,0},	// S_IFOG02
	{SPR_IFOG,32769,6,{NULL},S_IFOG3,0,0},	// S_IFOG2
	{SPR_IFOG,32770,6,{NULL},S_IFOG4,0,0},	// S_IFOG3
	{SPR_IFOG,32771,6,{NULL},S_IFOG5,0,0},	// S_IFOG4
	{SPR_IFOG,32772,6,{NULL},S_NULL,0,0},	// S_IFOG5
	{SPR_PLAY,0,-1,{NULL},S_NULL,0,0},	// S_PLAY
	{SPR_PLAY,0,4,{NULL},S_PLAY_RUN2,0,0},	// S_PLAY_RUN1
	{SPR_PLAY,1,4,{NULL},S_PLAY_RUN3,0,0},	// S_PLAY_RUN2
	{SPR_PLAY,2,4,{NULL},S_PLAY_RUN4,0,0},	// S_PLAY_RUN3
	{SPR_PLAY,3,4,{NULL},S_PLAY_RUN1,0,0},	// S_PLAY_RUN4
	{SPR_PLAY,4,12,{NULL},S_PLAY,0,0},	// S_PLAY_ATK1
	{SPR_PLAY,32773,6,{NULL},S_PLAY_ATK1,0,0},	// S_PLAY_ATK2
	{SPR_PLAY,6,4,{NULL},S_PLAY_PAIN2,0,0},	// S_PLAY_PAIN
	{SPR_PLAY,6,4,{A_Pain},S_PLAY,0,0},	// S_PLAY_PAIN2
	{SPR_PLAY,7,10,{NULL},S_PLAY_DIE2,0,0},	// S_PLAY_DIE1
	{SPR_PLAY,8,10,{A_PlayerScream},S_PLAY_DIE3,0,0},	// S_PLAY_DIE2
	{SPR_PLAY,9,10,{A_Fall},S_PLAY_DIE4,0,0},	// S_PLAY_DIE3
	{SPR_PLAY,10,10,{NULL},S_PLAY_DIE5,0,0},	// S_PLAY_DIE4
	{SPR_PLAY,11,10,{NULL},S_PLAY_DIE6,0,0},	// S_PLAY_DIE5
	{SPR_PLAY,12,10,{NULL},S_PLAY_DIE7,0,0},	// S_PLAY_DIE6
	{SPR_PLAY,13,-1,{NULL},S_NULL,0,0},	// S_PLAY_DIE7
	{SPR_PLAY,14,5,{NULL},S_PLAY_XDIE2,0,0},	// S_PLAY_XDIE1
	{SPR_PLAY,15,5,{A_XScream},S_PLAY_XDIE3,0,0},	// S_PLAY_XDIE2
	{SPR_PLAY,16,5,{A_Fall},S_PLAY_XDIE4,0,0},	// S_PLAY_XDIE3
	{SPR_PLAY,17,5,{NULL},S_PLAY_XDIE5,0,0},	// S_PLAY_XDIE4
	{SPR_PLAY,18,5,{NULL},S_PLAY_XDIE6,0,0},	// S_PLAY_XDIE5
	{SPR_PLAY,19,5,{NULL},S_PLAY_XDIE7,0,0},	// S_PLAY_XDIE6
	{SPR_PLAY,20,5,{NULL},S_PLAY_XDIE8,0,0},	// S_PLAY_XDIE7
	{SPR_PLAY,21,5,{NULL},S_PLAY_XDIE9,0,0},	// S_PLAY_XDIE8
	{SPR_PLAY,22,-1,{NULL},S_NULL,0,0},	// S_PLAY_XDIE9
	{SPR_POSS,0,10,{A_Look},S_POSS_STND2,0,0},	// S_POSS_STND
	{SPR_POSS,1,10,{A_Look},S_POSS_STND,0,0},	// S_POSS_STND2
	{SPR_POSS,0,4,{A_Chase},S_POSS_RUN2,0,0},	// S_POSS_RUN1
	{SPR_POSS,0,4,{A_Chase},S_POSS_RUN3,0,0},	// S_POSS_RUN2
	{SPR_POSS,1,4,{A_Chase},S_POSS_RUN4,0,0},	// S_POSS_RUN3
	{SPR_POSS,1,4,{A_Chase},S_POSS_RUN5,0,0},	// S_POSS_RUN4
	{SPR_POSS,2,4,{A_Chase},S_POSS_RUN6,0,0},	// S_POSS_RUN5
	{SPR_POSS,2,4,{A_Chase},S_POSS_RUN7,0,0},	// S_POSS_RUN6
	{SPR_POSS,3,4,{A_Chase},S_POSS_RUN8,0,0},	// S_POSS_RUN7
	{SPR_POSS,3,4,{A_Chase},S_POSS_RUN1,0,0},	// S_POSS_RUN8
	{SPR_POSS,4,10,{A_FaceTarget},S_POSS_ATK2,0,0},	// S_POSS_ATK1
	{SPR_POSS,5,8,{A_PosAttack},S_POSS_ATK3,0,0},	// S_POSS_ATK2
	{SPR_POSS,4,8,{NULL},S_POSS_RUN1,0,0},	// S_POSS_ATK3
	{SPR_POSS,6,3,{NULL},S_POSS_PAIN2,0,0},	// S_POSS_PAIN
	{SPR_POSS,6,3,{A_Pain},S_POSS_RUN1,0,0},	// S_POSS_PAIN2
	{SPR_POSS,7,5,{NULL},S_POSS_DIE2,0,0},	// S_POSS_DIE1
	{SPR_POSS,8,5,{A_Scream},S_POSS_DIE3,0,0},	// S_POSS_DIE2
	{SPR_POSS,9,5,{A_Fall},S_POSS_DIE4,0,0},	// S_POSS_DIE3
	{SPR_POSS,10,5,{NULL},S_POSS_DIE5,0,0},	// S_POSS_DIE4
	{SPR_POSS,11,-1,{NULL},S_NULL,0,0},	// S_POSS_DIE5
	{SPR_POSS,12,5,{NULL},S_POSS_XDIE2,0,0},	// S_POSS_XDIE1
	{SPR_POSS,13,5,{A_XScream},S_POSS_XDIE3,0,0},	// S_POSS_XDIE2
	{SPR_POSS,14,5,{A_Fall},S_POSS_XDIE4,0,0},	// S_POSS_XDIE3
	{SPR_POSS,15,5,{NULL},S_POSS_XDIE5,0,0},	// S_POSS_XDIE4
	{SPR_POSS,16,5,{NULL},S_POSS_XDIE6,0,0},	// S_POSS_XDIE5
	{SPR_POSS,17,5,{NULL},S_POSS_XDIE7,0,0},	// S_POSS_XDIE6
	{SPR_POSS,18,5,{NULL},S_POSS_XDIE8,0,0},	// S_POSS_XDIE7
	{SPR_POSS,19,5,{NULL},S_POSS_XDIE9,0,0},	// S_POSS_XDIE8
	{SPR_POSS,20,-1,{NULL},S_NULL,0,0},	// S_POSS_XDIE9
	{SPR_POSS,10,5,{NULL},S_POSS_RAISE2,0,0},	// S_POSS_RAISE1
	{SPR_POSS,9,5,{NULL},S_POSS_RAISE3,0,0},	// S_POSS_RAISE2
	{SPR_POSS,8,5,{NULL},S_POSS_RAISE4,0,0},	// S_POSS_RAISE3
	{SPR_POSS,7,5,{NULL},S_POSS_RUN1,0,0},	// S_POSS_RAISE4
	{SPR_SPOS,0,10,{A_Look},S_SPOS_STND2,0,0},	// S_SPOS_STND
	{SPR_SPOS,1,10,{A_Look},S_SPOS_STND,0,0},	// S_SPOS_STND2
	{SPR_SPOS,0,3,{A_Chase},S_SPOS_RUN2,0,0},	// S_SPOS_RUN1
	{SPR_SPOS,0,3,{A_Chase},S_SPOS_RUN3,0,0},	// S_SPOS_RUN2
	{SPR_SPOS,1,3,{A_Chase},S_SPOS_RUN4,0,0},	// S_SPOS_RUN3
	{SPR_SPOS,1,3,{A_Chase},S_SPOS_RUN5,0,0},	// S_SPOS_RUN4
	{SPR_SPOS,2,3,{A_Chase},S_SPOS_RUN6,0,0},	// S_SPOS_RUN5
	{SPR_SPOS,2,3,{A_Chase},S_SPOS_RUN7,0,0},	// S_SPOS_RUN6
	{SPR_SPOS,3,3,{A_Chase},S_SPOS_RUN8,0,0},	// S_SPOS_RUN7
	{SPR_SPOS,3,3,{A_Chase},S_SPOS_RUN1,0,0},	// S_SPOS_RUN8
	{SPR_SPOS,4,10,{A_FaceTarget},S_SPOS_ATK2,0,0},	// S_SPOS_ATK1
	{SPR_SPOS,32773,10,{A_SPosAttack},S_SPOS_ATK3,0,0},	// S_SPOS_ATK2
	{SPR_SPOS,4,10,{NULL},S_SPOS_RUN1,0,0},	// S_SPOS_ATK3
	{SPR_SPOS,6,3,{NULL},S_SPOS_PAIN2,0,0},	// S_SPOS_PAIN
	{SPR_SPOS,6,3,{A_Pain},S_SPOS_RUN1,0,0},	// S_SPOS_PAIN2
	{SPR_SPOS,7,5,{NULL},S_SPOS_DIE2,0,0},	// S_SPOS_DIE1
	{SPR_SPOS,8,5,{A_Scream},S_SPOS_DIE3,0,0},	// S_SPOS_DIE2
	{SPR_SPOS,9,5,{A_Fall},S_SPOS_DIE4,0,0},	// S_SPOS_DIE3
	{SPR_SPOS,10,5,{NULL},S_SPOS_DIE5,0,0},	// S_SPOS_DIE4
	{SPR_SPOS,11,-1,{NULL},S_NULL,0,0},	// S_SPOS_DIE5
	{SPR_SPOS,12,5,{NULL},S_SPOS_XDIE2,0,0},	// S_SPOS_XDIE1
	{SPR_SPOS,13,5,{A_XScream},S_SPOS_XDIE3,0,0},	// S_SPOS_XDIE2
	{SPR_SPOS,14,5,{A_Fall},S_SPOS_XDIE4,0,0},	// S_SPOS_XDIE3
	{SPR_SPOS,15,5,{NULL},S_SPOS_XDIE5,0,0},	// S_SPOS_XDIE4
	{SPR_SPOS,16,5,{NULL},S_SPOS_XDIE6,0,0},	// S_SPOS_XDIE5
	{SPR_SPOS,17,5,{NULL},S_SPOS_XDIE7,0,0},	// S_SPOS_XDIE6
	{SPR_SPOS,18,5,{NULL},S_SPOS_XDIE8,0,0},	// S_SPOS_XDIE7
	{SPR_SPOS,19,5,{NULL},S_SPOS_XDIE9,0,0},	// S_SPOS_XDIE8
	{SPR_SPOS,20,-1,{NULL},S_NULL,0,0},	// S_SPOS_XDIE9
	{SPR_SPOS,11,5,{NULL},S_SPOS_RAISE2,0,0},	// S_SPOS_RAISE1
	{SPR_SPOS,10,5,{NULL},S_SPOS_RAISE3,0,0},	// S_SPOS_RAISE2
	{SPR_SPOS,9,5,{NULL},S_SPOS_RAISE4,0,0},	// S_SPOS_RAISE3
	{SPR_SPOS,8,5,{NULL},S_SPOS_RAISE5,0,0},	// S_SPOS_RAISE4
	{SPR_SPOS,7,5,{NULL},S_SPOS_RUN1,0,0},	// S_SPOS_RAISE5
	{SPR_VILE,0,10,{A_Look},S_VILE_STND2,0,0},	// S_VILE_STND
	{SPR_VILE,1,10,{A_Look},S_VILE_STND,0,0},	// S_VILE_STND2
	{SPR_VILE,0,2,{A_VileChase},S_VILE_RUN2,0,0},	// S_VILE_RUN1
	{SPR_VILE,0,2,{A_VileChase},S_VILE_RUN3,0,0},	// S_VILE_RUN2
	{SPR_VILE,1,2,{A_VileChase},S_VILE_RUN4,0,0},	// S_VILE_RUN3
	{SPR_VILE,1,2,{A_VileChase},S_VILE_RUN5,0,0},	// S_VILE_RUN4
	{SPR_VILE,2,2,{A_VileChase},S_VILE_RUN6,0,0},	// S_VILE_RUN5
	{SPR_VILE,2,2,{A_VileChase},S_VILE_RUN7,0,0},	// S_VILE_RUN6
	{SPR_VILE,3,2,{A_VileChase},S_VILE_RUN8,0,0},	// S_VILE_RUN7
	{SPR_VILE,3,2,{A_VileChase},S_VILE_RUN9,0,0},	// S_VILE_RUN8
	{SPR_VILE,4,2,{A_VileChase},S_VILE_RUN10,0,0},	// S_VILE_RUN9
	{SPR_VILE,4,2,{A_VileChase},S_VILE_RUN11,0,0},	// S_VILE_RUN10
	{SPR_VILE,5,2,{A_VileChase},S_VILE_RUN12,0,0},	// S_VILE_RUN11
	{SPR_VILE,5,2,{A_VileChase},S_VILE_RUN1,0,0},	// S_VILE_RUN12
	{SPR_VILE,32774,0,{A_VileStart},S_VILE_ATK2,0,0},	// S_VILE_ATK1
	{SPR_VILE,32774,10,{A_FaceTarget},S_VILE_ATK3,0,0},	// S_VILE_ATK2
	{SPR_VILE,32775,8,{A_VileTarget},S_VILE_ATK4,0,0},	// S_VILE_ATK3
	{SPR_VILE,32776,8,{A_FaceTarget},S_VILE_ATK5,0,0},	// S_VILE_ATK4
	{SPR_VILE,32777,8,{A_FaceTarget},S_VILE_ATK6,0,0},	// S_VILE_ATK5
	{SPR_VILE,32778,8,{A_FaceTarget},S_VILE_ATK7,0,0},	// S_VILE_ATK6
	{SPR_VILE,32779,8,{A_FaceTarget},S_VILE_ATK8,0,0},	// S_VILE_ATK7
	{SPR_VILE,32780,8,{A_FaceTarget},S_VILE_ATK9,0,0},	// S_VILE_ATK8
	{SPR_VILE,32781,8,{A_FaceTarget},S_VILE_ATK10,0,0},	// S_VILE_ATK9
	{SPR_VILE,32782,8,{A_VileAttack},S_VILE_ATK11,0,0},	// S_VILE_ATK10
	{SPR_VILE,32783,20,{NULL},S_VILE_RUN1,0,0},	// S_VILE_ATK11
	{SPR_VILE,32794,10,{NULL},S_VILE_HEAL2,0,0},	// S_VILE_HEAL1
	{SPR_VILE,32795,10,{NULL},S_VILE_HEAL3,0,0},	// S_VILE_HEAL2
	{SPR_VILE,32796,10,{NULL},S_VILE_RUN1,0,0},	// S_VILE_HEAL3
	{SPR_VILE,16,5,{NULL},S_VILE_PAIN2,0,0},	// S_VILE_PAIN
	{SPR_VILE,16,5,{A_Pain},S_VILE_RUN1,0,0},	// S_VILE_PAIN2
	{SPR_VILE,16,7,{NULL},S_VILE_DIE2,0,0},	// S_VILE_DIE1
	{SPR_VILE,17,7,{A_Scream},S_VILE_DIE3,0,0},	// S_VILE_DIE2
	{SPR_VILE,18,7,{A_Fall},S_VILE_DIE4,0,0},	// S_VILE_DIE3
	{SPR_VILE,19,7,{NULL},S_VILE_DIE5,0,0},	// S_VILE_DIE4
	{SPR_VILE,20,7,{NULL},S_VILE_DIE6,0,0},	// S_VILE_DIE5
	{SPR_VILE,21,7,{NULL},S_VILE_DIE7,0,0},	// S_VILE_DIE6
	{SPR_VILE,22,7,{NULL},S_VILE_DIE8,0,0},	// S_VILE_DIE7
	{SPR_VILE,23,5,{NULL},S_VILE_DIE9,0,0},	// S_VILE_DIE8
	{SPR_VILE,24,5,{NULL},S_VILE_DIE10,0,0},	// S_VILE_DIE9
	{SPR_VILE,25,-1,{NULL},S_NULL,0,0},	// S_VILE_DIE10
	{SPR_FIRE,32768,2,{A_StartFire},S_FIRE2,0,0},	// S_FIRE1
	{SPR_FIRE,32769,2,{A_Fire},S_FIRE3,0,0},	// S_FIRE2
	{SPR_FIRE,32768,2,{A_Fire},S_FIRE4,0,0},	// S_FIRE3
	{SPR_FIRE,32769,2,{A_Fire},S_FIRE5,0,0},	// S_FIRE4
	{SPR_FIRE,32770,2,{A_FireCrackle},S_FIRE6,0,0},	// S_FIRE5
	{SPR_FIRE,32769,2,{A_Fire},S_FIRE7,0,0},	// S_FIRE6
	{SPR_FIRE,32770,2,{A_Fire},S_FIRE8,0,0},	// S_FIRE7
	{SPR_FIRE,32769,2,{A_Fire},S_FIRE9,0,0},	// S_FIRE8
	{SPR_FIRE,32770,2,{A_Fire},S_FIRE10,0,0},	// S_FIRE9
	{SPR_FIRE,32771,2,{A_Fire},S_FIRE11,0,0},	// S_FIRE10
	{SPR_FIRE,32770,2,{A_Fire},S_FIRE12,0,0},	// S_FIRE11
	{SPR_FIRE,32771,2,{A_Fire},S_FIRE13,0,0},	// S_FIRE12
	{SPR_FIRE,32770,2,{A_Fire},S_FIRE14,0,0},	// S_FIRE13
	{SPR_FIRE,32771,2,{A_Fire},S_FIRE15,0,0},	// S_FIRE14
	{SPR_FIRE,32772,2,{A_Fire},S_FIRE16,0,0},	// S_FIRE15
	{SPR_FIRE,32771,2,{A_Fire},S_FIRE17,0,0},	// S_FIRE16
	{SPR_FIRE,32772,2,{A_Fire},S_FIRE18,0,0},	// S_FIRE17
	{SPR_FIRE,32771,2,{A_Fire},S_FIRE19,0,0},	// S_FIRE18
	{SPR_FIRE,32772,2,{A_FireCrackle},S_FIRE20,0,0},	// S_FIRE19
	{SPR_FIRE,32773,2,{A_Fire},S_FIRE21,0,0},	// S_FIRE20
	{SPR_FIRE,32772,2,{A_Fire},S_FIRE22,0,0},	// S_FIRE21
	{SPR_FIRE,32773,2,{A_Fire},S_FIRE23,0,0},	// S_FIRE22
	{SPR_FIRE,32772,2,{A_Fire},S_FIRE24,0,0},	// S_FIRE23
	{SPR_FIRE,32773,2,{A_Fire},S_FIRE25,0,0},	// S_FIRE24
	{SPR_FIRE,32774,2,{A_Fire},S_FIRE26,0,0},	// S_FIRE25
	{SPR_FIRE,32775,2,{A_Fire},S_FIRE27,0,0},	// S_FIRE26
	{SPR_FIRE,32774,2,{A_Fire},S_FIRE28,0,0},	// S_FIRE27
	{SPR_FIRE,32775,2,{A_Fire},S_FIRE29,0,0},	// S_FIRE28
	{SPR_FIRE,32774,2,{A_Fire},S_FIRE30,0,0},	// S_FIRE29
	{SPR_FIRE,32775,2,{A_Fire},S_NULL,0,0},	// S_FIRE30
	{SPR_PUFF,1,4,{NULL},S_SMOKE2,0,0},	// S_SMOKE1
	{SPR_PUFF,2,4,{NULL},S_SMOKE3,0,0},	// S_SMOKE2
	{SPR_PUFF,1,4,{NULL},S_SMOKE4,0,0},	// S_SMOKE3
	{SPR_PUFF,2,4,{NULL},S_SMOKE5,0,0},	// S_SMOKE4
	{SPR_PUFF,3,4,{NULL},S_NULL,0,0},	// S_SMOKE5
	{SPR_FATB,32768,2,{A_Tracer},S_TRACER2,0,0},	// S_TRACER
	{SPR_FATB,32769,2,{A_Tracer},S_TRACER,0,0},	// S_TRACER2
	{SPR_FBXP,32768,8,{NULL},S_TRACEEXP2,0,0},	// S_TRACEEXP1
	{SPR_FBXP,32769,6,{NULL},S_TRACEEXP3,0,0},	// S_TRACEEXP2
	{SPR_FBXP,32770,4,{NULL},S_NULL,0,0},	// S_TRACEEXP3
	{SPR_SKEL,0,10,{A_Look},S_SKEL_STND2,0,0},	// S_SKEL_STND
	{SPR_SKEL,1,10,{A_Look},S_SKEL_STND,0,0},	// S_SKEL_STND2
	{SPR_SKEL,0,2,{A_Chase},S_SKEL_RUN2,0,0},	// S_SKEL_RUN1
	{SPR_SKEL,0,2,{A_Chase},S_SKEL_RUN3,0,0},	// S_SKEL_RUN2
	{SPR_SKEL,1,2,{A_Chase},S_SKEL_RUN4,0,0},	// S_SKEL_RUN3
	{SPR_SKEL,1,2,{A_Chase},S_SKEL_RUN5,0,0},	// S_SKEL_RUN4
	{SPR_SKEL,2,2,{A_Chase},S_SKEL_RUN6,0,0},	// S_SKEL_RUN5
	{SPR_SKEL,2,2,{A_Chase},S_SKEL_RUN7,0,0},	// S_SKEL_RUN6
	{SPR_SKEL,3,2,{A_Chase},S_SKEL_RUN8,0,0},	// S_SKEL_RUN7
	{SPR_SKEL,3,2,{A_Chase},S_SKEL_RUN9,0,0},	// S_SKEL_RUN8
	{SPR_SKEL,4,2,{A_Chase},S_SKEL_RUN10,0,0},	// S_SKEL_RUN9
	{SPR_SKEL,4,2,{A_Chase},S_SKEL_RUN11,0,0},	// S_SKEL_RUN10
	{SPR_SKEL,5,2,{A_Chase},S_SKEL_RUN12,0,0},	// S_SKEL_RUN11
	{SPR_SKEL,5,2,{A_Chase},S_SKEL_RUN1,0,0},	// S_SKEL_RUN12
	{SPR_SKEL,6,0,{A_FaceTarget},S_SKEL_FIST2,0,0},	// S_SKEL_FIST1
	{SPR_SKEL,6,6,{A_SkelWhoosh},S_SKEL_FIST3,0,0},	// S_SKEL_FIST2
	{SPR_SKEL,7,6,{A_FaceTarget},S_SKEL_FIST4,0,0},	// S_SKEL_FIST3
	{SPR_SKEL,8,6,{A_SkelFist},S_SKEL_RUN1,0,0},	// S_SKEL_FIST4
	{SPR_SKEL,32777,0,{A_FaceTarget},S_SKEL_MISS2,0,0},	// S_SKEL_MISS1
	{SPR_SKEL,32777,10,{A_FaceTarget},S_SKEL_MISS3,0,0},	// S_SKEL_MISS2
	{SPR_SKEL,10,10,{A_SkelMissile},S_SKEL_MISS4,0,0},	// S_SKEL_MISS3
	{SPR_SKEL,10,10,{A_FaceTarget},S_SKEL_RUN1,0,0},	// S_SKEL_MISS4
	{SPR_SKEL,11,5,{NULL},S_SKEL_PAIN2,0,0},	// S_SKEL_PAIN
	{SPR_SKEL,11,5,{A_Pain},S_SKEL_RUN1,0,0},	// S_SKEL_PAIN2
	{SPR_SKEL,11,7,{NULL},S_SKEL_DIE2,0,0},	// S_SKEL_DIE1
	{SPR_SKEL,12,7,{NULL},S_SKEL_DIE3,0,0},	// S_SKEL_DIE2
	{SPR_SKEL,13,7,{A_Scream},S_SKEL_DIE4,0,0},	// S_SKEL_DIE3
	{SPR_SKEL,14,7,{A_Fall},S_SKEL_DIE5,0,0},	// S_SKEL_DIE4
	{SPR_SKEL,15,7,{NULL},S_SKEL_DIE6,0,0},	// S_SKEL_DIE5
	{SPR_SKEL,16,-1,{NULL},S_NULL,0,0},	// S_SKEL_DIE6
	{SPR_SKEL,16,5,{NULL},S_SKEL_RAISE2,0,0},	// S_SKEL_RAISE1
	{SPR_SKEL,15,5,{NULL},S_SKEL_RAISE3,0,0},	// S_SKEL_RAISE2
	{SPR_SKEL,14,5,{NULL},S_SKEL_RAISE4,0,0},	// S_SKEL_RAISE3
	{SPR_SKEL,13,5,{NULL},S_SKEL_RAISE5,0,0},	// S_SKEL_RAISE4
	{SPR_SKEL,12,5,{NULL},S_SKEL_RAISE6,0,0},	// S_SKEL_RAISE5
	{SPR_SKEL,11,5,{NULL},S_SKEL_RUN1,0,0},	// S_SKEL_RAISE6
	{SPR_MANF,32768,4,{NULL},S_FATSHOT2,0,0},	// S_FATSHOT1
	{SPR_MANF,32769,4,{NULL},S_FATSHOT1,0,0},	// S_FATSHOT2
	{SPR_MISL,32769,8,{NULL},S_FATSHOTX2,0,0},	// S_FATSHOTX1
	{SPR_MISL,32770,6,{NULL},S_FATSHOTX3,0,0},	// S_FATSHOTX2
	{SPR_MISL,32771,4,{NULL},S_NULL,0,0},	// S_FATSHOTX3
	{SPR_FATT,0,15,{A_Look},S_FATT_STND2,0,0},	// S_FATT_STND
	{SPR_FATT,1,15,{A_Look},S_FATT_STND,0,0},	// S_FATT_STND2
	{SPR_FATT,0,4,{A_Chase},S_FATT_RUN2,0,0},	// S_FATT_RUN1
	{SPR_FATT,0,4,{A_Chase},S_FATT_RUN3,0,0},	// S_FATT_RUN2
	{SPR_FATT,1,4,{A_Chase},S_FATT_RUN4,0,0},	// S_FATT_RUN3
	{SPR_FATT,1,4,{A_Chase},S_FATT_RUN5,0,0},	// S_FATT_RUN4
	{SPR_FATT,2,4,{A_Chase},S_FATT_RUN6,0,0},	// S_FATT_RUN5
	{SPR_FATT,2,4,{A_Chase},S_FATT_RUN7,0,0},	// S_FATT_RUN6
	{SPR_FATT,3,4,{A_Chase},S_FATT_RUN8,0,0},	// S_FATT_RUN7
	{SPR_FATT,3,4,{A_Chase},S_FATT_RUN9,0,0},	// S_FATT_RUN8
	{SPR_FATT,4,4,{A_Chase},S_FATT_RUN10,0,0},	// S_FATT_RUN9
	{SPR_FATT,4,4,{A_Chase},S_FATT_RUN11,0,0},	// S_FATT_RUN10
	{SPR_FATT,5,4,{A_Chase},S_FATT_RUN12,0,0},	// S_FATT_RUN11
	{SPR_FATT,5,4,{A_Chase},S_FATT_RUN1,0,0},	// S_FATT_RUN12
	{SPR_FATT,6,20,{A_FatRaise},S_FATT_ATK2,0,0},	// S_FATT_ATK1
	{SPR_FATT,32775,10,{A_FatAttack1},S_FATT_ATK3,0,0},	// S_FATT_ATK2
	{SPR_FATT,8,5,{A_FaceTarget},S_FATT_ATK4,0,0},	// S_FATT_ATK3
	{SPR_FATT,6,5,{A_FaceTarget},S_FATT_ATK5,0,0},	// S_FATT_ATK4
	{SPR_FATT,32775,10,{A_FatAttack2},S_FATT_ATK6,0,0},	// S_FATT_ATK5
	{SPR_FATT,8,5,{A_FaceTarget},S_FATT_ATK7,0,0},	// S_FATT_ATK6
	{SPR_FATT,6,5,{A_FaceTarget},S_FATT_ATK8,0,0},	// S_FATT_ATK7
	{SPR_FATT,32775,10,{A_FatAttack3},S_FATT_ATK9,0,0},	// S_FATT_ATK8
	{SPR_FATT,8,5,{A_FaceTarget},S_FATT_ATK10,0,0},	// S_FATT_ATK9
	{SPR_FATT,6,5,{A_FaceTarget},S_FATT_RUN1,0,0},	// S_FATT_ATK10
	{SPR_FATT,9,3,{NULL},S_FATT_PAIN2,0,0},	// S_FATT_PAIN
	{SPR_FATT,9,3,{A_Pain},S_FATT_RUN1,0,0},	// S_FATT_PAIN2
	{SPR_FATT,10,6,{NULL},S_FATT_DIE2,0,0},	// S_FATT_DIE1
	{SPR_FATT,11,6,{A_Scream},S_FATT_DIE3,0,0},	// S_FATT_DIE2
	{SPR_FATT,12,6,{A_Fall},S_FATT_DIE4,0,0},	// S_FATT_DIE3
	{SPR_FATT,13,6,{NULL},S_FATT_DIE5,0,0},	// S_FATT_DIE4
	{SPR_FATT,14,6,{NULL},S_FATT_DIE6,0,0},	// S_FATT_DIE5
	{SPR_FATT,15,6,{NULL},S_FATT_DIE7,0,0},	// S_FATT_DIE6
	{SPR_FATT,16,6,{NULL},S_FATT_DIE8,0,0},	// S_FATT_DIE7
	{SPR_FATT,17,6,{NULL},S_FATT_DIE9,0,0},	// S_FATT_DIE8
	{SPR_FATT,18,6,{NULL},S_FATT_DIE10,0,0},	// S_FATT_DIE9
	{SPR_FATT,19,-1,{A_BossDeath},S_NULL,0,0},	// S_FATT_DIE10
	{SPR_FATT,17,5,{NULL},S_FATT_RAISE2,0,0},	// S_FATT_RAISE1
	{SPR_FATT,16,5,{NULL},S_FATT_RAISE3,0,0},	// S_FATT_RAISE2
	{SPR_FATT,15,5,{NULL},S_FATT_RAISE4,0,0},	// S_FATT_RAISE3
	{SPR_FATT,14,5,{NULL},S_FATT_RAISE5,0,0},	// S_FATT_RAISE4
	{SPR_FATT,13,5,{NULL},S_FATT_RAISE6,0,0},	// S_FATT_RAISE5
	{SPR_FATT,12,5,{NULL},S_FATT_RAISE7,0,0},	// S_FATT_RAISE6
	{SPR_FATT,11,5,{NULL},S_FATT_RAISE8,0,0},	// S_FATT_RAISE7
	{SPR_FATT,10,5,{NULL},S_FATT_RUN1,0,0},	// S_FATT_RAISE8
	{SPR_CPOS,0,10,{A_Look},S_CPOS_STND2,0,0},	// S_CPOS_STND
	{SPR_CPOS,1,10,{A_Look},S_CPOS_STND,0,0},	// S_CPOS_STND2
	{SPR_CPOS,0,3,{A_Chase},S_CPOS_RUN2,0,0},	// S_CPOS_RUN1
	{SPR_CPOS,0,3,{A_Chase},S_CPOS_RUN3,0,0},	// S_CPOS_RUN2
	{SPR_CPOS,1,3,{A_Chase},S_CPOS_RUN4,0,0},	// S_CPOS_RUN3
	{SPR_CPOS,1,3,{A_Chase},S_CPOS_RUN5,0,0},	// S_CPOS_RUN4
	{SPR_CPOS,2,3,{A_Chase},S_CPOS_RUN6,0,0},	// S_CPOS_RUN5
	{SPR_CPOS,2,3,{A_Chase},S_CPOS_RUN7,0,0},	// S_CPOS_RUN6
	{SPR_CPOS,3,3,{A_Chase},S_CPOS_RUN8,0,0},	// S_CPOS_RUN7
	{SPR_CPOS,3,3,{A_Chase},S_CPOS_RUN1,0,0},	// S_CPOS_RUN8
	{SPR_CPOS,4,10,{A_FaceTarget},S_CPOS_ATK2,0,0},	// S_CPOS_ATK1
	{SPR_CPOS,32773,4,{A_CPosAttack},S_CPOS_ATK3,0,0},	// S_CPOS_ATK2
	{SPR_CPOS,32772,4,{A_CPosAttack},S_CPOS_ATK4,0,0},	// S_CPOS_ATK3
	{SPR_CPOS,5,1,{A_CPosRefire},S_CPOS_ATK2,0,0},	// S_CPOS_ATK4
	{SPR_CPOS,6,3,{NULL},S_CPOS_PAIN2,0,0},	// S_CPOS_PAIN
	{SPR_CPOS,6,3,{A_Pain},S_CPOS_RUN1,0,0},	// S_CPOS_PAIN2
	{SPR_CPOS,7,5,{NULL},S_CPOS_DIE2,0,0},	// S_CPOS_DIE1
	{SPR_CPOS,8,5,{A_Scream},S_CPOS_DIE3,0,0},	// S_CPOS_DIE2
	{SPR_CPOS,9,5,{A_Fall},S_CPOS_DIE4,0,0},	// S_CPOS_DIE3
	{SPR_CPOS,10,5,{NULL},S_CPOS_DIE5,0,0},	// S_CPOS_DIE4
	{SPR_CPOS,11,5,{NULL},S_CPOS_DIE6,0,0},	// S_CPOS_DIE5
	{SPR_CPOS,12,5,{NULL},S_CPOS_DIE7,0,0},	// S_CPOS_DIE6
	{SPR_CPOS,13,-1,{NULL},S_NULL,0,0},	// S_CPOS_DIE7
	{SPR_CPOS,14,5,{NULL},S_CPOS_XDIE2,0,0},	// S_CPOS_XDIE1
	{SPR_CPOS,15,5,{A_XScream},S_CPOS_XDIE3,0,0},	// S_CPOS_XDIE2
	{SPR_CPOS,16,5,{A_Fall},S_CPOS_XDIE4,0,0},	// S_CPOS_XDIE3
	{SPR_CPOS,17,5,{NULL},S_CPOS_XDIE5,0,0},	// S_CPOS_XDIE4
	{SPR_CPOS,18,5,{NULL},S_CPOS_XDIE6,0,0},	// S_CPOS_XDIE5
	{SPR_CPOS,19,-1,{NULL},S_NULL,0,0},	// S_CPOS_XDIE6
	{SPR_CPOS,13,5,{NULL},S_CPOS_RAISE2,0,0},	// S_CPOS_RAISE1
	{SPR_CPOS,12,5,{NULL},S_CPOS_RAISE3,0,0},	// S_CPOS_RAISE2
	{SPR_CPOS,11,5,{NULL},S_CPOS_RAISE4,0,0},	// S_CPOS_RAISE3
	{SPR_CPOS,10,5,{NULL},S_CPOS_RAISE5,0,0},	// S_CPOS_RAISE4
	{SPR_CPOS,9,5,{NULL},S_CPOS_RAISE6,0,0},	// S_CPOS_RAISE5
	{SPR_CPOS,8,5,{NULL},S_CPOS_RAISE7,0,0},	// S_CPOS_RAISE6
	{SPR_CPOS,7,5,{NULL},S_CPOS_RUN1,0,0},	// S_CPOS_RAISE7
	{SPR_TROO,0,10,{A_Look},S_TROO_STND2,0,0},	// S_TROO_STND
	{SPR_TROO,1,10,{A_Look},S_TROO_STND,0,0},	// S_TROO_STND2
	{SPR_TROO,0,3,{A_Chase},S_TROO_RUN2,0,0},	// S_TROO_RUN1
	{SPR_TROO,0,3,{A_Chase},S_TROO_RUN3,0,0},	// S_TROO_RUN2
	{SPR_TROO,1,3,{A_Chase},S_TROO_RUN4,0,0},	// S_TROO_RUN3
	{SPR_TROO,1,3,{A_Chase},S_TROO_RUN5,0,0},	// S_TROO_RUN4
	{SPR_TROO,2,3,{A_Chase},S_TROO_RUN6,0,0},	// S_TROO_RUN5
	{SPR_TROO,2,3,{A_Chase},S_TROO_RUN7,0,0},	// S_TROO_RUN6
	{SPR_TROO,3,3,{A_Chase},S_TROO_RUN8,0,0},	// S_TROO_RUN7
	{SPR_TROO,3,3,{A_Chase},S_TROO_RUN1,0,0},	// S_TROO_RUN8
	{SPR_TROO,4,8,{A_FaceTarget},S_TROO_ATK2,0,0},	// S_TROO_ATK1
	{SPR_TROO,5,8,{A_FaceTarget},S_TROO_ATK3,0,0},	// S_TROO_ATK2
	{SPR_TROO,6,6,{A_TroopAttack},S_TROO_RUN1,0,0},	// S_TROO_ATK3
	{SPR_TROO,7,2,{NULL},S_TROO_PAIN2,0,0},	// S_TROO_PAIN
	{SPR_TROO,7,2,{A_Pain},S_TROO_RUN1,0,0},	// S_TROO_PAIN2
	{SPR_TROO,8,8,{NULL},S_TROO_DIE2,0,0},	// S_TROO_DIE1
	{SPR_TROO,9,8,{A_Scream},S_TROO_DIE3,0,0},	// S_TROO_DIE2
	{SPR_TROO,10,6,{NULL},S_TROO_DIE4,0,0},	// S_TROO_DIE3
	{SPR_TROO,11,6,{A_Fall},S_TROO_DIE5,0,0},	// S_TROO_DIE4
	{SPR_TROO,12,-1,{NULL},S_NULL,0,0},	// S_TROO_DIE5
	{SPR_TROO,13,5,{NULL},S_TROO_XDIE2,0,0},	// S_TROO_XDIE1
	{SPR_TROO,14,5,{A_XScream},S_TROO_XDIE3,0,0},	// S_TROO_XDIE2
	{SPR_TROO,15,5,{NULL},S_TROO_XDIE4,0,0},	// S_TROO_XDIE3
	{SPR_TROO,16,5,{A_Fall},S_TROO_XDIE5,0,0},	// S_TROO_XDIE4
	{SPR_TROO,17,5,{NULL},S_TROO_XDIE6,0,0},	// S_TROO_XDIE5
	{SPR_TROO,18,5,{NULL},S_TROO_XDIE7,0,0},	// S_TROO_XDIE6
	{SPR_TROO,19,5,{NULL},S_TROO_XDIE8,0,0},	// S_TROO_XDIE7
	{SPR_TROO,20,-1,{NULL},S_NULL,0,0},	// S_TROO_XDIE8
	{SPR_TROO,12,8,{NULL},S_TROO_RAISE2,0,0},	// S_TROO_RAISE1
	{SPR_TROO,11,8,{NULL},S_TROO_RAISE3,0,0},	// S_TROO_RAISE2
	{SPR_TROO,10,6,{NULL},S_TROO_RAISE4,0,0},	// S_TROO_RAISE3
	{SPR_TROO,9,6,{NULL},S_TROO_RAISE5,0,0},	// S_TROO_RAISE4
	{SPR_TROO,8,6,{NULL},S_TROO_RUN1,0,0},	// S_TROO_RAISE5
	{SPR_SARG,0,10,{A_Look},S_SARG_STND2,0,0},	// S_SARG_STND
	{SPR_SARG,1,10,{A_Look},S_SARG_STND,0,0},	// S_SARG_STND2
	{SPR_SARG,0,2,{A_Chase},S_SARG_RUN2,0,0},	// S_SARG_RUN1
	{SPR_SARG,0,2,{A_Chase},S_SARG_RUN3,0,0},	// S_SARG_RUN2
	{SPR_SARG,1,2,{A_Chase},S_SARG_RUN4,0,0},	// S_SARG_RUN3
	{SPR_SARG,1,2,{A_Chase},S_SARG_RUN5,0,0},	// S_SARG_RUN4
	{SPR_SARG,2,2,{A_Chase},S_SARG_RUN6,0,0},	// S_SARG_RUN5
	{SPR_SARG,2,2,{A_Chase},S_SARG_RUN7,0,0},	// S_SARG_RUN6
	{SPR_SARG,3,2,{A_Chase},S_SARG_RUN8,0,0},	// S_SARG_RUN7
	{SPR_SARG,3,2,{A_Chase},S_SARG_RUN1,0,0},	// S_SARG_RUN8
	{SPR_SARG,4,8,{A_FaceTarget},S_SARG_ATK2,0,0},	// S_SARG_ATK1
	{SPR_SARG,5,8,{A_FaceTarget},S_SARG_ATK3,0,0},	// S_SARG_ATK2
	{SPR_SARG,6,8,{A_SargAttack},S_SARG_RUN1,0,0},	// S_SARG_ATK3
	{SPR_SARG,7,2,{NULL},S_SARG_PAIN2,0,0},	// S_SARG_PAIN
	{SPR_SARG,7,2,{A_Pain},S_SARG_RUN1,0,0},	// S_SARG_PAIN2
	{SPR_SARG,8,8,{NULL},S_SARG_DIE2,0,0},	// S_SARG_DIE1
	{SPR_SARG,9,8,{A_Scream},S_SARG_DIE3,0,0},	// S_SARG_DIE2
	{SPR_SARG,10,4,{NULL},S_SARG_DIE4,0,0},	// S_SARG_DIE3
	{SPR_SARG,11,4,{A_Fall},S_SARG_DIE5,0,0},	// S_SARG_DIE4
	{SPR_SARG,12,4,{NULL},S_SARG_DIE6,0,0},	// S_SARG_DIE5
	{SPR_SARG,13,-1,{NULL},S_NULL,0,0},	// S_SARG_DIE6
	{SPR_SARG,13,5,{NULL},S_SARG_RAISE2,0,0},	// S_SARG_RAISE1
	{SPR_SARG,12,5,{NULL},S_SARG_RAISE3,0,0},	// S_SARG_RAISE2
	{SPR_SARG,11,5,{NULL},S_SARG_RAISE4,0,0},	// S_SARG_RAISE3
	{SPR_SARG,10,5,{NULL},S_SARG_RAISE5,0,0},	// S_SARG_RAISE4
	{SPR_SARG,9,5,{NULL},S_SARG_RAISE6,0,0},	// S_SARG_RAISE5
	{SPR_SARG,8,5,{NULL},S_SARG_RUN1,0,0},	// S_SARG_RAISE6
	{SPR_HEAD,0,10,{A_Look},S_HEAD_STND,0,0},	// S_HEAD_STND
	{SPR_HEAD,0,3,{A_Chase},S_HEAD_RUN1,0,0},	// S_HEAD_RUN1
	{SPR_HEAD,1,5,{A_FaceTarget},S_HEAD_ATK2,0,0},	// S_HEAD_ATK1
	{SPR_HEAD,2,5,{A_FaceTarget},S_HEAD_ATK3,0,0},	// S_HEAD_ATK2
	{SPR_HEAD,32771,5,{A_HeadAttack},S_HEAD_RUN1,0,0},	// S_HEAD_ATK3
	{SPR_HEAD,4,3,{NULL},S_HEAD_PAIN2,0,0},	// S_HEAD_PAIN
	{SPR_HEAD,4,3,{A_Pain},S_HEAD_PAIN3,0,0},	// S_HEAD_PAIN2
	{SPR_HEAD,5,6,{NULL},S_HEAD_RUN1,0,0},	// S_HEAD_PAIN3
	{SPR_HEAD,6,8,{NULL},S_HEAD_DIE2,0,0},	// S_HEAD_DIE1
	{SPR_HEAD,7,8,{A_Scream},S_HEAD_DIE3,0,0},	// S_HEAD_DIE2
	{SPR_HEAD,8,8,{NULL},S_HEAD_DIE4,0,0},	// S_HEAD_DIE3
	{SPR_HEAD,9,8,{NULL},S_HEAD_DIE5,0,0},	// S_HEAD_DIE4
	{SPR_HEAD,10,8,{A_Fall},S_HEAD_DIE6,0,0},	// S_HEAD_DIE5
	{SPR_HEAD,11,-1,{NULL},S_NULL,0,0},	// S_HEAD_DIE6
	{SPR_HEAD,11,8,{NULL},S_HEAD_RAISE2,0,0},	// S_HEAD_RAISE1
	{SPR_HEAD,10,8,{NULL},S_HEAD_RAISE3,0,0},	// S_HEAD_RAISE2
	{SPR_HEAD,9,8,{NULL},S_HEAD_RAISE4,0,0},	// S_HEAD_RAISE3
	{SPR_HEAD,8,8,{NULL},S_HEAD_RAISE5,0,0},	// S_HEAD_RAISE4
	{SPR_HEAD,7,8,{NULL},S_HEAD_RAISE6,0,0},	// S_HEAD_RAISE5
	{SPR_HEAD,6,8,{NULL},S_HEAD_RUN1,0,0},	// S_HEAD_RAISE6
	{SPR_BAL7,32768,4,{NULL},S_BRBALL2,0,0},	// S_BRBALL1
	{SPR_BAL7,32769,4,{NULL},S_BRBALL1,0,0},	// S_BRBALL2
	{SPR_BAL7,32770,6,{NULL},S_BRBALLX2,0,0},	// S_BRBALLX1
	{SPR_BAL7,32771,6,{NULL},S_BRBALLX3,0,0},	// S_BRBALLX2
	{SPR_BAL7,32772,6,{NULL},S_NULL,0,0},	// S_BRBALLX3
	{SPR_BOSS,0,10,{A_Look},S_BOSS_STND2,0,0},	// S_BOSS_STND
	{SPR_BOSS,1,10,{A_Look},S_BOSS_STND,0,0},	// S_BOSS_STND2
	{SPR_BOSS,0,3,{A_Chase},S_BOSS_RUN2,0,0},	// S_BOSS_RUN1
	{SPR_BOSS,0,3,{A_Chase},S_BOSS_RUN3,0,0},	// S_BOSS_RUN2
	{SPR_BOSS,1,3,{A_Chase},S_BOSS_RUN4,0,0},	// S_BOSS_RUN3
	{SPR_BOSS,1,3,{A_Chase},S_BOSS_RUN5,0,0},	// S_BOSS_RUN4
	{SPR_BOSS,2,3,{A_Chase},S_BOSS_RUN6,0,0},	// S_BOSS_RUN5
	{SPR_BOSS,2,3,{A_Chase},S_BOSS_RUN7,0,0},	// S_BOSS_RUN6
	{SPR_BOSS,3,3,{A_Chase},S_BOSS_RUN8,0,0},	// S_BOSS_RUN7
	{SPR_BOSS,3,3,{A_Chase},S_BOSS_RUN1,0,0},	// S_BOSS_RUN8
	{SPR_BOSS,4,8,{A_FaceTarget},S_BOSS_ATK2,0,0},	// S_BOSS_ATK1
	{SPR_BOSS,5,8,{A_FaceTarget},S_BOSS_ATK3,0,0},	// S_BOSS_ATK2
	{SPR_BOSS,6,8,{A_BruisAttack},S_BOSS_RUN1,0,0},	// S_BOSS_ATK3
	{SPR_BOSS,7,2,{NULL},S_BOSS_PAIN2,0,0},	// S_BOSS_PAIN
	{SPR_BOSS,7,2,{A_Pain},S_BOSS_RUN1,0,0},	// S_BOSS_PAIN2
	{SPR_BOSS,8,8,{NULL},S_BOSS_DIE2,0,0},	// S_BOSS_DIE1
	{SPR_BOSS,9,8,{A_Scream},S_BOSS_DIE3,0,0},	// S_BOSS_DIE2
	{SPR_BOSS,10,8,{NULL},S_BOSS_DIE4,0,0},	// S_BOSS_DIE3
	{SPR_BOSS,11,8,{A_Fall},S_BOSS_DIE5,0,0},	// S_BOSS_DIE4
	{SPR_BOSS,12,8,{NULL},S_BOSS_DIE6,0,0},	// S_BOSS_DIE5
	{SPR_BOSS,13,8,{NULL},S_BOSS_DIE7,0,0},	// S_BOSS_DIE6
	{SPR_BOSS,14,-1,{A_BossDeath},S_NULL,0,0},	// S_BOSS_DIE7
	{SPR_BOSS,14,8,{NULL},S_BOSS_RAISE2,0,0},	// S_BOSS_RAISE1
	{SPR_BOSS,13,8,{NULL},S_BOSS_RAISE3,0,0},	// S_BOSS_RAISE2
	{SPR_BOSS,12,8,{NULL},S_BOSS_RAISE4,0,0},	// S_BOSS_RAISE3
	{SPR_BOSS,11,8,{NULL},S_BOSS_RAISE5,0,0},	// S_BOSS_RAISE4
	{SPR_BOSS,10,8,{NULL},S_BOSS_RAISE6,0,0},	// S_BOSS_RAISE5
	{SPR_BOSS,9,8,{NULL},S_BOSS_RAISE7,0,0},	// S_BOSS_RAISE6
	{SPR_BOSS,8,8,{NULL},S_BOSS_RUN1,0,0},	// S_BOSS_RAISE7
	{SPR_BOS2,0,10,{A_Look},S_BOS2_STND2,0,0},	// S_BOS2_STND
	{SPR_BOS2,1,10,{A_Look},S_BOS2_STND,0,0},	// S_BOS2_STND2
	{SPR_BOS2,0,3,{A_Chase},S_BOS2_RUN2,0,0},	// S_BOS2_RUN1
	{SPR_BOS2,0,3,{A_Chase},S_BOS2_RUN3,0,0},	// S_BOS2_RUN2
	{SPR_BOS2,1,3,{A_Chase},S_BOS2_RUN4,0,0},	// S_BOS2_RUN3
	{SPR_BOS2,1,3,{A_Chase},S_BOS2_RUN5,0,0},	// S_BOS2_RUN4
	{SPR_BOS2,2,3,{A_Chase},S_BOS2_RUN6,0,0},	// S_BOS2_RUN5
	{SPR_BOS2,2,3,{A_Chase},S_BOS2_RUN7,0,0},	// S_BOS2_RUN6
	{SPR_BOS2,3,3,{A_Chase},S_BOS2_RUN8,0,0},	// S_BOS2_RUN7
	{SPR_BOS2,3,3,{A_Chase},S_BOS2_RUN1,0,0},	// S_BOS2_RUN8
	{SPR_BOS2,4,8,{A_FaceTarget},S_BOS2_ATK2,0,0},	// S_BOS2_ATK1
	{SPR_BOS2,5,8,{A_FaceTarget},S_BOS2_ATK3,0,0},	// S_BOS2_ATK2
	{SPR_BOS2,6,8,{A_BruisAttack},S_BOS2_RUN1,0,0},	// S_BOS2_ATK3
	{SPR_BOS2,7,2,{NULL},S_BOS2_PAIN2,0,0},	// S_BOS2_PAIN
	{SPR_BOS2,7,2,{A_Pain},S_BOS2_RUN1,0,0},	// S_BOS2_PAIN2
	{SPR_BOS2,8,8,{NULL},S_BOS2_DIE2,0,0},	// S_BOS2_DIE1
	{SPR_BOS2,9,8,{A_Scream},S_BOS2_DIE3,0,0},	// S_BOS2_DIE2
	{SPR_BOS2,10,8,{NULL},S_BOS2_DIE4,0,0},	// S_BOS2_DIE3
	{SPR_BOS2,11,8,{A_Fall},S_BOS2_DIE5,0,0},	// S_BOS2_DIE4
	{SPR_BOS2,12,8,{NULL},S_BOS2_DIE6,0,0},	// S_BOS2_DIE5
	{SPR_BOS2,13,8,{NULL},S_BOS2_DIE7,0,0},	// S_BOS2_DIE6
	{SPR_BOS2,14,-1,{NULL},S_NULL,0,0},	// S_BOS2_DIE7
	{SPR_BOS2,14,8,{NULL},S_BOS2_RAISE2,0,0},	// S_BOS2_RAISE1
	{SPR_BOS2,13,8,{NULL},S_BOS2_RAISE3,0,0},	// S_BOS2_RAISE2
	{SPR_BOS2,12,8,{NULL},S_BOS2_RAISE4,0,0},	// S_BOS2_RAISE3
	{SPR_BOS2,11,8,{NULL},S_BOS2_RAISE5,0,0},	// S_BOS2_RAISE4
	{SPR_BOS2,10,8,{NULL},S_BOS2_RAISE6,0,0},	// S_BOS2_RAISE5
	{SPR_BOS2,9,8,{NULL},S_BOS2_RAISE7,0,0},	// S_BOS2_RAISE6
	{SPR_BOS2,8,8,{NULL},S_BOS2_RUN1,0,0},	// S_BOS2_RAISE7
	{SPR_SKUL,32768,10,{A_Look},S_SKULL_STND2,0,0},	// S_SKULL_STND
	{SPR_SKUL,32769,10,{A_Look},S_SKULL_STND,0,0},	// S_SKULL_STND2
	{SPR_SKUL,32768,6,{A_Chase},S_SKULL_RUN2,0,0},	// S_SKULL_RUN1
	{SPR_SKUL,32769,6,{A_Chase},S_SKULL_RUN1,0,0},	// S_SKULL_RUN2
	{SPR_SKUL,32770,10,{A_FaceTarget},S_SKULL_ATK2,0,0},	// S_SKULL_ATK1
	{SPR_SKUL,32771,4,{A_SkullAttack},S_SKULL_ATK3,0,0},	// S_SKULL_ATK2
	{SPR_SKUL,32770,4,{NULL},S_SKULL_ATK4,0,0},	// S_SKULL_ATK3
	{SPR_SKUL,32771,4,{NULL},S_SKULL_ATK3,0,0},	// S_SKULL_ATK4
	{SPR_SKUL,32772,3,{NULL},S_SKULL_PAIN2,0,0},	// S_SKULL_PAIN
	{SPR_SKUL,32772,3,{A_Pain},S_SKULL_RUN1,0,0},	// S_SKULL_PAIN2
	{SPR_SKUL,32773,6,{NULL},S_SKULL_DIE2,0,0},	// S_SKULL_DIE1
	{SPR_SKUL,32774,6,{A_Scream},S_SKULL_DIE3,0,0},	// S_SKULL_DIE2
	{SPR_SKUL,32775,6,{NULL},S_SKULL_DIE4,0,0},	// S_SKULL_DIE3
	{SPR_SKUL,32776,6,{A_Fall},S_SKULL_DIE5,0,0},	// S_SKULL_DIE4
	{SPR_SKUL,9,6,{NULL},S_SKULL_DIE6,0,0},	// S_SKULL_DIE5
	{SPR_SKUL,10,6,{NULL},S_NULL,0,0},	// S_SKULL_DIE6
	{SPR_SPID,0,10,{A_Look},S_SPID_STND2,0,0},	// S_SPID_STND
	{SPR_SPID,1,10,{A_Look},S_SPID_STND,0,0},	// S_SPID_STND2
	{SPR_SPID,0,3,{A_Metal},S_SPID_RUN2,0,0},	// S_SPID_RUN1
	{SPR_SPID,0,3,{A_Chase},S_SPID_RUN3,0,0},	// S_SPID_RUN2
	{SPR_SPID,1,3,{A_Chase},S_SPID_RUN4,0,0},	// S_SPID_RUN3
	{SPR_SPID,1,3,{A_Chase},S_SPID_RUN5,0,0},	// S_SPID_RUN4
	{SPR_SPID,2,3,{A_Metal},S_SPID_RUN6,0,0},	// S_SPID_RUN5
	{SPR_SPID,2,3,{A_Chase},S_SPID_RUN7,0,0},	// S_SPID_RUN6
	{SPR_SPID,3,3,{A_Chase},S_SPID_RUN8,0,0},	// S_SPID_RUN7
	{SPR_SPID,3,3,{A_Chase},S_SPID_RUN9,0,0},	// S_SPID_RUN8
	{SPR_SPID,4,3,{A_Metal},S_SPID_RUN10,0,0},	// S_SPID_RUN9
	{SPR_SPID,4,3,{A_Chase},S_SPID_RUN11,0,0},	// S_SPID_RUN10
	{SPR_SPID,5,3,{A_Chase},S_SPID_RUN12,0,0},	// S_SPID_RUN11
	{SPR_SPID,5,3,{A_Chase},S_SPID_RUN1,0,0},	// S_SPID_RUN12
	{SPR_SPID,32768,20,{A_FaceTarget},S_SPID_ATK2,0,0},	// S_SPID_ATK1
	{SPR_SPID,32774,4,{A_SPosAttack},S_SPID_ATK3,0,0},	// S_SPID_ATK2
	{SPR_SPID,32775,4,{A_SPosAttack},S_SPID_ATK4,0,0},	// S_SPID_ATK3
	{SPR_SPID,32775,1,{A_SpidRefire},S_SPID_ATK2,0,0},	// S_SPID_ATK4
	{SPR_SPID,8,3,{NULL},S_SPID_PAIN2,0,0},	// S_SPID_PAIN
	{SPR_SPID,8,3,{A_Pain},S_SPID_RUN1,0,0},	// S_SPID_PAIN2
	{SPR_SPID,9,20,{A_Scream},S_SPID_DIE2,0,0},	// S_SPID_DIE1
	{SPR_SPID,10,10,{A_Fall},S_SPID_DIE3,0,0},	// S_SPID_DIE2
	{SPR_SPID,11,10,{NULL},S_SPID_DIE4,0,0},	// S_SPID_DIE3
	{SPR_SPID,12,10,{NULL},S_SPID_DIE5,0,0},	// S_SPID_DIE4
	{SPR_SPID,13,10,{NULL},S_SPID_DIE6,0,0},	// S_SPID_DIE5
	{SPR_SPID,14,10,{NULL},S_SPID_DIE7,0,0},	// S_SPID_DIE6
	{SPR_SPID,15,10,{NULL},S_SPID_DIE8,0,0},	// S_SPID_DIE7
	{SPR_SPID,16,10,{NULL},S_SPID_DIE9,0,0},	// S_SPID_DIE8
	{SPR_SPID,17,10,{NULL},S_SPID_DIE10,0,0},	// S_SPID_DIE9
	{SPR_SPID,18,30,{NULL},S_SPID_DIE11,0,0},	// S_SPID_DIE10
	{SPR_SPID,18,-1,{A_BossDeath},S_NULL,0,0},	// S_SPID_DIE11
	{SPR_BSPI,0,10,{A_Look},S_BSPI_STND2,0,0},	// S_BSPI_STND
	{SPR_BSPI,1,10,{A_Look},S_BSPI_STND,0,0},	// S_BSPI_STND2
	{SPR_BSPI,0,20,{NULL},S_BSPI_RUN1,0,0},	// S_BSPI_SIGHT
	{SPR_BSPI,0,3,{A_BabyMetal},S_BSPI_RUN2,0,0},	// S_BSPI_RUN1
	{SPR_BSPI,0,3,{A_Chase},S_BSPI_RUN3,0,0},	// S_BSPI_RUN2
	{SPR_BSPI,1,3,{A_Chase},S_BSPI_RUN4,0,0},	// S_BSPI_RUN3
	{SPR_BSPI,1,3,{A_Chase},S_BSPI_RUN5,0,0},	// S_BSPI_RUN4
	{SPR_BSPI,2,3,{A_Chase},S_BSPI_RUN6,0,0},	// S_BSPI_RUN5
	{SPR_BSPI,2,3,{A_Chase},S_BSPI_RUN7,0,0},	// S_BSPI_RUN6
	{SPR_BSPI,3,3,{A_BabyMetal},S_BSPI_RUN8,0,0},	// S_BSPI_RUN7
	{SPR_BSPI,3,3,{A_Chase},S_BSPI_RUN9,0,0},	// S_BSPI_RUN8
	{SPR_BSPI,4,3,{A_Chase},S_BSPI_RUN10,0,0},	// S_BSPI_RUN9
	{SPR_BSPI,4,3,{A_Chase},S_BSPI_RUN11,0,0},	// S_BSPI_RUN10
	{SPR_BSPI,5,3,{A_Chase},S_BSPI_RUN12,0,0},	// S_BSPI_RUN11
	{SPR_BSPI,5,3,{A_Chase},S_BSPI_RUN1,0,0},	// S_BSPI_RUN12
	{SPR_BSPI,32768,20,{A_FaceTarget},S_BSPI_ATK2,0,0},	// S_BSPI_ATK1
	{SPR_BSPI,32774,4,{A_BspiAttack},S_BSPI_ATK3,0,0},	// S_BSPI_ATK2
	{SPR_BSPI,32775,4,{NULL},S_BSPI_ATK4,0,0},	// S_BSPI_ATK3
	{SPR_BSPI,32775,1,{A_SpidRefire},S_BSPI_ATK2,0,0},	// S_BSPI_ATK4
	{SPR_BSPI,8,3,{NULL},S_BSPI_PAIN2,0,0},	// S_BSPI_PAIN
	{SPR_BSPI,8,3,{A_Pain},S_BSPI_RUN1,0,0},	// S_BSPI_PAIN2
	{SPR_BSPI,9,20,{A_Scream},S_BSPI_DIE2,0,0},	// S_BSPI_DIE1
	{SPR_BSPI,10,7,{A_Fall},S_BSPI_DIE3,0,0},	// S_BSPI_DIE2
	{SPR_BSPI,11,7,{NULL},S_BSPI_DIE4,0,0},	// S_BSPI_DIE3
	{SPR_BSPI,12,7,{NULL},S_BSPI_DIE5,0,0},	// S_BSPI_DIE4
	{SPR_BSPI,13,7,{NULL},S_BSPI_DIE6,0,0},	// S_BSPI_DIE5
	{SPR_BSPI,14,7,{NULL},S_BSPI_DIE7,0,0},	// S_BSPI_DIE6
	{SPR_BSPI,15,-1,{A_BossDeath},S_NULL,0,0},	// S_BSPI_DIE7
	{SPR_BSPI,15,5,{NULL},S_BSPI_RAISE2,0,0},	// S_BSPI_RAISE1
	{SPR_BSPI,14,5,{NULL},S_BSPI_RAISE3,0,0},	// S_BSPI_RAISE2
	{SPR_BSPI,13,5,{NULL},S_BSPI_RAISE4,0,0},	// S_BSPI_RAISE3
	{SPR_BSPI,12,5,{NULL},S_BSPI_RAISE5,0,0},	// S_BSPI_RAISE4
	{SPR_BSPI,11,5,{NULL},S_BSPI_RAISE6,0,0},	// S_BSPI_RAISE5
	{SPR_BSPI,10,5,{NULL},S_BSPI_RAISE7,0,0},	// S_BSPI_RAISE6
	{SPR_BSPI,9,5,{NULL},S_BSPI_RUN1,0,0},	// S_BSPI_RAISE7
	{SPR_APLS,32768,5,{NULL},S_ARACH_PLAZ2,0,0},	// S_ARACH_PLAZ
	{SPR_APLS,32769,5,{NULL},S_ARACH_PLAZ,0,0},	// S_ARACH_PLAZ2
	{SPR_APBX,32768,5,{NULL},S_ARACH_PLEX2,0,0},	// S_ARACH_PLEX
	{SPR_APBX,32769,5,{NULL},S_ARACH_PLEX3,0,0},	// S_ARACH_PLEX2
	{SPR_APBX,32770,5,{NULL},S_ARACH_PLEX4,0,0},	// S_ARACH_PLEX3
	{SPR_APBX,32771,5,{NULL},S_ARACH_PLEX5,0,0},	// S_ARACH_PLEX4
	{SPR_APBX,32772,5,{NULL},S_NULL,0,0},	// S_ARACH_PLEX5
	{SPR_CYBR,0,10,{A_Look},S_CYBER_STND2,0,0},	// S_CYBER_STND
	{SPR_CYBR,1,10,{A_Look},S_CYBER_STND,0,0},	// S_CYBER_STND2
	{SPR_CYBR,0,3,{A_Hoof},S_CYBER_RUN2,0,0},	// S_CYBER_RUN1
	{SPR_CYBR,0,3,{A_Chase},S_CYBER_RUN3,0,0},	// S_CYBER_RUN2
	{SPR_CYBR,1,3,{A_Chase},S_CYBER_RUN4,0,0},	// S_CYBER_RUN3
	{SPR_CYBR,1,3,{A_Chase},S_CYBER_RUN5,0,0},	// S_CYBER_RUN4
	{SPR_CYBR,2,3,{A_Chase},S_CYBER_RUN6,0,0},	// S_CYBER_RUN5
	{SPR_CYBR,2,3,{A_Chase},S_CYBER_RUN7,0,0},	// S_CYBER_RUN6
	{SPR_CYBR,3,3,{A_Metal},S_CYBER_RUN8,0,0},	// S_CYBER_RUN7
	{SPR_CYBR,3,3,{A_Chase},S_CYBER_RUN1,0,0},	// S_CYBER_RUN8
	{SPR_CYBR,4,6,{A_FaceTarget},S_CYBER_ATK2,0,0},	// S_CYBER_ATK1
	{SPR_CYBR,5,12,{A_CyberAttack},S_CYBER_ATK3,0,0},	// S_CYBER_ATK2
	{SPR_CYBR,4,12,{A_FaceTarget},S_CYBER_ATK4,0,0},	// S_CYBER_ATK3
	{SPR_CYBR,5,12,{A_CyberAttack},S_CYBER_ATK5,0,0},	// S_CYBER_ATK4
	{SPR_CYBR,4,12,{A_FaceTarget},S_CYBER_ATK6,0,0},	// S_CYBER_ATK5
	{SPR_CYBR,5,12,{A_CyberAttack},S_CYBER_RUN1,0,0},	// S_CYBER_ATK6
	{SPR_CYBR,6,10,{A_Pain},S_CYBER_RUN1,0,0},	// S_CYBER_PAIN
	{SPR_CYBR,7,10,{NULL},S_CYBER_DIE2,0,0},	// S_CYBER_DIE1
	{SPR_CYBR,8,10,{A_Scream},S_CYBER_DIE3,0,0},	// S_CYBER_DIE2
	{SPR_CYBR,9,10,{NULL},S_CYBER_DIE4,0,0},	// S_CYBER_DIE3
	{SPR_CYBR,10,10,{NULL},S_CYBER_DIE5,0,0},	// S_CYBER_DIE4
	{SPR_CYBR,11,10,{NULL},S_CYBER_DIE6,0,0},	// S_CYBER_DIE5
	{SPR_CYBR,12,10,{A_Fall},S_CYBER_DIE7,0,0},	// S_CYBER_DIE6
	{SPR_CYBR,13,10,{NULL},S_CYBER_DIE8,0,0},	// S_CYBER_DIE7
	{SPR_CYBR,14,10,{NULL},S_CYBER_DIE9,0,0},	// S_CYBER_DIE8
	{SPR_CYBR,15,30,{NULL},S_CYBER_DIE10,0,0},	// S_CYBER_DIE9
	{SPR_CYBR,15,-1,{A_BossDeath},S_NULL,0,0},	// S_CYBER_DIE10
	{SPR_PAIN,0,10,{A_Look},S_PAIN_STND,0,0},	// S_PAIN_STND
	{SPR_PAIN,0,3,{A_Chase},S_PAIN_RUN2,0,0},	// S_PAIN_RUN1
	{SPR_PAIN,0,3,{A_Chase},S_PAIN_RUN3,0,0},	// S_PAIN_RUN2
	{SPR_PAIN,1,3,{A_Chase},S_PAIN_RUN4,0,0},	// S_PAIN_RUN3
	{SPR_PAIN,1,3,{A_Chase},S_PAIN_RUN5,0,0},	// S_PAIN_RUN4
	{SPR_PAIN,2,3,{A_Chase},S_PAIN_RUN6,0,0},	// S_PAIN_RUN5
	{SPR_PAIN,2,3,{A_Chase},S_PAIN_RUN1,0,0},	// S_PAIN_RUN6
	{SPR_PAIN,3,5,{A_FaceTarget},S_PAIN_ATK2,0,0},	// S_PAIN_ATK1
	{SPR_PAIN,4,5,{A_FaceTarget},S_PAIN_ATK3,0,0},	// S_PAIN_ATK2
	{SPR_PAIN,32773,5,{A_FaceTarget},S_PAIN_ATK4,0,0},	// S_PAIN_ATK3
	{SPR_PAIN,32773,0,{A_PainAttack},S_PAIN_RUN1,0,0},	// S_PAIN_ATK4
	{SPR_PAIN,6,6,{NULL},S_PAIN_PAIN2,0,0},	// S_PAIN_PAIN
	{SPR_PAIN,6,6,{A_Pain},S_PAIN_RUN1,0,0},	// S_PAIN_PAIN2
	{SPR_PAIN,32775,8,{NULL},S_PAIN_DIE2,0,0},	// S_PAIN_DIE1
	{SPR_PAIN,32776,8,{A_Scream},S_PAIN_DIE3,0,0},	// S_PAIN_DIE2
	{SPR_PAIN,32777,8,{NULL},S_PAIN_DIE4,0,0},	// S_PAIN_DIE3
	{SPR_PAIN,32778,8,{NULL},S_PAIN_DIE5,0,0},	// S_PAIN_DIE4
	{SPR_PAIN,32779,8,{A_PainDie},S_PAIN_DIE6,0,0},	// S_PAIN_DIE5
	{SPR_PAIN,32780,8,{NULL},S_NULL,0,0},	// S_PAIN_DIE6
	{SPR_PAIN,12,8,{NULL},S_PAIN_RAISE2,0,0},	// S_PAIN_RAISE1
	{SPR_PAIN,11,8,{NULL},S_PAIN_RAISE3,0,0},	// S_PAIN_RAISE2
	{SPR_PAIN,10,8,{NULL},S_PAIN_RAISE4,0,0},	// S_PAIN_RAISE3
	{SPR_PAIN,9,8,{NULL},S_PAIN_RAISE5,0,0},	// S_PAIN_RAISE4
	{SPR_PAIN,8,8,{NULL},S_PAIN_RAISE6,0,0},	// S_PAIN_RAISE5
	{SPR_PAIN,7,8,{NULL},S_PAIN_RUN1,0,0},	// S_PAIN_RAISE6
	{SPR_SSWV,0,10,{A_Look},S_SSWV_STND2,0,0},	// S_SSWV_STND
	{SPR_SSWV,1,10,{A_Look},S_SSWV_STND,0,0},	// S_SSWV_STND2
	{SPR_SSWV,0,3,{A_Chase},S_SSWV_RUN2,0,0},	// S_SSWV_RUN1
	{SPR_SSWV,0,3,{A_Chase},S_SSWV_RUN3,0,0},	// S_SSWV_RUN2
	{SPR_SSWV,1,3,{A_Chase},S_SSWV_RUN4,0,0},	// S_SSWV_RUN3
	{SPR_SSWV,1,3,{A_Chase},S_SSWV_RUN5,0,0},	// S_SSWV_RUN4
	{SPR_SSWV,2,3,{A_Chase},S_SSWV_RUN6,0,0},	// S_SSWV_RUN5
	{SPR_SSWV,2,3,{A_Chase},S_SSWV_RUN7,0,0},	// S_SSWV_RUN6
	{SPR_SSWV,3,3,{A_Chase},S_SSWV_RUN8,0,0},	// S_SSWV_RUN7
	{SPR_SSWV,3,3,{A_Chase},S_SSWV_RUN1,0,0},	// S_SSWV_RUN8
	{SPR_SSWV,4,10,{A_FaceTarget},S_SSWV_ATK2,0,0},	// S_SSWV_ATK1
	{SPR_SSWV,5,10,{A_FaceTarget},S_SSWV_ATK3,0,0},	// S_SSWV_ATK2
	{SPR_SSWV,32774,4,{A_CPosAttack},S_SSWV_ATK4,0,0},	// S_SSWV_ATK3
	{SPR_SSWV,5,6,{A_FaceTarget},S_SSWV_ATK5,0,0},	// S_SSWV_ATK4
	{SPR_SSWV,32774,4,{A_CPosAttack},S_SSWV_ATK6,0,0},	// S_SSWV_ATK5
	{SPR_SSWV,5,1,{A_CPosRefire},S_SSWV_ATK2,0,0},	// S_SSWV_ATK6
	{SPR_SSWV,7,3,{NULL},S_SSWV_PAIN2,0,0},	// S_SSWV_PAIN
	{SPR_SSWV,7,3,{A_Pain},S_SSWV_RUN1,0,0},	// S_SSWV_PAIN2
	{SPR_SSWV,8,5,{NULL},S_SSWV_DIE2,0,0},	// S_SSWV_DIE1
	{SPR_SSWV,9,5,{A_Scream},S_SSWV_DIE3,0,0},	// S_SSWV_DIE2
	{SPR_SSWV,10,5,{A_Fall},S_SSWV_DIE4,0,0},	// S_SSWV_DIE3
	{SPR_SSWV,11,5,{NULL},S_SSWV_DIE5,0,0},	// S_SSWV_DIE4
	{SPR_SSWV,12,-1,{NULL},S_NULL,0,0},	// S_SSWV_DIE5
	{SPR_SSWV,13,5,{NULL},S_SSWV_XDIE2,0,0},	// S_SSWV_XDIE1
	{SPR_SSWV,14,5,{A_XScream},S_SSWV_XDIE3,0,0},	// S_SSWV_XDIE2
	{SPR_SSWV,15,5,{A_Fall},S_SSWV_XDIE4,0,0},	// S_SSWV_XDIE3
	{SPR_SSWV,16,5,{NULL},S_SSWV_XDIE5,0,0},	// S_SSWV_XDIE4
	{SPR_SSWV,17,5,{NULL},S_SSWV_XDIE6,0,0},	// S_SSWV_XDIE5
	{SPR_SSWV,18,5,{NULL},S_SSWV_XDIE7,0,0},	// S_SSWV_XDIE6
	{SPR_SSWV,19,5,{NULL},S_SSWV_XDIE8,0,0},	// S_SSWV_XDIE7
	{SPR_SSWV,20,5,{NULL},S_SSWV_XDIE9,0,0},	// S_SSWV_XDIE8
	{SPR_SSWV,21,-1,{NULL},S_NULL,0,0},	// S_SSWV_XDIE9
	{SPR_SSWV,12,5,{NULL},S_SSWV_RAISE2,0,0},	// S_SSWV_RAISE1
	{SPR_SSWV,11,5,{NULL},S_SSWV_RAISE3,0,0},	// S_SSWV_RAISE2
	{SPR_SSWV,10,5,{NULL},S_SSWV_RAISE4,0,0},	// S_SSWV_RAISE3
	{SPR_SSWV,9,5,{NULL},S_SSWV_RAISE5,0,0},	// S_SSWV_RAISE4
	{SPR_SSWV,8,5,{NULL},S_SSWV_RUN1,0,0},	// S_SSWV_RAISE5
	{SPR_KEEN,0,-1,{NULL},S_KEENSTND,0,0},	// S_KEENSTND
	{SPR_KEEN,0,6,{NULL},S_COMMKEEN2,0,0},	// S_COMMKEEN
	{SPR_KEEN,1,6,{NULL},S_COMMKEEN3,0,0},	// S_COMMKEEN2
	{SPR_KEEN,2,6,{A_Scream},S_COMMKEEN4,0,0},	// S_COMMKEEN3
	{SPR_KEEN,3,6,{NULL},S_COMMKEEN5,0,0},	// S_COMMKEEN4
	{SPR_KEEN,4,6,{NULL},S_COMMKEEN6,0,0},	// S_COMMKEEN5
	{SPR_KEEN,5,6,{NULL},S_COMMKEEN7,0,0},	// S_COMMKEEN6
	{SPR_KEEN,6,6,{NULL},S_COMMKEEN8,0,0},	// S_COMMKEEN7
	{SPR_KEEN,7,6,{NULL},S_COMMKEEN9,0,0},	// S_COMMKEEN8
	{SPR_KEEN,8,6,{NULL},S_COMMKEEN10,0,0},	// S_COMMKEEN9
	{SPR_KEEN,9,6,{NULL},S_COMMKEEN11,0,0},	// S_COMMKEEN10
	{SPR_KEEN,10,6,{A_KeenDie},S_COMMKEEN12,0,0},// S_COMMKEEN11
	{SPR_KEEN,11,-1,{NULL},S_NULL,0,0},		// S_COMMKEEN12
	{SPR_KEEN,12,4,{NULL},S_KEENPAIN2,0,0},	// S_KEENPAIN
	{SPR_KEEN,12,8,{A_Pain},S_KEENSTND,0,0},	// S_KEENPAIN2
	{SPR_BBRN,0,-1,{NULL},S_NULL,0,0},		// S_BRAIN
	{SPR_BBRN,1,36,{A_BrainPain},S_BRAIN,0,0},	// S_BRAIN_PAIN
	{SPR_BBRN,0,100,{A_BrainScream},S_BRAIN_DIE2,0,0},	// S_BRAIN_DIE1
	{SPR_BBRN,0,10,{NULL},S_BRAIN_DIE3,0,0},	// S_BRAIN_DIE2
	{SPR_BBRN,0,10,{NULL},S_BRAIN_DIE4,0,0},	// S_BRAIN_DIE3
	{SPR_BBRN,0,-1,{A_BrainDie},S_NULL,0,0},	// S_BRAIN_DIE4
	{SPR_SSWV,0,10,{A_Look},S_BRAINEYE,0,0},	// S_BRAINEYE
	{SPR_SSWV,0,181,{A_BrainAwake},S_BRAINEYE1,0,0},	// S_BRAINEYESEE
	{SPR_SSWV,0,150,{A_BrainSpit},S_BRAINEYE1,0,0},	// S_BRAINEYE1
	{SPR_BOSF,32768,3,{A_SpawnSound},S_SPAWN2,0,0},	// S_SPAWN1
	{SPR_BOSF,32769,3,{A_SpawnFly},S_SPAWN3,0,0},	// S_SPAWN2
	{SPR_BOSF,32770,3,{A_SpawnFly},S_SPAWN4,0,0},	// S_SPAWN3
	{SPR_BOSF,32771,3,{A_SpawnFly},S_SPAWN1,0,0},	// S_SPAWN4
	{SPR_FIRE,32768,4,{A_Fire},S_SPAWNFIRE2,0,0},	// S_SPAWNFIRE1
	{SPR_FIRE,32769,4,{A_Fire},S_SPAWNFIRE3,0,0},	// S_SPAWNFIRE2
	{SPR_FIRE,32770,4,{A_Fire},S_SPAWNFIRE4,0,0},	// S_SPAWNFIRE3
	{SPR_FIRE,32771,4,{A_Fire},S_SPAWNFIRE5,0,0},	// S_SPAWNFIRE4
	{SPR_FIRE,32772,4,{A_Fire},S_SPAWNFIRE6,0,0},	// S_SPAWNFIRE5
	{SPR_FIRE,32773,4,{A_Fire},S_SPAWNFIRE7,0,0},	// S_SPAWNFIRE6
	{SPR_FIRE,32774,4,{A_Fire},S_SPAWNFIRE8,0,0},	// S_SPAWNFIRE7
	{SPR_FIRE,32775,4,{A_Fire},S_NULL,0,0},		// S_SPAWNFIRE8
	{SPR_MISL,32769,10,{NULL},S_BRAINEXPLODE2,0,0},	// S_BRAINEXPLODE1
	{SPR_MISL,32770,10,{NULL},S_BRAINEXPLODE3,0,0},	// S_BRAINEXPLODE2
	{SPR_MISL,32771,10,{A_BrainExplode},S_NULL,0,0},	// S_BRAINEXPLODE3
	{SPR_ARM1,0,6,{NULL},S_ARM1A,0,0},	// S_ARM1
	{SPR_ARM1,32769,7,{NULL},S_ARM1,0,0},	// S_ARM1A
	{SPR_ARM2,0,6,{NULL},S_ARM2A,0,0},	// S_ARM2
	{SPR_ARM2,32769,6,{NULL},S_ARM2,0,0},	// S_ARM2A
	{SPR_BAR1,0,6,{NULL},S_BAR2,0,0},	// S_BAR1
	{SPR_BAR1,1,6,{NULL},S_BAR1,0,0},	// S_BAR2
	{SPR_BEXP,32768,5,{NULL},S_BEXP2,0,0},	// S_BEXP
	{SPR_BEXP,32769,5,{A_Scream},S_BEXP3,0,0},	// S_BEXP2
	{SPR_BEXP,32770,5,{NULL},S_BEXP4,0,0},	// S_BEXP3
	{SPR_BEXP,32771,10,{A_Explode},S_BEXP5,0,0},	// S_BEXP4
	{SPR_BEXP,32772,10,{NULL},S_NULL,0,0},	// S_BEXP5
	{SPR_FCAN,32768,4,{NULL},S_BBAR2,0,0},	// S_BBAR1
	{SPR_FCAN,32769,4,{NULL},S_BBAR3,0,0},	// S_BBAR2
	{SPR_FCAN,32770,4,{NULL},S_BBAR1,0,0},	// S_BBAR3
	{SPR_BON1,0,6,{NULL},S_BON1A,0,0},	// S_BON1
	{SPR_BON1,1,6,{NULL},S_BON1B,0,0},	// S_BON1A
	{SPR_BON1,2,6,{NULL},S_BON1C,0,0},	// S_BON1B
	{SPR_BON1,3,6,{NULL},S_BON1D,0,0},	// S_BON1C
	{SPR_BON1,2,6,{NULL},S_BON1E,0,0},	// S_BON1D
	{SPR_BON1,1,6,{NULL},S_BON1,0,0},	// S_BON1E
	{SPR_BON2,0,6,{NULL},S_BON2A,0,0},	// S_BON2
	{SPR_BON2,1,6,{NULL},S_BON2B,0,0},	// S_BON2A
	{SPR_BON2,2,6,{NULL},S_BON2C,0,0},	// S_BON2B
	{SPR_BON2,3,6,{NULL},S_BON2D,0,0},	// S_BON2C
	{SPR_BON2,2,6,{NULL},S_BON2E,0,0},	// S_BON2D
	{SPR_BON2,1,6,{NULL},S_BON2,0,0},	// S_BON2E
	{SPR_BKEY,0,10,{NULL},S_BKEY2,0,0},	// S_BKEY
	{SPR_BKEY,32769,10,{NULL},S_BKEY,0,0},	// S_BKEY2
	{SPR_RKEY,0,10,{NULL},S_RKEY2,0,0},	// S_RKEY
	{SPR_RKEY,32769,10,{NULL},S_RKEY,0,0},	// S_RKEY2
	{SPR_YKEY,0,10,{NULL},S_YKEY2,0,0},	// S_YKEY
	{SPR_YKEY,32769,10,{NULL},S_YKEY,0,0},	// S_YKEY2
	{SPR_BSKU,0,10,{NULL},S_BSKULL2,0,0},	// S_BSKULL
	{SPR_BSKU,32769,10,{NULL},S_BSKULL,0,0},	// S_BSKULL2
	{SPR_RSKU,0,10,{NULL},S_RSKULL2,0,0},	// S_RSKULL
	{SPR_RSKU,32769,10,{NULL},S_RSKULL,0,0},	// S_RSKULL2
	{SPR_YSKU,0,10,{NULL},S_YSKULL2,0,0},	// S_YSKULL
	{SPR_YSKU,32769,10,{NULL},S_YSKULL,0,0},	// S_YSKULL2
	{SPR_STIM,0,-1,{NULL},S_NULL,0,0},	// S_STIM
	{SPR_MEDI,0,-1,{NULL},S_NULL,0,0},	// S_MEDI
	{SPR_SOUL,32768,6,{NULL},S_SOUL2,0,0},	// S_SOUL
	{SPR_SOUL,32769,6,{NULL},S_SOUL3,0,0},	// S_SOUL2
	{SPR_SOUL,32770,6,{NULL},S_SOUL4,0,0},	// S_SOUL3
	{SPR_SOUL,32771,6,{NULL},S_SOUL5,0,0},	// S_SOUL4
	{SPR_SOUL,32770,6,{NULL},S_SOUL6,0,0},	// S_SOUL5
	{SPR_SOUL,32769,6,{NULL},S_SOUL,0,0},	// S_SOUL6
	{SPR_PINV,32768,6,{NULL},S_PINV2,0,0},	// S_PINV
	{SPR_PINV,32769,6,{NULL},S_PINV3,0,0},	// S_PINV2
	{SPR_PINV,32770,6,{NULL},S_PINV4,0,0},	// S_PINV3
	{SPR_PINV,32771,6,{NULL},S_PINV,0,0},	// S_PINV4
	{SPR_PSTR,32768,-1,{NULL},S_NULL,0,0},	// S_PSTR
	{SPR_PINS,32768,6,{NULL},S_PINS2,0,0},	// S_PINS
	{SPR_PINS,32769,6,{NULL},S_PINS3,0,0},	// S_PINS2
	{SPR_PINS,32770,6,{NULL},S_PINS4,0,0},	// S_PINS3
	{SPR_PINS,32771,6,{NULL},S_PINS,0,0},	// S_PINS4
	{SPR_MEGA,32768,6,{NULL},S_MEGA2,0,0},	// S_MEGA
	{SPR_MEGA,32769,6,{NULL},S_MEGA3,0,0},	// S_MEGA2
	{SPR_MEGA,32770,6,{NULL},S_MEGA4,0,0},	// S_MEGA3
	{SPR_MEGA,32771,6,{NULL},S_MEGA,0,0},	// S_MEGA4
	{SPR_SUIT,32768,-1,{NULL},S_NULL,0,0},	// S_SUIT
	{SPR_PMAP,32768,6,{NULL},S_PMAP2,0,0},	// S_PMAP
	{SPR_PMAP,32769,6,{NULL},S_PMAP3,0,0},	// S_PMAP2
	{SPR_PMAP,32770,6,{NULL},S_PMAP4,0,0},	// S_PMAP3
	{SPR_PMAP,32771,6,{NULL},S_PMAP5,0,0},	// S_PMAP4
	{SPR_PMAP,32770,6,{NULL},S_PMAP6,0,0},	// S_PMAP5
	{SPR_PMAP,32769,6,{NULL},S_PMAP,0,0},	// S_PMAP6
	{SPR_PVIS,32768,6,{NULL},S_PVIS2,0,0},	// S_PVIS
	{SPR_PVIS,1,6,{NULL},S_PVIS,0,0},	// S_PVIS2
	{SPR_CLIP,0,-1,{NULL},S_NULL,0,0},	// S_CLIP
	{SPR_AMMO,0,-1,{NULL},S_NULL,0,0},	// S_AMMO
	{SPR_ROCK,0,-1,{NULL},S_NULL,0,0},	// S_ROCK
	{SPR_BROK,0,-1,{NULL},S_NULL,0,0},	// S_BROK
	{SPR_CELL,0,-1,{NULL},S_NULL,0,0},	// S_CELL
	{SPR_CELP,0,-1,{NULL},S_NULL,0,0},	// S_CELP
	{SPR_SHEL,0,-1,{NULL},S_NULL,0,0},	// S_SHEL
	{SPR_SBOX,0,-1,{NULL},S_NULL,0,0},	// S_SBOX
	{SPR_BPAK,0,-1,{NULL},S_NULL,0,0},	// S_BPAK
	{SPR_BFUG,0,-1,{NULL},S_NULL,0,0},	// S_BFUG
	{SPR_MGUN,0,-1,{NULL},S_NULL,0,0},	// S_MGUN
	{SPR_CSAW,0,-1,{NULL},S_NULL,0,0},	// S_CSAW
	{SPR_LAUN,0,-1,{NULL},S_NULL,0,0},	// S_LAUN
	{SPR_PLAS,0,-1,{NULL},S_NULL,0,0},	// S_PLAS
	{SPR_SHOT,0,-1,{NULL},S_NULL,0,0},	// S_SHOT
	{SPR_SGN2,0,-1,{NULL},S_NULL,0,0},	// S_SHOT2
	{SPR_COLU,32768,-1,{NULL},S_NULL,0,0},	// S_COLU
	{SPR_SMT2,0,-1,{NULL},S_NULL,0,0},	// S_STALAG
	{SPR_GOR1,0,10,{NULL},S_BLOODYTWITCH2,0,0},	// S_BLOODYTWITCH
	{SPR_GOR1,1,15,{NULL},S_BLOODYTWITCH3,0,0},	// S_BLOODYTWITCH2
	{SPR_GOR1,2,8,{NULL},S_BLOODYTWITCH4,0,0},	// S_BLOODYTWITCH3
	{SPR_GOR1,1,6,{NULL},S_BLOODYTWITCH,0,0},	// S_BLOODYTWITCH4
	{SPR_PLAY,13,-1,{NULL},S_NULL,0,0},	// S_DEADTORSO
	{SPR_PLAY,18,-1,{NULL},S_NULL,0,0},	// S_DEADBOTTOM
	{SPR_POL2,0,-1,{NULL},S_NULL,0,0},	// S_HEADSONSTICK
	{SPR_POL5,0,-1,{NULL},S_NULL,0,0},	// S_GIBS
	{SPR_POL4,0,-1,{NULL},S_NULL,0,0},	// S_HEADONASTICK
	{SPR_POL3,32768,6,{NULL},S_HEADCANDLES2,0,0},	// S_HEADCANDLES
	{SPR_POL3,32769,6,{NULL},S_HEADCANDLES,0,0},	// S_HEADCANDLES2
	{SPR_POL1,0,-1,{NULL},S_NULL,0,0},	// S_DEADSTICK
	{SPR_POL6,0,6,{NULL},S_LIVESTICK2,0,0},	// S_LIVESTICK
	{SPR_POL6,1,8,{NULL},S_LIVESTICK,0,0},	// S_LIVESTICK2
	{SPR_GOR2,0,-1,{NULL},S_NULL,0,0},	// S_MEAT2
	{SPR_GOR3,0,-1,{NULL},S_NULL,0,0},	// S_MEAT3
	{SPR_GOR4,0,-1,{NULL},S_NULL,0,0},	// S_MEAT4
	{SPR_GOR5,0,-1,{NULL},S_NULL,0,0},	// S_MEAT5
	{SPR_SMIT,0,-1,{NULL},S_NULL,0,0},	// S_STALAGTITE
	{SPR_COL1,0,-1,{NULL},S_NULL,0,0},	// S_TALLGRNCOL
	{SPR_COL2,0,-1,{NULL},S_NULL,0,0},	// S_SHRTGRNCOL
	{SPR_COL3,0,-1,{NULL},S_NULL,0,0},	// S_TALLREDCOL
	{SPR_COL4,0,-1,{NULL},S_NULL,0,0},	// S_SHRTREDCOL
	{SPR_CAND,32768,-1,{NULL},S_NULL,0,0},	// S_CANDLESTIK
	{SPR_CBRA,32768,-1,{NULL},S_NULL,0,0},	// S_CANDELABRA
	{SPR_COL6,0,-1,{NULL},S_NULL,0,0},	// S_SKULLCOL
	{SPR_TRE1,0,-1,{NULL},S_NULL,0,0},	// S_TORCHTREE
	{SPR_TRE2,0,-1,{NULL},S_NULL,0,0},	// S_BIGTREE
	{SPR_ELEC,0,-1,{NULL},S_NULL,0,0},	// S_TECHPILLAR
	{SPR_CEYE,32768,6,{NULL},S_EVILEYE2,0,0},	// S_EVILEYE
	{SPR_CEYE,32769,6,{NULL},S_EVILEYE3,0,0},	// S_EVILEYE2
	{SPR_CEYE,32770,6,{NULL},S_EVILEYE4,0,0},	// S_EVILEYE3
	{SPR_CEYE,32769,6,{NULL},S_EVILEYE,0,0},	// S_EVILEYE4
	{SPR_FSKU,32768,6,{NULL},S_FLOATSKULL2,0,0},	// S_FLOATSKULL
	{SPR_FSKU,32769,6,{NULL},S_FLOATSKULL3,0,0},	// S_FLOATSKULL2
	{SPR_FSKU,32770,6,{NULL},S_FLOATSKULL,0,0},	// S_FLOATSKULL3
	{SPR_COL5,0,14,{NULL},S_HEARTCOL2,0,0},	// S_HEARTCOL
	{SPR_COL5,1,14,{NULL},S_HEARTCOL,0,0},	// S_HEARTCOL2
	{SPR_TBLU,32768,4,{NULL},S_BLUETORCH2,0,0},	// S_BLUETORCH
	{SPR_TBLU,32769,4,{NULL},S_BLUETORCH3,0,0},	// S_BLUETORCH2
	{SPR_TBLU,32770,4,{NULL},S_BLUETORCH4,0,0},	// S_BLUETORCH3
	{SPR_TBLU,32771,4,{NULL},S_BLUETORCH,0,0},	// S_BLUETORCH4
	{SPR_TGRN,32768,4,{NULL},S_GREENTORCH2,0,0},	// S_GREENTORCH
	{SPR_TGRN,32769,4,{NULL},S_GREENTORCH3,0,0},	// S_GREENTORCH2
	{SPR_TGRN,32770,4,{NULL},S_GREENTORCH4,0,0},	// S_GREENTORCH3
	{SPR_TGRN,32771,4,{NULL},S_GREENTORCH,0,0},	// S_GREENTORCH4
	{SPR_TRED,32768,4,{NULL},S_REDTORCH2,0,0},	// S_REDTORCH
	{SPR_TRED,32769,4,{NULL},S_REDTORCH3,0,0},	// S_REDTORCH2
	{SPR_TRED,32770,4,{NULL},S_REDTORCH4,0,0},	// S_REDTORCH3
	{SPR_TRED,32771,4,{NULL},S_REDTORCH,0,0},	// S_REDTORCH4
	{SPR_SMBT,32768,4,{NULL},S_BTORCHSHRT2,0,0},	// S_BTORCHSHRT
	{SPR_SMBT,32769,4,{NULL},S_BTORCHSHRT3,0,0},	// S_BTORCHSHRT2
	{SPR_SMBT,32770,4,{NULL},S_BTORCHSHRT4,0,0},	// S_BTORCHSHRT3
	{SPR_SMBT,32771,4,{NULL},S_BTORCHSHRT,0,0},	// S_BTORCHSHRT4
	{SPR_SMGT,32768,4,{NULL},S_GTORCHSHRT2,0,0},	// S_GTORCHSHRT
	{SPR_SMGT,32769,4,{NULL},S_GTORCHSHRT3,0,0},	// S_GTORCHSHRT2
	{SPR_SMGT,32770,4,{NULL},S_GTORCHSHRT4,0,0},	// S_GTORCHSHRT3
	{SPR_SMGT,32771,4,{NULL},S_GTORCHSHRT,0,0},	// S_GTORCHSHRT4
	{SPR_SMRT,32768,4,{NULL},S_RTORCHSHRT2,0,0},	// S_RTORCHSHRT
	{SPR_SMRT,32769,4,{NULL},S_RTORCHSHRT3,0,0},	// S_RTORCHSHRT2
	{SPR_SMRT,32770,4,{NULL},S_RTORCHSHRT4,0,0},	// S_RTORCHSHRT3
	{SPR_SMRT,32771,4,{NULL},S_RTORCHSHRT,0,0},	// S_RTORCHSHRT4
	{SPR_HDB1,0,-1,{NULL},S_NULL,0,0},	// S_HANGNOGUTS
	{SPR_HDB2,0,-1,{NULL},S_NULL,0,0},	// S_HANGBNOBRAIN
	{SPR_HDB3,0,-1,{NULL},S_NULL,0,0},	// S_HANGTLOOKDN
	{SPR_HDB4,0,-1,{NULL},S_NULL,0,0},	// S_HANGTSKULL
	{SPR_HDB5,0,-1,{NULL},S_NULL,0,0},	// S_HANGTLOOKUP
	{SPR_HDB6,0,-1,{NULL},S_NULL,0,0},	// S_HANGTNOBRAIN
	{SPR_POB1,0,-1,{NULL},S_NULL,0,0},	// S_COLONGIBS
	{SPR_POB2,0,-1,{NULL},S_NULL,0,0},	// S_SMALLPOOL
	{SPR_BRS1,0,-1,{NULL},S_NULL,0,0},		// S_BRAINSTEM
	{SPR_TLMP,32768,4,{NULL},S_TECHLAMP2,0,0},	// S_TECHLAMP
	{SPR_TLMP,32769,4,{NULL},S_TECHLAMP3,0,0},	// S_TECHLAMP2
	{SPR_TLMP,32770,4,{NULL},S_TECHLAMP4,0,0},	// S_TECHLAMP3
	{SPR_TLMP,32771,4,{NULL},S_TECHLAMP,0,0},	// S_TECHLAMP4
	{SPR_TLP2,32768,4,{NULL},S_TECH2LAMP2,0,0},	// S_TECH2LAMP
	{SPR_TLP2,32769,4,{NULL},S_TECH2LAMP3,0,0},	// S_TECH2LAMP2
	{SPR_TLP2,32770,4,{NULL},S_TECH2LAMP4,0,0},	// S_TECH2LAMP3
	{SPR_TLP2,32771,4,{NULL},S_TECH2LAMP,0,0}	// S_TECH2LAMP4
};

// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
animdef_t		animdefs[] =
{
	{false,	"NUKAGE3",	"NUKAGE1",	8},
	{false,	"FWATER4",	"FWATER1",	8},
	{false,	"SWATER4",	"SWATER1", 	8},
	{false,	"LAVA4",	"LAVA1",	8},
	{false,	"BLOOD3",	"BLOOD1",	8},

	// DOOM II flat animations.
	{false,	"RROCK08",	"RROCK05",	8},
	{false,	"SLIME04",	"SLIME01",	8},
	{false,	"SLIME08",	"SLIME05",	8},
	{false,	"SLIME12",	"SLIME09",	8},

	{true,	"BLODGR4",	"BLODGR1",	8},
	{true,	"SLADRIP3",	"SLADRIP1",	8},

	{true,	"BLODRIP4",	"BLODRIP1",	8},
	{true,	"FIREWALL",	"FIREWALA",	8},
	{true,	"GSTFONT3",	"GSTFONT1",	8},
	{true,	"FIRELAVA",	"FIRELAV3",	8},
	{true,	"FIREMAG3",	"FIREMAG1",	8},
	{true,	"FIREBLU2",	"FIREBLU1",	8},
	{true,	"ROCKRED3",	"ROCKRED1",	8},

	{true,	"BFALL4",	"BFALL1",	8},
	{true,	"SFALL4",	"SFALL1",	8},
	{true,	"WFALL4",	"WFALL1",	8},
	{true,	"DBRAIN4",	"DBRAIN1",	8},

	{-1}
};


//
//	HU_stuff.C
//
#define HUSTR_E1M1	"E1M1: Hangar"
#define HUSTR_E1M2	"E1M2: Nuclear Plant"
#define HUSTR_E1M3	"E1M3: Toxin Refinery"
#define HUSTR_E1M4	"E1M4: Command Control"
#define HUSTR_E1M5	"E1M5: Phobos Lab"
#define HUSTR_E1M6	"E1M6: Central Processing"
#define HUSTR_E1M7	"E1M7: Computer Station"
#define HUSTR_E1M8	"E1M8: Phobos Anomaly"
#define HUSTR_E1M9	"E1M9: Military Base"

#define HUSTR_E2M1	"E2M1: Deimos Anomaly"
#define HUSTR_E2M2	"E2M2: Containment Area"
#define HUSTR_E2M3	"E2M3: Refinery"
#define HUSTR_E2M4	"E2M4: Deimos Lab"
#define HUSTR_E2M5	"E2M5: Command Center"
#define HUSTR_E2M6	"E2M6: Halls of the Damned"
#define HUSTR_E2M7	"E2M7: Spawning Vats"
#define HUSTR_E2M8	"E2M8: Tower of Babel"
#define HUSTR_E2M9	"E2M9: Fortress of Mystery"

#define HUSTR_E3M1	"E3M1: Hell Keep"
#define HUSTR_E3M2	"E3M2: Slough of Despair"
#define HUSTR_E3M3	"E3M3: Pandemonium"
#define HUSTR_E3M4	"E3M4: House of Pain"
#define HUSTR_E3M5	"E3M5: Unholy Cathedral"
#define HUSTR_E3M6	"E3M6: Mt. Erebus"
#define HUSTR_E3M7	"E3M7: Limbo"
#define HUSTR_E3M8	"E3M8: Dis"
#define HUSTR_E3M9	"E3M9: Warrens"

#define HUSTR_E4M1	"E4M1: Hell Beneath"
#define HUSTR_E4M2	"E4M2: Perfect Hatred"
#define HUSTR_E4M3	"E4M3: Sever The Wicked"
#define HUSTR_E4M4	"E4M4: Unruly Evil"
#define HUSTR_E4M5	"E4M5: They Will Repent"
#define HUSTR_E4M6	"E4M6: Against Thee Wickedly"
#define HUSTR_E4M7	"E4M7: And Hell Followed"
#define HUSTR_E4M8	"E4M8: Unto The Cruel"
#define HUSTR_E4M9	"E4M9: Fear"

#define HUSTR_1	"level 1: entryway"
#define HUSTR_2	"level 2: underhalls"
#define HUSTR_3	"level 3: the gantlet"
#define HUSTR_4	"level 4: the focus"
#define HUSTR_5	"level 5: the waste tunnels"
#define HUSTR_6	"level 6: the crusher"
#define HUSTR_7	"level 7: dead simple"
#define HUSTR_8	"level 8: tricks and traps"
#define HUSTR_9	"level 9: the pit"
#define HUSTR_10	"level 10: refueling base"
#define HUSTR_11	"level 11: 'o' of destruction!"

#define HUSTR_12	"level 12: the factory"
#define HUSTR_13	"level 13: downtown"
#define HUSTR_14	"level 14: the inmost dens"
#define HUSTR_15	"level 15: industrial zone"
#define HUSTR_16	"level 16: suburbs"
#define HUSTR_17	"level 17: tenements"
#define HUSTR_18	"level 18: the courtyard"
#define HUSTR_19	"level 19: the citadel"
#define HUSTR_20	"level 20: gotcha!"

#define HUSTR_21	"level 21: nirvana"
#define HUSTR_22	"level 22: the catacombs"
#define HUSTR_23	"level 23: barrels o' fun"
#define HUSTR_24	"level 24: the chasm"
#define HUSTR_25	"level 25: bloodfalls"
#define HUSTR_26	"level 26: the abandoned mines"
#define HUSTR_27	"level 27: monster condo"
#define HUSTR_28	"level 28: the spirit world"
#define HUSTR_29	"level 29: the living end"
#define HUSTR_30	"level 30: icon of sin"

#define HUSTR_31	"level 31: wolfenstein"
#define HUSTR_32	"level 32: grosse"

#define PHUSTR_1	"level 1: congo"
#define PHUSTR_2	"level 2: well of souls"
#define PHUSTR_3	"level 3: aztec"
#define PHUSTR_4	"level 4: caged"
#define PHUSTR_5	"level 5: ghost town"
#define PHUSTR_6	"level 6: baron's lair"
#define PHUSTR_7	"level 7: caughtyard"
#define PHUSTR_8	"level 8: realm"
#define PHUSTR_9	"level 9: abattoire"
#define PHUSTR_10	"level 10: onslaught"
#define PHUSTR_11	"level 11: hunted"

#define PHUSTR_12	"level 12: speed"
#define PHUSTR_13	"level 13: the crypt"
#define PHUSTR_14	"level 14: genesis"
#define PHUSTR_15	"level 15: the twilight"
#define PHUSTR_16	"level 16: the omen"
#define PHUSTR_17	"level 17: compound"
#define PHUSTR_18	"level 18: neurosphere"
#define PHUSTR_19	"level 19: nme"
#define PHUSTR_20	"level 20: the death domain"

#define PHUSTR_21	"level 21: slayer"
#define PHUSTR_22	"level 22: impossible mission"
#define PHUSTR_23	"level 23: tombstone"
#define PHUSTR_24	"level 24: the final frontier"
#define PHUSTR_25	"level 25: the temple of darkness"
#define PHUSTR_26	"level 26: bunker"
#define PHUSTR_27	"level 27: anti-christ"
#define PHUSTR_28	"level 28: the sewers"
#define PHUSTR_29	"level 29: odyssey of noises"
#define PHUSTR_30	"level 30: the gateway of hell"

#define PHUSTR_31	"level 31: cyberden"
#define PHUSTR_32	"level 32: go 2 it"

#define THUSTR_1	"level 1: system control"
#define THUSTR_2	"level 2: human bbq"
#define THUSTR_3	"level 3: power control"
#define THUSTR_4	"level 4: wormhole"
#define THUSTR_5	"level 5: hanger"
#define THUSTR_6	"level 6: open season"
#define THUSTR_7	"level 7: prison"
#define THUSTR_8	"level 8: metal"
#define THUSTR_9	"level 9: stronghold"
#define THUSTR_10	"level 10: redemption"
#define THUSTR_11	"level 11: storage facility"

#define THUSTR_12	"level 12: crater"
#define THUSTR_13	"level 13: nukage processing"
#define THUSTR_14	"level 14: steel works"
#define THUSTR_15	"level 15: dead zone"
#define THUSTR_16	"level 16: deepest reaches"
#define THUSTR_17	"level 17: processing area"
#define THUSTR_18	"level 18: mill"
#define THUSTR_19	"level 19: shipping/respawning"
#define THUSTR_20	"level 20: central processing"

#define THUSTR_21	"level 21: administration center"
#define THUSTR_22	"level 22: habitat"
#define THUSTR_23	"level 23: lunar mining project"
#define THUSTR_24	"level 24: quarry"
#define THUSTR_25	"level 25: baron's den"
#define THUSTR_26	"level 26: ballistyx"
#define THUSTR_27	"level 27: mount pain"
#define THUSTR_28	"level 28: heck"
#define THUSTR_29	"level 29: river styx"
#define THUSTR_30	"level 30: last call"

#define THUSTR_31	"level 31: pharaoh"
#define THUSTR_32	"level 32: caribbean"

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

char* mapnames[] =	// DOOM shareware/registered/retail (Ultimate) names.
{

	HUSTR_E1M1,
	HUSTR_E1M2,
	HUSTR_E1M3,
	HUSTR_E1M4,
	HUSTR_E1M5,
	HUSTR_E1M6,
	HUSTR_E1M7,
	HUSTR_E1M8,
	HUSTR_E1M9,

	HUSTR_E2M1,
	HUSTR_E2M2,
	HUSTR_E2M3,
	HUSTR_E2M4,
	HUSTR_E2M5,
	HUSTR_E2M6,
	HUSTR_E2M7,
	HUSTR_E2M8,
	HUSTR_E2M9,

	HUSTR_E3M1,
	HUSTR_E3M2,
	HUSTR_E3M3,
	HUSTR_E3M4,
	HUSTR_E3M5,
	HUSTR_E3M6,
	HUSTR_E3M7,
	HUSTR_E3M8,
	HUSTR_E3M9,

	HUSTR_E4M1,
	HUSTR_E4M2,
	HUSTR_E4M3,
	HUSTR_E4M4,
	HUSTR_E4M5,
	HUSTR_E4M6,
	HUSTR_E4M7,
	HUSTR_E4M8,
	HUSTR_E4M9,

	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL",
	"NEWLEVEL"
};

char* mapnames2[] =	// DOOM 2 map names.
{
	HUSTR_1,
	HUSTR_2,
	HUSTR_3,
	HUSTR_4,
	HUSTR_5,
	HUSTR_6,
	HUSTR_7,
	HUSTR_8,
	HUSTR_9,
	HUSTR_10,
	HUSTR_11,

	HUSTR_12,
	HUSTR_13,
	HUSTR_14,
	HUSTR_15,
	HUSTR_16,
	HUSTR_17,
	HUSTR_18,
	HUSTR_19,
	HUSTR_20,

	HUSTR_21,
	HUSTR_22,
	HUSTR_23,
	HUSTR_24,
	HUSTR_25,
	HUSTR_26,
	HUSTR_27,
	HUSTR_28,
	HUSTR_29,
	HUSTR_30,
	HUSTR_31,
	HUSTR_32
};


char* mapnamesp[] =	// Plutonia WAD map names.
{
	PHUSTR_1,
	PHUSTR_2,
	PHUSTR_3,
	PHUSTR_4,
	PHUSTR_5,
	PHUSTR_6,
	PHUSTR_7,
	PHUSTR_8,
	PHUSTR_9,
	PHUSTR_10,
	PHUSTR_11,

	PHUSTR_12,
	PHUSTR_13,
	PHUSTR_14,
	PHUSTR_15,
	PHUSTR_16,
	PHUSTR_17,
	PHUSTR_18,
	PHUSTR_19,
	PHUSTR_20,

	PHUSTR_21,
	PHUSTR_22,
	PHUSTR_23,
	PHUSTR_24,
	PHUSTR_25,
	PHUSTR_26,
	PHUSTR_27,
	PHUSTR_28,
	PHUSTR_29,
	PHUSTR_30,
	PHUSTR_31,
	PHUSTR_32
};


char* mapnamest[] =	// TNT WAD map names.
{
	THUSTR_1,
	THUSTR_2,
	THUSTR_3,
	THUSTR_4,
	THUSTR_5,
	THUSTR_6,
	THUSTR_7,
	THUSTR_8,
	THUSTR_9,
	THUSTR_10,
	THUSTR_11,

	THUSTR_12,
	THUSTR_13,
	THUSTR_14,
	THUSTR_15,
	THUSTR_16,
	THUSTR_17,
	THUSTR_18,
	THUSTR_19,
	THUSTR_20,

	THUSTR_21,
	THUSTR_22,
	THUSTR_23,
	THUSTR_24,
	THUSTR_25,
	THUSTR_26,
	THUSTR_27,
	THUSTR_28,
	THUSTR_29,
	THUSTR_30,
	THUSTR_31,
	THUSTR_32
};

const TCHAR* GetLevelName(INT iGame, INT iLevel)
{
	const char* Result = nullptr;
	switch (iGame)
	{
	case 0:
		Result = mapnames[iLevel];
		break;
	case 1:
		Result = mapnames2[iLevel];
		break;
	case 2:
		Result = mapnamesp[iLevel];
		break;
	default:
		Result = mapnamest[iLevel];
		break;
	}
	if (!Result)
		Result = "Unknown level";
	return appFromAnsi(Result);
}

//
// Information about all the music
//
musicinfo_t S_music[] =
{
	{ 0 },
	{ "e1m1", 0 },
	{ "e1m2", 0 },
	{ "e1m3", 0 },
	{ "e1m4", 0 },
	{ "e1m5", 0 },
	{ "e1m6", 0 },
	{ "e1m7", 0 },
	{ "e1m8", 0 },
	{ "e1m9", 0 },
	{ "e2m1", 0 },
	{ "e2m2", 0 },
	{ "e2m3", 0 },
	{ "e2m4", 0 },
	{ "e2m5", 0 },
	{ "e2m6", 0 },
	{ "e2m7", 0 },
	{ "e2m8", 0 },
	{ "e2m9", 0 },
	{ "e3m1", 0 },
	{ "e3m2", 0 },
	{ "e3m3", 0 },
	{ "e3m4", 0 },
	{ "e3m5", 0 },
	{ "e3m6", 0 },
	{ "e3m7", 0 },
	{ "e3m8", 0 },
	{ "e3m9", 0 },
	{ "inter", 0 },
	{ "intro", 0 },
	{ "bunny", 0 },
	{ "victor", 0 },
	{ "introa", 0 },
	{ "runnin", 0 },
	{ "stalks", 0 },
	{ "countd", 0 },
	{ "betwee", 0 },
	{ "doom", 0 },
	{ "the_da", 0 },
	{ "shawn", 0 },
	{ "ddtblu", 0 },
	{ "in_cit", 0 },
	{ "dead", 0 },
	{ "stlks2", 0 },
	{ "theda2", 0 },
	{ "doom2", 0 },
	{ "ddtbl2", 0 },
	{ "runni2", 0 },
	{ "dead2", 0 },
	{ "stlks3", 0 },
	{ "romero", 0 },
	{ "shawn2", 0 },
	{ "messag", 0 },
	{ "count2", 0 },
	{ "ddtbl3", 0 },
	{ "ampie", 0 },
	{ "theda3", 0 },
	{ "adrian", 0 },
	{ "messg2", 0 },
	{ "romer2", 0 },
	{ "tense", 0 },
	{ "shawn3", 0 },
	{ "openin", 0 },
	{ "evil", 0 },
	{ "ultima", 0 },
	{ "read_m", 0 },
	{ "dm2ttl", 0 },
	{ "dm2int", 0 },
	{ nullptr, 0 }
};
enum musicenum_t : INT
{
	mus_None,
	mus_e1m1,
	mus_e1m2,
	mus_e1m3,
	mus_e1m4,
	mus_e1m5,
	mus_e1m6,
	mus_e1m7,
	mus_e1m8,
	mus_e1m9,
	mus_e2m1,
	mus_e2m2,
	mus_e2m3,
	mus_e2m4,
	mus_e2m5,
	mus_e2m6,
	mus_e2m7,
	mus_e2m8,
	mus_e2m9,
	mus_e3m1,
	mus_e3m2,
	mus_e3m3,
	mus_e3m4,
	mus_e3m5,
	mus_e3m6,
	mus_e3m7,
	mus_e3m8,
	mus_e3m9,
	mus_inter,
	mus_intro,
	mus_bunny,
	mus_victor,
	mus_introa,
	mus_runnin,
	mus_stalks,
	mus_countd,
	mus_betwee,
	mus_doom,
	mus_the_da,
	mus_shawn,
	mus_ddtblu,
	mus_in_cit,
	mus_dead,
	mus_stlks2,
	mus_theda2,
	mus_doom2,
	mus_ddtbl2,
	mus_runni2,
	mus_dead2,
	mus_stlks3,
	mus_romero,
	mus_shawn2,
	mus_messag,
	mus_count2,
	mus_ddtbl3,
	mus_ampie,
	mus_theda3,
	mus_adrian,
	mus_messg2,
	mus_romer2,
	mus_tense,
	mus_shawn3,
	mus_openin,
	mus_evil,
	mus_ultima,
	mus_read_m,
	mus_dm2ttl,
	mus_dm2int,
	NUMMUSIC
};

const TCHAR* GetMapMusic(INT MapIndex, INT EpisodeIndex)
{
	musicinfo_t* M = nullptr;
	if (EpisodeIndex == INDEX_NONE)
		M = &S_music[mus_runnin + MapIndex - 1];
	else
	{
		int spmus[] =
		{
			// Song - Who? - Where?

			mus_e3m4,	// American	e4m1
			mus_e3m2,	// Romero	e4m2
			mus_e3m3,	// Shawn	e4m3
			mus_e1m5,	// American	e4m4
			mus_e2m7,	// Tim 	e4m5
			mus_e2m4,	// Romero	e4m6
			mus_e2m6,	// J.Anderson	e4m7 CHIRON.WAD
			mus_e2m5,	// Shawn	e4m8
			mus_e1m9	// Tim		e4m9
		};

		if (EpisodeIndex < 4)
			M = &S_music[mus_e1m1 + (EpisodeIndex - 1) * 9 + MapIndex - 1];
		else
			M = &S_music[spmus[MapIndex - 1]];
	}
	TCHAR* Result = appStaticString1024();
	appSprintf(Result, TEXT("D_%ls"), appFromAnsi(M->name));
	return Result;
}

//
// Information about all the sfx
//
sfxinfo_t S_sfx[] =
{
	// S_sfx[0] needs to be a dummy for odd reasons.
	{ "none", false,  0, 0, -1, -1, 0 },

	{ "pistol", false, 64, 0, -1, -1, 0 },
	{ "shotgn", false, 64, 0, -1, -1, 0 },
	{ "sgcock", false, 64, 0, -1, -1, 0 },
	{ "dshtgn", false, 64, 0, -1, -1, 0 },
	{ "dbopn", false, 64, 0, -1, -1, 0 },
	{ "dbcls", false, 64, 0, -1, -1, 0 },
	{ "dbload", false, 64, 0, -1, -1, 0 },
	{ "plasma", false, 64, 0, -1, -1, 0 },
	{ "bfg", false, 64, 0, -1, -1, 0 },
	{ "sawup", false, 64, 0, -1, -1, 0 },
	{ "sawidl", false, 118, 0, -1, -1, 0 },
	{ "sawful", false, 64, 0, -1, -1, 0 },
	{ "sawhit", false, 64, 0, -1, -1, 0 },
	{ "rlaunc", false, 64, 0, -1, -1, 0 },
	{ "rxplod", false, 70, 0, -1, -1, 0 },
	{ "firsht", false, 70, 0, -1, -1, 0 },
	{ "firxpl", false, 70, 0, -1, -1, 0 },
	{ "pstart", false, 100, 0, -1, -1, 0 },
	{ "pstop", false, 100, 0, -1, -1, 0 },
	{ "doropn", false, 100, 0, -1, -1, 0 },
	{ "dorcls", false, 100, 0, -1, -1, 0 },
	{ "stnmov", false, 119, 0, -1, -1, 0 },
	{ "swtchn", false, 78, 0, -1, -1, 0 },
	{ "swtchx", false, 78, 0, -1, -1, 0 },
	{ "plpain", false, 96, 0, -1, -1, 0 },
	{ "dmpain", false, 96, 0, -1, -1, 0 },
	{ "popain", false, 96, 0, -1, -1, 0 },
	{ "vipain", false, 96, 0, -1, -1, 0 },
	{ "mnpain", false, 96, 0, -1, -1, 0 },
	{ "pepain", false, 96, 0, -1, -1, 0 },
	{ "slop", false, 78, 0, -1, -1, 0 },
	{ "itemup", true, 78, 0, -1, -1, 0 },
	{ "wpnup", true, 78, 0, -1, -1, 0 },
	{ "oof", false, 96, 0, -1, -1, 0 },
	{ "telept", false, 32, 0, -1, -1, 0 },
	{ "posit1", true, 98, 0, -1, -1, 0 },
	{ "posit2", true, 98, 0, -1, -1, 0 },
	{ "posit3", true, 98, 0, -1, -1, 0 },
	{ "bgsit1", true, 98, 0, -1, -1, 0 },
	{ "bgsit2", true, 98, 0, -1, -1, 0 },
	{ "sgtsit", true, 98, 0, -1, -1, 0 },
	{ "cacsit", true, 98, 0, -1, -1, 0 },
	{ "brssit", true, 94, 0, -1, -1, 0 },
	{ "cybsit", true, 92, 0, -1, -1, 0 },
	{ "spisit", true, 90, 0, -1, -1, 0 },
	{ "bspsit", true, 90, 0, -1, -1, 0 },
	{ "kntsit", true, 90, 0, -1, -1, 0 },
	{ "vilsit", true, 90, 0, -1, -1, 0 },
	{ "mansit", true, 90, 0, -1, -1, 0 },
	{ "pesit", true, 90, 0, -1, -1, 0 },
	{ "sklatk", false, 70, 0, -1, -1, 0 },
	{ "sgtatk", false, 70, 0, -1, -1, 0 },
	{ "skepch", false, 70, 0, -1, -1, 0 },
	{ "vilatk", false, 70, 0, -1, -1, 0 },
	{ "claw", false, 70, 0, -1, -1, 0 },
	{ "skeswg", false, 70, 0, -1, -1, 0 },
	{ "pldeth", false, 32, 0, -1, -1, 0 },
	{ "pdiehi", false, 32, 0, -1, -1, 0 },
	{ "podth1", false, 70, 0, -1, -1, 0 },
	{ "podth2", false, 70, 0, -1, -1, 0 },
	{ "podth3", false, 70, 0, -1, -1, 0 },
	{ "bgdth1", false, 70, 0, -1, -1, 0 },
	{ "bgdth2", false, 70, 0, -1, -1, 0 },
	{ "sgtdth", false, 70, 0, -1, -1, 0 },
	{ "cacdth", false, 70, 0, -1, -1, 0 },
	{ "skldth", false, 70, 0, -1, -1, 0 },
	{ "brsdth", false, 32, 0, -1, -1, 0 },
	{ "cybdth", false, 32, 0, -1, -1, 0 },
	{ "spidth", false, 32, 0, -1, -1, 0 },
	{ "bspdth", false, 32, 0, -1, -1, 0 },
	{ "vildth", false, 32, 0, -1, -1, 0 },
	{ "kntdth", false, 32, 0, -1, -1, 0 },
	{ "pedth", false, 32, 0, -1, -1, 0 },
	{ "skedth", false, 32, 0, -1, -1, 0 },
	{ "posact", true, 120, 0, -1, -1, 0 },
	{ "bgact", true, 120, 0, -1, -1, 0 },
	{ "dmact", true, 120, 0, -1, -1, 0 },
	{ "bspact", true, 100, 0, -1, -1, 0 },
	{ "bspwlk", true, 100, 0, -1, -1, 0 },
	{ "vilact", true, 100, 0, -1, -1, 0 },
	{ "noway", false, 78, 0, -1, -1, 0 },
	{ "barexp", false, 60, 0, -1, -1, 0 },
	{ "punch", false, 64, 0, -1, -1, 0 },
	{ "hoof", false, 70, 0, -1, -1, 0 },
	{ "metal", false, 70, 0, -1, -1, 0 },
	{ "chgun", false, 64, &S_sfx[sfx_pistol], 150, 0, 0 },
	{ "tink", false, 60, 0, -1, -1, 0 },
	{ "bdopn", false, 100, 0, -1, -1, 0 },
	{ "bdcls", false, 100, 0, -1, -1, 0 },
	{ "itmbk", false, 100, 0, -1, -1, 0 },
	{ "flame", false, 32, 0, -1, -1, 0 },
	{ "flamst", false, 32, 0, -1, -1, 0 },
	{ "getpow", false, 60, 0, -1, -1, 0 },
	{ "bospit", false, 70, 0, -1, -1, 0 },
	{ "boscub", false, 70, 0, -1, -1, 0 },
	{ "bossit", false, 70, 0, -1, -1, 0 },
	{ "bospn", false, 70, 0, -1, -1, 0 },
	{ "bosdth", false, 70, 0, -1, -1, 0 },
	{ "manatk", false, 70, 0, -1, -1, 0 },
	{ "mandth", false, 70, 0, -1, -1, 0 },
	{ "sssit", false, 70, 0, -1, -1, 0 },
	{ "ssdth", false, 70, 0, -1, -1, 0 },
	{ "keenpn", false, 70, 0, -1, -1, 0 },
	{ "keendt", false, 70, 0, -1, -1, 0 },
	{ "skeact", false, 70, 0, -1, -1, 0 },
	{ "skesit", false, 70, 0, -1, -1, 0 },
	{ "skeatk", false, 70, 0, -1, -1, 0 },
	{ "radio", false, 60, 0, -1, -1, 0 }
};

static TMap<FString, sfxenum_t> sxLookupMap;
#define ST(val) sxLookupMap.Set(TEXT(#val),val)

sfxenum_t LookupSfxName(const TCHAR* inName)
{
	if (!sxLookupMap.Num())
	{
		ST(sfx_pistol);
		ST(sfx_shotgn);
		ST(sfx_sgcock);
		ST(sfx_dshtgn);
		ST(sfx_dbopn);
		ST(sfx_dbcls);
		ST(sfx_dbload);
		ST(sfx_plasma);
		ST(sfx_bfg);
		ST(sfx_sawup);
		ST(sfx_sawidl);
		ST(sfx_sawful);
		ST(sfx_sawhit);
		ST(sfx_rlaunc);
		ST(sfx_rxplod);
		ST(sfx_firsht);
		ST(sfx_firxpl);
		ST(sfx_pstart);
		ST(sfx_pstop);
		ST(sfx_doropn);
		ST(sfx_dorcls);
		ST(sfx_stnmov);
		ST(sfx_swtchn);
		ST(sfx_swtchx);
		ST(sfx_plpain);
		ST(sfx_dmpain);
		ST(sfx_popain);
		ST(sfx_vipain);
		ST(sfx_mnpain);
		ST(sfx_pepain);
		ST(sfx_slop);
		ST(sfx_itemup);
		ST(sfx_wpnup);
		ST(sfx_oof);
		ST(sfx_telept);
		ST(sfx_posit1);
		ST(sfx_posit2);
		ST(sfx_posit3);
		ST(sfx_bgsit1);
		ST(sfx_bgsit2);
		ST(sfx_sgtsit);
		ST(sfx_cacsit);
		ST(sfx_brssit);
		ST(sfx_cybsit);
		ST(sfx_spisit);
		ST(sfx_bspsit);
		ST(sfx_kntsit);
		ST(sfx_vilsit);
		ST(sfx_mansit);
		ST(sfx_pesit);
		ST(sfx_sklatk);
		ST(sfx_sgtatk);
		ST(sfx_skepch);
		ST(sfx_vilatk);
		ST(sfx_claw);
		ST(sfx_skeswg);
		ST(sfx_pldeth);
		ST(sfx_pdiehi);
		ST(sfx_podth1);
		ST(sfx_podth2);
		ST(sfx_podth3);
		ST(sfx_bgdth1);
		ST(sfx_bgdth2);
		ST(sfx_sgtdth);
		ST(sfx_cacdth);
		ST(sfx_skldth);
		ST(sfx_brsdth);
		ST(sfx_cybdth);
		ST(sfx_spidth);
		ST(sfx_bspdth);
		ST(sfx_vildth);
		ST(sfx_kntdth);
		ST(sfx_pedth);
		ST(sfx_skedth);
		ST(sfx_posact);
		ST(sfx_bgact);
		ST(sfx_dmact);
		ST(sfx_bspact);
		ST(sfx_bspwlk);
		ST(sfx_vilact);
		ST(sfx_noway);
		ST(sfx_barexp);
		ST(sfx_punch);
		ST(sfx_hoof);
		ST(sfx_metal);
		ST(sfx_chgun);
		ST(sfx_tink);
		ST(sfx_bdopn);
		ST(sfx_bdcls);
		ST(sfx_itmbk);
		ST(sfx_flame);
		ST(sfx_flamst);
		ST(sfx_getpow);
		ST(sfx_bospit);
		ST(sfx_boscub);
		ST(sfx_bossit);
		ST(sfx_bospn);
		ST(sfx_bosdth);
		ST(sfx_manatk);
		ST(sfx_mandth);
		ST(sfx_sssit);
		ST(sfx_ssdth);
		ST(sfx_keenpn);
		ST(sfx_keendt);
		ST(sfx_skeact);
		ST(sfx_skesit);
		ST(sfx_skeatk);
		ST(sfx_radio);
	}
	sfxenum_t* res = sxLookupMap.Find(inName);
	return res ? *res : sfx_None;
}
#undef ST

statenum_t LookupStateName(const TCHAR* InName)
{
	if (!appStricmp(InName, TEXT("S_VILE_HEAL1")))
		return S_VILE_HEAL1;
	return S_NULL;
}

UBOOL IsProjectile(const INT type)
{
	switch (type)
	{
	case MT_TRACER:
	case MT_FATSHOT:
	case MT_BRUISERSHOT:
	case MT_SPAWNSHOT:
	case MT_TROOPSHOT:
	case MT_HEADSHOT:
	case MT_ROCKET:
	case MT_PLASMA:
	case MT_BFG:
	case MT_ARACHPLAZ:
	case MT_FIRE:
		return TRUE;
	default:
		return FALSE;
	}
}
UBOOL IsMonster(const INT type)
{
	switch (type)
	{
	case MT_PLAYER:
	case MT_POSSESSED:
	case MT_SHOTGUY:
	case MT_VILE:
	case MT_UNDEAD:
	case MT_FATSO:
	case MT_CHAINGUY:
	case MT_TROOP:
	case MT_SERGEANT:
	case MT_SHADOWS:
	case MT_HEAD:
	case MT_BRUISER:
	case MT_KNIGHT:
	case MT_SPIDER:
	case MT_BABY:
	case MT_CYBORG:
	case MT_PAIN:
	case MT_WOLFSS:
	case MT_SKULL:
	case MT_KEEN:
	case MT_BOSSBRAIN:
		return TRUE;
	default:
		return FALSE;
	}
}
UBOOL IsWeapon(const INT type)
{
	switch (type)
	{
	case MT_MISC25:
	case MT_CHAINGUN:
	case MT_MISC26:
	case MT_MISC27:
	case MT_MISC28:
	case MT_SHOTGUN:
	case MT_SUPERSHOTGUN:
		return TRUE;
	default:
		return FALSE;
	}
}
UBOOL IsAmmo(const INT type)
{
	switch (type)
	{
	case MT_CLIP:
	case MT_MISC17:
	case MT_MISC18:
	case MT_MISC19:
	case MT_MISC20:
	case MT_MISC21:
	case MT_MISC22:
	case MT_MISC23:
		return TRUE;
	default:
		return FALSE;
	}
}
UBOOL IsItem(const INT type)
{
	switch (type)
	{
	case MT_MISC0:
	case MT_MISC1:
	case MT_MISC2:
	case MT_MISC3:
	case MT_MISC4:
	case MT_MISC5:
	case MT_MISC6:
	case MT_MISC7:
	case MT_MISC8:
	case MT_MISC9:
	case MT_MISC10:
	case MT_MISC11:
	case MT_MISC12:
	case MT_INV:
	case MT_MISC13:
	case MT_INS:
	case MT_MISC14:
	case MT_MISC15:
	case MT_MISC16:
	case MT_MEGA:
	case MT_MISC24:
		return TRUE;
	default:
		return FALSE;
	}
}

switchlist_t alphSwitchList[] =
{
	// Doom shareware episode 1 switches
	{"SW1BRCOM",	"SW2BRCOM",	1},
	{"SW1BRN1",	"SW2BRN1",	1},
	{"SW1BRN2",	"SW2BRN2",	1},
	{"SW1BRNGN",	"SW2BRNGN",	1},
	{"SW1BROWN",	"SW2BROWN",	1},
	{"SW1COMM",	"SW2COMM",	1},
	{"SW1COMP",	"SW2COMP",	1},
	{"SW1DIRT",	"SW2DIRT",	1},
	{"SW1EXIT",	"SW2EXIT",	1},
	{"SW1GRAY",	"SW2GRAY",	1},
	{"SW1GRAY1",	"SW2GRAY1",	1},
	{"SW1METAL",	"SW2METAL",	1},
	{"SW1PIPE",	"SW2PIPE",	1},
	{"SW1SLAD",	"SW2SLAD",	1},
	{"SW1STARG",	"SW2STARG",	1},
	{"SW1STON1",	"SW2STON1",	1},
	{"SW1STON2",	"SW2STON2",	1},
	{"SW1STONE",	"SW2STONE",	1},
	{"SW1STRTN",	"SW2STRTN",	1},

	// Doom registered episodes 2&3 switches
	{"SW1BLUE",	"SW2BLUE",	2},
	{"SW1CMT",		"SW2CMT",	2},
	{"SW1GARG",	"SW2GARG",	2},
	{"SW1GSTON",	"SW2GSTON",	2},
	{"SW1HOT",		"SW2HOT",	2},
	{"SW1LION",	"SW2LION",	2},
	{"SW1SATYR",	"SW2SATYR",	2},
	{"SW1SKIN",	"SW2SKIN",	2},
	{"SW1VINE",	"SW2VINE",	2},
	{"SW1WOOD",	"SW2WOOD",	2},

	// Doom II switches
	{"SW1PANEL",	"SW2PANEL",	3},
	{"SW1ROCK",	"SW2ROCK",	3},
	{"SW1MET2",	"SW2MET2",	3},
	{"SW1WDMET",	"SW2WDMET",	3},
	{"SW1BRIK",	"SW2BRIK",	3},
	{"SW1MOD1",	"SW2MOD1",	3},
	{"SW1ZIM",		"SW2ZIM",	3},
	{"SW1STON6",	"SW2STON6",	3},
	{"SW1TEK",		"SW2TEK",	3},
	{"SW1MARB",	"SW2MARB",	3},
	{"SW1SKULL",	"SW2SKULL",	3},

	{"\0",		"\0",		0}
};

#define ST(tpn) case tpn: return TEXT(#tpn);
const TCHAR* EntNameToStr(const INT Index)
{
	switch (Index)
	{
		ST(MT_PLAYER);
		ST(MT_POSSESSED);
		ST(MT_SHOTGUY);
		ST(MT_VILE);
		ST(MT_FIRE);
		ST(MT_UNDEAD);
		ST(MT_TRACER);
		ST(MT_SMOKE);
		ST(MT_FATSO);
		ST(MT_FATSHOT);
		ST(MT_CHAINGUY);
		ST(MT_TROOP);
		ST(MT_SERGEANT);
		ST(MT_SHADOWS);
		ST(MT_HEAD);
		ST(MT_BRUISER);
		ST(MT_BRUISERSHOT);
		ST(MT_KNIGHT);
		ST(MT_SKULL);
		ST(MT_SPIDER);
		ST(MT_BABY);
		ST(MT_CYBORG);
		ST(MT_PAIN);
		ST(MT_WOLFSS);
		ST(MT_KEEN);
		ST(MT_BOSSBRAIN);
		ST(MT_BOSSSPIT);
		ST(MT_BOSSTARGET);
		ST(MT_SPAWNSHOT);
		ST(MT_SPAWNFIRE);
		ST(MT_BARREL);
		ST(MT_TROOPSHOT);
		ST(MT_HEADSHOT);
		ST(MT_ROCKET);
		ST(MT_PLASMA);
		ST(MT_BFG);
		ST(MT_ARACHPLAZ);
		ST(MT_PUFF);
		ST(MT_BLOOD);
		ST(MT_TFOG);
		ST(MT_IFOG);
		ST(MT_TELEPORTMAN);
		ST(MT_EXTRABFG);
		ST(MT_MISC0);
		ST(MT_MISC1);
		ST(MT_MISC2);
		ST(MT_MISC3);
		ST(MT_MISC4);
		ST(MT_MISC5);
		ST(MT_MISC6);
		ST(MT_MISC7);
		ST(MT_MISC8);
		ST(MT_MISC9);
		ST(MT_MISC10);
		ST(MT_MISC11);
		ST(MT_MISC12);
		ST(MT_INV);
		ST(MT_MISC13);
		ST(MT_INS);
		ST(MT_MISC14);
		ST(MT_MISC15);
		ST(MT_MISC16);
		ST(MT_MEGA);
		ST(MT_CLIP);
		ST(MT_MISC17);
		ST(MT_MISC18);
		ST(MT_MISC19);
		ST(MT_MISC20);
		ST(MT_MISC21);
		ST(MT_MISC22);
		ST(MT_MISC23);
		ST(MT_MISC24);
		ST(MT_MISC25);
		ST(MT_CHAINGUN);
		ST(MT_MISC26);
		ST(MT_MISC27);
		ST(MT_MISC28);
		ST(MT_SHOTGUN);
		ST(MT_SUPERSHOTGUN);
		ST(MT_MISC29);
		ST(MT_MISC30);
		ST(MT_MISC31);
		ST(MT_MISC32);
		ST(MT_MISC33);
		ST(MT_MISC34);
		ST(MT_MISC35);
		ST(MT_MISC36);
		ST(MT_MISC37);
		ST(MT_MISC38);
		ST(MT_MISC39);
		ST(MT_MISC40);
		ST(MT_MISC41);
		ST(MT_MISC42);
		ST(MT_MISC43);
		ST(MT_MISC44);
		ST(MT_MISC45);
		ST(MT_MISC46);
		ST(MT_MISC47);
		ST(MT_MISC48);
		ST(MT_MISC49);
		ST(MT_MISC50);
		ST(MT_MISC51);
		ST(MT_MISC52);
		ST(MT_MISC53);
		ST(MT_MISC54);
		ST(MT_MISC55);
		ST(MT_MISC56);
		ST(MT_MISC57);
		ST(MT_MISC58);
		ST(MT_MISC59);
		ST(MT_MISC60);
		ST(MT_MISC61);
		ST(MT_MISC62);
		ST(MT_MISC63);
		ST(MT_MISC64);
		ST(MT_MISC65);
		ST(MT_MISC66);
		ST(MT_MISC67);
		ST(MT_MISC68);
		ST(MT_MISC69);
		ST(MT_MISC70);
		ST(MT_MISC71);
		ST(MT_MISC72);
		ST(MT_MISC73);
		ST(MT_MISC74);
		ST(MT_MISC75);
		ST(MT_MISC76);
		ST(MT_MISC77);
		ST(MT_MISC78);
		ST(MT_MISC79);
		ST(MT_MISC80);
		ST(MT_MISC81);
		ST(MT_MISC82);
		ST(MT_MISC83);
		ST(MT_MISC84);
		ST(MT_MISC85);
		ST(MT_MISC86);
	default:
		return TEXT("MT_UNDEFINED");
	}
}
#undef ST

const TCHAR* EntTypeToStr(const INT InType)
{
#define CASE_EXP(t,n) case t: return TEXT(#n)
	switch (InType)
	{
		CASE_EXP(MT_PLAYER, PP_DoomGuy);
		CASE_EXP(MT_POSSESSED, M_ZombieMan);
		CASE_EXP(MT_SHOTGUY, M_Shotgunner);
		CASE_EXP(MT_VILE, M_ArchVile);
		CASE_EXP(MT_FIRE, P_ArchFlames);
		CASE_EXP(MT_UNDEAD, M_Revenant);
		CASE_EXP(MT_TRACER, P_RevenantMissile);
		CASE_EXP(MT_SMOKE, FX_Smoke);
		CASE_EXP(MT_FATSO, M_Mancubus);
		CASE_EXP(MT_FATSHOT, P_MancubProj);
		CASE_EXP(MT_CHAINGUY, M_Chaingunner);
		CASE_EXP(MT_TROOP, M_Imp);
		CASE_EXP(MT_SERGEANT, M_Demon);
		CASE_EXP(MT_SHADOWS, M_Spectre);
		CASE_EXP(MT_HEAD, M_Cacodemon);
		CASE_EXP(MT_BRUISER, M_Baron);
		CASE_EXP(MT_BRUISERSHOT, P_BaronFireball);
		CASE_EXP(MT_KNIGHT, M_HellKnight);
		CASE_EXP(MT_SKULL, M_LostSoul);
		CASE_EXP(MT_SPIDER, M_SpiderMM);
		CASE_EXP(MT_BABY, M_Spider);
		CASE_EXP(MT_CYBORG, M_Cyberdemon);
		CASE_EXP(MT_PAIN, M_PainElemental);
		CASE_EXP(MT_WOLFSS, M_WolfSS);
		CASE_EXP(MT_KEEN, M_Keen);
		CASE_EXP(MT_BOSSBRAIN, M_Romero);
		CASE_EXP(MT_SPAWNSHOT, P_RomeroProj);
		CASE_EXP(MT_SPAWNFIRE, FX_RomeroSpawn);
		CASE_EXP(MT_BARREL, D_Barrel);
		CASE_EXP(MT_TROOPSHOT, P_ImpFireball);
		CASE_EXP(MT_HEADSHOT, P_CacoFireball);
		CASE_EXP(MT_ROCKET, P_Rocket);
		CASE_EXP(MT_PLASMA, P_Plasma);
		CASE_EXP(MT_BFG, P_BFGProj);
		CASE_EXP(MT_ARACHPLAZ, P_SpiderProj);
		CASE_EXP(MT_PUFF, FX_MissileSmoke);
		CASE_EXP(MT_BLOOD, FX_Blood);
		CASE_EXP(MT_TFOG, FX_TeleportFX);
		CASE_EXP(MT_IFOG, FX_ItemRespawnFX);
		CASE_EXP(MT_EXTRABFG, FX_BFGTinyExplosion);
		CASE_EXP(MT_MISC0, I_GreenArmor);
		CASE_EXP(MT_MISC1, I_BlueArmor);
		CASE_EXP(MT_MISC2, I_HealthVial);
		CASE_EXP(MT_MISC3, I_ArmorBonus);
		CASE_EXP(MT_MISC4, I_BlueKey);
		CASE_EXP(MT_MISC5, I_RedKey);
		CASE_EXP(MT_MISC6, I_YellowKey);
		CASE_EXP(MT_MISC7, I_YellowSkull);
		CASE_EXP(MT_MISC8, I_RedSkull);
		CASE_EXP(MT_MISC9, I_BlueSkull);
		CASE_EXP(MT_MISC10, I_SmallHealth);
		CASE_EXP(MT_MISC11, I_MedHealth);
		CASE_EXP(MT_MISC12, I_HealthSphere);
		CASE_EXP(MT_INV, I_InvulnSphere);
		CASE_EXP(MT_MISC13, I_Berserk);
		CASE_EXP(MT_INS, I_InvisSphere);
		CASE_EXP(MT_MISC14, I_ToxinSuit);
		//CASE_EXP(MT_MISC15, MT_MISC15); <- fullmap, skipped.
		CASE_EXP(MT_MISC16, I_FullbrightGog);
		CASE_EXP(MT_MEGA, I_MaxSphere);
		CASE_EXP(MT_CLIP, A_Clip);
		CASE_EXP(MT_MISC17, A_AmmoBox);
		CASE_EXP(MT_MISC18, A_Rocket);
		CASE_EXP(MT_MISC19, A_RocketBox);
		CASE_EXP(MT_MISC20, A_PlasmaAmmo);
		CASE_EXP(MT_MISC21, A_PlasmaBox);
		CASE_EXP(MT_MISC22, A_ShotgunShells);
		CASE_EXP(MT_MISC23, A_ShotgunBox);
		CASE_EXP(MT_MISC24, I_Backpack);
		CASE_EXP(MT_MISC25, W_BFG);
		CASE_EXP(MT_CHAINGUN, W_Chaingun);
		CASE_EXP(MT_MISC26, W_Chainsaw);
		CASE_EXP(MT_MISC27, W_RocketLauncher);
		CASE_EXP(MT_MISC28, W_PlasmaGun);
		CASE_EXP(MT_SHOTGUN, W_Shotgun);
		CASE_EXP(MT_SUPERSHOTGUN, W_SuperShotgun);
		CASE_EXP(MT_MISC29, D_BlueLampPost);
		CASE_EXP(MT_MISC30, D_BlueLampPostB);
		CASE_EXP(MT_MISC31, D_YellowLampPost);
		CASE_EXP(MT_MISC32, D_DemonPillar);
		CASE_EXP(MT_MISC33, D_DemonPillarB);
		CASE_EXP(MT_MISC34, D_RedDemonPillar);
		CASE_EXP(MT_MISC35, D_RedDemonPillarB);
		CASE_EXP(MT_MISC36, D_RedDemonPillarSkull);
		CASE_EXP(MT_MISC37, D_DemonPillarC);
		CASE_EXP(MT_MISC38, D_DemonEye);
		CASE_EXP(MT_MISC39, D_DemonIsle);
		CASE_EXP(MT_MISC40, D_TreeScorched);
		CASE_EXP(MT_MISC41, D_BlueFlame);
		CASE_EXP(MT_MISC42, D_GreenFlame);
		CASE_EXP(MT_MISC43, D_RedFlame);
		CASE_EXP(MT_MISC44, D_BlueFlameStick);
		CASE_EXP(MT_MISC45, D_GreenFlameStick);
		CASE_EXP(MT_MISC46, D_RedFlameStick);
		CASE_EXP(MT_MISC47, D_TreeSpike);
		CASE_EXP(MT_MISC48, D_TechPillar);
		CASE_EXP(MT_MISC49, D_BlackCandle);
		CASE_EXP(MT_MISC50, D_CandleBar);
		CASE_EXP(MT_MISC51, D_HangingTorso);
		CASE_EXP(MT_MISC52, D_HangingTorsoB);
		CASE_EXP(MT_MISC53, D_HangingTorsoC);
		CASE_EXP(MT_MISC54, D_HangingTorsoD);
		CASE_EXP(MT_MISC55, D_HangingTorsoE);
		CASE_EXP(MT_MISC56, D_HangingTorsoF);
		CASE_EXP(MT_MISC57, D_HangingTorsoG);
		CASE_EXP(MT_MISC58, D_HangingTorsoH);
		CASE_EXP(MT_MISC59, D_HangingTorsoI);
		CASE_EXP(MT_MISC60, D_HangingTorsoJ);
		CASE_EXP(MT_MISC61, D_DeadCaco);
		CASE_EXP(MT_MISC62, D_DeadPlayer);
		CASE_EXP(MT_MISC63, D_DeadZombieman);
		CASE_EXP(MT_MISC64, D_DeadDemon);
		//CASE_EXP(MT_MISC65); <- dead lost soul???
		CASE_EXP(MT_MISC66, D_DeadImp);
		CASE_EXP(MT_MISC67, D_DeadShotgunner);
		CASE_EXP(MT_MISC68, D_DeadPlayerB);
		CASE_EXP(MT_MISC69, D_DeadPlayerC);
		CASE_EXP(MT_MISC70, D_SkullStick);
		CASE_EXP(MT_MISC71, D_BloodPile);
		CASE_EXP(MT_MISC72, D_BloodStick);
		CASE_EXP(MT_MISC73, D_SkullShrine);
		CASE_EXP(MT_MISC74, D_ImpaledBody);
		CASE_EXP(MT_MISC75, D_ImpaledBodyB);
		CASE_EXP(MT_MISC76, D_TreeDead);
		CASE_EXP(MT_MISC77, D_BarrelBurning);
		CASE_EXP(MT_MISC78, D_HangingTorsoK);
		CASE_EXP(MT_MISC79, D_HangingTorsoL);
		CASE_EXP(MT_MISC80, D_HangingTorsoM);
		CASE_EXP(MT_MISC81, D_HangingTorsoN);
		CASE_EXP(MT_MISC82, D_HangingTorsoO);
		CASE_EXP(MT_MISC83, D_HangingTorsoP);
		CASE_EXP(MT_MISC84, D_BloodPileB);
		CASE_EXP(MT_MISC85, D_BloodPileC);
		CASE_EXP(MT_MISC86, D_BloodPileD);
	default:
		return nullptr;
	}
#undef CASE_EXP
}

ESfxType GetSfxType(sfxenum_t t)
{
	switch (t)
	{
	case sfx_doropn:
	case sfx_dorcls:
	case sfx_bdcls:
	case sfx_bdopn:
	case sfx_swtchn:
	case sfx_swtchx:
	case sfx_stnmov:
	case sfx_pstart:
	case sfx_pstop:
		return SFX_LevelSfx;
	case sfx_noway:
		return SFX_GameSfx;
	default:
		return SFX_EntitySfx;
	}
}
