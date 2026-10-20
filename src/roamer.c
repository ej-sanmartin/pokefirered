#include "global.h"
#include "random.h"
#include "overworld.h"
#include "field_specials.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

// Despite having a variable to track it, the roamer is
// hard-coded to only ever be in map group 3
#define ROAMER_MAP_GROUP 3

enum
{
    MAP_GRP, // map group
    MAP_NUM, // map number
};

// Mod: all three legendary beasts roam. Roamer 0 is the one tied to the
// player's starter (the vanilla roamer); 1 and 2 live in spare SaveBlock1 space.
#define NUM_ROAMERS 3
EWRAM_DATA u8 sLocationHistory[3][2] = {};
EWRAM_DATA u8 sRoamerLocation[NUM_ROAMERS][2] = {};
EWRAM_DATA u8 sEncounteredRoamer = 0;

// Returns roamer slot i (0 = primary).
static struct Roamer *GetRoamer(u8 i)
{
    return i == 0 ? &gSaveBlock1Ptr->roamer : &gSaveBlock1Ptr->extraRoamers[i - 1];
}

#define ___ MAP_NUM(MAP_UNDEFINED) // For empty spots in the location table

// Note: There are two potential softlocks that can occur with this table if its maps are
//       changed in particular ways. They can be avoided by ensuring the following:
//       - There must be at least 2 location sets that start with a different map,
//         i.e. every location set cannot start with the same map. This is because of
//         the while loop in RoamerMoveToOtherLocationSet.
//       - Each location set must have at least 3 unique maps. This is because of
//         the while loop in RoamerMove. In this loop the first map in the set is
//         ignored, and an additional map is ignored if the roamer was there recently.
//       - Additionally, while not a softlock, it's worth noting that if for any
//         map in the location table there is not a location set that starts with
//         that map then the roamer will be significantly less likely to move away
//         from that map when it lands there.
static const u8 sRoamerLocations[][7] = {
    {MAP_NUM(MAP_ROUTE1), MAP_NUM(MAP_ROUTE2), MAP_NUM(MAP_ROUTE21_NORTH), MAP_NUM(MAP_ROUTE22), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE2), MAP_NUM(MAP_ROUTE1), MAP_NUM(MAP_ROUTE3), MAP_NUM(MAP_ROUTE22), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE3), MAP_NUM(MAP_ROUTE2), MAP_NUM(MAP_ROUTE4), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE4), MAP_NUM(MAP_ROUTE3), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE9), MAP_NUM(MAP_ROUTE24), ___, ___},
    {MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE4), MAP_NUM(MAP_ROUTE6), MAP_NUM(MAP_ROUTE7), MAP_NUM(MAP_ROUTE8), MAP_NUM(MAP_ROUTE9), MAP_NUM(MAP_ROUTE24)},
    {MAP_NUM(MAP_ROUTE6), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE7), MAP_NUM(MAP_ROUTE8), MAP_NUM(MAP_ROUTE11), ___, ___},
    {MAP_NUM(MAP_ROUTE7), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE6), MAP_NUM(MAP_ROUTE8), MAP_NUM(MAP_ROUTE16), ___, ___},
    {MAP_NUM(MAP_ROUTE8), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE6), MAP_NUM(MAP_ROUTE7), MAP_NUM(MAP_ROUTE10), MAP_NUM(MAP_ROUTE12), ___},
    {MAP_NUM(MAP_ROUTE9), MAP_NUM(MAP_ROUTE4), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE10), MAP_NUM(MAP_ROUTE24), ___, ___},
    {MAP_NUM(MAP_ROUTE10), MAP_NUM(MAP_ROUTE8), MAP_NUM(MAP_ROUTE9), MAP_NUM(MAP_ROUTE12), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE11), MAP_NUM(MAP_ROUTE6), MAP_NUM(MAP_ROUTE12), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE12), MAP_NUM(MAP_ROUTE10), MAP_NUM(MAP_ROUTE11), MAP_NUM(MAP_ROUTE13), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE13), MAP_NUM(MAP_ROUTE12), MAP_NUM(MAP_ROUTE14), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE14), MAP_NUM(MAP_ROUTE13), MAP_NUM(MAP_ROUTE15), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE15), MAP_NUM(MAP_ROUTE14), MAP_NUM(MAP_ROUTE18), MAP_NUM(MAP_ROUTE19), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE16), MAP_NUM(MAP_ROUTE7), MAP_NUM(MAP_ROUTE17), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE17), MAP_NUM(MAP_ROUTE16), MAP_NUM(MAP_ROUTE18), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE18), MAP_NUM(MAP_ROUTE15), MAP_NUM(MAP_ROUTE17), MAP_NUM(MAP_ROUTE19), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE19), MAP_NUM(MAP_ROUTE15), MAP_NUM(MAP_ROUTE18), MAP_NUM(MAP_ROUTE20), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE20), MAP_NUM(MAP_ROUTE19), MAP_NUM(MAP_ROUTE21_NORTH), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE21_NORTH), MAP_NUM(MAP_ROUTE1), MAP_NUM(MAP_ROUTE20), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE22), MAP_NUM(MAP_ROUTE1), MAP_NUM(MAP_ROUTE2), MAP_NUM(MAP_ROUTE23), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE23), MAP_NUM(MAP_ROUTE22), MAP_NUM(MAP_ROUTE2), ___, ___, ___, ___},
    {MAP_NUM(MAP_ROUTE24), MAP_NUM(MAP_ROUTE4), MAP_NUM(MAP_ROUTE5), MAP_NUM(MAP_ROUTE9), ___, ___, ___},
    {MAP_NUM(MAP_ROUTE25), MAP_NUM(MAP_ROUTE24), MAP_NUM(MAP_ROUTE9), ___, ___, ___, ___},
    {___, ___, ___, ___, ___, ___, ___}
};

#undef ___
#define NUM_LOCATION_SETS (ARRAY_COUNT(sRoamerLocations) - 1)
#define NUM_LOCATIONS_PER_SET (ARRAY_COUNT(sRoamerLocations[0]))

void ClearRoamerData(void)
{
    u32 i;

    for (i = 0; i < NUM_ROAMERS; i++)
    {
        *GetRoamer(i) = (struct Roamer){};
        sRoamerLocation[i][MAP_GRP] = 0;
        sRoamerLocation[i][MAP_NUM] = 0;
    }
    sEncounteredRoamer = 0;
    for (i = 0; i < ARRAY_COUNT(sLocationHistory); i++)
    {
        sLocationHistory[i][MAP_GRP] = 0;
        sLocationHistory[i][MAP_NUM] = 0;
    }
}

// Returns the species of roamer i: the starter's beast first, then the
// other two in order (Raikou, Entei, Suicune).
static u16 GetRoamerSpecies(u8 i)
{
    static const u16 sBeasts[NUM_ROAMERS] = {SPECIES_RAIKOU, SPECIES_ENTEI, SPECIES_SUICUNE};
    u8 first;

    switch (GetStarterSpecies())
    {
    default:
        first = 0;
        break;
    case SPECIES_BULBASAUR:
        first = 1;
        break;
    case SPECIES_CHARMANDER:
        first = 2;
        break;
    }
    return sBeasts[(first + i) % NUM_ROAMERS];
}

static void CreateInitialRoamerMon(u8 i)
{
    struct Pokemon * mon = &gEnemyParty[0];
    struct Roamer * roamer = GetRoamer(i);
    u16 species = GetRoamerSpecies(i);

    CreateMon(mon, species, 50, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    roamer->species = species;
    roamer->level = 50;
    roamer->status = 0;
    roamer->active = TRUE;
    roamer->ivs = GetMonData(mon, MON_DATA_IVS);
    roamer->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    roamer->hp = GetMonData(mon, MON_DATA_MAX_HP);
    roamer->cool = GetMonData(mon, MON_DATA_COOL);
    roamer->beauty = GetMonData(mon, MON_DATA_BEAUTY);
    roamer->cute = GetMonData(mon, MON_DATA_CUTE);
    roamer->smart = GetMonData(mon, MON_DATA_SMART);
    roamer->tough = GetMonData(mon, MON_DATA_TOUGH);
    sRoamerLocation[i][MAP_GRP] = ROAMER_MAP_GROUP;
    sRoamerLocation[i][MAP_NUM] = sRoamerLocations[Random() % NUM_LOCATION_SETS][0];
}

void InitRoamer(void)
{
    u8 i;

    ClearRoamerData();
    for (i = 0; i < NUM_ROAMERS; i++)
        CreateInitialRoamerMon(i);
}

void UpdateLocationHistoryForRoamer(void)
{
    sLocationHistory[2][MAP_GRP] = sLocationHistory[1][MAP_GRP];
    sLocationHistory[2][MAP_NUM] = sLocationHistory[1][MAP_NUM];

    sLocationHistory[1][MAP_GRP] = sLocationHistory[0][MAP_GRP];
    sLocationHistory[1][MAP_NUM] = sLocationHistory[0][MAP_NUM];

    sLocationHistory[0][MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocationHistory[0][MAP_NUM] = gSaveBlock1Ptr->location.mapNum;
}

static void MoveRoamerToOtherLocationSet(u8 i)
{
    u8 mapNum = 0;

    if (!GetRoamer(i)->active)
        return;

    sRoamerLocation[i][MAP_GRP] = ROAMER_MAP_GROUP;

    // Choose a location set that starts with a map
    // different from the roamer's current map
    while (1)
    {
        mapNum = sRoamerLocations[Random() % NUM_LOCATION_SETS][0];
        if (sRoamerLocation[i][MAP_NUM] != mapNum)
        {
            sRoamerLocation[i][MAP_NUM] = mapNum;
            return;
        }
    }
}

void RoamerMoveToOtherLocationSet(void)
{
    u8 i;

    for (i = 0; i < NUM_ROAMERS; i++)
        MoveRoamerToOtherLocationSet(i);
}

static void MoveRoamer(u8 i)
{
    u8 locSet = 0;

    if ((Random() % 16) == 0)
    {
        MoveRoamerToOtherLocationSet(i);
    }
    else
    {
        if (!GetRoamer(i)->active)
            return;

        while (locSet < NUM_LOCATION_SETS)
        {
            // Find the location set that starts with the roamer's current map
            if (sRoamerLocation[i][MAP_NUM] == sRoamerLocations[locSet][0])
            {
                u8 mapNum;

                while (1)
                {
                    // Choose a new map (excluding the first) within this set
                    // Also exclude a map if the player was there 2 moves ago
                    mapNum = sRoamerLocations[locSet][(Random() % (NUM_LOCATIONS_PER_SET - 1)) + 1];
                    if (!(sLocationHistory[2][MAP_GRP] == ROAMER_MAP_GROUP
                       && sLocationHistory[2][MAP_NUM] == mapNum)
                       && mapNum != MAP_NUM(MAP_UNDEFINED))
                        break;
                }
                sRoamerLocation[i][MAP_NUM] = mapNum;
                return;
            }
            locSet++;
        }
    }
}

void RoamerMove(void)
{
    u8 i;

    for (i = 0; i < NUM_ROAMERS; i++)
        MoveRoamer(i);
}

// Returns the first active roamer on the given map, or -1.
static s32 GetRoamerIndexAt(u8 mapGroup, u8 mapNum)
{
    u8 i;

    for (i = 0; i < NUM_ROAMERS; i++)
    {
        if (GetRoamer(i)->active && mapGroup == sRoamerLocation[i][MAP_GRP]
         && mapNum == sRoamerLocation[i][MAP_NUM])
            return i;
    }
    return -1;
}

bool8 IsRoamerAt(u8 mapGroup, u8 mapNum)
{
    return GetRoamerIndexAt(mapGroup, mapNum) >= 0;
}

// Builds gEnemyParty[0] from the roamer being fought (sEncounteredRoamer).
void CreateRoamerMonInstance(void)
{
    u32 status;
    struct Pokemon *mon = &gEnemyParty[0];
    struct Roamer *roamer = GetRoamer(sEncounteredRoamer);

    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, roamer->species, roamer->level, roamer->ivs, roamer->personality);
// The roamer's status field is u8, but SetMonData expects status to be u32, so will set the roamer's status
// using the status field and the following 3 bytes (cool, beauty, and cute).
#ifdef BUGFIX
    status = roamer->status;
    SetMonData(mon, MON_DATA_STATUS, &status);
#else
    SetMonData(mon, MON_DATA_STATUS, &roamer->status);
#endif
    SetMonData(mon, MON_DATA_HP, &roamer->hp);
    SetMonData(mon, MON_DATA_COOL, &roamer->cool);
    SetMonData(mon, MON_DATA_BEAUTY, &roamer->beauty);
    SetMonData(mon, MON_DATA_CUTE, &roamer->cute);
    SetMonData(mon, MON_DATA_SMART, &roamer->smart);
    SetMonData(mon, MON_DATA_TOUGH, &roamer->tough);
}

bool8 TryStartRoamerEncounter(void)
{
    s32 i = GetRoamerIndexAt(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);

    if (i >= 0 && (Random() % 4) == 0)
    {
        sEncounteredRoamer = i;
        CreateRoamerMonInstance();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// After a battle: saves the fought roamer's HP/status and moves only it away.
void UpdateRoamerHPStatus(struct Pokemon *mon)
{
    struct Roamer *roamer = GetRoamer(sEncounteredRoamer);

    roamer->hp = GetMonData(mon, MON_DATA_HP);
    roamer->status = GetMonData(mon, MON_DATA_STATUS);

    MoveRoamerToOtherLocationSet(sEncounteredRoamer);
}

void SetRoamerInactive(void)
{
    GetRoamer(sEncounteredRoamer)->active = FALSE;
}

void GetRoamerLocation(u8 *mapGroup, u8 *mapNum)
{
    *mapGroup = sRoamerLocation[0][MAP_GRP];
    *mapNum = sRoamerLocation[0][MAP_NUM];
}

// Returns the region map section of the active roamer with this species, or
// MAPSEC_NONE if it is not roaming.
u16 GetRoamerLocationMapSectionId(u16 species)
{
    u8 i;

    for (i = 0; i < NUM_ROAMERS; i++)
    {
        if (GetRoamer(i)->active && GetRoamer(i)->species == species)
            return Overworld_GetMapHeaderByGroupAndId(sRoamerLocation[i][MAP_GRP], sRoamerLocation[i][MAP_NUM])->regionMapSectionId;
    }
    return MAPSEC_NONE;
}
