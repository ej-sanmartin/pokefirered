PalletTown_PlayersHouse_1F_MapScripts::
	.byte 0

PalletTown_PlayersHouse_1F_EventScript_Mom::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, PalletTown_PlayersHouse_1F_EventScript_MomChampion
	goto_if_set FLAG_BEAT_RIVAL_IN_OAKS_LAB, PalletTown_PlayersHouse_1F_EventScript_MomHeal
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouFemale
	closemessage
	applymovement LOCALID_MOM, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouMale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_Nick::
	lock
	faceplayer
	msgbox PalletTown_PlayersHouse_1F_Text_Nick
	closemessage
	release
	end

@ Mod: Boston, the player's husky, paces back and forth in the living room.
PalletTown_PlayersHouse_1F_EventScript_Boston::
	lock
	faceplayer
	msgbox PalletTown_PlayersHouse_1F_Text_Boston
	release
	end

PalletTown_PlayersHouse_1F_EventScript_MomHeal::
	msgbox PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest
	closemessage
	call EventScript_OutOfCenterPartyHeal
	msgbox PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare
	release
	end

@ Displays special text if interacted with from side or back (which are normally inaccessible)
PalletTown_PlayersHouse_1F_EventScript_TV::
	lockall
	goto_if_eq VAR_FACING, DIR_NORTH, PalletTown_PlayersHouse_1F_EventScript_TVScreen
	msgbox PalletTown_PlayersHouse_1F_Text_OopsWrongSide
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreen::
.ifdef DEBUG_SHORTCUT
	goto Debug_EventScript_Menu
.endif
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreenMale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad
	return

PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad
	return

@ Mod: once the player is Champion, Mom gives a one-time gift of the two event
@ tickets (not obtainable in vanilla). The National Dex (Oak) and the Rainbow
@ Pass (Celio) are left to their normal sources.
@ Each ticket is only given if the Key Items pocket has room; otherwise Mom
@ asks the player to make space and the gift stays available for next time.
PalletTown_PlayersHouse_1F_EventScript_MomChampion::
	goto_if_set FLAG_GOT_MOM_POSTGAME_GIFT, PalletTown_PlayersHouse_1F_EventScript_MomHeal
	msgbox PalletTown_PlayersHouse_1F_Text_MomYouAreChampion
	checkitem ITEM_MYSTIC_TICKET
	goto_if_eq VAR_RESULT, TRUE, PalletTown_PlayersHouse_1F_EventScript_MomGiveAurora
	checkitemspace ITEM_MYSTIC_TICKET
	goto_if_eq VAR_RESULT, FALSE, PalletTown_PlayersHouse_1F_EventScript_MomBagFull
	giveitem_msg PalletTown_PlayersHouse_1F_Text_ObtainedMysticTicket, ITEM_MYSTIC_TICKET, 1, MUS_OBTAIN_KEY_ITEM
PalletTown_PlayersHouse_1F_EventScript_MomGiveAurora::
	checkitem ITEM_AURORA_TICKET
	goto_if_eq VAR_RESULT, TRUE, PalletTown_PlayersHouse_1F_EventScript_MomGiftDone
	checkitemspace ITEM_AURORA_TICKET
	goto_if_eq VAR_RESULT, FALSE, PalletTown_PlayersHouse_1F_EventScript_MomBagFull
	giveitem_msg PalletTown_PlayersHouse_1F_Text_ObtainedAuroraTicket, ITEM_AURORA_TICKET, 1, MUS_OBTAIN_KEY_ITEM
PalletTown_PlayersHouse_1F_EventScript_MomGiftDone::
	setflag FLAG_ENABLE_SHIP_NAVEL_ROCK
	setflag FLAG_RECEIVED_MYSTIC_TICKET
	setflag FLAG_ENABLE_SHIP_BIRTH_ISLAND
	setflag FLAG_RECEIVED_AURORA_TICKET
	setflag FLAG_GOT_MOM_POSTGAME_GIFT
	msgbox PalletTown_PlayersHouse_1F_Text_MomEnjoyTheIslands
	goto PalletTown_PlayersHouse_1F_EventScript_MomHeal
	end

PalletTown_PlayersHouse_1F_EventScript_MomBagFull::
	msgbox PalletTown_PlayersHouse_1F_Text_MomBagFull
	goto PalletTown_PlayersHouse_1F_EventScript_MomHeal
	end
