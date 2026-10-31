#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "faraway_island.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"

// Mod: Mew's hide-and-seek movement on Faraway Island, ported from
// pokeemerald. Mew only ever walks through grass, tries to keep away from
// the player and is invisible except on every 8th step.

static s16 sPlayerToMewDeltaX;
static s16 sPlayerToMewDeltaY;
static u8 sMewDirectionCandidates[4];

// Rocks Mew must avoid getting pinned against.
static const s16 sFarawayIslandRockCoords[4][2] =
{
    {14 + MAP_OFFSET,  9 + MAP_OFFSET},
    {18 + MAP_OFFSET,  9 + MAP_OFFSET},
    { 9 + MAP_OFFSET, 10 + MAP_OFFSET},
    {13 + MAP_OFFSET, 13 + MAP_OFFSET},
};

// Directions to try in order: the one that moves away from the player, then
// two perpendicular ones for a random pick.
static const u8 sAwayDirections[4][3] =
{
    {DIR_NORTH, DIR_EAST, DIR_WEST},
    {DIR_SOUTH, DIR_EAST, DIR_WEST},
    {DIR_EAST, DIR_NORTH, DIR_SOUTH},
    {DIR_WEST, DIR_NORTH, DIR_SOUTH},
};

static struct ObjectEvent *GetPlayerObject(void)
{
    return &gObjectEvents[gPlayerAvatar.objectEventId];
}

// Mew can move to any tall/long grass tile the player isn't standing on.
static bool8 CanMewMoveToCoords(s16 x, s16 y)
{
    struct ObjectEvent *player = GetPlayerObject();

    if (player->currentCoords.x == x && player->currentCoords.y == y)
        return FALSE;
    return MetatileBehavior_IsPokeGrass(MapGridGetMetatileBehaviorAt(x, y));
}

// Stores `dir` as candidate `index` and returns TRUE if stepping that way
// moves Mew away from the player into a valid tile.
static bool8 ShouldMewMove(struct ObjectEvent *mew, u8 dir, u8 index)
{
    s16 dx = 0;
    s16 dy = 0;
    bool8 movesAway;

    switch (dir)
    {
    case DIR_NORTH:
        dy = -1;
        movesAway = sPlayerToMewDeltaY > 0;
        break;
    case DIR_SOUTH:
        dy = 1;
        movesAway = sPlayerToMewDeltaY < 0;
        break;
    case DIR_EAST:
        dx = 1;
        movesAway = sPlayerToMewDeltaX < 0;
        break;
    default:
        dx = -1;
        movesAway = sPlayerToMewDeltaX > 0;
        break;
    }
    if (!movesAway
     || !CanMewMoveToCoords(mew->currentCoords.x + dx, mew->currentCoords.y + dy))
        return FALSE;
    sMewDirectionCandidates[index] = dir;
    return TRUE;
}

// Moves Mew away from the player, picking randomly between two good options.
// Returns DIR_NONE if every move would bring Mew closer.
static u8 GetAwayDirection(struct ObjectEvent *mew)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sAwayDirections); i++)
    {
        if (!ShouldMewMove(mew, sAwayDirections[i][0], 0))
            continue;
        if (ShouldMewMove(mew, sAwayDirections[i][1], 1)
         || ShouldMewMove(mew, sAwayDirections[i][2], 1))
            return sMewDirectionCandidates[VarGet(VAR_FARAWAY_ISLAND_STEP_COUNTER) % 2];
        return sAwayDirections[i][0];
    }
    return DIR_NONE;
}

// Last resort: collect every walkable direction and pick one by step count.
static u8 GetAnyValidMewDirection(struct ObjectEvent *mew)
{
    u8 count = 0;
    s16 x = mew->currentCoords.x;
    s16 y = mew->currentCoords.y;
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sMewDirectionCandidates); i++)
        sMewDirectionCandidates[i] = DIR_NONE;
    if (CanMewMoveToCoords(x, y - 1))
        sMewDirectionCandidates[count++] = DIR_NORTH;
    if (CanMewMoveToCoords(x + 1, y))
        sMewDirectionCandidates[count++] = DIR_EAST;
    if (CanMewMoveToCoords(x, y + 1))
        sMewDirectionCandidates[count++] = DIR_SOUTH;
    if (CanMewMoveToCoords(x - 1, y))
        sMewDirectionCandidates[count++] = DIR_WEST;
    if (count > 1)
        return sMewDirectionCandidates[VarGet(VAR_FARAWAY_ISLAND_STEP_COUNTER) % count];
    return sMewDirectionCandidates[0];
}

// Returns a direction that frees Mew if the player is about to pin it
// against the rock at `rock`, or DIR_NONE if Mew is safe.
static u8 GetRockTrapEscape(struct ObjectEvent *mew, const s16 *rock)
{
    struct ObjectEvent *player = GetPlayerObject();
    s16 px = player->previousCoords.x;
    s16 py = player->previousCoords.y;
    s16 mx = mew->currentCoords.x;
    s16 my = mew->currentCoords.y;
    bool8 safe;

    if (px == rock[0])
    {
        safe = (py < rock[1]) ? my <= rock[1] : my >= rock[1];
        if (!safe)
        {
            if (sPlayerToMewDeltaX > 0 && mx + 1 == px
             && CanMewMoveToCoords(mx + 1, my))
                return DIR_EAST;
            if (sPlayerToMewDeltaX < 0 && mx - 1 == px
             && CanMewMoveToCoords(mx - 1, my))
                return DIR_WEST;
            if (mx == px)
            {
                if (sPlayerToMewDeltaY > 0 && CanMewMoveToCoords(mx, my - 1))
                    return DIR_NORTH;
                if (sPlayerToMewDeltaY <= 0 && CanMewMoveToCoords(mx, my + 1))
                    return DIR_SOUTH;
            }
        }
    }
    if (py == rock[1])
    {
        safe = (px < rock[0]) ? mx <= rock[0] : mx >= rock[0];
        if (!safe)
        {
            if (sPlayerToMewDeltaY > 0 && my + 1 == py
             && CanMewMoveToCoords(mx, my + 1))
                return DIR_SOUTH;
            if (sPlayerToMewDeltaY < 0 && my - 1 == py
             && CanMewMoveToCoords(mx, my - 1))
                return DIR_NORTH;
            if (my == py)
            {
                if (sPlayerToMewDeltaX > 0 && CanMewMoveToCoords(mx - 1, my))
                    return DIR_WEST;
                if (sPlayerToMewDeltaX <= 0 && CanMewMoveToCoords(mx + 1, my))
                    return DIR_EAST;
            }
        }
    }
    return DIR_NONE;
}

// Mew is cornered on the player's row or column: slip past along the other
// axis. Returns DIR_NONE if neither axis offers a way out.
static u8 GetCorneredDirection(struct ObjectEvent *mew)
{
    struct ObjectEvent *player = GetPlayerObject();
    s16 mx = mew->currentCoords.x;
    s16 my = mew->currentCoords.y;

    if (sPlayerToMewDeltaY == 0)
    {
        if (player->currentCoords.y > my && CanMewMoveToCoords(mx, my - 1))
            return DIR_NORTH;
        if (player->currentCoords.y < my && CanMewMoveToCoords(mx, my + 1))
            return DIR_SOUTH;
        if (CanMewMoveToCoords(mx, my - 1))
            return DIR_NORTH;
        if (CanMewMoveToCoords(mx, my + 1))
            return DIR_SOUTH;
    }
    if (sPlayerToMewDeltaX == 0)
    {
        if (player->currentCoords.x > mx && CanMewMoveToCoords(mx - 1, my))
            return DIR_WEST;
        if (player->currentCoords.x < mx && CanMewMoveToCoords(mx + 1, my))
            return DIR_EAST;
        if (CanMewMoveToCoords(mx + 1, my))
            return DIR_EAST;
        if (CanMewMoveToCoords(mx - 1, my))
            return DIR_WEST;
    }
    return DIR_NONE;
}

// Returns the direction Mew steps this turn, or DIR_NONE to stay put.
// Runs every time the player takes a step in the interior.
u8 GetMewMoveDirection(void)
{
    u8 i;
    u8 dir;
    struct ObjectEvent *mew = GetFarawayIslandMew();
    struct ObjectEvent *player = GetPlayerObject();
    u16 steps = VarGet(VAR_FARAWAY_ISLAND_STEP_COUNTER);

    if (mew == NULL)
        return DIR_NONE;
    sPlayerToMewDeltaX = player->previousCoords.x - mew->currentCoords.x;
    sPlayerToMewDeltaY = player->previousCoords.y - mew->currentCoords.y;
    for (i = 0; i < ARRAY_COUNT(sMewDirectionCandidates); i++)
        sMewDirectionCandidates[i] = DIR_NONE;

    // The player only turned; Mew stays.
    if (player->previousCoords.x == player->currentCoords.x
     && player->previousCoords.y == player->currentCoords.y)
        return DIR_NONE;

    // Mew is invisible except on every 8th step, and stays put for the step
    // after it was seen.
    mew->invisible = steps % 8 != 0;
    if (steps % 9 == 0)
        return DIR_NONE;

    for (i = 0; i < ARRAY_COUNT(sFarawayIslandRockCoords); i++)
    {
        dir = GetRockTrapEscape(mew, sFarawayIslandRockCoords[i]);
        if (dir != DIR_NONE)
            return dir;
    }
    dir = GetAwayDirection(mew);
    if (dir != DIR_NONE)
        return dir;
    dir = GetCorneredDirection(mew);
    if (dir != DIR_NONE)
        return dir;
    return GetAnyValidMewDirection(mew);
}
