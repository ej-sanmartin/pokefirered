Route1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route1_OnTransition
	.byte 0

@ Mod: after the Elite Four, the rival's starter waits on Route 1 until caught.
Route1_OnTransition::
	setflag FLAG_HIDE_ROUTE1_STARTER
	goto_if_unset FLAG_SYS_GAME_CLEAR, Route1_EventScript_TransitionEnd
	goto_if_set FLAG_CAUGHT_ROUTE1_STARTER, Route1_EventScript_TransitionEnd
	clearflag FLAG_HIDE_ROUTE1_STARTER
Route1_EventScript_TransitionEnd::
	end

Route1_EventScript_MartClerk::
	lock
	faceplayer
	goto_if_set FLAG_GOT_POTION_ON_ROUTE_1, Route1_EventScript_AlreadyGotPotion
	msgbox Route1_Text_WorkAtPokeMartTakeSample
	textcolor NPC_TEXT_COLOR_NEUTRAL
	checkitemspace ITEM_POTION
	goto_if_eq VAR_RESULT, FALSE, EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POTION
	playfanfare MUS_LEVEL_UP
	message Text_ObtainedTheX
	waitmessage
	waitfanfare
	additem ITEM_POTION
	msgbox Route1_Text_PutPotionAway
	call EventScript_RestorePrevTextColor
	setflag FLAG_GOT_POTION_ON_ROUTE_1
	release
	end

Route1_EventScript_AlreadyGotPotion::
	msgbox Route1_Text_ComeSeeUsIfYouNeedPokeBalls
	release
	end

Route1_EventScript_Boy::
	msgbox Route1_Text_CanJumpFromLedges, MSGBOX_NPC
	end

Route1_EventScript_RouteSign::
	msgbox Route1_Text_RouteSign, MSGBOX_SIGN
	end

Route1_EventScript_RivalStarter::
	goto_if_questlog EventScript_ReleaseEnd
	lock
	faceplayer
	goto_if_eq VAR_STARTER_MON, 0, Route1_EventScript_RivalStarterCharmander
	goto_if_eq VAR_STARTER_MON, 1, Route1_EventScript_RivalStarterBulbasaur
	setwildbattle SPECIES_SQUIRTLE, 5
	playmoncry SPECIES_SQUIRTLE, CRY_MODE_ENCOUNTER
	goto Route1_EventScript_RivalStarterBattle
	end

Route1_EventScript_RivalStarterCharmander::
	setwildbattle SPECIES_CHARMANDER, 5
	playmoncry SPECIES_CHARMANDER, CRY_MODE_ENCOUNTER
	goto Route1_EventScript_RivalStarterBattle
	end

Route1_EventScript_RivalStarterBulbasaur::
	setwildbattle SPECIES_BULBASAUR, 5
	playmoncry SPECIES_BULBASAUR, CRY_MODE_ENCOUNTER
	goto Route1_EventScript_RivalStarterBattle
	end

@ The starter stays until caught, so fainting or running from it is safe.
Route1_EventScript_RivalStarterBattle::
	delay 40
	waitmoncry
	setflag FLAG_SYS_SPECIAL_WILD_BATTLE
	dowildbattle
	clearflag FLAG_SYS_SPECIAL_WILD_BATTLE
	special QuestLog_CutRecording
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Route1_EventScript_RivalStarterStays
	setflag FLAG_CAUGHT_ROUTE1_STARTER
	removeobject VAR_LAST_TALKED
Route1_EventScript_RivalStarterStays::
	release
	end
