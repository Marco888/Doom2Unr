
#include "DoomWadPrivate.h"

void line_t::InitSpecial()
{
	guard(line_t::InitSpecial);
	if (!special)
		return;

	DWORD SpecialFlags = SPLT_None;
	EMoverFlags MoveType = MFT_None;
	EDoorMoveSpeed MoverSpeed = DMS_NormalDoor;
	ELockType LockType = LOCKTYPE_None;

	switch (special)
	{
		// TRIGGERS.
		// All from here to RETRIGGERS.
	case 2:
		// Open Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		MoveType = MFT_LowestCeiling;
		break;

	case 3:
		// Close Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestCeiling;
		break;

	case 4:
		// Raise Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover);
		MoveType = MFT_LowestCeiling;
		break;

	case 5:
		// Raise Floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestCeiling;
		break;

	case 6:
		// Fast Ceiling Crush & Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_CrushToFloor;
		break;

	case 8:
		// Build Stairs
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_Stairs;
		break;

	case 10:
		// PlatDownWaitUp
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestFloor;
		break;

	case 12:
		// Light Turn On - brightest near
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 13:
		// Light Turn On 255
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 16:
		// Close Door 30
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_CloseToFloor;
		break;

	case 17:
		// Start Light Strobing
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 19:
		// Lower Floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_NearestFloorDown;
		break;

	case 22:
		// Raise floor to nearest height and change texture
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_NearestFloorUp;
		break;

	case 25:
		// Ceiling Crush and Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_CrushToFloor;
		break;

	case 30:
		// Raise floor to shortest texture height
		//  on either side of lines.
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_NearestFloorDown;
		break;

	case 35:
		// Lights Very Dark
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 36:
		// Lower Floor (TURBO)
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_HighestFloor;
		break;

	case 37:
		// LowerAndChange
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestFloor;
		break;

	case 38:
		// Lower Floor To Lowest
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestFloor;
		break;

	case 39:
		// TELEPORT!
		SpecialFlags = (SPLT_PassTrigger | SPLT_Teleport | SPLT_TriggerOnce | SPLT_PlayerOnly);
		break;

	case 40:
		// RaiseCeilingLowerFloor
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestFloor;
		break;

	case 44:
		// Ceiling Crush
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_CrushToFloor;
		break;

	case 52:
		// EXIT!
		SpecialFlags = (SPLT_PassTrigger | SPLT_Exit | SPLT_PlayerOnly);
		break;

	case 53:
		// Perpetual Platform Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestFloor;
		break;

	case 54:
		// Platform Stop
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 56:
		// Raise Floor Crush
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_CrushToCeiling;
		break;

	case 57:
		// Ceiling Crush Stop
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 58:
		// Raise Floor 24
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 59:
		// Raise Floor 24 And Change
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 104:
		// Turn lights off in sector(tag)
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 108:
		// Blazing Door Raise (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestCeiling;
		break;

	case 109:
		// Blazing Door Open (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_LowestCeiling;
		break;

	case 100:
		// Build Stairs Turbo 16
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 110:
		// Blazing Door Close (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		MoveType = MFT_CloseToFloor;
		break;

	case 119:
		// Raise floor to nearest surr. floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_NearestFloorUp;
		break;

	case 121:
		// Blazing PlatDownWaitUpStay
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 124:
		// Secret EXIT
		SpecialFlags = (SPLT_PassTrigger | SPLT_Exit | SPLT_SecretExit | SPLT_PlayerOnly);
		break;

	case 125:
		// TELEPORT MonsterONLY
		SpecialFlags = (SPLT_PassTrigger | SPLT_Teleport | SPLT_TriggerOnce | SPLT_MonsterOnly);
		break;

	case 130:
		// Raise Floor Turbo
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 141:
		// Silent Ceiling Crush & Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

		// RETRIGGERS.  All from here till end.
	case 72:
		// Ceiling Crush
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 73:
		// Ceiling Crush and Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 74:
		// Ceiling Crush Stop
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 75:
		// Close Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 76:
		// Close Door 30
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 77:
		// Fast Ceiling Crush & Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 79:
		// Lights Very Dark
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 80:
		// Light Turn On - brightest near
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 81:
		// Light Turn On 255
		SpecialFlags = (SPLT_PassTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 82:
		// Lower Floor To Lowest
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 83:
		// Lower Floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 84:
		// LowerAndChange
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 86:
		// Open Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 87:
		// Perpetual Platform Raise
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 88:
		// PlatDownWaitUp
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 89:
		// Platform Stop
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 90:
		// Raise Door
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 91:
		// Raise Floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 92:
		// Raise Floor 24
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 93:
		// Raise Floor 24 And Change
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 94:
		// Raise Floor Crush
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 95:
		// Raise floor to nearest height
		// and change texture.
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 96:
		// Raise floor to shortest texture height
		// on either side of lines.
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 97:
		// TELEPORT!
		SpecialFlags = (SPLT_PassTrigger | SPLT_Teleport | SPLT_PlayerOnly);
		break;

	case 98:
		// Lower Floor (TURBO)
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 105:
		// Blazing Door Raise (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 106:
		// Blazing Door Open (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		MoveType = MFT_LowestCeiling;
		MoverSpeed = DMS_BlazingDoor;
		break;

	case 107:
		// Blazing Door Close (faster than TURBO!)
		SpecialFlags = (SPLT_PassTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 120:
		// Blazing PlatDownWaitUpStay.
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 126:
		// TELEPORT MonsterONLY.
		SpecialFlags = (SPLT_PassTrigger | SPLT_Teleport | SPLT_MonsterOnly);
		break;

	case 128:
		// Raise To Nearest Floor
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 129:
		// Raise Floor Turbo
		SpecialFlags = (SPLT_PassTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

		// SHOOT!
	case 24:
		// RAISE FLOOR
		SpecialFlags = (SPLT_ProjectileTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 46:
		// OPEN DOOR
		SpecialFlags = (SPLT_ProjectileTrigger | SPLT_CeilMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		MoveType = MFT_LowestCeiling;
		break;

	case 47:
		// RAISE FLOOR NEAR AND CHANGE
		SpecialFlags = (SPLT_ProjectileTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

		// MANUALS
	case 1:			// Vertical Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover);
		MoveType = MFT_LowestCeiling;
		break;

	case 26:		// Blue Door/Locked
	case 27:		// Yellow Door /Locked
	case 28:		// Red Door /Locked
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		if (special == 26)
			LockType = LOCKTYPE_Blue;
		else if (special == 27)
			LockType = LOCKTYPE_Red;
		else if (special == 28)
			LockType = LOCKTYPE_Yellow;
		MoveType = MFT_LowestCeiling;
		break;

	case 31:		// Manual door open
	case 32:		// Blue locked door open
	case 33:		// Red locked door open
	case 34:		// Yellow locked door open
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		if (special == 32)
			LockType = LOCKTYPE_Blue;
		else if (special == 33)
			LockType = LOCKTYPE_Red;
		else if (special == 34)
			LockType = LOCKTYPE_Yellow;
		MoveType = MFT_LowestCeiling;
		break;

	case 117:		// Blazing door raise
	case 118:		// Blazing door open
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		if (special == 118)
			SpecialFlags |= SPLT_TriggerOnce;
		MoverSpeed = DMS_BlazingDoor;
		MoveType = MFT_LowestCeiling;
		break;

		// SWITCHES
	case 7:
		// Build Stairs
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 9:
		// Change Donut
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 11:
		// Exit level
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_Exit | SPLT_PlayerOnly);
		break;

	case 14:
		// Raise Floor 32 and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 15:
		// Raise Floor 24 and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 18:
		// Raise Floor to next highest floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 20:
		// Raise Plat next highest floor and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 21:
		// PlatDownWaitUpStay
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 23:
		// Lower Floor to Lowest
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 29:
		// Raise Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 41:
		// Lower Ceiling to Floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 71:
		// Turbo Lower Floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 49:
		// Ceiling Crush And Raise
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 50:
		// Close Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 51:
		// Secret EXIT
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_Exit | SPLT_SecretExit | SPLT_PlayerOnly);
		break;

	case 55:
		// Raise Floor Crush
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 101:
		// Raise Floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 102:
		// Lower Floor to Surrounding floor height
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		MoveType = MFT_HighestFloor;
		MoverSpeed = DMS_NormalFloor;
		break;

	case 103:
		// Open Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly | SPLT_TriggerOnce);
		MoveType = MFT_LowestCeiling;
		break;

	case 111:
		// Blazing Door Raise (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 112:
		// Blazing Door Open (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 113:
		// Blazing Door Close (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 122:
		// Blazing PlatDownWaitUpStay
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 127:
		// Build Stairs Turbo 16
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 131:
		// Raise Floor Turbo
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 133:
		// BlzOpenDoor BLUE
	case 135:
		// BlzOpenDoor RED
	case 137:
		// BlzOpenDoor YELLOW
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 140:
		// Raise Floor 512
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

		// BUTTONS
	case 42:
		// Close Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 43:
		// Lower Ceiling to Floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 45:
		// Lower Floor to Surrounding floor height
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 60:
		// Lower Floor to Lowest
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 61:
		// Open Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 62:
		// PlatDownWaitUpStay
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_HighestFloor;
		MoverSpeed = DMS_NormalPlatform;
		break;

	case 63:
		// Raise Door
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 64:
		// Raise Floor to ceiling
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 66:
		// Raise Floor 24 and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 67:
		// Raise Floor 32 and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 65:
		// Raise Floor Crush
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 68:
		// Raise Plat to next highest floor and change texture
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 69:
		// Raise Floor to next highest floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 70:
		// Turbo Lower Floor
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 114:
		// Blazing Door Raise (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 115:
		// Blazing Door Open (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 116:
		// Blazing Door Close (faster than TURBO!)
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 123:
		// Blazing PlatDownWaitUpStay
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		MoveType = MFT_HighestFloor;
		MoverSpeed = DMS_BlazingPlatform;
		break;

	case 132:
		// Raise Floor Turbo
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_FloorMover | SPLT_PlayerOnly);
		break;

	case 99:
		// BlzOpenDoor BLUE
	case 134:
		// BlzOpenDoor RED
	case 136:
		// BlzOpenDoor YELLOW
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_CeilMover | SPLT_PlayerOnly);
		break;

	case 138:
		// Light Turn On
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	case 139:
		// Light Turn Off
		SpecialFlags = (SPLT_SwitchTrigger | SPLT_Light | SPLT_PlayerOnly);
		break;

	default:
		return;
	}
	TriggerData = new FTriggerData(SpecialFlags, MoveType, MoverSpeed, LockType);
	unguard;
}
