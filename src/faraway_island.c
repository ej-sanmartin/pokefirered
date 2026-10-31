#include "global.h"
#include "gflib.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "faraway_island.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/maps.h"

// Mod: state and hooks for Mew's hide-and-seek on Faraway Island.
// The movement AI lives in faraway_island_mew_ai.c.

// Sprite id of the grass Mew rises from, plus 1 (0 = none).
static u8 sGrassSpriteIdPlusOne;

// Returns TRUE while the player is inside the Faraway Island interior.
static bool8 IsInFarawayIslandInterior(void)
{
    return gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_FARAWAY_ISLAND_INTERIOR)
        && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_FARAWAY_ISLAND_INTERIOR);
}

// Returns Mew's object event, or NULL if it is not on the current map.
struct ObjectEvent *GetFarawayIslandMew(void)
{
    u8 objectEventId;

    if (!IsInFarawayIslandInterior())
        return NULL;
    if (TryGetObjectEventIdByLocalIdAndMap(LOCALID_FARAWAY_ISLAND_MEW,
                                           gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup,
                                           &objectEventId))
        return NULL;
    return &gObjectEvents[objectEventId];
}

// Counts the player's steps inside the interior; Mew's AI keys off it.
// Called once per player step.
void UpdateFarawayIslandStepCounter(void)
{
    u16 steps;

    if (!IsInFarawayIslandInterior())
        return;
    steps = VarGet(VAR_FARAWAY_ISLAND_STEP_COUNTER) + 1;
    VarSet(VAR_FARAWAY_ISLAND_STEP_COUNTER, steps >= 9999 ? 0 : steps);
}

// Returns TRUE if the object event is Mew inside the interior.
bool8 ObjectEventIsFarawayIslandMew(struct ObjectEvent *objectEvent)
{
    return IsInFarawayIslandInterior()
        && objectEvent->graphicsId == OBJ_EVENT_GFX_MEW;
}

// Mew only disturbs the grass it steps into every 4th step.
bool8 ShouldMewShakeGrass(struct ObjectEvent *objectEvent)
{
    u16 steps = VarGet(VAR_FARAWAY_ISLAND_STEP_COUNTER);

    return steps != 0xFFFF && steps % 4 == 0;
}

// Returns TRUE if walking in `direction` bumps the player into the spot Mew
// just left. Avoids the collision sound/bump while chasing Mew.
bool8 IsPlayerCollidingWithFarawayIslandMew(u8 direction)
{
    struct ObjectEvent *mew = GetFarawayIslandMew();
    s16 x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    s16 y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;

    if (mew == NULL)
        return FALSE;
    MoveCoords(direction, &x, &y);
    if (mew->previousCoords.x != x || mew->previousCoords.y != y)
        return FALSE;
    return mew->currentCoords.x != x || mew->currentCoords.y != y;
}

// Special: draws Mew above the grass.
// gSpecialVar_0x8004 == 1: after a battle Mew survived; just keep it on top.
// Otherwise: Mew was found, so it rises out of a grass sprite that is
// created underneath it.
void SetMewAboveGrass(void)
{
    s16 x;
    s16 y;
    u8 grassSpriteId;
    struct Sprite *sprite;
    struct ObjectEvent *mew = GetFarawayIslandMew();

    if (mew == NULL)
        return;
    mew->invisible = FALSE;
    mew->fixedPriority = TRUE;
    sprite = &gSprites[mew->spriteId];
    sprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;
    if (gSpecialVar_0x8004 == 1)
    {
        sprite->subpriority = 1;
        return;
    }

    VarSet(VAR_FARAWAY_ISLAND_STEP_COUNTER, 0xFFFF);
    if (gSpecialVar_Facing != DIR_NORTH)
        sprite->subpriority = 1;
    LoadSpritePalette(&gSpritePalette_GeneralFieldEffect1);
    UpdateSpritePaletteWithWeather(
        IndexOfSpritePaletteTag(gSpritePalette_GeneralFieldEffect1.tag));
    x = mew->currentCoords.x;
    y = mew->currentCoords.y;
    SetSpritePosToOffsetMapCoords(&x, &y, 8, 8);
    grassSpriteId = CreateSpriteAtEnd(
        gFieldEffectObjectTemplatePointers[FLDEFFOBJ_LONG_GRASS], x, y,
        sprite->subpriority - 1);
    if (grassSpriteId != MAX_SPRITES)
    {
        gSprites[grassSpriteId].coordOffsetEnabled = TRUE;
        gSprites[grassSpriteId].oam.priority = 2;
        gSprites[grassSpriteId].callback = SpriteCallbackDummy;
        sGrassSpriteIdPlusOne = grassSpriteId + 1;
    }
}

// Special: removes the grass sprite created by SetMewAboveGrass.
void DestroyMewEmergingGrassSprite(void)
{
    if (sGrassSpriteIdPlusOne != 0)
        DestroySprite(&gSprites[sGrassSpriteIdPlusOne - 1]);
    sGrassSpriteIdPlusOne = 0;
}
