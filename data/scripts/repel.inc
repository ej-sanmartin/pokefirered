EventScript_RepelWoreOff::
	lockall
	msgbox Text_RepelWoreOff, MSGBOX_DEFAULT
	specialvar VAR_RESULT, HasLastRepelItem
	goto_if_eq VAR_RESULT, FALSE, EventScript_RepelWoreOffEnd
	bufferitemname STR_VAR_1, VAR_LAST_REPEL_ITEM
	msgbox Text_UseAnotherRepel, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RepelWoreOffEnd
	special UseLastRepelItem
	msgbox Text_PlayerUsedRepel, MSGBOX_DEFAULT
EventScript_RepelWoreOffEnd::
	releaseall
	end

Text_RepelWoreOff::
	.string "REPEL's effect wore off…$"

Text_UseAnotherRepel::
	.string "Would you like to use another\n"
	.string "{STR_VAR_1}?$"

Text_PlayerUsedRepel::
	.string "{PLAYER} used the {STR_VAR_1}.$"
