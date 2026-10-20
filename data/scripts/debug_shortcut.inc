@ Test builds only: make leafgreen DEBUG_SHORTCUT=1 (outputs
@ pokeleafgreen_debug.gba). Stand north of the TV in the player's house and
@ press A. Never part of the normal ROM.
Debug_EventScript_Menu::
	lockall
	msgbox Debug_Text_Unlock, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Debug_EventScript_Unlock
	msgbox Debug_Text_Dex, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Debug_EventScript_CompleteDex
	msgbox Debug_Text_Mons, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Debug_EventScript_GiveMons
	msgbox Debug_Text_WarpLab, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Debug_EventScript_WarpLab
	msgbox Debug_Text_WarpVermilion, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Debug_EventScript_WarpVermilion
	releaseall
	end

@ Champion, badges, Celio quest at the post-Bill stage (Mom's gift needs it),
@ Vermilion ferry ready, and some items.
Debug_EventScript_Unlock::
	setflag FLAG_SYS_POKEDEX_GET
	special SetUnlockedPokedexFlags
	setflag FLAG_SYS_POKEMON_GET
	setflag FLAG_SYS_GAME_CLEAR
	setflag FLAG_BADGE01_GET
	setflag FLAG_BADGE02_GET
	setflag FLAG_BADGE03_GET
	setflag FLAG_BADGE04_GET
	setflag FLAG_BADGE05_GET
	setflag FLAG_BADGE06_GET
	setflag FLAG_BADGE07_GET
	setflag FLAG_BADGE08_GET
	setflag FLAG_WORLD_MAP_ONE_ISLAND
	setvar VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 3
	setvar VAR_MAP_SCENE_VERMILION_CITY, 3
	setvar VAR_VERMILION_CITY_TICKET_CHECK_TRIGGER, 1
	additem ITEM_MASTER_BALL, 10
	additem ITEM_REPEL, 10
	additem ITEM_ULTRA_BALL, 30
	addmoney 99999
	msgbox Debug_Text_Done
	return

Debug_EventScript_CompleteDex::
	special DebugCompleteKantoDex
	msgbox Debug_Text_Done
	return

Debug_EventScript_GiveMons::
	givemon SPECIES_KADABRA, 30
	givemon SPECIES_ONIX, 30, ITEM_METAL_COAT
	msgbox Debug_Text_Done
	return

Debug_EventScript_WarpLab::
	warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 0
	waitstate
	end

Debug_EventScript_WarpVermilion::
	warp MAP_VERMILION_CITY, 23, 32
	waitstate
	end

Debug_Text_Unlock::
	.string "DEBUG: Champion + badges + Celio\n"
	.string "quest + items?$"

Debug_Text_Dex::
	.string "DEBUG: Complete the KANTO POKéDEX?$"

Debug_Text_Mons::
	.string "DEBUG: Give KADABRA and ONIX\n"
	.string "(METAL COAT)?$"

Debug_Text_WarpLab::
	.string "DEBUG: Warp to PROF. OAK's LAB?$"

Debug_Text_WarpVermilion::
	.string "DEBUG: Warp to the VERMILION pier?$"

Debug_Text_Done::
	.string "Done.$"
