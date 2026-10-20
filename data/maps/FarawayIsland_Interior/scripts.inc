FarawayIsland_Interior_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FarawayIsland_Interior_OnTransition
	.byte 0

@ Mew is only gone for good once it has been caught. Otherwise it is back on
@ every visit, even after it fled.
FarawayIsland_Interior_OnTransition::
	call_if_set FLAG_CAUGHT_MEW, FarawayIsland_Interior_EventScript_HideMew
	call_if_unset FLAG_CAUGHT_MEW, FarawayIsland_Interior_EventScript_ShowMew
	end

FarawayIsland_Interior_EventScript_HideMew::
	setflag FLAG_HIDE_MEW
	return

FarawayIsland_Interior_EventScript_ShowMew::
	clearflag FLAG_HIDE_MEW
	return

@ Mew dodges around in the grass (MOVEMENT_TYPE_COPY_PLAYER_OPPOSITE_IN_GRASS);
@ the player has to corner it against a wall to talk to it.
FarawayIsland_Interior_EventScript_Mew::
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MEW, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	seteventmon SPECIES_MEW, 30
	setflag FLAG_SYS_SPECIAL_WILD_BATTLE
	special StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_SPECIAL_WILD_BATTLE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FarawayIsland_Interior_EventScript_CaughtMew
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, FarawayIsland_Interior_EventScript_MewFled
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, FarawayIsland_Interior_EventScript_MewFled
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, FarawayIsland_Interior_EventScript_MewFled
	release
	end

FarawayIsland_Interior_EventScript_CaughtMew::
	setflag FLAG_CAUGHT_MEW
	goto EventScript_RemoveStaticMon
	end

FarawayIsland_Interior_EventScript_MewFled::
	setvar VAR_0x8004, SPECIES_MEW
	goto EventScript_MonFlewAway
	end
