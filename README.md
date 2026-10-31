# This mod is for my nephew

A personalized *Pokémon LeafGreen* built on the pokefirered decompilation.
It keeps the original story and adds the following.

## Personalization

- Pallet Town is renamed **West Springfield**; the hero defaults to **Landon**
  and the rival to **Tato**
- Custom Red protagonist art (overworld, battle, trainer, region map) and a
  custom title logo
- **Nick** in the player's house, and **Boston**, the pettable husky pacing
  around it (custom sprite)
- Optional encouraging bonus dialogue from a dozen NPCs across Kanto and Sevii

## Quality of life

- Wild encounters mix FireRed and LeafGreen: 50% of the time a species from
  the other version's table appears (the Pokédex area view counts both)
- TMs are reusable, move tutors are repeatable, and running works indoors
- Repel asks to re-use when one wears off
- Pokémon Center evolve option: the Cable Club attendant evolves trade
  evolutions without trading (Gen 2 targets need the National Dex)
- Celadon Dept. Store sells evolution stones and trade items

## Post-game (after the Elite Four)

- Mom gives the Mystic and Aurora Tickets; Oak gives the National Dex and the
  last starter Poké Ball; Celio gives the Rainbow Pass; all Sevii islands open
- The rival's starter appears on Route 1 until caught (Oak's aide hints at it)
- All three legendary beasts roam, and Teleport/Roar/Whirlwind no longer
  deactivate roaming legendaries
- Both Mt. Moon fossils are obtainable: take the leftover one in Mt. Moon, or
  get it as a prize from the Old Amber scientist in the Pewter Museum
- **Deoxys** has separate Normal/Attack/Defense/Speed forms (same in FireRed
  and LeafGreen). The new **Meteor Shard** key item cycles its form, and one
  is left on Birth Island after Deoxys is caught
- **Faraway Island** and **Mew**: Professor Oak gives the Old Sea Map for a
  complete Kanto Pokédex (150 without Mew, one Hitmon allowed), and the
  Vermilion ferry then sails to Faraway Island. Mew plays hide-and-seek in
  the grass and can flee. Ported from Emerald, including its music

## Test build

`make leafgreen DEBUG_SHORTCUT=1` builds a separate `pokeleafgreen_debug.gba`
with a debug menu on the TV in the player's house: unlock everything, skip
Celio's quest, complete the Pokédex, give a test team, and warp to Oak's Lab,
the Vermilion pier, or the Pewter Museum. It is never part of the normal ROM.

---

# Pokémon FireRed and LeafGreen

This is a decompilation of English Pokémon FireRed and LeafGreen.

It builds the following ROM images:

* [**pokefirered.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1616) `sha1: 41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc`
* [**pokeleafgreen.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1617) `sha1: 574fa542ffebb14be69902d1d36f1ec0a4afd71e`
* [**pokefirered_rev1.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1672) `sha1: dd5945db9b930750cb39d00c84da8571feebf417`
* [**pokeleafgreen_rev1.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=1668) `sha1: 7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`
* [**pokefirered_switch.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=x550) `sha1: baa452d0b24629dd7782cfc07a8984085dde1311`
* [**pokeleafgreen_switch.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=x551) `sha1: 62b9fc77549dbc67032eb6cbd0ea6ad3b825690f`

To set up the repository, see [INSTALL.md](INSTALL.md).

For contacts and other pret projects, see [pret.github.io](https://pret.github.io/).
