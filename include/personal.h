#ifndef __PERSONAL_H
#define __PERSONAL_H

#include "swan/swantypes.h"

enum PersonalField {
  Personal_HP = 0x0,
  Personal_ATK = 0x1,
  Personal_DEF = 0x2,
  Personal_SPE = 0x3,
  Personal_SPA = 0x4,
  Personal_SPD = 0x5,
  Personal_Type1 = 0x6,
  Personal_Type2 = 0x7,
  Personal_CaptureRate = 0x8,
  Personal_BaseEXP = 0x9,
  Personal_EvHP = 0xA,
  Personal_EvATK = 0xB,
  Personal_EvDEF = 0xC,
  Personal_EvSPE = 0xD,
  Personal_EvSPA = 0xE,
  Personal_EvSPD = 0xF,
  Personal_SpriteGrounded = 0x10,
  Personal_WildItem50 = 0x11,
  Personal_WildItem5 = 0x12,
  Personal_WildItem1 = 0x13,
  Personal_GenderProb = 0x14,
  Personal_EggHappiness = 0x15,
  Personal_BaseHappiness = 0x16,
  Personal_GrowthRate = 0x17,
  Personal_EggGroup1 = 0x18,
  Personal_EggGroup2 = 0x19,
  Personal_Abil1 = 0x1A,
  Personal_Abil2 = 0x1B,
  Personal_AbilH = 0x1C,
  Personal_EscapeRate = 0x1D,
  Personal_FormeDataOffs = 0x1E,
  Personal_FormeSpritesOffset = 0x1F,
  Personal_FormeCount = 0x20,
  Personal_Color = 0x21,
  Personal_SpriteFlip = 0x22,
  Personal_SpriteForme = 0x23,
  Personal_EvoStage = 0x24,
  Personal_Height = 0x25,
  Personal_Weight = 0x26,
  Personal_TMHM1 = 0x27,
  Personal_TMHM2 = 0x28,
  Personal_TMHM3 = 0x29,
  Personal_TMHM4 = 0x2A,
  Personal_TypeTutor = 0x2B,
  Personal_SpecialTutor1 = 0x2C,
  Personal_SpecialTutor2 = 0x2D,
  Personal_SpecialTutor3 = 0x2E,
  Personal_SpecialTutor4 = 0x2F,
};

typedef struct PersonalData {
  u8 BaseHP;
  u8 BaseATK;
  u8 BaseDEF;
  u8 BaseSPE;
  u8 BaseSPA;
  u8 BaseSPD;
  u8 Type1;
  u8 Type2;
  u8 CaptureRate;
  u8 EvoStage;
  u16 EVYield;
  u16 WildItem50;
  u16 WildItem5;
  u16 WildItem1;
  u8 GenderProb;
  u8 EggHappiness;
  u8 BaseHappiness;
  u8 ExpGroup;
  u8 EggGroup1;
  u8 EggGroup2;
  u8 Abil1;
  u8 Abil2;
  u8 AbilHidden;
  u8 EscapeRate;
  u16 FormeDataOffs;
  u16 FormeSpriteOffs;
  u8 FormeCount;
  u8 Color;
  u16 BaseEXP;
  u16 HeightCm;
  u16 WeightCg;
  s32 TMHM1;
  s32 TMHM2;
  s32 TMHM3;
  s32 TMHM4;
  s32 TypeTutors;
  s32 SpecialTutors[4];
} PERSONAL_DATA;

#endif