#ifndef GUARD_FARAWAY_ISLAND_H
#define GUARD_FARAWAY_ISLAND_H

// Mod: Mew's hide-and-seek on Faraway Island, ported from pokeemerald.
struct ObjectEvent *GetFarawayIslandMew(void);
u8 GetMewMoveDirection(void);
void UpdateFarawayIslandStepCounter(void);
bool8 ObjectEventIsFarawayIslandMew(struct ObjectEvent *objectEvent);
bool8 ShouldMewShakeGrass(struct ObjectEvent *objectEvent);
bool8 IsPlayerCollidingWithFarawayIslandMew(u8 direction);
void SetMewAboveGrass(void);
void DestroyMewEmergingGrassSprite(void);

#endif // GUARD_FARAWAY_ISLAND_H
