//#define __3D_IMAGE_OBJECT__					// by sonee
//----------------------------------------------------------------------
// MTopViewDraw.cpp
//----------------------------------------------------------------------
//
// ë³µìž¡í•œ Draw í•¨ìˆ˜ ë¶€ë¶„ë§Œ ë”°ë¡œ ë¹¼ë†“ëŠ”ë‹¤.
// ìž‘ì—…í•˜ê¸°ê°€ ì˜ ë¶ˆíŽ¸í•´ì„œ 
//----------------------------------------------------------------------
#pragma warning(disable:4786)
#include "Client_PCH.h"
// DX3D.h removed (SDL2) - Direct3D has been replaced with SDL2
#include <math.h>
#include <list>
#include <stdio.h>
#include <fstream>
#include "MZone.h"
#include "MCreature.h"
#include "MFakeCreature.h"
#include "MPlayer.h"
#include "MItem.h"
#include "MImageObject.h"
#include "MAnimationObject.h"
#include "MShadowAnimationObject.h"
#include "assert.h"

#include "MMovingEffect.h"
#include "MScreenEffect.h"
#include "MTopView.h"
#include "MItemTable.h"
#include "MCreatureTable.h"
#include "MWeather.h"
#include "CMessageArray.h"
#include "DXLib.h"
#include "SP.h"
#include "FL2.h"
//#include "2D.h"
#include "ClientConfig.h"
#include "VS_UI_BASE.h"
#include "VS_UI_Mouse_Pointer.h"
#include "UIFunction.h"
#include "AddonDef.h"
#include "MZoneTable.h"
#include "MGameTime.h"
#include "UserOption.h"
#include "UserInformation.h"
#include "SpriteIDDef.h"
#include "MGameStringTable.h"
//#include "MZoneInfo.h"
#include "MObjectSelector.h"
#include "MPortal.h"
#include "EffectSpriteTypeDef.h"
#include "MScreenEffectManager.h"
#include "MEffectSpriteTypeTable.h"
#include "Profiler.h"
//#include "MFileDef.h"
#include "Properties.h"
#include "MEventManager.h"
#include "DebugInfo.h"
#include "MItemOptionTable.h"
#include "Client.h"
#include "SkillDef.h"
#include "vs_ui_item.h"
#include "MTopViewDraw.inl"

#define	SHIFT_SNIPPING			2

#ifdef __METROTECH_TEST__
	extern bool g_bLight;
#endif

#ifdef OUTPUT_DEBUG
	int g_WipeValue = 0;
	int g_WipeStep = 0;
#endif


static int g_nRocketPosValue[4] = {0,2,4,2};
extern int defaultSkinColor;
extern int defaultCoatColor;
extern int defaultTrouserColor;

namespace {
	POINT pointTemp;
	RECT rect;
};

int AdvancementOustersActionConvTable[ ACTION_MAX_SLAYER ] =
{
	ACTION_STAND,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”
	ACTION_MOVE,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”
	ACTION_ADVANCEMENT_OUSTERS_ATTACK_NORMAL,
	ACTION_ADVANCEMENT_OUSTERS_MAGIC,
	ACTION_ADVANCEMENT_OUSTERS_DAMAGED,
	ACTION_ADVANCEMENT_OUSTERS_DRAINED,
	ACTION_ADVANCEMENT_OUSTERS_DIE,
	ACTION_STAND,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”	(ì„œìžˆê¸°)
	ACTION_MOVE,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”	(ê±·ê¸°)
	ACTION_ADVANCEMENT_OUSTERS_SKILL_NORMAL,
	ACTION_ADVANCEMENT_OUSTERS_MAGIC_ATTACK,
	ACTION_ADVANCEMENT_OUSTERS_ABSORB_SOUL,
	ACTION_ADVANCEMENT_OUSTERS_FAST_MOVE_STOP,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”	(ê³µì¤‘ì •ì§€)
	ACTION_ADVANCEMENT_OUSTERS_FAST_MOVE,		// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”  (ê³µì¤‘ì´ë™)
	ACTION_ADVANCEMENT_OUSTERS_ATTACK_SLOW,
	ACTION_ADVANCEMENT_OUSTERS_ATTACK_FAST,
	ACTION_ADVANCEMENT_OUSTERS_SKILL_SLOW,
	ACTION_ADVANCEMENT_OUSTERS_SKILL_FAST,
	-1,
};
// by viva
int AdvancementSlayerActionConvTable[ ACTION_MAX_SLAYER ] = 
{
	ACTION_STAND,	// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”
	ACTION_MOVE,	// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”
	-1,	// ë”°ë¡œ ì¡°ê±´ ë³€í™˜ í•„ìš”
	ACTION_ADVANCEMENT_SLAYER_MAGIC,
	ACTION_DAMAGED,
	ACTION_ADVANCEMENT_SLAYER_DRAINED,
	ACTION_ADVANCEMENT_SLAYER_DIE,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_NORMAL,	// SR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_NORMAL,	// AR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SWORD_NORMAL,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_BLADE_NORMAL,
	-1,												// í­íƒ„ ë˜ì§€ê¸°
	ACTION_ADVANCEMENT_SLAYER_BIKE_MOVE,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_NORMAL,	// SG
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_NORMAL,	// SMG
	ACTION_ADVANCEMENT_SLAYER_SKILL_SWORD_NORMAL,
	ACTION_ADVANCEMENT_SLAYER_SKILL_BLADE_NORMAL,
	-1,												// ì„±ìˆ˜ ë˜ì§€ê¸°
	ACTION_ADVANCEMENT_SLAYER_BIKE_STOP,			// 18
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_SLOW,		//SR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_FAST,		//SR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_SLOW,		//AR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_FAST,		//AR
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_SLOW,		//SG
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_FAST,		//SG
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_SLOW,		//SMG
	ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_FAST,		//SMG
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SWORD_SLOW,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_SWORD_FAST,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_BLADE_SLOW,
	ACTION_ADVANCEMENT_SLAYER_ATTACK_BLADE_FAST,
	ACTION_ADVANCEMENT_SLAYER_SKILL_SWORD_SLOW,
	ACTION_ADVANCEMENT_SLAYER_SKILL_SWORD_FAST,
	ACTION_ADVANCEMENT_SLAYER_SKILL_BLADE_SLOW,
	ACTION_ADVANCEMENT_SLAYER_SKILL_BLADE_FAST,
};
	
int AdvancementVampireActionConvTable[ ACTION_MAX_SLAYER ] =
{
	ACTION_ADVANCEMENT_STOP,
	ACTION_ADVANCEMENT_MOVE,
	ACTION_ADVANCEMENT_ATTACK_NORMAL,
	ACTION_ADVANCEMENT_MAGIC,
	ACTION_ADVANCEMENT_DAMAGED,
	ACTION_ADVANCEMENT_DRAINED,
	ACTION_ADVANCEMENT_DIE,
	ACTION_ADVANCEMENT_DRAIN,
	ACTION_ADVANCEMENT_ATTACK_SLOW,				
	ACTION_ADVANCEMENT_ATTACK_FAST,
	ACTION_ADVANCEMENT_SKILL_SLOW,		// 10
	ACTION_ADVANCEMENT_SKILL_NORMAL,	// 11
	ACTION_ADVANCEMENT_SKILL_FAST,		// 12
	ACTION_ADVANCEMENT_MAGIC_ATTACK,
	ACTION_ADVANCEMENT_SPECIAL,
	ACTION_ADVANCEMENT_CREATE_WEAPON,
	ACTION_ADVANCEMENT_DESTROY_WEAPON,	// 16
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1, // 30
	-1,
	-1,
	-1,
	-1,
};

BYTE GetCreatureActionCountMax( const MCreature* pCreature, int action );

int GetAdvancementVampireActionFromVampireAction( int CurAction, const MCreature* pCreature )
{
	if( CurAction >= ADVANCEMENT_ACTION_START )
		return CurAction;

	return AdvancementVampireActionConvTable[ CurAction ];
}
//add by viva
int GetNewVampireActionFromVampireAction( int CurAction )
{
	int Action=CurAction;
	if(CurAction==13)
		Action=16;
	else if(CurAction==12)
		Action=7;
	return Action;
}
int GetAdvancementPartFromItemClass( ITEM_CLASS itemClass , TYPE_FRAMEID frameID)
{
//	assert(false && "ë‹˜í•˜ ì¦ë“œ333");

	switch( itemClass )
	{
	case ITEM_CLASS_MOTORCYCLE :
		{
			if(frameID == AC_BIKE_2)
				return AC_BIKE_2;
			return AC_BIKE_1;
		}
		
	case ITEM_CLASS_SWORD :
		return AC_SWORD;

	case ITEM_CLASS_BLADE :
		return AC_BLADE;

	case ITEM_CLASS_AR :
		return AC_GUN_AR;

	case ITEM_CLASS_SG :
	case ITEM_CLASS_SMG :
	case ITEM_CLASS_SR :
		return AC_GUN_SR;

	case ITEM_CLASS_MACE :
		return AC_MACE;

	case ITEM_CLASS_CROSS :
		return AC_CROSS;

	case ITEM_CLASS_SHIELD :
		return AC_SHIELD;

	case ITEM_CLASS_HELM :
		return AC_HELMET;

	case ITEM_CLASS_COAT :
	case ITEM_CLASS_TROUSER :
		return AC_BODY;

	case ITEM_CLASS_SHOULDER_ARMOR :
		return AC_SHOULDER;
	}

	return -1;
}

int	GetAdvancementSlayerStandActionFromItemClass( ITEM_CLASS itemclass )
{
	switch( itemclass )
	{
	case ITEM_CLASS_SWORD :
		return ACTION_ADVANCEMENT_SLAYER_STOP_SWORD;

	case ITEM_CLASS_BLADE :
		return ACTION_ADVANCEMENT_SLAYER_STOP_BLADE;

	case ITEM_CLASS_SG :
	case ITEM_CLASS_SMG :
	case ITEM_CLASS_AR :
	case ITEM_CLASS_SR :
		return ACTION_ADVANCEMENT_SLAYER_STOP_GUN;		
	}

	return ACTION_ADVANCEMENT_SLAYER_STOP_MACE_AND_CROSS;
}

int GetAdvancementSlayerMoveActionFromItemClass( ITEM_CLASS itemclass )
{
	switch( itemclass )
	{
	case ITEM_CLASS_SWORD :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_SWORD;

	case ITEM_CLASS_BLADE :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_BLADE;

	case ITEM_CLASS_SG :
	case ITEM_CLASS_SMG :
	case ITEM_CLASS_AR :
	case ITEM_CLASS_SR :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_GUN;
	}

	return ACTION_ADVANCEMENT_SLAYER_MOVE_MACE_AND_CROSS;
}

int GetAdvancementSlayerDamagedActionFromItemClass( ITEM_CLASS itemclass )
{
	switch( itemclass )
	{
	case ITEM_CLASS_SWORD :
		return ACTION_ADVANCEMENT_SLAYER_DAMAGED_SWORD;

	case ITEM_CLASS_BLADE :
		return ACTION_ADVANCEMENT_SLAYER_DAMAGED_BLADE;

	case ITEM_CLASS_SG :
	case ITEM_CLASS_SMG :
	case ITEM_CLASS_AR :
	case ITEM_CLASS_SR :
		return ACTION_ADVANCEMENT_SLAYER_DAMAGED_GUN;
	}

	return ACTION_ADVANCEMENT_SLAYER_DAMAGED_CROSS_MACE;
}

int GetAdvancementSlayerAttackActionFromItemClass( ITEM_CLASS itemClass, MCreature* pCreature )
{
	switch( itemClass )
	{
	case ITEM_CLASS_AR :
		return ACTION_ADVANCEMENT_SLAYER_ATTACK_AR_GUN_SLOW + pCreature->GetWeaponSpeed();
	
	case ITEM_CLASS_SG :
	case ITEM_CLASS_SMG :
	case ITEM_CLASS_SR :
		return ACTION_ADVANCEMENT_SLAYER_ATTACK_SR_GUN_SLOW + pCreature->GetWeaponSpeed();
	}

	return -1;
}
//add by viva
int ConvNewSlayerActionFromSlayerAction(int CurAction, MCreatureWear* pCreatureWear)
{
	assert(pCreatureWear!=NULL);
	int Action=CurAction;
	if(CurAction==ACTION_STAND)
	{
		const MCreatureWear::ADDON_INFO& addon = pCreatureWear->GetAddonInfo( ADDON_RIGHTHAND );
		if(	addon.ItemClass==ITEM_CLASS_SG
		||addon.ItemClass==ITEM_CLASS_SMG
		||addon.ItemClass==ITEM_CLASS_AR
		||addon.ItemClass==ITEM_CLASS_SR
		)
			return ACTION_NEW_SLAYER_STOP_GUN;
	}
	else if(CurAction==ACTION_MOVE)
	{
		const MCreatureWear::ADDON_INFO& addon = pCreatureWear->GetAddonInfo( ADDON_RIGHTHAND );
		if(	addon.ItemClass==ITEM_CLASS_SG
		||addon.ItemClass==ITEM_CLASS_SMG
		||addon.ItemClass==ITEM_CLASS_AR
		||addon.ItemClass==ITEM_CLASS_SR
		)
			return ACTION_NEW_SLAYER_MOVE_GUN;	
	}
	else if(CurAction==ACTION_SLAYER_GUN_SG)	//9
		Action=ACTION_NEW_SLAYER_GUN_SG;
	else if(CurAction==ACTION_SLAYER_GUN_SMG)	//10
		Action=ACTION_NEW_SLAYER_GUN_SMG;
	else if(CurAction==ACTION_SLAYER_SWORD)	//11
		Action=ACTION_NEW_SLAYER_SWORD;
	else if(CurAction==ACTION_SLAYER_BLADE)	//12
		Action=ACTION_NEW_SLAYER_BLADE;
	else if(CurAction==ACTION_SLAYER_SWORD_2)	//13
		Action=ACTION_NEW_SLAYER_SWORD_2;
	else if(CurAction==ACTION_SLAYER_BLADE_2)	//14
		Action=ACTION_NEW_SLAYER_BLADE_2;
	else if(CurAction==ACTION_SLAYER_MOTOR_MOVE)	//15
		Action=ACTION_NEW_SLAYER_MOTOR_MOVE;
	else if(CurAction==ACTION_SLAYER_MOTOR_STAND)	//16
		Action=ACTION_NEW_SLAYER_MOTOR_STAND;
	else if(CurAction==ACTION_SLAYER_GUN_SR_SLOW)	//17
		Action=ACTION_NEW_SLAYER_GUN_SR_SLOW;
	else if(CurAction==ACTION_SLAYER_GUN_SR_FAST)	//18
		Action=ACTION_NEW_SLAYER_GUN_SR_FAST;
	else if(CurAction==ACTION_SLAYER_GUN_AR_SLOW)	//19
		Action=ACTION_NEW_SLAYER_GUN_AR_SLOW;
	else if(CurAction==ACTION_SLAYER_GUN_AR_FAST)	//20
		Action=ACTION_NEW_SLAYER_GUN_AR_FAST;
	else if(CurAction==ACTION_SLAYER_GUN_SG_SLOW)	//21
		Action=ACTION_NEW_SLAYER_GUN_SG_SLOW;
	else if(CurAction==ACTION_SLAYER_GUN_SG_FAST)	//22
		Action=ACTION_NEW_SLAYER_GUN_SG_FAST;
	else if(CurAction==ACTION_SLAYER_GUN_SMG_SLOW)	//23
		Action=ACTION_NEW_SLAYER_GUN_SMG_SLOW;
	else if(CurAction==ACTION_SLAYER_GUN_SMG_FAST)	//24
		Action=ACTION_NEW_SLAYER_GUN_SMG_FAST;
	else if(CurAction==ACTION_SLAYER_SWORD_SLOW)	//25
		Action=ACTION_NEW_SLAYER_SWORD_SLOW;
	else if(CurAction==ACTION_SLAYER_SWORD_FAST)	//26
		Action=ACTION_NEW_SLAYER_SWORD_FAST;
	else if(CurAction==ACTION_SLAYER_BLADE_SLOW)	//27
		Action=ACTION_NEW_SLAYER_BLADE_SLOW;
	else if(CurAction==ACTION_SLAYER_BLADE_FAST)	//28
		Action=ACTION_NEW_SLAYER_BLADE_FAST;
	else if(CurAction==ACTION_SLAYER_SWORD_2_SLOW)	//29
		Action=ACTION_NEW_SLAYER_SWORD_2_SLOW;
	else if(CurAction==ACTION_SLAYER_SWORD_2_FAST)	//30
		Action=ACTION_NEW_SLAYER_SWORD_2_FAST;
	else if(CurAction==ACTION_SLAYER_BLADE_2_SLOW)	//31
		Action=ACTION_NEW_SLAYER_BLADE_2_SLOW;
	else if(CurAction==ACTION_SLAYER_BLADE_2_FAST)	//32
		Action=ACTION_NEW_SLAYER_BLADE_2_FAST;
	else if(CurAction==130)//í…éƒ½åŠ ä¸¹
		Action=37;
	return Action;
}
int ConvAdvancementSlayerActionFromSlayerAction( int CurAction, MCreatureWear* pCreatureWear )
{
	assert( pCreatureWear != NULL );

	if( ADVANCEMENT_ACTION_START <= CurAction )
		return CurAction;

	const MCreatureWear::ADDON_INFO& addon = pCreatureWear->GetAddonInfo( ADDON_RIGHTHAND );

	switch( CurAction )
	{
	case ACTION_DAMAGED :
		return GetAdvancementSlayerDamagedActionFromItemClass( addon.ItemClass );

	case ACTION_STAND :
		return GetAdvancementSlayerStandActionFromItemClass( addon.ItemClass );		
		
	case ACTION_MOVE :
		return GetAdvancementSlayerMoveActionFromItemClass( addon.ItemClass );
/*
	case ACTION_SLAYER_GUN_SR :
	case ACTION_SLAYER_GUN_AR :
	case ACTION_SLAYER_GUN_SG :
	case ACTION_SLAYER_GUN_SMG :
		case ACTION_SLAYER_GUN_SR_SLOW :
		return GetAdvancementSlayerAttackActionFromItemClass( addon.ItemClass, pCreatureWear );*/
	}	

	return AdvancementSlayerActionConvTable[ CurAction ];
};

int ConvAdvancementOustersActionFromOusterAction( int CurAction, bool bChakram )
{
	if( ADVANCEMENT_ACTION_START <= CurAction )
		return CurAction;

	switch( CurAction )
	{
	case ACTION_STAND :
		if( bChakram )
			return ACTION_ADVANCEMENT_OUSTERS_CHAKRAM_STOP;
		else
			return ACTION_ADVANCEMENT_OUSTERS_WRISTLET_STOP;
		
	case ACTION_MOVE :
		if( bChakram )
			return ACTION_ADVANCEMENT_OUSTERS_CHAKRAM_MOVE;
		else
			return ACTION_ADVANCEMENT_OUSTERS_WRISTLET_MOVE;
	}
	
	if( CurAction > ACTION_OUSTERS_CHAKRAM_FAST )
		return -1;

	return AdvancementOustersActionConvTable[ CurAction ];
}

bool IsEscapeDrawCreatureFunction( MCreature* pCreature )
{
	bool InGroundElemental = pCreature->IsOusters() && pCreature->IsPlayer() && g_pPlayer->IsOusters() == false
							&& pCreature->IsInGroundElemental();

	bool bGhost = pCreature->HasEffectStatus(EFFECTSTATUS_GHOST) || g_pEventManager->GetEventByFlag(EVENTFLAG_NOT_DRAW_CREATURE)
		|| pCreature->IsGhost(1) && pCreature->IsGhost(2) && pCreature->IsGhost(4);

		
	// 2004, 6, 7 sobeit add start - íŽ«ì¼ ê²½ìš° ì£¼ì¸ì¸ ë‹¤í¬ë‹ˆìŠ¤ ì•ˆì— ìžˆìœ¼ë©´ ì•ˆë³´ì´ê²Œ..
	bool bPet1 = false;
	if(pCreature->IsFakeCreature())
	{
		MCreature *OwnerCreature = g_pZone->GetCreature(((MFakeCreature*)pCreature)->GetOwnerID());
		if(NULL != OwnerCreature)
		{
			if(OwnerCreature->IsInDarkness())
				bPet1 = true;
		}
	}

	bool bBlind = false;
	if(
		(g_pPlayer->HasEffectStatus(EFFECTSTATUS_YELLOW_POISON_TO_CREATURE) || 
		g_pPlayer->HasEffectStatus(EFFECTSTATUS_FLARE )||
		g_pPlayer->HasEffectStatus(EFFECTSTATUS_BLINDNESS))
		&& pCreature != g_pPlayer 
		)
	{
		// ê±°ë¦¬ì— ë”°ë¼ì„œ ì¶œë ¥ ì—¬ë¶€ë¥¼ ê²°ì •í•œë‹¤.
		int sx,sy,ex,ey;

		sx = g_pPlayer->GetX() - 1;
		ex = g_pPlayer->GetX() + 1;

		sy = g_pPlayer->GetY() - 1;
		ey = g_pPlayer->GetY() + 1;

		if( pCreature->GetX() < sx || pCreature->GetX() > ex ||
			pCreature->GetY() < sy || pCreature->GetY() > ey )
			bBlind = true;
	}


	return InGroundElemental || bGhost || bPet1 || bBlind;
}

//----------------------------------------------------------------------
// Draw Creature : character ì¶œë ¥í•˜ê¸°	
//----------------------------------------------------------------------
// pSurfaceì˜ pPointì— pCreatureë¥¼ ì¶œë ¥í•œë‹¤.
//----------------------------------------------------------------
void	
MTopView::DrawCreature(POINT* pPoint, MCreature* pCreature)
{
    m_SOMOutlineColor = 0x0000; // Initialize to black/transparent	
	// EFFECTSTATUS_GHOST ì— ê±¸ë ¤ìžˆìœ¼ë©´ ë‚¨ë„ ëª»ë³´ê³  ë‚˜ë„ ëª»ë³¸ë‹¤
	//DEBUG_ADD_FORMAT("[DrawCreature] Start %s %s %s", pCreature->IsGhost(1)?"true":"false", pCreature->IsGhost(2)?"true":"false", pCreature->IsGhost(4)?"true":"false");
	if( IsEscapeDrawCreatureFunction( pCreature ) )
		return;

	// 2004, 6, 7 sobeit add end
	__BEGIN_PROFILE("DrawCreature");

	int body, action, direction, frame, creature_type, frameCount;

	frameCount  = pCreature->GetCreatureFrameCount();

	for(int FrameIndex = 0; FrameIndex< frameCount ; FrameIndex++)
	{
		body		= pCreature->GetCreatureFrameID(FrameIndex);
		action		= pCreature->GetAction();
		direction	= pCreature->GetDirection();
		frame		= pCreature->GetFrame();//%m_CreatureFPK[body][action][direction].GetCount();
		creature_type = pCreature->GetCreatureType();

	//	if(action>ACTION_OUSTERS_MAGIC_ATTACK)
	//		int i = 0;
		//----------------------------------------------------------
		// Charmì— ê±¸ë ¤ ìžˆëŠ” ìºë¦­í„°..
		// í¡í˜ˆë‹¹í•˜ëŠ” ë™ìž‘ìœ¼ë¡œ í‘œí˜„..
		//----------------------------------------------------------
		if (pCreature->HasEffectStatus(EFFECTSTATUS_CHARM))
		{
			action = ACTION_DRAINED;
			frame = 6 + (frame & 0x07);		// í¡í˜ˆ ë™ìž‘ì—ì„œ ë°˜ë³µ frameì˜ ì‹œìž‘ì´ 6ì´ë‹¤.		
		}
		if(pCreature->HasEffectStatus(EFFECTSTATUS_BIKE_CRASH))
			frame = 0;
		
		bool isSlayerCharacter	= pCreature->IsSlayerCharacter();
		//bool isOustersCharacter = pCreature->IsOusters();

		bool isOustersCharacter = (pCreature->IsOusters() && pCreature->GetCreatureType() != CREATURETYPE_OUSTERS_OPERATOR
								&&	creature_type != 717 && creature_type != 721 && creature_type != 723);
							
		
		// í• ë£¨ê°€ ê±¸ë ¸ìœ¼ë©´ ì•¡ì…˜ í”„ë ˆìž„ë“±ì„ ë°”ê¾¸ëŠ”ë°... ë‹¨ ëŠ‘ëŒ€ ìƒíƒœë‚˜ ë°•ì¥ ìƒíƒœì—ì„œëŠ” 
		// ë°”ê¾¸ì§€ ì•Šë„ë¡ í•œë‹¤. ì¼ë‹¨ ê¸°íšíŒ€ì— ì˜í•´ì„œ ì£¼ì„ì²˜ë¦¬í•´ë†“ì•˜ë‹¤.
		if (pCreature->IsHallu()
	//		&& !pCreature->HasEffectStatus( EFFECTSTATUS_TRANSFORM_TO_BAT )
	//		&& !pCreature->HasEffectStatus( EFFECTSTATUS_TRANSFORM_TO_WOLF )
			&& !g_pPlayer->HasEffectStatus( EFFECTSTATUS_GHOST )
	#ifdef __METROTECH_TEST__
			&& !g_bLight
	#endif
			)
		{
			isSlayerCharacter = false;
			isOustersCharacter = false;

			body	= pCreature->GetHalluCreatureFrameID();
			action	= pCreature->GetHalluAction();
			frame	= pCreature->GetHalluFrame();
			creature_type = pCreature->GetHalluCreatureType();
		}

		bool bGildreDead = false;
		bool bSlayerPet_ShowTurret = false;
		GENERATE_OPTION  OutLineOption = GENERATE_ALL;
		// 2004, 7, 23 sobeit modify start

		// 2004.12.9 created exception processing function by sonee
		std::map< DWORD, DrawCreatureExceptionProc >::iterator itr = m_ExceptionProcMap.find( creature_type );

		if( m_ExceptionProcMap.end() != itr )
			itr->second( pCreature, action, frame, direction );

		switch(creature_type)
		{		
		case 717:
		case 721:
			if(pCreature->IsDead())
			{
				if(pCreature->GetActionCount()>=pCreature->GetActionCountMax())
					bGildreDead = true;
			}
			break;
		case 702:
		case 703:
		case 704:
			if( action == ACTION_STAND || action == ACTION_MOVE )
				bSlayerPet_ShowTurret = true;
			break;
		case 726: // ì„±ë¬¸ 4ì¢…
		case 727:
		case 728:
		case 729:
			{
				MCreature* TempCreature = g_pZone->GetCreature(m_SelectCreatureID);
				if(TempCreature != NULL)
				{
					if(	TempCreature->GetCreatureType() == 726 ||
						TempCreature->GetCreatureType() == 727 ||
						TempCreature->GetCreatureType() == 728 ||
						TempCreature->GetCreatureType() == 729 )
					{
						if(creature_type == 726)
							OutLineOption = GENERATE_EXCEPT_RIGHT;
						else if(creature_type == 727 || creature_type == 728 )
							OutLineOption = GENERATE_EXCEPT_SIDE;
						else if(creature_type == 729)
							OutLineOption = GENERATE_EXCEPT_LEFT;
					}
				}
			}
			break;
		case 753 :
			if(pCreature->IsDead())
			{
				action = ACTION_DIE;
				if(pCreature->GetActionCount()>=pCreature->GetActionCountMax()) return;
			}
			break;

		case 767:// ë¡œì¼“ ëŸ°ì³
			{
				if(direction == 0 || direction == 4)
				{
					pPoint->y -= g_nRocketPosValue[frame];
//					pPoint->y -= sin((double)pCreature->GetMoveCount()*3.141592/(double)pCreature->GetMoveCountMax())*5;
				}
				else 
				{
					pPoint->x -= g_nRocketPosValue[frame];
//					pPoint->x -= sin((double)pCreature->GetMoveCount()*3.141592/(double)pCreature->GetMoveCountMax())*5;
				}
			}
			frame = 0;
			action = ACTION_MOVE;
		
			break;
		case 769:
			if(pCreature->IsDead())
				return;
			break;

		}
		// 2004, 7, 23 sobeit modify end
		
		// 2004, 11, 22, sobeit add start - set afire ê´€ë ¨
		if(pCreature->GetID() != g_pPlayer->GetID())
		{
			if(pCreature->GetSpecialActionInfo() == SKILL_SET_AFIRE)
			{
				if(action == ACTION_ATTACK || action == ACTION_VAMPIRE_ATTACK_SLOW || action == ACTION_VAMPIRE_ATTACK_FAST)
				{
					action = ACTION_ATTACK;
					frame = 3+((frame)%4);
				}
			}
		}
		else
		{
			if(pCreature->GetUsedActionInfo() == SKILL_SET_AFIRE)
			{
				if(action == ACTION_ATTACK || action == ACTION_VAMPIRE_ATTACK_SLOW || action == ACTION_VAMPIRE_ATTACK_FAST)
				{
					action = ACTION_ATTACK;
					frame = 3+((frame)%4);
				}
			}
		}
		// 2004, 11, 22, sobeit add end - set afire ê´€ë ¨
		// ì„±ë¬¼ì€ ë¬´ì¡°ê±´ ì•„ëž˜, í¬ë¦¬ìŠ¤ë§ˆìŠ¤íŠ¸ë¦¬


		if(pCreature->IsFakeDie())
			frame = 0;
	
		int	creatureID			= pCreature->GetID();
		bool bPlayerVampire = 
				(*g_pCreatureSpriteTable)[(*g_pCreatureTable)[creature_type].SpriteTypes[0]].IsPlayerVampireOnlySprite()
	//			&& (pCreature==g_pPlayer || pCreature->HasEffectStatus(EFFECTSTATUS_COMA))
				&& (pCreature->GetActionCount()>=pCreature->GetActionCountMax()	)
				&& !pCreature->IsExistAttachEffect()
				&& pCreature->IsDead();

		bool bTeenVersion = 
				(
					g_pUserInformation->GoreLevel == false
					&& !(
					creature_type >= 526 && creature_type <= 549 || 
					creature_type >= 371 && creature_type <= 376 || 
					creature_type >= 560 && creature_type <= 563 || 
					creature_type == 482 || 
					creature_type == 650 || 
					creature_type == 670 || 
					creature_type == 672 ||
					creature_type == 673 ||
					creature_type == 730 || // ì›¨ì´ í¬ì¸íŠ¸1
					creature_type == 731 || // ì›¨ì´ í¬ì¸íŠ¸2
					creature_type == 732 || // ì›¨ì´ í¬ì¸íŠ¸3
					creature_type == 636 // ìƒì„  ê°€ê²Œ ì•„ì €ì”¨

					)	// ì„±ë¬¼ // í¬ë¦¬ìŠ¤ ë§ˆìŠ¤ íŠ¸ë¦¬
					|| creature_type >= 377 && creature_type <= 386	// ë‹¤í¬ ê°€ë””ì–¸ì€ ë¨¼ì§€ë¡œ í‘œì‹œ
					|| creature_type == 480 
				)
				&&
				(
					pCreature->GetActionCount()>=pCreature->GetActionCountMax()/2
					&& pCreature->IsDead()
				)
				&&
				(
					creature_type < 431		// ë°”í† ë¦¬
					|| creature_type > 436	// í…ŒíŽ˜ì¦ˆ
				);

		//---------------------------------------------------------
		// PC Vampireì¸ ê²½ìš° ì£½ìœ¼ë©´ 'ë¨¼ì§€'ë¡œ í‘œí˜„í•œë‹¤.
		//
		// playerëŠ” ì£½ì–´ë„ ì•„ì´í…œìœ¼ë¡œ ë³€í•˜ì§€ ì•Šê¸° ë•Œë¬¸ì—..
		// í‹´ë²„ì „ì—ì„  ëª¨ë“  ì‹œì²´ë¥¼ 'ë¨¼ì§€'ë¡œ í‘œí˜„í•œë‹¤.
		//---------------------------------------------------------	
		if (bPlayerVampire || bTeenVersion || bGildreDead )
		{
			{
				CSprite* pSprite;
				
				if(pCreature->IsSlayer())
				{
					pSprite = &m_EtcSPK[ SPRITEID_SLAYER_CROSS ];
					// ì¢Œí‘œ ë³´ì •
					pointTemp.x = pPoint->x - 5+20;
					pointTemp.y = pPoint->y - 30;				
				}
				//else if((*g_pCreatureSpriteTable)[(*g_pCreatureTable)[creature_type].SpriteType].IsPlayerVampireOnlySprite())
				else if((*g_pCreatureSpriteTable)[(*g_pCreatureTable)[creature_type].SpriteTypes[0]].IsPlayerVampireOnlySprite())
				{
					pSprite = &m_EtcSPK[ SPRITEID_VAMPIRE_DUST ];
					// ì¢Œí‘œ ë³´ì •
					pointTemp.x = pPoint->x - 5;
					pointTemp.y = pPoint->y;				
				}
				else
				{
					pSprite = &m_EtcSPK[ SPRITEID_MONSTER_DUST ];
					// ì¢Œí‘œ ë³´ì •
					pointTemp.x = pPoint->x - 5;
					pointTemp.y = pPoint->y;				
				}


				// ì„ íƒëœ ê²ƒì¸ ê²½ìš°
				if (m_SelectItemID == creatureID
					|| m_SelectCreatureID == creatureID)
				{
					m_SOMOutlineColor = m_ColorOutlineItem;
					
					m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
					
					m_SOM.Generate();

					m_pSurface->BltSpriteOutline( &m_SOM, m_SOMOutlineColor );
					
					//---------------------------------------- 	
					// ì´ë¦„ ì¶œë ¥í•  ì¢Œí‘œ ì§€ì •
					//---------------------------------------- 	
					const int FontHeight = g_pClientConfig->FONT_HEIGHT;
					const int FontHeight2 = FontHeight << 1;
					
					pointTemp.x = pPoint->x;			
					if (pointTemp.x<0) pointTemp.x=0;
					
					//---------------------------------------- 	
					// Level Name ì°ì„ ìœ„ì¹˜ë„ ê³„ì‚°
					//---------------------------------------- 	
					if (pCreature->HasLevelName())
					{
						pointTemp.y = pPoint->y - FontHeight2;
						
						if (pointTemp.y < FontHeight2)
						{
							pointTemp.y = FontHeight2;
						}
					}
					//---------------------------------------- 	
					// ê·¸ëƒ¥ ì´ë¦„ë§Œ ì°ì„ ë•Œ
					//---------------------------------------- 	
					else
					{
						pointTemp.y = pPoint->y - FontHeight;
						
						if (pointTemp.y < FontHeight) 
						{
							pointTemp.y = FontHeight;
						}
					}
					
					m_pointChatString	= pointTemp;
					m_pSelectedCreature = pCreature;
				}
				else
				{
					m_pSurface->BltSprite(&pointTemp, pSprite);	
				}			

				//------------------------------------------------
				// ì£½ì€ ìºë¦­í„°ì´ë©´ ì„ íƒ ì‚¬ê°í˜•ì„
				// í•œ Tile í¬ê¸°ë¡œ ì œí•œí•œë‹¤.
				//------------------------------------------------
				RECT rect;
				rect.left	= pPoint->x;
				rect.top	= pPoint->y;
				rect.right	= pPoint->x + TILE_X;
				rect.bottom = pPoint->y + TILE_Y;
				if(FrameIndex ==0 )	
					pCreature->SetScreenRect( &rect );
				
	#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("EndDrawCreature-S");
	#endif
				
				
				if(pCreature->GetActionCount()>=pCreature->GetActionCountMax()/2)
				{
					if (pCreature->IsExistAttachEffect())
					{	
	#ifdef OUTPUT_DEBUG_DRAW_PROCESS
						DEBUG_ADD("DrawAttachEffect");
	#endif
						
						pointTemp = *pPoint;
						//pointTemp.x += pCreature->GetSX();
						//pointTemp.y += pCreature->GetSY();
						
						DrawAttachEffect(&pointTemp, 
							pCreature->GetAttachEffectIterator(), 
							pCreature->GetAttachEffectSize(),
							pCreature,
							1);	// foregroundì¶œë ¥
						
	#ifdef OUTPUT_DEBUG_DRAW_PROCESS
						DEBUG_ADD("DAE ok");
	#endif
					}
				}
				m_pSurface->Unlock();
				const int firstY = DrawChatString(&pointTemp, pCreature, pCreature->GetChatColor());
				pointTemp.y = firstY;
				DrawCreatureHPModify(&pointTemp, pCreature);
				m_pSurface->Lock();	
				return;
			}
		}

		//------------------------------------------------
		// Slayerì¸ ê²½ìš° Darkness ì•ˆì„ ë³¼ ìˆ˜ ì—†ë‹¤.
		//------------------------------------------------
		if (
			!( creature_type >= 526 && creature_type <= 549 || 
			   creature_type == 670 || 
			   creature_type == 672 ||
			   creature_type == 673 ||
			   creature_type >= 371 && creature_type <= 376 || 
			   creature_type >= 560 && creature_type <= 563
			 ) &&
			pCreature->IsInDarkness() && !pCreature->IsNPC() && !g_pPlayer->IsVampire() && g_pPlayer!=pCreature
			&& !g_pPlayer->HasEffectStatus( EFFECTSTATUS_LIGHTNESS )
			&& g_pZone->GetID() != 3001
			&& !g_pPlayer->HasEffectStatus( EFFECTSTATUS_GHOST )
	#ifdef __METROTECH_TEST__
			&& !g_bLight
	#endif
			)
		{
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("EndDrawCreature-D");
			#endif

			return;
		}

		//------------------------------------------------
		//
		// ë•… ì†ì— íŒŒë¬»ížŒ ì• ë“¤ì€ ê³ ì •ëœ ê·¸ë¦¼..
		//
		//------------------------------------------------
		// 2004, 04, 24 sobeit modify - ì§ˆë“œë ˆ ë ˆì–´ ghostëŠ” ë•…ì† í¬ë¦¬ì³ì§€ë§Œ ê³ ì •ëœ ê·¸ë¦¼ì´ ì•„ë‹˜..^^;
		if (pCreature->IsUndergroundCreature() && pCreature->GetCreatureType() != CREATURETYPE_GHOST)
		{
			DrawUndergroundCreature(pPoint, pCreature );
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD( "End DrawCreature" );
			#endif		

			return;
		}

		//----------------------------------------------------------
		// ë¨¸ë¦¬ê°€ ì—†ëŠ” ê²½ìš°
		//----------------------------------------------------------
		if (!pCreature->HasHead()
			&& pCreature->GetActionCount()==pCreature->GetActionCountMax()
			&& g_pUserInformation->GoreLevel == true)
		{
			if (action==ACTION_DIE)
			{
				// ì‹¤ì œë¡œëŠ”..ë§ˆì§€ë§‰ frame + 1ì´ë‹¤.
				//frame	= (*g_pCreatureTable)[creature_type].GetActionCount( action );
				frame	= GetCreatureActionCountMax( pCreature, action);
			}
		}

		// Teenë²„ì „ì¸ ê²½ìš° ì£½ëŠ” ë™ìž‘ ì• ë‹ˆë©”ì´ì…˜ ì•ˆí•¨
		// ì²˜ìŒì—” ë§ˆì§€ë§‰í”„ë ˆìž„ì „ê¹Œì§€ì˜ ë™ìž‘ë§Œ 0ìœ¼ë¡œ ì„¸íŒ…í–ˆëŠ”ë° ì € ìœ„ìª½ì—ì„œ ìž¬ë¡œ ì°ì„ ê²½ìš° returní•´ë²„ë ¤ì„œ ì˜ë¯¸ì—†ìŒ
		if(g_pUserInformation->GoreLevel == false && action == ACTION_DIE
			&&	(
					creature_type < 431		// ë°”í† ë¦¬
					|| creature_type > 436	// í…ŒíŽ˜ì¦ˆ
				)
			)
		{
			frame = 0;
		}
		
		// Creatureê°€ ì¡´ìž¬í•˜ëŠ” ë†’ì´ë§Œí¼ ë¹¼ì¤€ë‹¤.
		pPoint->y -= pCreature->GetZ();

		if(!pCreature->IsNPC() &&  pCreature->HasEffectStatus(EFFECTSTATUS_DIVINE_GUIDANCE))
		{	
			if(isSlayerCharacter)
			{
				if( pCreature->IsAdvancementClass() )
					DrawDivineGuidanceAdvancementClassSlayerCharacter( pPoint, pCreature, direction );
				else
					DrawDivineGuidanceSlayerCharacter( pPoint, pCreature, direction );
			}
			else if(isOustersCharacter)
			{
				if( pCreature->IsAdvancementClass() )
					DrawDivineGuidanceAdvancementClassOustersCharacter( pPoint, pCreature, direction );
				else
					DrawDivineGuidanceOustersCharacter( pPoint, pCreature, direction );				
			}
			else
			{
				if( pCreature->IsAdvancementClass() )
					DrawDivineGuidanceAdvancementClassVampireCharacter( pPoint, pCreature, direction, body );
				else
					DrawDivineGuidanceVampireCharacter( pPoint, pCreature, direction, body );
			}
		}

		//----------------------------------------
		// ë°”ë‹¥ì— ë¶™ì€ Effect ì¶œë ¥
		//----------------------------------------
		if (pCreature->IsExistGroundAttachEffect())
		{				
			pointTemp = *pPoint;
			//pointTemp.x += pCreature->GetSX();
			//pointTemp.y += pCreature->GetSY();

			DrawAttachEffect(&pointTemp, pCreature->GetGroundAttachEffectIterator(), pCreature->GetGroundAttachEffectSize(), pCreature);
		}

		//---------------------------------------------------------------
		//
		//                  Background AttachEffectNode ì¶œë ¥
		//
		//---------------------------------------------------------------
		if (pCreature->IsExistAttachEffect())
		{				
	//		DEBUG_ADD("BackAttachEffect");

			pointTemp = *pPoint;
			//pointTemp.x += pCreature->GetSX();
			//pointTemp.y += pCreature->GetSY();

			DrawAttachEffect(&pointTemp, 
								pCreature->GetAttachEffectIterator(), 
								pCreature->GetAttachEffectSize(),
								pCreature,
								2);	// backgroundì¶œë ¥

	//		DEBUG_ADD("BAE ok");
		}

		//---------------------------------------------------------------
		//
		//                  Mouseë¡œ ì„ íƒ --> ì™¸ê³½ì„ 
		//
		//---------------------------------------------------------------

		//------------------------------------------------
		// Mouseê°€ ê°€ë¦¬í‚¤ê³  ìžˆëŠ” Creature
		//------------------------------------------------
		
		if (m_SelectCreatureID == creatureID || OutLineOption)
		{
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("Sel Creature");
		#endif

			// SpriteOutlineManagerì— ì¶”ê°€
			if (pCreature->IsNPC()							// NPCì¸ ê²½ìš°
				// ë°”í† ë¦¬ì¸ ê²½ìš°ëŠ” ì•ˆëœë‹¤. í•˜ë“œì½”ë”© - -;;
				&& !(creature_type==217)
				|| IsRequestMode()								// trade modeì¸ ê²½ìš°
					// playerë§Œ ëœë‹¤.
					//pCreature->GetCreatureType()<=CREATURETYPE_VAMPIRE_FEMALE
				//	&& (*g_pCreatureSpriteTable)[(*g_pCreatureTable)[creature_type].SpriteType].IsPlayerOnlySprite()
				&& (*g_pCreatureSpriteTable)[(*g_pCreatureTable)[creature_type].SpriteTypes[0]].IsPlayerOnlySprite()
				|| !IsRequestMode()							// tradeí• ë ¤ê³  ì¶”ì ì¤‘ì¸ creatureì¸ ê²½ìš°
					&& g_pPlayer->IsRequestMode()
					&& g_pPlayer->IsTraceCreature()
					&& g_pPlayer->GetTraceID()==creatureID)
			{
				m_SOMOutlineColor = m_ColorOutlineNPC;
			}
			else if (g_pObjectSelector->CanAttack( pCreature )
				|| creature_type==217)		// ë°”í† ë¦¬ì¸ ê²½ìš°. í•˜ë“œì½”ë”© - -;;
			{
				m_SOMOutlineColor = m_ColorOutlineAttackPossible;
			}
			else
			{
				m_SOMOutlineColor = m_ColorOutlineAttackImpossible;
			}

			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("drawBody");
			#endif

			//----------------------------------------
			//
			//         ëª¸ì´ ìžˆëŠ” ê²½ìš°ì— ì¶œë ¥
			//
			//----------------------------------------
			if (body!=FRAMEID_NULL)
			{
				// body ìˆ˜ì • í•„ìš”
				if( pCreature->IsAdvancementClass()
					)		//by viva Selected Vampire
					DrawSelectedAdvancementVampireCreature( pPoint, pCreature, action, direction, frame, 0, FrameIndex );
				else
					DrawSelectedVampireCreature( pPoint, pCreature, action, direction, frame, body, FrameIndex );
			}
			else
			{
				//----------------------------------------------------
				// ìºë¦­í„°ì˜ ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ì„ ì§€ì›Œì¤€ë‹¤.
				//----------------------------------------------------
				pCreature->ClearScreenRect();
			}

			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("draw clothes");
			#endif

			//----------------------------------------
			//
			//            ë³µìž¥ ì¶œë ¥
			//
			//----------------------------------------
			// ëª¨ë“  ìž…ì„ ìˆ˜ ìžˆëŠ” ì¢…ë¥˜ì˜ ì˜· ì¤‘ì—ì„œ 
			// ìž…ê³  ìžˆëŠ” ì˜·ì„ í™•ì¸í•´ì„œ ì¶œë ¥í•´ì¤˜ì•¼ í•œë‹¤.		
			if (isSlayerCharacter)
			{		
				// 2004, 9, 15, sobeit add start  - install turret
				if(pCreature->HasEffectStatus(EFFECTSTATUS_INSTALL_TURRET))
					DrawInstallTurret( pPoint, pCreature, action, direction, frame, FrameIndex );
				else
				{
					if( pCreature->IsAdvancementClass() )
						DrawSelectedAdvancementSlayerCreature(pPoint, pCreature, action, direction, frame );
					else
						DrawSelectedSlayerCreature( pPoint, pCreature, action, direction, frame );
				}
			}
			else if(isOustersCharacter)
			{
				if( pCreature->IsAdvancementClass() )
					DrawSelectedAdvancementOustersCreature(pPoint, pCreature, action, direction, frame );
				else
					DrawSelectedOustersCreature( pPoint, pCreature, action, direction, frame );
			}
			
			// ìŠ¬ë ˆ íŽ«ì¸ ê²½ìš°ëŠ” í„°ë › ì°ì–´ì¤˜ì•¼ ëœë‹¤-ã……-;;;; í•˜ë“œí•˜ë“œ..ì•„ì•„-_-/~
			if( bSlayerPet_ShowTurret )
			{
				DrawCentauroTurret( pPoint, pCreature, action, direction, frame , body);
			}

			// FIX: Draw the actual sprites before drawing the outline
			// BltSpriteOutline only draws the outline, not the sprites themselves
			// So we need to draw the sprites first
			const std::vector<OutlineEntry>& outlines = m_SOM.GetOutlines();
			for (const auto& entry : outlines) {
				if (entry.pIndexSprite) {
					POINT spritePoint = { entry.x, entry.y };
					m_pSurface->BltIndexSprite(&spritePoint, entry.pIndexSprite);
				} else if (entry.pSprite) {
					POINT spritePoint = { entry.x, entry.y };
					m_pSurface->BltSprite(&spritePoint, entry.pSprite);
				}
			}

			m_SOM.Generate(OutLineOption);
			
	//			// 2004, 8, 18 sobeit add start - ê³µì„±ì „ ì„±ë¬¸ íƒ€ê²ŸíŒ…ë•Œë¬¸ì— ì—¬ì°¨ì—¬ì°¨ ì¶”ê°€..
	//		enum GENERATE_OPTION{
	//			GENERATE_ALL = 0,
	//			GENERATE_EXCEPT_LEFT,	// ì™¼ìª½ ì•„ì›ƒë¼ì¸ì€ ì œì™¸
	//			GENERATE_EXCEPT_RIGHT,  // ì˜¤ë¥¸ìª½ ì•„ì›ƒë¼ì¸ì€ ì œì™¸
	//			GENERATE_EXCEPT_SIDE,	// ì–‘ ì‚¬ì´ë“œ ì•„ì›ƒë¼ì¸ì€ ì œì™¸
	//			// -_- ìœ„,ì•„ëž˜ë„ ë§Œë“¤ì–´ ë†”ì•¼ í•˜ë‚˜..-_-;
	//		};
	//		// 2004, 8, 18 sobeit add start
			//if (true || 
			//	DarkBits==0 || 
			//	g_pPlayer->GetID()==pCreature->GetID())
			//{	
			
				if (pCreature->IsInvisible() || pCreature->IsFakeDie()
	//				#ifdef OUTPUT_DEBUG
	//					||	g_pSDLInput->KeyDown(DIK_RCONTROL)
	//				#endif
					)
				{
					// slayerì˜ invisibleì¸ snipping
					if (pCreature->HasEffectStatus(EFFECTSTATUS_SNIPPING_MODE))
					{
						m_pSurface->BltSpriteOutlineDarkness( &m_SOM,  m_SOMOutlineColor, SHIFT_SNIPPING );
					}
					m_pSurface->BltSpriteOutlineOnly( &m_SOM,  m_SOMOutlineColor );
				}
				else
				{
					// ì–´ë‘¡ê²Œ ì°ê¸°
					if (pCreature->IsFade())
					{
						m_pSurface->BltSpriteOutlineDarkness( &m_SOM,  m_SOMOutlineColor, 1 );
					}
					else
					{
						m_pSurface->BltSpriteOutline( &m_SOM,  m_SOMOutlineColor );
					}
				}
				//m_pSurface->BltSpriteOutlineOnly( &m_SOM,  0xF800 );
			//}
			//else
			//{
			//	m_pSurface->BltSpriteOutlineDarkness( &m_SOM, m_SOMOutlineColor, DarkBits );
			//}		
				m_SOM.Clear();
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("drawSelOK");
			#endif
		}
		//------------------------------------------------
		//
		//       ì¼ë°˜ì ì¸ Creature ì¶œë ¥
		//
		//------------------------------------------------
		else
		{
			
			//------------------------------------------------
			//
			//         Bodyì¶œë ¥
			//
			//------------------------------------------------
			if (body!=FRAMEID_NULL)
			{
				bool bBat = pCreature->GetCreatureType() == CREATURETYPE_BAT;
				bool bWolf = pCreature->GetCreatureType() == CREATURETYPE_WOLF;
				bool bWerWolf = pCreature->GetCreatureType() == CREATURETYPE_WER_WOLF;

				if( pCreature->IsAdvancementClass() && !bBat && !bWolf && !bWerWolf
					) //by viva
					DrawAdvancementClassVampireCharacter( pPoint, pCreature, action, direction, frame, body, FrameIndex );
				else
					DrawVampireCharacter( pPoint, pCreature, action, direction, frame, body, FrameIndex );			
				
			}
			else
			{
				//----------------------------------------------------
				// ìºë¦­í„°ì˜ ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ì„ ì§€ì›Œì¤€ë‹¤.
				//----------------------------------------------------
				pCreature->ClearScreenRect();		
			}
			
			//----------------------------------------
			//
			//            ë³µìž¥ ì¶œë ¥
			//
			//----------------------------------------
			// ëª¨ë“  ìž…ì„ ìˆ˜ ìžˆëŠ” ì¢…ë¥˜ì˜ ì˜· ì¤‘ì—ì„œ 
			// ìž…ê³  ìžˆëŠ” ì˜·ì„ í™•ì¸í•´ì„œ ì¶œë ¥í•´ì¤˜ì•¼ í•œë‹¤.
			//WORD colorSet = pCreature->GetAttachEffectColor();
							
			//if (pCreature->IsWear())
			if (isSlayerCharacter)
			{
				if( pCreature->IsAdvancementClass() )   //by viva
					DrawAdvancementClassSlayerCharacter( pPoint, pCreature, action, direction, frame, FrameIndex );
				else
					DrawSlayerCharacter( pPoint, pCreature, action, direction, frame, FrameIndex );
			}
			else if(isOustersCharacter)
			{
				if( pCreature->IsAdvancementClass() )
					DrawAdvancementClassOustersCharacter( pPoint, pCreature, action, direction, frame );
				else
					DrawOustersCharacter( pPoint, pCreature, action, direction, frame );
			}

			// ìŠ¬ë ˆ íŽ«ì¸ ê²½ìš°ëŠ” í„°ë › ì°ì–´ì¤˜ì•¼ ëœë‹¤-ã……-;;;; í•˜ë“œí•˜ë“œ..ì•„ì•„-_-/~
			if( bSlayerPet_ShowTurret )
			{
				DrawCentauroTurret( pPoint, pCreature, action, direction, frame , body);				
			}
		}


		//---------------------------------------------------------------
		//
		//                  AttachEffectNode ì¶œë ¥
		//
		//---------------------------------------------------------------
		if (pCreature->IsExistAttachEffect())
		{	
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("DrawAttachEffect");
			#endif

			pointTemp = *pPoint;
			//pointTemp.x += pCreature->GetSX();
			//pointTemp.y += pCreature->GetSY();

			DrawAttachEffect(&pointTemp, 
								pCreature->GetAttachEffectIterator(), 
								pCreature->GetAttachEffectSize(),
								pCreature,
								1);	// foregroundì¶œë ¥

			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("DAE ok");
			#endif
		}



		//pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
		//pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
		bool bPlayerParty = (pCreature->IsPlayerParty() 
								&& pCreature!=g_pPlayer
								// í• ë£¨ ê±¸ë¦¬ë©´ íŒŒí‹°ì›ì„ ëª» ì•Œì•„ë³¸ë‹¤.
								&& !g_pPlayer->HasEffectStatus(EFFECTSTATUS_HALLUCINATION));	

		//------------------------------------------------
		// Chat String ì¶œë ¥
		//------------------------------------------------
		// Mouseë¡œ ì„ íƒëœ ìºë¦­í„°ì´ë©´..
		if (m_SelectCreatureID == creatureID)
		{
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("SelectedCreature");
			#endif

			//----------------------------------------
			// ì´ë¦„ ì¶œë ¥í•  ì¢Œí‘œ ì§€ì •
			//----------------------------------------
			const int FontHeight = g_pClientConfig->FONT_HEIGHT;
			const int FontHeight2 = FontHeight << 1;

			pointTemp.x = pPoint->x;
			if (pointTemp.x<0) pointTemp.x=0;

			pointTemp.y = pPoint->y - pCreature->GetHeight();

			//----------------------------------------
			// Level Name ì°ì„ ìœ„ì¹˜ë„ ê³„ì‚°
			//----------------------------------------
			if (pCreature->HasLevelName())
			{
				if (pointTemp.y < FontHeight2)
				{
					pointTemp.y = FontHeight2;
				}
			}
			//----------------------------------------
			// ê·¸ëƒ¥ ì´ë¦„ë§Œ ì°ì„ ë•Œ
			//----------------------------------------
			else
			{
				if (pointTemp.y < FontHeight)
				{
					pointTemp.y = FontHeight;
				}
			}

			m_pointChatString	= pointTemp;
			m_pSelectedCreature = pCreature;

		}
		else
		{		
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("noSelCre");
			#endif

			pointTemp.x = pPoint->x;
			pointTemp.y = pPoint->y - pCreature->GetHeight();
				
			//------------------------------------------------
			// íŒŒí‹°ì›ì¸ ê²½ìš° HP ì¶œë ¥í•´ì¤€ë‹¤.
			//------------------------------------------------
			if (bPlayerParty)
			{			
				DrawPartyHP( &pointTemp, pCreature );

				pointTemp.y = pPoint->y - pCreature->GetHeight() - m_EtcSPK[SPRITEID_PARTY_HP].GetHeight();
			}
			else
			{
				pointTemp.y = pPoint->y - pCreature->GetHeight();
			}

			pointTemp.x = pPoint->x;			

			//
			// ì´ê±° ë¶€í•˜ê°€ ê½¤ ê±¸ë¦°ë‹¤... ì©~~
			//
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("DrawChat");
			#endif

			m_pSurface->Unlock();
			const int firstY = DrawChatString(&pointTemp, pCreature, pCreature->GetChatColor());
			pointTemp.y = firstY;
			DrawCreatureHPModify(&pointTemp, pCreature);
			m_pSurface->Lock();	

			// 2004, 8, 18, sobeit add start - ê³µì„±ì „ ê¸¸ë“œë§ˆí¬ í‘œì‹œ
			DrawGuildMarkInSiegeWar(pCreature,pointTemp.y);
			// 2004, 8, 18, sobeit add end
		}	

		//------------------------------------------------
		// íŒŒí‹°ì›ì˜ ì‹œì•¼ë¥¼ ë³´ì—¬ì¤€ë‹¤.
		//------------------------------------------------
		if (bPlayerParty)
		{
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("partySight");
			#endif

			int pX = pCreature->GetPixelX() - m_FirstZonePixel.x + 24;
			int pY = pCreature->GetPixelY() - m_FirstZonePixel.y - TILE_Y;

			//int addLight = (pCreature->GetCreatureType()==CREATURETYPE_BAT ? 3 : 0);
	//		int addLight = (pCreature->IsFlyingCreature() ? 3 : 0);
			// playerë§Œí¼ì˜ ì‹œì•¼ë¼ê³  ìƒê°í•œë‹¤. - -;
			int creatureLight = g_pPlayer->GetLightSight();// + addLight);

			// SDL2: Unified 2D lighting path for all platforms
			AddLightFilter2D( pX,
								pY - pCreature->GetZ(),
								creatureLight,
								false,	// screenPixelì¢Œí‘œ
								true);	// ë¬´ì¡°ê±´ ì¶œë ¥í•´ì•¼í•˜ëŠ” ë¹›

			// ì˜¤í† ë°”ì´ ë¶ˆë¹›
			ADD_MOTORCYCLE_LIGHT_XY_2D( pCreature, pX, pY, true );
		}
		//------------------------------------------------
		// íŒŒí‹°ì› ì•„ë‹Œ ê²½ìš°ì—...
		//------------------------------------------------
		else
		{
			#ifdef OUTPUT_DEBUG_DRAW_PROCESS
				DEBUG_ADD("noPParty?");
			#endif

			// ì˜¤í† ë°”ì´ ë¶ˆë¹›
			ADD_MOTORCYCLE_LIGHT_2D( pCreature, false );
		}

		if(gpC_item != NULL && pCreature != NULL && pCreature->GetHeadSkin() != 0)
		{
			int spriteID = pCreature->GetHeadSkin();
			int pX = pCreature->GetPixelX() - m_FirstZonePixel.x + 24-gpC_item->GetWidth(spriteID)/2;
			int pY = pCreature->GetPixelY() - m_FirstZonePixel.y - TILE_Y -18-gpC_item->GetHeight(spriteID);
			CIndexSprite::SetUsingColorSet(377, 0);
			gpC_item->BltLocked(pX, pY, spriteID);
		}
	}

	__END_PROFILE("DrawCreature")

	#ifdef OUTPUT_DEBUG_DRAW_PROCESS
		DEBUG_ADD( "End DrawCreature" );
	#endif
//	DEBUG_ADD("[DrawCreature] End");

}

void	MTopView::DrawSelectedSlayerCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	MCreatureWear*	pCreatureWear = (MCreatureWear*)pCreature;
	// Creatureì˜ Actionì— ë§žëŠ” add-onì„ ì¶œë ¥í•œë‹¤.
	//action = pCreature->GetAction();	
	WORD clothes;
	BYTE clothesType;
	
	for (int i=0; i<ADDON_MAX; i++)
	{
		// Creatureì˜ í˜„ìž¬ ë°©í–¥ì— ë”°ë¼ì„œ...
		// ì˜·ì„ ì¶œë ¥í•´ì£¼ëŠ” ìˆœì„œê°€ ë‹¤ë¥¼ ìˆ˜ ìžˆë‹¤.
		clothesType = MCreatureWear::s_AddonOrder[pCreature->GetDirection()][i];
		
		// ië²ˆì§¸ ì¢…ë¥˜ì˜ ì˜·ì„ ìž…ê³  ìžˆë‹¤ë©´ ì¶œë ¥í•´ ì¤€ë‹¤.
		const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(clothesType);
		
		if (addonInfo.bAddon)
		{
			clothes = addonInfo.FrameID;
			
			FRAME_ARRAY &FA = m_AddonFPK[clothes][action][direction];
			
			// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
			if (FA.GetSize() > frame)
			{
				CFrame &Frame = FA[frame];					
				int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
				int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
				int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
				
				pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
				pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
				
				CIndexSprite* pSprite = &m_AddonSPK[ sprite ];
				
				// ë³µìž¥Spriteê°€ ì´ˆê¸°í™” ë˜ì§€ ì•Šì€ ê²½ìš°
				//						if (pSprite->IsNotInit())
				//						{
				//							LoadFromFileAddonSPK( clothes, action );
				//							//continue;
				//						}
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				pCreature->AddScreenRect( &rect );
				
				// SpriteOutlineManagerì— ì¶”ê°€
				//CIndexSprite::SetUsingColorSet((pCreature->GetID()+3*(clothesType,1),7)%MAX_COLORSET, ((pCreature->GetID()*pCreature->GetID(),7*(clothesType+2)))%MAX_COLORSET);
				//CIndexSprite::SetUsingColorSet((c1+3*(clothesType+1)+7)%MAX_COLORSET, ((c2*c2+7*(clothesType+2)))%MAX_COLORSET);
				//---------------------------------------- 
				// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
				//---------------------------------------- 
				if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
				{
					m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
				}
				//---------------------------------------- 
				// ì›ëž˜ìƒ‰ëŒ€ë¡œ..
				//---------------------------------------- 
				else
				{
					int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
					if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
					{
						if(colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
							colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
						
						if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
							colorSet1 = colorSet2;
					}							
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
				}
			}
		}
	}
}



void	MTopView::DrawSelectedOustersCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	//  ì± í¬ëžŒì´ ìžˆë‹¤
	MCreatureWear *pCreatureWear = (MCreatureWear *)pCreature;
	
	const MCreatureWear::ADDON_INFO& addonInfoChakram = pCreatureWear->GetAddonInfo(ADDON_RIGHTHAND);
	bool bChakram = addonInfoChakram.bAddon && addonInfoChakram.ItemClass == ITEM_CLASS_OUSTERS_CHAKRAM;
	
	int tempAction = action;
	if(bChakram == false)
	{
		switch(tempAction)
		{
		case ACTION_STAND:
			tempAction = ACTION_OUSTERS_STAND;
			break;
			
		case ACTION_MOVE:
			tempAction = ACTION_OUSTERS_MOVE;
			break;
		}
	}
	
	// ëª¸ì„ ì°ê³  ì± í¬ëžŒì„ ì°ëŠ”ë‹¤
	const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(ADDON_COAT);
	const MCreatureWear::ADDON_INFO& bootsAddonInfo = pCreatureWear->GetAddonInfo(ADDON_TROUSER);
	
	if (addonInfo.bAddon && !pCreatureWear->IsGhost(1))
	{
		int clothes = addonInfo.FrameID;
		
		
		FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			pCreature->AddScreenRect( &rect );
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				CIndexSprite::SetUsingColorSetOnly( 0, pCreature->GetBodyColor1() );	// ë¨¸ë¦¬ìƒ‰
				
				int coatColor = addonInfo.ColorSet2, Colorset;
				if( coatColor == QUEST_ITEM_COLOR || coatColor == UNIQUE_ITEM_COLOR )
					coatColor = MItem::GetSpecialColorItemColorset( coatColor );						
				CIndexSprite::SetUsingColorSetOnly( 1, coatColor );	// ì˜·ìƒ‰
				if(bChakram == false)	// ë¦¬ìŠ¤í‹€ë¦¿ì„ ì°¨ê³  ìžˆìœ¼ë©´
				{
					if(addonInfoChakram.bAddon)
					{
						Colorset = addonInfoChakram.ColorSet2;
						if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”								
					}
					else
					{
						Colorset = defaultCoatColor;								
						if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
				}
				else
				{
					Colorset = defaultCoatColor;
					if(addonInfoChakram.bAddon)
						Colorset = addonInfoChakram.ColorSet2;
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
				}
				
				if(bootsAddonInfo.bAddon)
				{
					Colorset = bootsAddonInfo.ColorSet2;
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// ë¶€ì¸ ìƒ‰
				}
				else
				{
					Colorset = defaultCoatColor;
					
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// íŒ”
				}
				
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
			}						
			
		}
	}
	
	if (bChakram && !pCreatureWear->IsGhost(2))
	{
		int clothes = addonInfoChakram.FrameID;
		
		FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfoChakram.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				int colorset1, colorset2;
				
				colorset1 = addonInfoChakram.ColorSet1;
				colorset2 = addonInfoChakram.ColorSet2;
				if( colorset1 == QUEST_ITEM_COLOR || colorset1 == UNIQUE_ITEM_COLOR )
					colorset1 = MItem::GetSpecialColorItemColorset( colorset1 );
				if( colorset2 == QUEST_ITEM_COLOR || colorset2 == UNIQUE_ITEM_COLOR )
					colorset2 = MItem::GetSpecialColorItemColorset( colorset2 );
				
				
				CIndexSprite::SetUsingColorSet(colorset1, colorset2);
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
			}						
			
		}
	}
}

void	MTopView::DrawVampireCharacter( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame, int body, int frameindex )
{
	//------------------------------------------------
	// ê´€ ì¶œë ¥ì •ë³´
	//------------------------------------------------
	int casketValue = pCreature->GetCasketCount();
	int creature_type = pCreature->GetCreatureType();  //add by viva
	//int creature_type = 814;
	bool bInCasket = pCreature->IsInCasket();
	bool bCasketOnly = (bInCasket && casketValue==0);
	
	//----------------------------------------------------
	// ì™„ì „ížˆ ê´€ë§Œ ì¶œë ¥í•˜ëŠ” ê²½ìš°ëŠ” ëª¸ ì¶œë ¥ì„ ì•ˆí•œë‹¤.
	//----------------------------------------------------
	if (!bCasketOnly)
	{
		FRAME_ARRAY& FA = m_CreatureFPK[body][action][direction];
	//	try{	
		if (FA.GetSize() > frame)
		{
			CFrame& Frame =	FA[frame];
			int sprite = Frame.GetSpriteID(),	//m_CreatureFPK[body][action][direction][frame].GetSpriteID(),
				cx		= Frame.GetCX(),	//m_CreatureFPK[body][action][direction][frame].GetCX(),
				cy		= Frame.GetCY();	//m_CreatureFPK[body][action][direction][frame].GetCY();
			
			// ì¢Œí‘œ ë³´ì •
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			CIndexSprite* pSprite = &m_CreatureSPK[ sprite ];
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			if(frameindex==0 )
				pCreature->SetScreenRect( &rect );
			
			// Playerê°€ ì¶”ì ì¤‘ì¸ Character í‘œì‹œ
			//if (g_pPlayer->GetTraceCreatureID()==pCreature->GetID())			
			
			//CIndexSprite::SetUsingColorSet(c1%MAX_COLORSET, c2%MAX_COLORSET);
			
			//-----------------------------------------------------------
			// CutHeight
			//-----------------------------------------------------------
			bool bCutHeight = pCreature->IsCutHeight();
			int previousClipBottom;
			if (bCutHeight)
			{
				// SDL2: Clipping is automatic in SDL backend
				// previousClipBottom = m_pSurface->GetClipBottom();
				// m_pSurface->SetClipBottom( rect.bottom - 24 );
				previousClipBottom = rect.bottom;
				
				pointTemp.y += pCreature->GetCutHeightCount() - 24;
			}
			
			//-----------------------------------------------------------
			// íˆ¬ëª… ìƒíƒœ ì¶œë ¥ 			
			//-----------------------------------------------------------
			// vampire
			if (pCreature->IsFakeDie())
			{
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					// IndexSpriteì—ë„ EFFECT...ë„£ê³ ...
					// ìŠ¤ë¥´ë¥µ~ ë‹¨ê³„ì ìœ¼ë¡œ ë‚˜íƒ€ë‚˜ê²Œ/ì—†ì–´ì§€ê²Œ í•´ì•¼ë¨.. - -;
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				else
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int netValue = pCreature->GetFakeDieCount()/5;
					
					CSpriteSurface::s_Value1 = netValue;
					
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_NET );
					CIndexSprite::SetEffect( CIndexSprite::EFFECT_NET );
					
					m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					
				}
			}
			else if (pCreature->IsInvisible())
			{
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					// IndexSpriteì—ë„ EFFECT...ë„£ê³ ...
					// ìŠ¤ë¥´ë¥µ~ ë‹¨ê³„ì ìœ¼ë¡œ ë‚˜íƒ€ë‚˜ê²Œ/ì—†ì–´ì§€ê²Œ í•´ì•¼ë¨.. - -;
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				else
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int wipeValue = pCreature->GetInvisibleCount();
					
					if (wipeValue==0)
					{
						m_pSurface->BltIndexSprite(&pointTemp, pSprite);							
					}
					else if (wipeValue==64)
					{
						CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
						CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
						
						m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					}
					else
					{
						CSpriteSurface::s_Value1 = wipeValue;
						
						CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
						CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
						
						m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
						
						//CSpriteSurface::s_Value1 = wipeValue/10+1;
						
						//CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_NET );
						//CIndexSprite::SetEffect( CIndexSprite::EFFECT_NET );
						
						//m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					}							
				}
			}					
			else
			{	
				//-----------------------------------------------------------
				// ë¹ ë¥´ê²Œ ì›€ì§ì´ëŠ” ì¤‘
				//-----------------------------------------------------------
				if (pCreature->IsFastMove() || pCreature->IsBloodyZenith())
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					POINT pointTemp2;
					
					//CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_NET );
					//CIndexSprite::SetEffect( CIndexSprite::EFFECT_NET );
					
					int fastMoveShadowMax = 2;
					
					if(creature_type == CREATURETYPE_WILD_WOLF)
						fastMoveShadowMax = 6;
					if(pCreature->IsBloodyZenith())
					{
						fastMoveShadowMax = pCreature->GetBloodyZenithCount();
						pCreature->UpdateBloodyZenith();
					}
					for(int fastMoveShadow = fastMoveShadowMax; fastMoveShadow > 0; fastMoveShadow--)
					{
						// 1 pixel skip
						pointTemp2.x = pointTemp.x - (pCreature->GetCX() >> 1)*fastMoveShadow;
						pointTemp2.y = pointTemp.y - (pCreature->GetCY() >> 1)*fastMoveShadow;
						if(pCreature->IsBloodyZenith())
						{
							pointTemp2.y -= sin((double)max(0, pCreature->GetMoveCount()-fastMoveShadow)*3.141592/(double)pCreature->GetMoveCountMax())*100;
						}
						
						//CSpriteSurface::s_Value1 = 1;
						//m_pSurface->BltIndexSpriteEffect(&pointTemp2, pSprite);
						m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite, min(2, fastMoveShadow));
					}
					
				}
				
				if(pCreature->IsBloodyZenith())
				{
					pointTemp.y -= sin((double)pCreature->GetMoveCount()*3.141592/(double)pCreature->GetMoveCountMax())*100;
				}
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				//---------------------------------------- 
				// ì •ìƒì ì¸ ìƒ‰ê¹” ì¶œë ¥
				//---------------------------------------- 					
				else
				{				
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int shadowCount = pCreature->GetShadowCount();
					
					//----------------------------------------
					// Shadowì¶œë ¥
					//----------------------------------------
					if (shadowCount)
					{
						bool bDrawShadowFirst = (direction<=4) || creature_type == 764 ||  creature_type == 765;
						if (!bDrawShadowFirst)
						{
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							}
						}
						
						// 0, 1, 2, 3, 4, 5
						const int darkbits[6] =
						{
							0, 1, 1, 2, 2, 3
						};
						
						POINT pointTemp2;
						
						// ì´ë™ì´ë©´ ì¢Œí‘œë„ ë°”ë€ë‹¤
						int cxStep = 0, cyStep = 0, cxInc =0 , cyInc = 0;
						if (action==ACTION_MOVE)
						{
							cxInc = -pCreature->GetCX();
							cyInc = -pCreature->GetCY();
						}
						
						
						int faNum = FA.GetSize();
						for (int f=1; f<=shadowCount; f++)
						{
							int frame_f = frame-f;
							
							//---------------------------------------- 
							// f frame ì „
							//---------------------------------------- 
							if (frame_f < 0)
							{
								frame_f += faNum;
							}
							
							CFrame &Frame_f = FA[frame_f];
							int sprite_f	= Frame_f.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
							
							CIndexSprite* pSprite_f = &m_CreatureSPK[ sprite_f ];
							
							//									if (pSprite_f->IsNotInit())
							//									{
							//										LoadFromFileCreatureActionSPK( body, action );
							//									}
							
							int cx_f		= Frame_f.GetCX();
							int cy_f		= Frame_f.GetCY();
							
							pointTemp2.x = pPoint->x + cx_f + cxStep;
							pointTemp2.y = pPoint->y + cy_f + cyStep;
							cxStep += cxInc;
							cyStep += cyInc;
							
							m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite_f, darkbits[f]);																		
						}
						
						if (bDrawShadowFirst)
						{
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							}
						}
					}
					//----------------------------------------
					// ëª¸ë§Œ ì°ê¸°
					//----------------------------------------
					else
					{
						// ì–´ë‘¡ê²Œ ì°ê¸°
						if (pCreature->IsFade())
						{
							m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
						}
						else
						{
							m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							
						}
					}
				}
			}

			if (bCutHeight)
			{
				// SDL2: Clipping is automatic in SDL backend
				// m_pSurface->SetClipBottom( previousClipBottom );
			}

			// 2004, 10, 28, sobeit add start  - ëª¬ìŠ¤í„° í‚¬ í€˜ìŠ¤íŠ¸ í•´ë‹¹ ëª¬ìŠ¤í„°ì— í‘œì‹œ.
			if(pCreature->IsAlive() && (g_CurrentFrame&0x04)&&UI_IsMonsterKillQuest_Monster((*g_pCreatureTable)[pCreature->GetCreatureType()].SpriteTypes[0]))
			//if(pCreature->IsAlive() && (g_CurrentFrame&0x04))
			{
				UI_ShowTargetArrow(pPoint->x + 12 ,pPoint->y- pCreature->GetHeight()-20);
			}
		// 2004, 10, 28, sobeit add end
		

		}
//		}catch(...){} 
	}
		
	//----------------------------------------------------
	// ê´€ ì¶œë ¥
	//----------------------------------------------------
	if (bInCasket)
	{
		// ê·¸ë¦¼ìž ë•Œë¬¸ì— *2í•´ì•¼í•œë‹¤.
		int casketID = SPRITEID_CASKET_1 + pCreature->GetCasketType()*2;
		
		if (casketID < m_EtcSPK.GetSize())
		{
			CSprite* pSprite = &m_EtcSPK[casketID];
			
			if (pSprite!=NULL)
			{
				// tileì˜ ì¤‘ì‹¬ì—ì„œ ì„¸ìš´ë‹¤.
				pointTemp.x = pPoint->x + 24 - (pSprite->GetWidth()>>1);
				pointTemp.y = pPoint->y + TILE_Y - pSprite->GetHeight();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				if(frameindex==0 )
					pCreature->SetScreenRect( &rect );
				
				if (casketValue==0)
				{
					m_pSurface->BltSprite(&pointTemp, pSprite);						
				}
				else if (casketValue==64)
				{
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
					
					m_pSurface->BltSpriteEffect(&pointTemp, pSprite);
				}
				else
				{
					CSpriteSurface::s_Value1 = casketValue;
					
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
					
					m_pSurface->BltSpriteEffect(&pointTemp, pSprite);
				}
			}
		}
	}		
}

void	MTopView::DrawOustersCharacter( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	//  ì± í¬ëžŒì´ ìžˆë‹¤
	MCreatureWear *pCreatureWear = (MCreatureWear *)pCreature;
	
	const MCreatureWear::ADDON_INFO& addonInfoChakram = pCreatureWear->GetAddonInfo(ADDON_RIGHTHAND);
	bool bChakram = addonInfoChakram.bAddon && addonInfoChakram.ItemClass == ITEM_CLASS_OUSTERS_CHAKRAM;
	
	int tempAction = action;
	if(bChakram == false)
	{
		switch(tempAction)
		{
		case ACTION_STAND:
			tempAction = ACTION_OUSTERS_STAND;
			break;
			
		case ACTION_MOVE:
			tempAction = ACTION_OUSTERS_MOVE;
			break;
		}
	}
	
	// ëª¸ì„ ì°ê³  ì± í¬ëžŒì„ ì°ëŠ”ë‹¤
	const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(ADDON_COAT);
	const MCreatureWear::ADDON_INFO& bootsAddonInfo = pCreatureWear->GetAddonInfo(ADDON_TROUSER);
	
	if (addonInfo.bAddon && !pCreatureWear->IsGhost(1))
	{
		//				DEBUG_ADD("[DrawCreature] DrawCloth");
		int clothes = addonInfo.FrameID;
		
		
		FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			pCreature->AddScreenRect( &rect );
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
					addonInfo.EffectColorSet);
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				CIndexSprite::SetUsingColorSetOnly( 0, pCreature->GetBodyColor1() );	// ë¨¸ë¦¬ìƒ‰
				
				int Colorset;
				
				Colorset = addonInfo.ColorSet2;
				
				if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
					Colorset = MItem::GetSpecialColorItemColorset( Colorset );
				CIndexSprite::SetUsingColorSetOnly( 1, Colorset );	// ì˜·ìƒ‰
				
				if(bChakram == false)	// ë¦¬ìŠ¤í‹€ë¦¿ì„ ì°¨ê³  ìžˆìœ¼ë©´
				{
					if(addonInfoChakram.bAddon)
					{
						Colorset = addonInfoChakram.ColorSet2;
						if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
					else
					{
						Colorset = defaultCoatColor;
						if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
				}
				else
				{
					Colorset = defaultCoatColor;
					if(addonInfoChakram.bAddon)
						Colorset = addonInfoChakram.ColorSet2;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					
				}
				
				if(bootsAddonInfo.bAddon)
				{
					Colorset = bootsAddonInfo.ColorSet2;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// ë¶€ì¸ ìƒ‰
				}
				else
				{
					Colorset = defaultCoatColor ;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// íŒ”
				}
				
				// ì–´ë‘¡ê²Œ ì°ê¸°
				if (pCreature->IsFade())
				{
					m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
				}
				else
				{
					m_pSurface->BltIndexSprite(&pointTemp, pSprite);
				}
			}			
		}
	}
	
	if (bChakram && !pCreatureWear->IsGhost(2))
	{
		//				DEBUG_ADD("[DrawCreature] DrawChakram");
		int clothes = addonInfoChakram.FrameID;
		
		FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			//					DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() > frame %d %d", FA.GetSize(), frame);
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfoChakram.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
					addonInfoChakram.EffectColorSet);
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				int colorset1, colorset2;
				colorset1 = addonInfoChakram.ColorSet1;
				colorset2 = addonInfoChakram.ColorSet2;
				
				if( colorset1 == QUEST_ITEM_COLOR || colorset1 == UNIQUE_ITEM_COLOR )
					colorset1 = MItem::GetSpecialColorItemColorset( colorset1 );
				if( colorset2 == QUEST_ITEM_COLOR || colorset2 == UNIQUE_ITEM_COLOR )
					colorset2 = MItem::GetSpecialColorItemColorset( colorset2 );
				CIndexSprite::SetUsingColorSet( colorset1, colorset2 );
				// ì–´ë‘¡ê²Œ ì°ê¸°
				if (pCreature->IsFade())
				{
					m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
				}
				else
				{
					m_pSurface->BltIndexSprite(&pointTemp, pSprite);
				}
			}						
			
		}
		else
		{
			DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() <= frame %d %d %d %d", FA.GetSize(), frame, pCreatureWear->GetActionCount(), pCreatureWear->GetActionCountMax());
			//					DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() <= frame %d %d %d", clothes, tempAction, direction);
		}
	}
	
#ifdef OUTPUT_DEBUG_DRAW_PROCESS
	DEBUG_ADD("dcOK");
#endif
}

void	MTopView::DrawAdvancementClassOustersCharacter( 
				POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	//  ì± í¬ëžŒì´ ìžˆë‹¤
	MCreatureWear *pCreatureWear = (MCreatureWear *)pCreature;
	
	const MCreatureWear::ADDON_INFO& addonInfoChakram = pCreatureWear->GetAddonInfo(ADDON_RIGHTHAND);
	bool bChakram = addonInfoChakram.bAddon && addonInfoChakram.ItemClass == ITEM_CLASS_OUSTERS_CHAKRAM;
	
	int tempAction = ConvAdvancementOustersActionFromOusterAction( action, bChakram );

	if( tempAction == -1 )
		return;
	else
		tempAction -= ADVANCEMENT_ACTION_START;
	
	// ëª¸ì„ ì°ê³  ì± í¬ëžŒì„ ì°ëŠ”ë‹¤
	const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(ADDON_COAT);
	const MCreatureWear::ADDON_INFO& bootsAddonInfo = pCreatureWear->GetAddonInfo(ADDON_TROUSER);
	
	if (addonInfo.bAddon && !pCreatureWear->IsGhost(1))
	{
		//				DEBUG_ADD("[DrawCreature] DrawCloth");
		int clothes = addonInfo.FrameID;
		
		
		//FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		FRAME_ARRAY &FA = m_AdvancementOustersFPK[ 1 ][ tempAction ][ direction ];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			//CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			CIndexSprite* pSprite = &m_AdvancementOustersSPK[ sprite ];
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			pCreature->AddScreenRect( &rect );
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
					addonInfo.EffectColorSet);
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				CIndexSprite::SetUsingColorSetOnly( 0, pCreature->GetBodyColor1() );	// ë¨¸ë¦¬ìƒ‰
				
				int Colorset;
				
				Colorset = addonInfo.ColorSet2;
				
				if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
					Colorset = MItem::GetSpecialColorItemColorset( Colorset );
				CIndexSprite::SetUsingColorSetOnly( 1, Colorset );	// ì˜·ìƒ‰
				
				if(bChakram == false)	// ë¦¬ìŠ¤í‹€ë¦¿ì„ ì°¨ê³  ìžˆìœ¼ë©´
				{
					if(addonInfoChakram.bAddon)
					{
						Colorset = addonInfoChakram.ColorSet2;
						if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
					else
					{
						Colorset = defaultCoatColor;
						if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
				}
				else
				{
					Colorset = defaultCoatColor;
					if(addonInfoChakram.bAddon)
						Colorset = addonInfoChakram.ColorSet2;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					
				}
				
				if(bootsAddonInfo.bAddon)
				{
					Colorset = bootsAddonInfo.ColorSet2;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// ë¶€ì¸ ìƒ‰
				}
				else
				{
					Colorset = defaultCoatColor ;
					if (Colorset == QUEST_ITEM_COLOR||Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// íŒ”
				}
				
				// ì–´ë‘¡ê²Œ ì°ê¸°
				if (pCreature->IsFade())
				{
					m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
				}
				else
				{
					m_pSurface->BltIndexSprite(&pointTemp, pSprite);
				}
			}			
		}
	}
	
	if (bChakram && !pCreatureWear->IsGhost(2))
	{
		//				DEBUG_ADD("[DrawCreature] DrawChakram");
		int clothes = addonInfoChakram.FrameID;
		
		//FRAME_ARRAY &FA = m_OustersFPK[clothes][tempAction][direction];
		FRAME_ARRAY &FA = m_AdvancementOustersFPK[ 0 ][ tempAction ][ direction ];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			//					DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() > frame %d %d", FA.GetSize(), frame);
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			//CIndexSprite* pSprite = &m_OustersSPK[ sprite ];					
			CIndexSprite* pSprite = &m_AdvancementOustersSPK[ sprite ];
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfoChakram.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
					addonInfoChakram.EffectColorSet);
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				int colorset1, colorset2;
				colorset1 = addonInfoChakram.ColorSet1;
				colorset2 = addonInfoChakram.ColorSet2;
				
				if( colorset1 == QUEST_ITEM_COLOR || colorset1 == UNIQUE_ITEM_COLOR )
					colorset1 = MItem::GetSpecialColorItemColorset( colorset1 );
				if( colorset2 == QUEST_ITEM_COLOR || colorset2 == UNIQUE_ITEM_COLOR )
					colorset2 = MItem::GetSpecialColorItemColorset( colorset2 );
				CIndexSprite::SetUsingColorSet( colorset1, colorset2 );
				// ì–´ë‘¡ê²Œ ì°ê¸°
				if (pCreature->IsFade())
				{
					m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
				}
				else
				{
					m_pSurface->BltIndexSprite(&pointTemp, pSprite);
				}
			}						
			
		}
		else
		{
			DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() <= frame %d %d %d %d", FA.GetSize(), frame, pCreatureWear->GetActionCount(), pCreatureWear->GetActionCountMax());
			//					DEBUG_ADD_FORMAT("[DrawCreature] FA.GetSize() <= frame %d %d %d", clothes, tempAction, direction);
		}
	}
	
#ifdef OUTPUT_DEBUG_DRAW_PROCESS
	DEBUG_ADD("dcOK");
#endif
}


void	MTopView::DrawSlayerCharacter( POINT *pPoint, MCreature* pCreature, int action, int direction, int frame, int FrameIndex )
{
	if(pCreature->HasEffectStatus(EFFECTSTATUS_INSTALL_TURRET))
		DrawInstallTurret( pPoint, pCreature, action, direction, frame, FrameIndex );
	else
	{
		MCreatureWear*	pCreatureWear = (MCreatureWear*)pCreature;
		
		// Creatureì˜ Actionì— ë§žëŠ” add-onì„ ì¶œë ¥í•œë‹¤.
		//action = pCreature->GetAction();
		
		WORD clothes;
		BYTE clothesType;
		
		//------------------------------------------------------------
		// Fade Out ì¶œë ¥
		//------------------------------------------------------------
		if (pCreature->HasEffectStatus( EFFECTSTATUS_FADE_OUT ))
		{
			DrawFadeOut( pPoint, pCreature, action, direction, frame);
		}
		
		//------------------------------------------------------------
		// Fast Move ì¶œë ¥
		//------------------------------------------------------------
		if (pCreature->IsFastMove())
		{
			DrawFastMove( pPoint, pCreature, action, direction, frame );						
		}
		//------------------------------------------------------------
		// Invisible  - SlayerëŠ” snipping modeì—ì„œ íˆ¬ëª…ì´ë‹¤.
		//------------------------------------------------------------
		else if (pCreature->IsInvisible())
		{
			DrawInvisible( pPoint, pCreature, action, direction, frame );
		}
		//------------------------------------------------------------
		// ë¬´ê¸° ìž”ìƒ ê¸°ìˆ 
		//------------------------------------------------------------
		else if (
			(pCreature->HasEffectStatus( EFFECTSTATUS_DANCING_SWORD )
			|| pCreature->HasEffectStatus( EFFECTSTATUS_GHOST_BLADE )
			|| pCreature->HasEffectStatus( EFFECTSTATUS_REDIANCE ) && pCreature->GetAction() == ACTION_STAND
			)
			)
		{
			DrawWeaponFadeOut( pPoint, pCreature, action, direction, frame );			
		}
		//------------------------------------------------------------
		// ì¼ë°˜ì ì¸ ì¶œë ¥
		//------------------------------------------------------------
		else
		{
			for (int i=0; i<ADDON_MAX; i++)
			{
				// Creatureì˜ í˜„ìž¬ ë°©í–¥ì— ë”°ë¼ì„œ...
				// ì˜·ì„ ì¶œë ¥í•´ì£¼ëŠ” ìˆœì„œê°€ ë‹¤ë¥¼ ìˆ˜ ìžˆë‹¤.
				clothesType = MCreatureWear::s_AddonOrder[pCreature->GetDirection()][i];
				
				// ië²ˆì§¸ ì¢…ë¥˜ì˜ ì˜·ì„ ìž…ê³  ìžˆë‹¤ë©´ ì¶œë ¥í•´ ì¤€ë‹¤.
				const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(clothesType);
				//by viva NewSlayer
				if (addonInfo.bAddon)
				{
					clothes = addonInfo.FrameID;

					// fix: m_AddonFPK can still be unloaded (m_Size==0) the
					// first time a creature is fully drawn this session
					// (e.g. right after leaving EFFECTSTATUS_GHOST, which
					// skips DrawCreature entirely while active) - indexing
					// it with operator[] then is a null-pointer access
					// violation (see DrawShadowSlayerCharacter for the same
					// guard already applied to m_AddonShadowFPK).
					if (m_AddonFPK.GetSize() == 0)
						continue;

					FRAME_ARRAY &FA = m_AddonFPK[clothes][action][direction];
					
					// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
					if (FA.GetSize() > frame)
					{
						CFrame &Frame = FA[frame];					
						int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
						int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
						int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
						
						CIndexSprite* pSprite = &m_AddonSPK[ sprite ];
								
						pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
						pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
						
						//---------------------------------------- 
						// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
						//---------------------------------------- 	
						rect.left	= pointTemp.x;
						rect.top	= pointTemp.y;
						rect.right	= rect.left + pSprite->GetWidth();
						rect.bottom = rect.top + pSprite->GetHeight();
						pCreature->AddScreenRect( &rect );
						
						//---------------------------------------- 
						// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
						//---------------------------------------- 
						if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
						{
							m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
								addonInfo.EffectColorSet);
						}
						//---------------------------------------- 
						// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
						//---------------------------------------- 
						else
						{
							int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
							if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
							{
								// ìœ ë‹ˆí¬ ì•„ì´í…œì´ë©´
								//									if(colorSet2 == QUEST_ITEM_COLOR)
								//										colorSet2 = MItem::GetQuestItemColorset();
								//									else
								//										colorSet2 = MItem::GetUniqueItemColorset();										
								
								colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
								if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
									colorSet1 = colorSet2;
							}
							
							CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
							
							// ì–´ë‘¡ê²Œ ì°ê¸°
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							}
						}						
						
						// ì •ìƒì ì¸ ì¶œë ¥
						//CIndexSprite::SetUsingColorSet( addonInfo.ColorSet1, addonInfo.ColorSet2 );
						//m_pSurface->BltIndexSprite(&pointTemp, pSprite);
						
					}
				}
			}
		}
	}
#ifdef OUTPUT_DEBUG_DRAW_PROCESS
DEBUG_ADD("dcOK");
#endif
}

void	MTopView::DrawAdvancementClassSlayerCharacter( POINT *pPoint, MCreature* pCreature, int action, int direction, int frame, int FrameIndex )
{
	//add by viva
	int action_viva = action;
	if(pCreature->HasEffectStatus(EFFECTSTATUS_INSTALL_TURRET))
		DrawInstallTurret( pPoint, pCreature, action, direction, frame, FrameIndex );
	else
	{
		// ToT ì‹œê°„ì—†ë‹¤.. í•˜ë“œ ì½”ë”©.. by sonee
		action = ConvAdvancementSlayerActionFromSlayerAction( action, dynamic_cast< MCreatureWear* >(pCreature) );
		
		if( action == -1 )
			return;
		else
			action -= ADVANCEMENT_ACTION_START;
		
		
		MCreatureWear*	pCreatureWear = (MCreatureWear*)pCreature;
		
		// Creatureì˜ Actionì— ë§žëŠ” add-onì„ ì¶œë ¥í•œë‹¤.
		//action = pCreature->GetAction();
		
		int clothes;
		BYTE clothesType;
		
		//------------------------------------------------------------
		// Fade Out ì¶œë ¥
		//------------------------------------------------------------
		if (pCreature->HasEffectStatus( EFFECTSTATUS_FADE_OUT ))
		{
			DrawFadeOutForACSlayer( pPoint, pCreature, action_viva, direction, frame);	//by viva	modifyed	try
		}
		
		//------------------------------------------------------------
		// Fast Move ì¶œë ¥
		//------------------------------------------------------------
		if (pCreature->IsFastMove())
		{
			DrawFastMoveForACSlayer( pPoint, pCreature, action_viva, direction, frame );	//by viva	modifyed try		
		}
		//------------------------------------------------------------
		// Invisible  - SlayerëŠ” snipping modeì—ì„œ íˆ¬ëª…ì´ë‹¤.
		//------------------------------------------------------------
		else if (pCreature->IsInvisible())
		{
			DrawInvisibleForACSlayer( pPoint, pCreature, action_viva, direction, frame );		//by viva	modifyed try
		}
		//------------------------------------------------------------
		// ë¬´ê¸° ìž”ìƒ ê¸°ìˆ 
		//------------------------------------------------------------
		else if (
			(pCreature->HasEffectStatus( EFFECTSTATUS_DANCING_SWORD )
			|| pCreature->HasEffectStatus( EFFECTSTATUS_GHOST_BLADE )
			|| pCreature->HasEffectStatus( EFFECTSTATUS_REDIANCE ) && pCreature->GetAction() == ACTION_STAND
			)
			)
		{
			DrawWeaponFadeOutForACSlayer( pPoint, pCreature, action_viva, direction, frame );	//by viva	modifyed try
		}
		//------------------------------------------------------------
		// ì¼ë°˜ì ì¸ ì¶œë ¥
		//------------------------------------------------------------
		else
		{
			//by viva AdvancemnetSlayer
			action_viva = ConvNewSlayerActionFromSlayerAction(action_viva, dynamic_cast< MCreatureWear* >(pCreature));
//			if(action_viva>32)	
//				action_viva=36;
			
			CCreatureFramePack& slayerFPK = pCreature->IsMale() ? m_AdvancementSlayerManFPK : m_AdvancementSlayerWomanFPK;
			CIndexSpritePack& addonISPK = pCreature->IsMale() ? m_AdvancementSlayerManSPK : m_AdvancementSlayerWomanSPK;
			
			for( int i = 0; i < AC_ADDON_MAX; ++i )
			{
				clothesType = MCreatureWear::s_ACAddonOrder[ direction ][ i ];
				
				const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetACAddonInfo( clothesType );
				//if(addonInfo.ItemClass==ITEM_CLASS_MOTORCYCLE) break;
				if( addonInfo.bAddon )
				{	
					if(addonInfo.ItemClass==ITEM_CLASS_COAT && addonInfo.ItemType>=28			//11
						||	addonInfo.ItemClass==ITEM_CLASS_SWORD && addonInfo.ItemType>=16		//14
						||	addonInfo.ItemClass==ITEM_CLASS_BLADE && addonInfo.ItemType>=16		//15
						||	addonInfo.ItemClass==ITEM_CLASS_SHIELD && addonInfo.ItemType>=15	//16
						||  addonInfo.ItemClass==ITEM_CLASS_CROSS && addonInfo.ItemType>=14		//17
						||	addonInfo.ItemClass==ITEM_CLASS_HELM && addonInfo.ItemType>=15		//19
						||  addonInfo.ItemClass==ITEM_CLASS_AR && addonInfo.ItemType>=16		//22
						||  addonInfo.ItemClass==ITEM_CLASS_SR && addonInfo.ItemType>=16		//23
						||  addonInfo.ItemClass==ITEM_CLASS_MACE && addonInfo.ItemType>=14		//35
//						||	addonInfo.ItemClass==ITEM_CLASS_MOTORCYCLE	//1
						)
					{
						int k = 1;
						if(clothesType == AC_ADDON_BODY)	k = 2;
						for(int i=0;i<k;i++)
						{	
						//	if(addonInfo.ItemClass==ITEM_CLASS_MOTORCYCLE)
						//		clothes = 14;
						//	else
								clothes=addonInfo.FrameID + i;
							if( clothes == -1 ) continue;
							if( clothes > slayerFPK.GetSize() -1 ) continue;
							if(action_viva > slayerFPK[clothes].GetSize() -1) continue;
							try{
							FRAME_ARRAY& FA = slayerFPK[ clothes ][ action_viva ][ direction ];
							
							if( FA.GetSize() > frame )
							{
								CFrame &Frame = FA[frame];					
								int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
								int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
								int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
								
								CIndexSprite* pSprite = &addonISPK[ sprite ];
								pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
								pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
								
								//---------------------------------------- 
								// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
								//---------------------------------------- 	
								rect.left	= pointTemp.x;
								rect.top	= pointTemp.y;
								rect.right	= rect.left + pSprite->GetWidth();
								rect.bottom = rect.top + pSprite->GetHeight();
								pCreature->AddScreenRect( &rect );
								
								//---------------------------------------- 
								// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
								//---------------------------------------- 
								if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
								{
									m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
										addonInfo.EffectColorSet);
								}
								//---------------------------------------- 
								// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
								//---------------------------------------- 
								else
								{
									if( addonInfo.bAddon )
									{
										int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
										if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
										{
											colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
											if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
												colorSet1 = colorSet2;
										}
										CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
									}
									else
										CIndexSprite::SetUsingColorSet( 375, 375 );
									
									// ì–´ë‘¡ê²Œ ì°ê¸°
									if (pCreature->IsFade())
									{
										m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
									}
									else
									{
										m_pSurface->BltIndexSprite(&pointTemp, pSprite);
									}
								}						
							}
							}catch(...){}
						}
					}
					else
					{
						
						// ì§€ê¸ˆì€ íŒŒíŠ¸ ê·¸ëŒ€ë¡œë¥¼ ì‚¬ìš©í•œë‹¤. ì¶”í›„ì— ì˜·ì´ ì¶”ê°€ ë˜ì—ˆì„ë•Œ ë‹¤ìŒ ë³€ìˆ˜ë¥¼ ì„¸íŒ… í•´ì„œ ì“´ë‹¤.
						// addonInfo.FrameID ì— ì˜· ì •ë³´ë¥¼ ë„£ê³  ì„¸íŒ…í•˜ì—¬ ì“°ë©´ëœë‹¤.
						
						clothes = GetAdvancementPartFromItemClass( addonInfo.ItemClass, addonInfo.FrameID );
						
						if( clothes == -1 ) continue;
//						clothes =14;
						try{
						FRAME_ARRAY& FA = slayerFPK[ clothes ][ action ][ direction ];
						
						
						if( FA.GetSize() > frame )
						{
							CFrame &Frame = FA[frame];					
							int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
							int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
							int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
							
							CIndexSprite* pSprite = &addonISPK[ sprite ];
							pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
							pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
							
							//---------------------------------------- 
							// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
							//---------------------------------------- 	
							rect.left	= pointTemp.x;
							rect.top	= pointTemp.y;
							rect.right	= rect.left + pSprite->GetWidth();
							rect.bottom = rect.top + pSprite->GetHeight();
							pCreature->AddScreenRect( &rect );
							
							//---------------------------------------- 
							// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
							//---------------------------------------- 
							if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
							{
								m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, 
									addonInfo.EffectColorSet);
							}
							//---------------------------------------- 
							// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
							//---------------------------------------- 
							else
							{
								if( addonInfo.bAddon )
								{
									int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
									if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
									{
										colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
										if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
											colorSet1 = colorSet2;
									}
									CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
								}
								else
									CIndexSprite::SetUsingColorSet( 375, 375 );
								
								// ì–´ë‘¡ê²Œ ì°ê¸°
								if (pCreature->IsFade())
								{
									m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
								}
								else
								{
									m_pSurface->BltIndexSprite(&pointTemp, pSprite);
								}
							}						
						}
						}catch(...){}
					}
				}
			}
		}
	}
}

void	MTopView::DrawAdvancementClassVampireCharacter( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame, int body, int frameindex )
{
	//------------------------------------------------
	// ê´€ ì¶œë ¥ì •ë³´
	//------------------------------------------------
	int casketValue = pCreature->GetCasketCount();
	int creature_type = pCreature->GetCreatureType();
	bool bInCasket = pCreature->IsInCasket();
	bool bCasketOnly = (bInCasket && casketValue==0);

	action = GetAdvancementVampireActionFromVampireAction( action, pCreature );

	if( action == -1 )
		return;
	else
		action -= ADVANCEMENT_ACTION_START;
//add by viva

	//CCreatureFramePack& advanceVampireFPK = pCreature->IsMale() ? m_AdvancementVampireManFPK : m_AdvancementVampireWomanFPK;
	//CIndexSpritePack& advanceVampireSPK = pCreature->IsMale() ? m_AdvancementVampireManSPK : m_AdvancementVampireWomanSPK;
	CCreatureFramePack& advanceVampireFPK = (creature_type>=807&&creature_type<=812) ? m_AdvancementVampireManFPK : pCreature->IsMale() ? m_AdvancementVampireManFPK : m_AdvancementVampireWomanFPK;
	CIndexSpritePack& advanceVampireSPK = (creature_type>=807&&creature_type<=812) ? m_AdvancementVampireManSPK : pCreature->IsMale() ? m_AdvancementVampireManSPK : m_AdvancementVampireWomanSPK;

//end

	if(creature_type>=807&&creature_type<=812)
		action = GetNewVampireActionFromVampireAction(action);
	//----------------------------------------------------
	// ì™„ì „ížˆ ê´€ë§Œ ì¶œë ¥í•˜ëŠ” ê²½ìš°ëŠ” ëª¸ ì¶œë ¥ì„ ì•ˆí•œë‹¤.
	//----------------------------------------------------
	if (!bCasketOnly)
	{
		//add by viva
		int viva_body=0;
		if(creature_type==807)
			viva_body=2;
		else if(creature_type==808)
			viva_body=3;
		else if(creature_type==809)
			viva_body=4;
		else if(creature_type==810)
			viva_body=5;
		else if(creature_type==811)
			viva_body=6;
		else if(creature_type==812)
			viva_body=7;
		else
			viva_body=0;

		if(action>advanceVampireFPK[viva_body].GetSize()-1) return;
		FRAME_ARRAY& FA = advanceVampireFPK[viva_body][action][direction];
	
		//end
		
		if (FA.GetSize() > frame)
		{
			CFrame& Frame =	FA[frame];
			int sprite = Frame.GetSpriteID(),	//m_CreatureFPK[body][action][direction][frame].GetSpriteID(),
				cx		= Frame.GetCX(),	//m_CreatureFPK[body][action][direction][frame].GetCX(),
				cy		= Frame.GetCY();	//m_CreatureFPK[body][action][direction][frame].GetCY();
			
			// ì¢Œí‘œ ë³´ì •
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			CIndexSprite* pSprite = &advanceVampireSPK[ sprite ];
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			if(frameindex==0 )
				pCreature->SetScreenRect( &rect );
			//-----------------------------------------------------------
			// CutHeight
			//-----------------------------------------------------------
			bool bCutHeight = pCreature->IsCutHeight();
			int previousClipBottom;
			if (bCutHeight)
			{
				previousClipBottom = rect.bottom; // SDL2: Clipping is automatic

				pointTemp.y += pCreature->GetCutHeightCount() - 24;
			}
			
			//-----------------------------------------------------------
			// íˆ¬ëª… ìƒíƒœ ì¶œë ¥ 			
			//-----------------------------------------------------------
			// vampire
			if (pCreature->IsFakeDie())
			{
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				else
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int netValue = pCreature->GetFakeDieCount()/5;
					
					CSpriteSurface::s_Value1 = netValue;
					
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_NET );
					CIndexSprite::SetEffect( CIndexSprite::EFFECT_NET );
					
					m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					
				}
			}
			else if (pCreature->IsInvisible())
			{
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					// IndexSpriteì—ë„ EFFECT...ë„£ê³ ...
					// ìŠ¤ë¥´ë¥µ~ ë‹¨ê³„ì ìœ¼ë¡œ ë‚˜íƒ€ë‚˜ê²Œ/ì—†ì–´ì§€ê²Œ í•´ì•¼ë¨.. - -;
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				else
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int wipeValue = pCreature->GetInvisibleCount();
					
					if (wipeValue==0)
					{
						m_pSurface->BltIndexSprite(&pointTemp, pSprite);							
					}
					else if (wipeValue==64)
					{
						CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
						CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
						
						m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					}
					else
					{
						CSpriteSurface::s_Value1 = wipeValue;
						
						CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
						CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
						
						m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
					}							
				}
			}					
			else
			{	
				//-----------------------------------------------------------
				// ë¹ ë¥´ê²Œ ì›€ì§ì´ëŠ” ì¤‘
				//-----------------------------------------------------------
				if (pCreature->IsFastMove() || pCreature->IsBloodyZenith())
				{
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					POINT pointTemp2;
					
					int fastMoveShadowMax = 2;
					if(pCreature->IsBloodyZenith())
					{
						fastMoveShadowMax = pCreature->GetBloodyZenithCount();
						pCreature->UpdateBloodyZenith();
					}
					for(int fastMoveShadow = fastMoveShadowMax; fastMoveShadow > 0; fastMoveShadow--)
					{
						// 1 pixel skip
						pointTemp2.x = pointTemp.x - (pCreature->GetCX() >> 1)*fastMoveShadow;
						pointTemp2.y = pointTemp.y - (pCreature->GetCY() >> 1)*fastMoveShadow;
						if(pCreature->IsBloodyZenith())
						{
							pointTemp2.y -= sin((double)max(0, pCreature->GetMoveCount()-fastMoveShadow)*3.141592/(double)pCreature->GetMoveCountMax())*100;
						}
						
						m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite, min(2, fastMoveShadow));
					}
					
				}
				
				if(pCreature->IsBloodyZenith())
				{
					pointTemp.y -= sin((double)pCreature->GetMoveCount()*3.141592/(double)pCreature->GetMoveCountMax())*100;
				}
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
				}
				//---------------------------------------- 
				// ì •ìƒì ì¸ ìƒ‰ê¹” ì¶œë ¥
				//---------------------------------------- 					
				else
				{				
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if( colorSet1 == QUEST_ITEM_COLOR || colorSet1 == UNIQUE_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					
					if( colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					int shadowCount = pCreature->GetShadowCount();
					
					//----------------------------------------
					// Shadowì¶œë ¥
					//----------------------------------------
					if (shadowCount)
					{
						bool bDrawShadowFirst = (direction<=4) || creature_type == 764 ||  creature_type == 765;
						if (!bDrawShadowFirst)
						{
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							}
						}
						
						// 0, 1, 2, 3, 4, 5
						const int darkbits[6] =
						{
							0, 1, 1, 2, 2, 3
						};
						
						POINT pointTemp2;
						
						// ì´ë™ì´ë©´ ì¢Œí‘œë„ ë°”ë€ë‹¤
						int cxStep = 0, cyStep = 0, cxInc =0 , cyInc = 0;
						if (action==ACTION_MOVE)
						{
							cxInc = -pCreature->GetCX();
							cyInc = -pCreature->GetCY();
						}
						
						
						int faNum = FA.GetSize();
						for (int f=1; f<=shadowCount; f++)
						{
							int frame_f = frame-f;
							
							//---------------------------------------- 
							// f frame ì „
							//---------------------------------------- 
							if (frame_f < 0)
							{
								frame_f += faNum;
							}
							
							CFrame &Frame_f = FA[frame_f];
							int sprite_f	= Frame_f.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
							
							CIndexSprite* pSprite_f = &m_CreatureSPK[ sprite_f ];
							
							int cx_f		= Frame_f.GetCX();
							int cy_f		= Frame_f.GetCY();
							
							pointTemp2.x = pPoint->x + cx_f + cxStep;
							pointTemp2.y = pPoint->y + cy_f + cyStep;
							cxStep += cxInc;
							cyStep += cyInc;
							
							m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite_f, darkbits[f]);																		
						}
						
						if (bDrawShadowFirst)
						{
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							}
						}
					}
					//----------------------------------------
					// ëª¸ë§Œ ì°ê¸°
					//----------------------------------------
					else
					{
						// ì–´ë‘¡ê²Œ ì°ê¸°
						if (pCreature->IsFade())
						{
							m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
						}
						else
						{
							m_pSurface->BltIndexSprite(&pointTemp, pSprite);
							
						}
					}
				}
			}

			if (bCutHeight)
			{
				// SDL2: Clipping is automatic - no restore needed
			}

			// 2004, 10, 28, sobeit add start  - ëª¬ìŠ¤í„° í‚¬ í€˜ìŠ¤íŠ¸ í•´ë‹¹ ëª¬ìŠ¤í„°ì— í‘œì‹œ.
			if(pCreature->IsAlive() && (g_CurrentFrame&0x04)&&UI_IsMonsterKillQuest_Monster((*g_pCreatureTable)[pCreature->GetCreatureType()].SpriteTypes[0]))
			//if(pCreature->IsAlive() && (g_CurrentFrame&0x04))
			{
				UI_ShowTargetArrow(pPoint->x + 12 ,pPoint->y- pCreature->GetHeight()-20);
			}
		// 2004, 10, 28, sobeit add end
		}

		bool bWeapon = true;

		bool bBat = pCreature->GetCreatureType() == CREATURETYPE_BAT;
		bool bWolf = pCreature->GetCreatureType() == CREATURETYPE_WOLF;
		bool bWerWolf = pCreature->GetCreatureType() == CREATURETYPE_WER_WOLF;

		if( bWeapon && !bBat && !bWolf && !bWerWolf )
		{

			FRAME_ARRAY& FAWEAPON = advanceVampireFPK[1][action][direction];
			if (FAWEAPON.GetSize() > frame)
			{
				CFrame& Frame =	FAWEAPON[frame];
				int sprite = Frame.GetSpriteID(),	//m_CreatureFPK[body][action][direction][frame].GetSpriteID(),
					cx		= Frame.GetCX(),	//m_CreatureFPK[body][action][direction][frame].GetCX(),
					cy		= Frame.GetCY();	//m_CreatureFPK[body][action][direction][frame].GetCY();
				
				// ì¢Œí‘œ ë³´ì •
				pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
				pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
				
				CIndexSprite* pSprite = &advanceVampireSPK[ sprite ];
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
//				if(frameindex==0 )
//					pCreature->SetScreenRect( &rect );
				//-----------------------------------------------------------
				// CutHeight
				//-----------------------------------------------------------
				bool bCutHeight = pCreature->IsCutHeight();
int previousClipBottom;
			if (bCutHeight)
			{
				// SDL2: Clipping is automatic in SDL backend
				// previousClipBottom = m_pSurface->GetClipBottom();
				// m_pSurface->SetClipBottom( rect.bottom - 24 );
				previousClipBottom = rect.bottom;

				pointTemp.y += pCreature->GetCutHeightCount() - 24;
			}
				
				//-----------------------------------------------------------
				// íˆ¬ëª… ìƒíƒœ ì¶œë ¥ 			
				//-----------------------------------------------------------
				// vampire
				if (pCreature->IsFakeDie())
				{
					WORD colorSet = pCreature->GetAttachEffectColor();
					
					//---------------------------------------- 
					// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
					//---------------------------------------- 
					if (colorSet < MAX_COLORSET)
					{
						m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
					}
					else
					{
						int netValue = pCreature->GetFakeDieCount()/5;
						
						CSpriteSurface::s_Value1 = netValue;
						
						CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_NET );
						CIndexSprite::SetEffect( CIndexSprite::EFFECT_NET );
						
						m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
						
					}
				}
				else if (pCreature->IsInvisible())
				{
					WORD colorSet = pCreature->GetAttachEffectColor();
					
					if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
						colorSet = pCreature->GetBatColor();
					
					//---------------------------------------- 
					// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
					//---------------------------------------- 
					if (colorSet < MAX_COLORSET)
					{
						// IndexSpriteì—ë„ EFFECT...ë„£ê³ ...
						// ìŠ¤ë¥´ë¥µ~ ë‹¨ê³„ì ìœ¼ë¡œ ë‚˜íƒ€ë‚˜ê²Œ/ì—†ì–´ì§€ê²Œ í•´ì•¼ë¨.. - -;
						m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
					}
					else
					{
						int wipeValue = pCreature->GetInvisibleCount();
						
						if (wipeValue==0)
						{
							m_pSurface->BltIndexSprite(&pointTemp, pSprite);							
						}
						else if (wipeValue==64)
						{
							CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
							CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
							
							m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
						}
						else
						{
							CSpriteSurface::s_Value1 = wipeValue;
							
							CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
							CIndexSprite::SetEffect( CIndexSprite::EFFECT_WIPE_OUT );
							
							m_pSurface->BltIndexSpriteEffect(&pointTemp, pSprite);
						}							
					}
				}					
				else
				{	
					//-----------------------------------------------------------
					// ë¹ ë¥´ê²Œ ì›€ì§ì´ëŠ” ì¤‘
					//-----------------------------------------------------------
					if (pCreature->IsFastMove() || pCreature->IsBloodyZenith())
					{
						POINT pointTemp2;
						
						int fastMoveShadowMax = 2;
						if(pCreature->IsBloodyZenith())
						{
							fastMoveShadowMax = pCreature->GetBloodyZenithCount();
							pCreature->UpdateBloodyZenith();
						}
						for(int fastMoveShadow = fastMoveShadowMax; fastMoveShadow > 0; fastMoveShadow--)
						{
							// 1 pixel skip
							pointTemp2.x = pointTemp.x - (pCreature->GetCX() >> 1)*fastMoveShadow;
							pointTemp2.y = pointTemp.y - (pCreature->GetCY() >> 1)*fastMoveShadow;
							if(pCreature->IsBloodyZenith())
							{
								pointTemp2.y -= sin((double)max(0, pCreature->GetMoveCount()-fastMoveShadow)*3.141592/(double)pCreature->GetMoveCountMax())*100;
							}
							
							m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite, min(2, fastMoveShadow));
						}
						
					}
					
					if(pCreature->IsBloodyZenith())
					{
						pointTemp.y -= sin((double)pCreature->GetMoveCount()*3.141592/(double)pCreature->GetMoveCountMax())*100;
					}
					WORD colorSet = pCreature->GetAttachEffectColor();
					
					//---------------------------------------- 
					// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°					
					//---------------------------------------- 
					if (colorSet < MAX_COLORSET)
					{
						m_pSurface->BltIndexSpriteColorSet(&pointTemp, pSprite, colorSet);
					}
					//---------------------------------------- 
					// ì •ìƒì ì¸ ìƒ‰ê¹” ì¶œë ¥
					//---------------------------------------- 					
					else
					{				
						int shadowCount = pCreature->GetShadowCount();
						
						//----------------------------------------
						// Shadowì¶œë ¥
						//----------------------------------------
						if (shadowCount)
						{
							bool bDrawShadowFirst = (direction<=4) || creature_type == 764 ||  creature_type == 765;
							if (!bDrawShadowFirst)
							{
								if (pCreature->IsFade())
								{
									m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
								}
								else
								{
									m_pSurface->BltIndexSprite(&pointTemp, pSprite);
								}
							}
							
							// 0, 1, 2, 3, 4, 5
							const int darkbits[6] =
							{
								0, 1, 1, 2, 2, 3
							};
							
							POINT pointTemp2;
							
							// ì´ë™ì´ë©´ ì¢Œí‘œë„ ë°”ë€ë‹¤
							int cxStep = 0, cyStep = 0, cxInc =0 , cyInc = 0;
							if (action==ACTION_MOVE)
							{
								cxInc = -pCreature->GetCX();
								cyInc = -pCreature->GetCY();
							}
							
							
							int faNum = FA.GetSize();
							for (int f=1; f<=shadowCount; f++)
							{
								int frame_f = frame-f;
								
								//---------------------------------------- 
								// f frame ì „
								//---------------------------------------- 
								if (frame_f < 0)
								{
									frame_f += faNum;
								}
								
								CFrame &Frame_f = FA[frame_f];
								int sprite_f	= Frame_f.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
								
								CIndexSprite* pSprite_f = &m_CreatureSPK[ sprite_f ];
								
								int cx_f		= Frame_f.GetCX();
								int cy_f		= Frame_f.GetCY();
								
								pointTemp2.x = pPoint->x + cx_f + cxStep;
								pointTemp2.y = pPoint->y + cy_f + cyStep;
								cxStep += cxInc;
								cyStep += cyInc;
								
								m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite_f, darkbits[f]);																		
							}
							
							if (bDrawShadowFirst)
							{
								if (pCreature->IsFade())
								{
									m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
								}
								else
								{
									m_pSurface->BltIndexSprite(&pointTemp, pSprite);
								}
							}
						}
						//----------------------------------------
						// ëª¸ë§Œ ì°ê¸°
						//----------------------------------------
						else
						{
							// ì–´ë‘¡ê²Œ ì°ê¸°
							if (pCreature->IsFade())
							{
								m_pSurface->BltIndexSpriteDarkness(&pointTemp, pSprite, 1);
							}
							else
							{
								m_pSurface->BltIndexSprite(&pointTemp, pSprite);
								
							}
						}
					}
				}
			}
		}
	}
		
	//----------------------------------------------------
	// ê´€ ì¶œë ¥
	//----------------------------------------------------
	if (bInCasket)
	{
		// ê·¸ë¦¼ìž ë•Œë¬¸ì— *2í•´ì•¼í•œë‹¤.
		int casketID = SPRITEID_CASKET_1 + pCreature->GetCasketType()*2;
		
		if (casketID < m_EtcSPK.GetSize())
		{
			CSprite* pSprite = &m_EtcSPK[casketID];
			
			if (pSprite!=NULL)
			{
				// tileì˜ ì¤‘ì‹¬ì—ì„œ ì„¸ìš´ë‹¤.
				pointTemp.x = pPoint->x + 24 - (pSprite->GetWidth()>>1);
				pointTemp.y = pPoint->y + TILE_Y - pSprite->GetHeight();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				if(frameindex==0 )
					pCreature->SetScreenRect( &rect );
				
				if (casketValue==0)
				{
					m_pSurface->BltSprite(&pointTemp, pSprite);						
				}
				else if (casketValue==64)
				{
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
					
					m_pSurface->BltSpriteEffect(&pointTemp, pSprite);
				}
				else
				{
					CSpriteSurface::s_Value1 = casketValue;
					
					CSpriteSurface::SetEffect( CSpriteSurface::EFFECT_WIPE_OUT );
					
					m_pSurface->BltSpriteEffect(&pointTemp, pSprite);
				}
			}
		}
	}
}

void	
MTopView::DrawSelectedVampireCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame, int body, int frameindex )
{
	//------------------------------------------------
	// ê´€ ì¶œë ¥ì •ë³´
	//------------------------------------------------
	int casketValue = pCreature->GetCasketCount();
	bool bInCasket = pCreature->IsInCasket();
	bool bCasketOnly = (bInCasket && casketValue==0);
	
	if (!bCasketOnly)
	{
		FRAME_ARRAY& FA = m_CreatureFPK[body][action][direction];
		
		if (FA.GetSize() > frame)
		{			
			CFrame&	Frame = FA[frame];
			
			int sprite = Frame.GetSpriteID(),	//m_CreatureFPK[body][action][direction][frame].GetSpriteID(),
				cx		= Frame.GetCX(),	//m_CreatureFPK[body][action][direction][frame].GetCX(),
				cy		= Frame.GetCY();	//m_CreatureFPK[body][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_CreatureSPK[ sprite ];
			
			//					if (pSprite->IsNotInit())
			//					{
			//						LoadFromFileCreatureActionSPK( body, action );
			//					}
			
			// ì¢Œí‘œ ë³´ì •
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();			
			
			//---------------------------------------- 		
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			if(frameindex ==0 )
				pCreature->SetScreenRect( &rect );
			
			WORD colorSet = pCreature->GetAttachEffectColor();
			
			if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
				colorSet = pCreature->GetBatColor();
			
			//---------------------------------------- 
			// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (colorSet < MAX_COLORSET)
			{
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, colorSet );
			}					
			//---------------------------------------- 
			// ì •ìƒì ì¸ ì¶œë ¥
			//---------------------------------------- 										
			else
			{					
				int colorSet1 = pCreature->GetBodyColor1();
				int colorSet2 = pCreature->GetBodyColor2();
				
				if ( colorSet1 == UNIQUE_ITEM_COLOR || colorSet1 == QUEST_ITEM_COLOR )
					colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
				if ( colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR )
					colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
				
				CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
				
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
			}
			
			
			int shadowCount = pCreature->GetShadowCount();
			
			//----------------------------------------
			// Shadowì¶œë ¥
			//----------------------------------------
			if (shadowCount)
			{
				// 0, 1, 2, 3, 4, 5
				const int darkbits[6] =
				{
					0, 1, 1, 2, 2, 3
				};
				
				POINT pointTemp2;
				
				// ì´ë™ì´ë©´ ì¢Œí‘œë„ ë°”ë€ë‹¤
				int cxStep = 0, cyStep = 0, cxInc =0 , cyInc = 0;
				if (action==ACTION_MOVE)
				{
					cxInc = -pCreature->GetCX();
					cyInc = -pCreature->GetCY();
				}
				
				
				int faNum = FA.GetSize();
				for (int f=1; f<=shadowCount; f++)
				{
					int frame_f = frame-f;
					
					//---------------------------------------- 
					// f frame ì „
					//---------------------------------------- 
					if (frame_f < 0)
					{
						frame_f += faNum;
					}
					
					CFrame &Frame_f = FA[frame_f];
					int sprite_f	= Frame_f.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
					
					CIndexSprite* pSprite_f = &m_CreatureSPK[ sprite_f ];
					
					//							if (pSprite_f->IsNotInit())
					//							{	
					//								LoadFromFileCreatureActionSPK(body, action);
					//							}
					
					int cx_f		= Frame_f.GetCX();
					int cy_f		= Frame_f.GetCY();
					
					pointTemp2.x = pPoint->x + cx_f + cxStep;
					pointTemp2.y = pPoint->y + cy_f + cyStep;
					cxStep += cxInc;
					cyStep += cyInc;
					
					m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite_f, darkbits[f]);																
				}
			}	// shadowcount			
		}
	}

	//----------------------------------------------------
	// ê´€ ì¶œë ¥
	//----------------------------------------------------
	if (bInCasket)
	{
		// ê·¸ë¦¼ìž ë•Œë¬¸ì— *2í•œë‹¤.
		int casketID = SPRITEID_CASKET_1 + pCreature->GetCasketType()*2;
		
		if (casketID < m_EtcSPK.GetSize())
		{
			CSprite* pSprite = &m_EtcSPK[casketID];
			
			if (pSprite!=NULL)
			{
				// tileì˜ ì¤‘ì‹¬ì—ì„œ ì„¸ìš´ë‹¤.
				pointTemp.x = pPoint->x + 24 - (pSprite->GetWidth()>>1);
				pointTemp.y = pPoint->y + TILE_Y - pSprite->GetHeight();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				if(frameindex ==0 )
					pCreature->SetScreenRect( &rect );
				
				//---------------------------------------- 	
				// ë¬´ì¡°ê±´ ë‹¤ ì¶œë ¥
				//---------------------------------------- 	
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );						
			}
		}
	}
}

void	MTopView::DrawSelectedAdvancementVampireCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame, int body, int frameindex )
{
		//------------------------------------------------
	// ê´€ ì¶œë ¥ì •ë³´
	//------------------------------------------------
	int casketValue = pCreature->GetCasketCount();
	bool bInCasket = pCreature->IsInCasket();
	bool bCasketOnly = (bInCasket && casketValue==0);
	int creature_type = pCreature->GetCreatureType();	//add by viva
	action = GetAdvancementVampireActionFromVampireAction( action, pCreature );

	if( action == -1 )
		return;
	else
		action -= ADVANCEMENT_ACTION_START;
//by viva selected vampire
	//CCreatureFramePack& advanceVampireFPK = pCreature->IsMale() ? m_AdvancementVampireManFPK : m_AdvancementVampireWomanFPK;
	//CIndexSpritePack& advanceVampireSPK = pCreature->IsMale() ? m_AdvancementVampireManSPK : m_AdvancementVampireWomanSPK;	
	
	CCreatureFramePack& advanceVampireFPK = creature_type>=807&&creature_type<=812 ? m_AdvancementVampireManFPK : pCreature->IsMale() ? m_AdvancementVampireManFPK : m_AdvancementVampireWomanFPK;
	CIndexSpritePack& advanceVampireSPK = (creature_type>=807&&creature_type<=812) ? m_AdvancementVampireManSPK : pCreature->IsMale() ? m_AdvancementVampireManSPK : m_AdvancementVampireWomanSPK;

//end

	//----------------------------------------------------
	// ì™„ì „ížˆ ê´€ë§Œ ì¶œë ¥í•˜ëŠ” ê²½ìš°ëŠ” ëª¸ ì¶œë ¥ì„ ì•ˆí•œë‹¤.
	//----------------------------------------------------

	if (!bCasketOnly)
	{	
		for( int i = body; i <= body+1; ++i )
		{
			// 0:body 1: ë¬´ê¸° ì°©íƒˆì— ë”°ë¼ ë‚˜ì¤‘ì— ìˆ˜ì • 

			//add by viva	select vampire
			int viva_body=i;
			if(creature_type==807)
				viva_body=2;
			else if(creature_type==808)
				viva_body=3;
			else if(creature_type==809)
				viva_body=4;
			else if(creature_type==810)
				viva_body=5;
			else if(creature_type==811)
				viva_body=6;
			else if(creature_type==812)
				viva_body=7;
			else
				viva_body=i;

			FRAME_ARRAY& FA = advanceVampireFPK[viva_body][action][direction];
		//	FRAME_ARRAY& FA = advanceVampireFPK[i][action][direction];
			//end
			if (FA.GetSize() > frame)
			{			
				CFrame&	Frame = FA[frame];
				
				int sprite = Frame.GetSpriteID(),	//m_CreatureFPK[body][action][direction][frame].GetSpriteID(),
					cx		= Frame.GetCX(),	//m_CreatureFPK[body][action][direction][frame].GetCX(),
					cy		= Frame.GetCY();	//m_CreatureFPK[body][action][direction][frame].GetCY();
				
				CIndexSprite* pSprite = &advanceVampireSPK[ sprite ];
				
				//					if (pSprite->IsNotInit())
				//					{
				//						LoadFromFileCreatureActionSPK( body, action );
				//					}
				
				// ì¢Œí‘œ ë³´ì •
				pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
				pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();			
				
				//---------------------------------------- 		
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				if(frameindex ==0 && i == body)
					pCreature->SetScreenRect( &rect );
				
				WORD colorSet = pCreature->GetAttachEffectColor();
				
				if( pCreature->GetCreatureType() == CREATURETYPE_BAT && pCreature->GetBatColor() != 0xFFFF)
					colorSet = pCreature->GetBatColor();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì „ì²´ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
				//---------------------------------------- 
				if (colorSet < MAX_COLORSET)
				{
					m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, colorSet );
				}					
				//---------------------------------------- 
				// ì •ìƒì ì¸ ì¶œë ¥
				//---------------------------------------- 										
				else
				{					
					int colorSet1 = pCreature->GetBodyColor1();
					int colorSet2 = pCreature->GetBodyColor2();
					
					if ( colorSet1 == UNIQUE_ITEM_COLOR || colorSet1 == QUEST_ITEM_COLOR )
						colorSet1 = MItem::GetSpecialColorItemColorset( colorSet1 );
					if ( colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR )
						colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
					
					CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
					
					m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
				}
				
				
				int shadowCount = pCreature->GetShadowCount();
				
				//----------------------------------------
				// Shadowì¶œë ¥
				//----------------------------------------
				if (shadowCount)
				{
					// 0, 1, 2, 3, 4, 5
					const int darkbits[6] =
					{
						0, 1, 1, 2, 2, 3
					};
					
					POINT pointTemp2;
					
					// ì´ë™ì´ë©´ ì¢Œí‘œë„ ë°”ë€ë‹¤
					int cxStep = 0, cyStep = 0, cxInc =0 , cyInc = 0;
					if (action==ACTION_MOVE)
					{
						cxInc = -pCreature->GetCX();
						cyInc = -pCreature->GetCY();
					}
					
					
					int faNum = FA.GetSize();
					for (int f=1; f<=shadowCount; f++)
					{
						int frame_f = frame-f;
						
						//---------------------------------------- 
						// f frame ì „
						//---------------------------------------- 
						if (frame_f < 0)
						{
							frame_f += faNum;
						}
						
						CFrame &Frame_f = FA[frame_f];
						int sprite_f	= Frame_f.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
						
						CIndexSprite* pSprite_f = &advanceVampireSPK[ sprite_f ];
						
						//							if (pSprite_f->IsNotInit())
						//							{	
						//								LoadFromFileCreatureActionSPK(body, action);
						//							}
						
						int cx_f		= Frame_f.GetCX();
						int cy_f		= Frame_f.GetCY();
						
						pointTemp2.x = pPoint->x + cx_f + cxStep;
						pointTemp2.y = pPoint->y + cy_f + cyStep;
						cxStep += cxInc;
						cyStep += cyInc;
						
						m_pSurface->BltIndexSpriteDarkness(&pointTemp2, pSprite_f, darkbits[f]);																
					}
				}	// shadowcount			
			}
		}
	}

	//----------------------------------------------------
	// ê´€ ì¶œë ¥
	//----------------------------------------------------
	if (bInCasket)
	{
		// ê·¸ë¦¼ìž ë•Œë¬¸ì— *2í•œë‹¤.
		int casketID = SPRITEID_CASKET_1 + pCreature->GetCasketType()*2;
		
		if (casketID < m_EtcSPK.GetSize())
		{
			CSprite* pSprite = &m_EtcSPK[casketID];
			
			if (pSprite!=NULL)
			{
				// tileì˜ ì¤‘ì‹¬ì—ì„œ ì„¸ìš´ë‹¤.
				pointTemp.x = pPoint->x + 24 - (pSprite->GetWidth()>>1);
				pointTemp.y = pPoint->y + TILE_Y - pSprite->GetHeight();
				
				//---------------------------------------- 
				// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
				//---------------------------------------- 	
				rect.left	= pointTemp.x;
				rect.top	= pointTemp.y;
				rect.right	= rect.left + pSprite->GetWidth();
				rect.bottom = rect.top + pSprite->GetHeight();
				if(frameindex ==0 )
					pCreature->SetScreenRect( &rect );
				
				//---------------------------------------- 	
				// ë¬´ì¡°ê±´ ë‹¤ ì¶œë ¥
				//---------------------------------------- 	
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );						
			}
		}
	}
}

void	MTopView::DrawSelectedAdvancementSlayerCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	int action_viva=action;
	action = ConvAdvancementSlayerActionFromSlayerAction( action, dynamic_cast< MCreatureWear* >(pCreature) );
	
	if( action == -1 )
		return;
	else
		action -= ADVANCEMENT_ACTION_START;
	
	MCreatureWear*	pCreatureWear = (MCreatureWear*)pCreature;
	// Creatureì˜ Actionì— ë§žëŠ” add-onì„ ì¶œë ¥í•œë‹¤.
	//action = pCreature->GetAction();	
	int clothes;
	BYTE clothesType;
	
	//by viva AdvancemnetSlayer
	action_viva = ConvNewSlayerActionFromSlayerAction(action_viva, dynamic_cast< MCreatureWear* >(pCreature));
	
	
	CCreatureFramePack& slayerFPK = pCreature->IsMale() ? m_AdvancementSlayerManFPK : m_AdvancementSlayerWomanFPK;
	CIndexSpritePack& addonISPK = pCreature->IsMale() ? m_AdvancementSlayerManSPK : m_AdvancementSlayerWomanSPK;
	
	for( int i = 0; i < AC_ADDON_MAX; ++i )
	{
		clothesType = MCreatureWear::s_ACAddonOrder[ direction ][ i ];
		
		const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetACAddonInfo( clothesType );
		
		if( addonInfo.bAddon )
		{	
			if(addonInfo.ItemClass==ITEM_CLASS_COAT && addonInfo.ItemType>=28			//11
				||	addonInfo.ItemClass==ITEM_CLASS_SWORD && addonInfo.ItemType>=16		//14
				||	addonInfo.ItemClass==ITEM_CLASS_BLADE && addonInfo.ItemType>=16		//15
				||	addonInfo.ItemClass==ITEM_CLASS_SHIELD && addonInfo.ItemType>=15	//16
				||  addonInfo.ItemClass==ITEM_CLASS_CROSS && addonInfo.ItemType>=14		//17
				||	addonInfo.ItemClass==ITEM_CLASS_HELM && addonInfo.ItemType>=15		//19
				||  addonInfo.ItemClass==ITEM_CLASS_AR && addonInfo.ItemType>=16		//22
				||  addonInfo.ItemClass==ITEM_CLASS_SR && addonInfo.ItemType>=16		//23
				||  addonInfo.ItemClass==ITEM_CLASS_MACE && addonInfo.ItemType>=14		//35
				)
			{
				int k = 1;
				if(clothesType == AC_ADDON_BODY)	k = 2;
				for(int i=0;i<k;i++)
				{							
					clothes=addonInfo.FrameID + i;
					if( clothes == -1 ) continue;
					if( clothes > slayerFPK.GetSize() -1 ) continue;
					if(action_viva > slayerFPK[clothes].GetSize() -1) continue;
					
					FRAME_ARRAY& FA = slayerFPK[ clothes ][ action_viva ][ direction ];
					// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
					if (FA.GetSize() > frame)
					{
						CFrame &Frame = FA[frame];					
						int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
						int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
						int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
						
						pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
						pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
						
						CIndexSprite* pSprite = &addonISPK[ sprite ];
						
						// ë³µìž¥Spriteê°€ ì´ˆê¸°í™” ë˜ì§€ ì•Šì€ ê²½ìš°
						//						if (pSprite->IsNotInit())
						//						{
						//							LoadFromFileAddonSPK( clothes, action );
						//							//continue;
						//						}
						
						//---------------------------------------- 
						// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
						//---------------------------------------- 	
						rect.left	= pointTemp.x;
						rect.top	= pointTemp.y;
						rect.right	= rect.left + pSprite->GetWidth();
						rect.bottom = rect.top + pSprite->GetHeight();
						pCreature->AddScreenRect( &rect );
						
						// SpriteOutlineManagerì— ì¶”ê°€
						//CIndexSprite::SetUsingColorSet((pCreature->GetID()+3*(clothesType,1),7)%MAX_COLORSET, ((pCreature->GetID()*pCreature->GetID(),7*(clothesType+2)))%MAX_COLORSET);
						//CIndexSprite::SetUsingColorSet((c1+3*(clothesType+1)+7)%MAX_COLORSET, ((c2*c2+7*(clothesType+2)))%MAX_COLORSET);
						//---------------------------------------- 
						// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
						//---------------------------------------- 
						if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
						{
							m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
						}
						//---------------------------------------- 
						// ì›ëž˜ìƒ‰ëŒ€ë¡œ..
						//---------------------------------------- 
						else
						{
							int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
							if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
							{
								if(colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
									colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
								
								if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
									colorSet1 = colorSet2;
							}							
							
							CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
							m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
						}
					}
					
				}
			}
			else
			{		
				// ì§€ê¸ˆì€ íŒŒíŠ¸ ê·¸ëŒ€ë¡œë¥¼ ì‚¬ìš©í•œë‹¤. ì¶”í›„ì— ì˜·ì´ ì¶”ê°€ ë˜ì—ˆì„ë•Œ ë‹¤ìŒ ë³€ìˆ˜ë¥¼ ì„¸íŒ… í•´ì„œ ì“´ë‹¤.
				// addonInfo.FrameID ì— ì˜· ì •ë³´ë¥¼ ë„£ê³  ì„¸íŒ…í•˜ì—¬ ì“°ë©´ëœë‹¤.
				
				clothes = GetAdvancementPartFromItemClass( addonInfo.ItemClass , addonInfo.FrameID);
				
				if( clothes == -1 )
					continue;
				
				FRAME_ARRAY &FA = slayerFPK[clothes][action][direction];
				
				// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
				if (FA.GetSize() > frame)
				{
					CFrame &Frame = FA[frame];					
					int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
					int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
					int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
					
					pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
					pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
					
					CIndexSprite* pSprite = &addonISPK[ sprite ];
					
					// ë³µìž¥Spriteê°€ ì´ˆê¸°í™” ë˜ì§€ ì•Šì€ ê²½ìš°
					//						if (pSprite->IsNotInit())
					//						{
					//							LoadFromFileAddonSPK( clothes, action );
					//							//continue;
					//						}
					
					//---------------------------------------- 
					// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
					//---------------------------------------- 	
					rect.left	= pointTemp.x;
					rect.top	= pointTemp.y;
					rect.right	= rect.left + pSprite->GetWidth();
					rect.bottom = rect.top + pSprite->GetHeight();
					pCreature->AddScreenRect( &rect );
					
					// SpriteOutlineManagerì— ì¶”ê°€
					//CIndexSprite::SetUsingColorSet((pCreature->GetID()+3*(clothesType,1),7)%MAX_COLORSET, ((pCreature->GetID()*pCreature->GetID(),7*(clothesType+2)))%MAX_COLORSET);
					//CIndexSprite::SetUsingColorSet((c1+3*(clothesType+1)+7)%MAX_COLORSET, ((c2*c2+7*(clothesType+2)))%MAX_COLORSET);
					//---------------------------------------- 
					// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
					//---------------------------------------- 
					if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
					{
						m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
					}
					//---------------------------------------- 
					// ì›ëž˜ìƒ‰ëŒ€ë¡œ..
					//---------------------------------------- 
					else
					{
						int colorSet1 = addonInfo.ColorSet1, colorSet2 = addonInfo.ColorSet2;
						if(colorSet2 == UNIQUE_ITEM_COLOR || colorSet2 == QUEST_ITEM_COLOR)
						{
							if(colorSet2 == QUEST_ITEM_COLOR || colorSet2 == UNIQUE_ITEM_COLOR )
								colorSet2 = MItem::GetSpecialColorItemColorset( colorSet2 );
							
							if(addonInfo.ItemClass != ITEM_CLASS_COAT && addonInfo.ItemClass != ITEM_CLASS_TROUSER)
								colorSet1 = colorSet2;
						}							
						
						CIndexSprite::SetUsingColorSet( colorSet1, colorSet2 );
						m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
					}
				}
			}
		}
	}
}

void	MTopView::DrawSelectedAdvancementOustersCreature( POINT* pPoint, MCreature* pCreature, int action, int direction, int frame )
{
	MCreatureWear *pCreatureWear = (MCreatureWear *)pCreature;
	
	const MCreatureWear::ADDON_INFO& addonInfoChakram = pCreatureWear->GetAddonInfo(ADDON_RIGHTHAND);
	bool bChakram = addonInfoChakram.bAddon && addonInfoChakram.ItemClass == ITEM_CLASS_OUSTERS_CHAKRAM;

	int tempAction = ConvAdvancementOustersActionFromOusterAction( action, bChakram );

	if( tempAction == -1 )
		return;
	else
		tempAction -= ADVANCEMENT_ACTION_START;

	const MCreatureWear::ADDON_INFO& addonInfo = pCreatureWear->GetAddonInfo(ADDON_COAT);
	const MCreatureWear::ADDON_INFO& bootsAddonInfo = pCreatureWear->GetAddonInfo(ADDON_TROUSER);
	
	if (addonInfo.bAddon && !pCreatureWear->IsGhost(1))
	{
		int clothes = 1;		
		
		FRAME_ARRAY &FA = m_AdvancementOustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_AdvancementOustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ì„ íƒ ì‚¬ê°í˜• ì˜ì—­ ì„¤ì •
			//---------------------------------------- 	
			rect.left	= pointTemp.x;
			rect.top	= pointTemp.y;
			rect.right	= rect.left + pSprite->GetWidth();
			rect.bottom = rect.top + pSprite->GetHeight();
			pCreature->AddScreenRect( &rect );
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfo.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				CIndexSprite::SetUsingColorSetOnly( 0, pCreature->GetBodyColor1() );	// ë¨¸ë¦¬ìƒ‰
				
				int coatColor = addonInfo.ColorSet2, Colorset;
				if( coatColor == QUEST_ITEM_COLOR || coatColor == UNIQUE_ITEM_COLOR )
					coatColor = MItem::GetSpecialColorItemColorset( coatColor );						
				CIndexSprite::SetUsingColorSetOnly( 1, coatColor );	// ì˜·ìƒ‰
				if(bChakram == false)	// ë¦¬ìŠ¤í‹€ë¦¿ì„ ì°¨ê³  ìžˆìœ¼ë©´
				{
					if(addonInfoChakram.bAddon)
					{
						Colorset = addonInfoChakram.ColorSet2;
						if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”								
					}
					else
					{
						Colorset = defaultCoatColor;								
						if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
							Colorset = MItem::GetSpecialColorItemColorset( Colorset );
						
						CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
					}
				}
				else
				{
					Colorset = defaultCoatColor;
					if(addonInfoChakram.bAddon)
						Colorset = addonInfoChakram.ColorSet2;
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 2, Colorset );	// íŒ”
				}
				
				if(bootsAddonInfo.bAddon)
				{
					Colorset = bootsAddonInfo.ColorSet2;
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// ë¶€ì¸ ìƒ‰
				}
				else
				{
					Colorset = defaultCoatColor;
					
					if( Colorset == QUEST_ITEM_COLOR || Colorset == UNIQUE_ITEM_COLOR )
						Colorset = MItem::GetSpecialColorItemColorset( Colorset );
					CIndexSprite::SetUsingColorSetOnly( 3, Colorset );	// íŒ”
				}
				
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
			}						
			
		}
	}
	
	if (bChakram && !pCreatureWear->IsGhost(2))
	{
		int clothes = 0;
		
		FRAME_ARRAY &FA = m_AdvancementOustersFPK[clothes][tempAction][direction];
		
		// ìžˆëŠ” ë™ìž‘ì¸ ê²½ìš°
		if (FA.GetSize() > frame)
		{
			CFrame &Frame = FA[frame];					
			int sprite	= Frame.GetSpriteID();	//m_AddonFPK[clothes][action][direction][frame].GetSpriteID();
			int cx		= Frame.GetCX();	//m_AddonFPK[clothes][action][direction][frame].GetCX();
			int cy		= Frame.GetCY();	//m_AddonFPK[clothes][action][direction][frame].GetCY();
			
			CIndexSprite* pSprite = &m_AdvancementOustersSPK[ sprite ];					
			
			pointTemp.x = pPoint->x + cx;// + pCreature->GetSX();
			pointTemp.y = pPoint->y + cy;// + pCreature->GetSY();
			
			//---------------------------------------- 
			// ìºë¦­í„° ë¶€ë¶„ ìƒ‰ê¹”ì´ ë°”ë€ŒëŠ” ê²½ìš°
			//---------------------------------------- 
			if (addonInfoChakram.bEffectColor)	//colorSet < MAX_COLORSET)
			{
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite, addonInfo.EffectColorSet );
			}
			//---------------------------------------- 
			// ì •ìƒì ì¸ ìºë¦­í„° ì¶œë ¥
			//---------------------------------------- 
			else
			{
				int colorset1, colorset2;
				
				colorset1 = addonInfoChakram.ColorSet1;
				colorset2 = addonInfoChakram.ColorSet2;
				if( colorset1 == QUEST_ITEM_COLOR || colorset1 == UNIQUE_ITEM_COLOR )
					colorset1 = MItem::GetSpecialColorItemColorset( colorset1 );
				if( colorset2 == QUEST_ITEM_COLOR || colorset2 == UNIQUE_ITEM_COLOR )
					colorset2 = MItem::GetSpecialColorItemColorset( colorset2 );
				
				
				CIndexSprite::SetUsingColorSet(colorset1, colorset2);
				m_SOM.Add( pointTemp.x, pointTemp.y, pSprite );
			}						
			
		}
	}
}

