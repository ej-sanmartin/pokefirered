@ Test builds only: make leafgreen DEBUG_SHORTCUT=1 (outputs
@ pokeleafgreen_debug.gba). Stand north of the TV in the player's house and
@ press A. Never part of the normal ROM.
Debug_EventScript_Menu::
	lockall
	msgbox Debug_Text_Unlock, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Debug_EventScript_Unlock
	msgbox Debug_Text_SkipCelio, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Debug_EventScript_SkipCelio
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
	clearflag FLAG_GOT_MOM_POSTGAME_GIFT
	setvar VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 3
	@ Cinnabar scene 4 (Bill's return trip done) is what makes the island
	@ ferries offer Vermilion; the PC lock is normally cleared at Three Island.
	setvar VAR_MAP_SCENE_CINNABAR_ISLAND, 4
	clearflag FLAG_SYS_PC_STORAGE_DISABLED
	setvar VAR_MAP_SCENE_VERMILION_CITY, 3
	setvar VAR_VERMILION_CITY_TICKET_CHECK_TRIGGER, 1
	additem ITEM_MASTER_BALL, 10
	additem ITEM_REPEL, 10
	additem ITEM_ULTRA_BALL, 100
	addmoney 99999
	msgbox Debug_Text_Done
	return

@ Replays the end state of Celio's Ruby/Sapphire quest (the Rainbow Pass trade
@ and the Hoenn link) without the cutscenes. Skipped if already done so that
@ InitRoamer is not run twice.
Debug_EventScript_SkipCelio::
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 6, Debug_EventScript_CelioAlreadyDone
	setflag FLAG_SYS_SEVII_MAP_123
	setflag FLAG_GOT_RUBY
	setflag FLAG_HIDE_RUBY
	setflag FLAG_RECOVERED_SAPPHIRE
	removeitem ITEM_RUBY
	removeitem ITEM_SAPPHIRE
	removeitem ITEM_TRI_PASS
	additem ITEM_RAINBOW_PASS
	setflag FLAG_SYS_SEVII_MAP_4567
	special SetPostgameFlags
	setflag FLAG_SYS_CAN_LINK_WITH_RS
	setflag FLAG_HIDE_CERULEAN_CAVE_GUARD
	setflag FLAG_HIDE_LORELEI_IN_HER_HOUSE
	setvar VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 6
	special InitRoamer
	msgbox Debug_Text_Done
	return

Debug_EventScript_CelioAlreadyDone::
	msgbox Debug_Text_CelioAlreadyDone
	return

Debug_EventScript_CompleteDex::
	special DebugCompleteKantoDex
	msgbox Debug_Text_Done
	return

@ Skipped when the party already has 3+ mons, so running the menu twice does
@ not hand out the test team a second time.
Debug_EventScript_GiveMons::
	getpartysize
	goto_if_ge VAR_RESULT, 3, Debug_EventScript_TeamAlreadyGiven
	givemon SPECIES_CHARIZARD, 60
	givemon SPECIES_KADABRA, 30
	givemon SPECIES_ONIX, 30, ITEM_METAL_COAT
	@ HM carriers. A partyIndex above PARTY_SIZE targets the last party mon.
	givemon SPECIES_FARFETCHD, 40
	setmonmove 7, 0, MOVE_CUT
	setmonmove 7, 1, MOVE_FLY
	setmonmove 7, 2, MOVE_ROCK_SMASH
	givemon SPECIES_GYARADOS, 40
	setmonmove 7, 0, MOVE_SURF
	setmonmove 7, 1, MOVE_STRENGTH
	setmonmove 7, 2, MOVE_WATERFALL
	msgbox Debug_Text_Done
	return

Debug_EventScript_TeamAlreadyGiven::
	msgbox Debug_Text_TeamAlreadyGiven
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
	.string "DEBUG: Champion + badges + items\n"
	.string "(100 ULTRA BALLS, etc.)?$"

Debug_Text_SkipCelio::
	.string "DEBUG: Skip CELIO's quest?\n"
	.string "(RUBY + SAPPHIRE done)$"

Debug_Text_CelioAlreadyDone::
	.string "CELIO's quest is already done.\n"
	.string "Skipped.$"

Debug_Text_Dex::
	.string "DEBUG: Complete the KANTO POKéDEX?$"

Debug_Text_Mons::
	.string "DEBUG: Give test team?\n"
	.string "(CHARIZARD Lv60, FARFETCH'D, etc.)$"

Debug_Text_TeamAlreadyGiven::
	.string "Party already has a team.\n"
	.string "Skipped.$"

Debug_Text_WarpLab::
	.string "DEBUG: Warp to PROF. OAK's LAB?$"

Debug_Text_WarpVermilion::
	.string "DEBUG: Warp to the VERMILION pier?$"

Debug_Text_Done::
	.string "Done.$"
