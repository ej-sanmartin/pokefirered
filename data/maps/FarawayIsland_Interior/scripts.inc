FarawayIsland_Interior_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, FarawayIsland_Interior_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, FarawayIsland_Interior_OnTransition
	.byte 0

@ Follows the official legendary encounters (e.g. Articuno): Mew is gone for
@ good once it is caught or knocked out, but returns after it flees.
FarawayIsland_Interior_OnResume::
	call_if_set FLAG_SYS_SPECIAL_WILD_BATTLE, FarawayIsland_Interior_EventScript_TryRemoveMew
	end

FarawayIsland_Interior_EventScript_TryRemoveMew::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, EventScript_Return
	removeobject VAR_LAST_TALKED
	return

FarawayIsland_Interior_OnTransition::
	call_if_set FLAG_FOUGHT_MEW, FarawayIsland_Interior_EventScript_HideMew
	call_if_unset FLAG_FOUGHT_MEW, FarawayIsland_Interior_EventScript_ShowMew
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
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, FarawayIsland_Interior_EventScript_DefeatedMew
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, FarawayIsland_Interior_EventScript_RanFromMew
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, FarawayIsland_Interior_EventScript_RanFromMew
	setflag FLAG_FOUGHT_MEW
	release
	end

FarawayIsland_Interior_EventScript_DefeatedMew::
	setflag FLAG_FOUGHT_MEW
	goto EventScript_RemoveStaticMon
	end

FarawayIsland_Interior_EventScript_RanFromMew::
	setvar VAR_0x8004, SPECIES_MEW
	goto EventScript_MonFlewAway
	end
