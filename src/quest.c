/***************************************************************************
 *                         _____ ________  _____  ________              // *
 *                        / ___// ____/  |/  / / / / ____/       #### //   *
 *                        \__ \/ /_  / /|_/ / / / / / __       #####//#    *
 *                       ___/ / __/ / /  / / /_/ / /_/ /       ###//###    *
 *                      /____/_/   /_/  /_/\____/\____/         //###      *
 *                                                            //           *
 *                 Simulated Fururistic Multi Universe Game                *
 ***************************************************************************
 * - SFMUG 0.3  (c) 2003 by Bill Sica (Lews)                               *
 ***************************************************************************
 * - Chronicles Copyright 2001, 2002 by Brad Ensley (Orion Elder)          *
 ***************************************************************************
 * - SWR 2.0 (c) 1997, 1998 by Sean Cooper (specs@golden.net)              *
 *   based on a concept and ideas from the original SWR 1.0 immortals:     *
 *   Himself (Durga), Mark Matt (Merth), Jp Coldarone (Exar),              *
 *   Greg Baily (Thrawn), Ackbar, Satin, Streen and Bib                    *
 *   as well as much input from our other builders and players.            *
 ***************************************************************************
 * - SMAUG 1.4  Copyright 1994, 1995, 1996, 1998 by Derek Snider           *
 * - Merc  2.1  Copyright 1992, 1993 by Michael Chastain, Michael Quan,    *
 *   and Mitchell Tse.                                                     *
 * - DikuMud    Copyright 1990, 1991 by Sebastian Hammer, Michael Seifert, *
 *   Hans-Henrik Stærfeldt, Tom Madsen, and Katja Nyboe.                   *
 ***************************************************************************
 * - Quest module                                                          *
 ***************************************************************************/

#include <string.h>
#include "mud.h"
#include "quest.h"

/*
 * Global Variables
 */
QPRIZE_DATA *first_qprize;
QPRIZE_DATA *last_qprize;
QTARG_DATA *first_qtarg;
QTARG_DATA *last_qtarg;


/*
 * Runs the major points of the Quest Module. Calls up Quest Based
 * Information, allows the purchasing of Quest Objects, and allows
 * 'ch' to Request or Quit a Quest. Allows Immortals to set, adjust,
 * or clear a quest
 */
void do_quest( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
	return;

    if ( !argument || argument[0] == '\0' )
    {
	if ( IS_IMMORTAL( ch ) )
	    do_help( ch, "immquest" );
	else
	    do_help( ch, "quest" );
	return;
    }

    argument = one_argument( argument, arg1 );

    if ( !str_cmp( arg1, "buy" ) )
    {
	int count = 1, targ_value = 0;
	char target[MAX_STRING_LENGTH];
	char quest_buf[MAX_STRING_LENGTH];
	bool foundPrize = FALSE, minLevel = TRUE, maxLevel = TRUE;
	OBJ_DATA *prize_object;
	CHAR_DATA *qm = find_questmaster( ch, FALSE );
	QPRIZE_DATA *qprize = NULL;

	argument = one_argument( argument, target );

	if ( !target || target[0] == '\0' || !is_number( target ) )
	{
	    if ( IS_IMMORTAL( ch ) )
		do_help( ch, "immquest" );
	    else
		do_help( ch, "quest" );
	    return;
	}

	targ_value = atoi( target );

	if ( !qm )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "A voice whispers in your ear, 'Nice try, buddy!'\n\r", ch );
	    send_to_char( "A voice whispers in your ear, 'Right... where is the questmaster? Oh yeah... NOT HERE!'\n\r", ch );
	    return;
	}

	for( qprize = first_qprize; qprize != NULL; qprize = qprize->next )
	{
	    if ( count == targ_value )
	    {
		foundPrize = TRUE;
		break;
	    }

	    count++;
	}

	if ( foundPrize == FALSE )
	{
	    sprintf( quest_buf, "0.%s %s, I am sorry that you can't read... that prize isn't in the list... try again.", ch->name, ch->name );
	    do_whisper( qm, quest_buf );
	    return;
	}

	if ( qprize->min_level == -1 )
	    minLevel = FALSE;

	if ( qprize->max_level == -1 )
	    maxLevel = FALSE;

	if ( ch->pcdata->quest_curr < qprize->cost )
	{
	    sprintf( quest_buf, "0.%s %s, I am sorry that you can't read... you aren't glorified enough for that!", ch->name, ch->name );
	    do_whisper( qm, quest_buf );
	    return;
	}

	if ( minLevel == TRUE )
	{
	    if ( ch->level < qprize->min_level )
	    {
		sprintf( quest_buf, "0.%s %s, You are not yet powerful enough for that item... try again later.", ch->name, ch->name );
		do_whisper( qm, quest_buf );
		return;
	    }
	}

	if ( maxLevel == TRUE )
	{
	    if ( ch->level > qprize->max_level )
	    {
		sprintf( quest_buf, "0.%s %s, You are too powerful for that item, now... I am truly sorry.", ch->name, ch->name );
		do_whisper( qm, quest_buf );
		return;
	    }
	}

	ch->pcdata->quest_curr -= qprize->cost;

	prize_object = create_object( get_obj_index( qprize->vnum ), ch->level );
	obj_to_char( prize_object, ch );

	set_char_color( AT_BLUE, ch );
	send_to_char( "Your prize slowly fades into existence within your hands...\n\r", ch );
	if ( ch->pcdata->quest_curr > 0 )
	    sprintf( quest_buf, "0.%s I hope you enjoy your new prize, %s... you more than deserve it!", ch->name, ch->name );
	else
	    sprintf( quest_buf, "0.%s I hope you enjoy your new prize, %s... you deserve it.", ch->name, ch->name );
	do_whisper( qm, quest_buf );
	return;
    }
    else if ( !str_cmp( arg1, "clear" ) )
    {
	char target[MAX_STRING_LENGTH];
	CHAR_DATA *victim = NULL;

	if ( !IS_IMMORTAL( ch ) )
	{
	    do_quest( ch, "" );
	    return;
	}

	argument = one_argument( argument, target );

	if ( !target || target[0] == '\0' )
	{
	    do_quest( ch, "" );
	    return;
	}

	victim = get_char_world( ch, target );

	if ( !victim )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "They're nowhere to be found.\n\r", ch );
	    return;
	}

	if ( IS_NPC( victim ) )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "Mobs can not have quest data.\n\r", ch );
	    return;
	}

	clear_questdata( victim, FALSE );
	if ( ch != victim )
	{
	    set_char_color( AT_GREEN, ch );
	    ch_printf( ch, "%s's quest data has been cleared.\n\r", PERS( victim, ch ) );
	}
	if ( ch != victim )
	{
	    set_char_color( AT_GREEN, victim );
	    ch_printf( victim, "Your quest data has been cleared by %s.\n\r", PERS( ch, victim ) );
	}
	else
	{
	    set_char_color( AT_GREEN, victim );
	    send_to_char( "Your quest data has been cleared.\n\r", victim );
	}
	return;
    }
    else if ( !str_cmp( arg1, "complete" ) )
    {
	int reward = 0;
	char quest_buf[MAX_STRING_LENGTH];
	CHAR_DATA *qm = find_questmaster( ch, TRUE );

	if ( !qm )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    send_to_char( "'What was I thinking... there is no Questmaster here...'\n\r", ch );
	    return;
	}

	if ( !IS_QUESTING( ch ) )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    ch_printf( ch, "'What was I thinking... %s didn't give me a quest yet!'\n\r", NAME( qm ) );
	    return;
	}

	if ( qm != ch->pcdata->questmaster )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    ch_printf( ch, "'What was I thinking... %s didn't give me this quest... %s did!'\n\r", NAME( qm ), NAME( ch->pcdata->questmaster ) );
	    return;
	}

	if ( ch->pcdata->quest_type == QT_OBJ )
	{
	    OBJ_DATA *target;

	    for( target = ch->first_carrying; target != NULL; target = target->next_content )
	    {
		if ( target == ch->pcdata->quest_obj )
		{
		    ch->pcdata->quest_type = QT_COMPLETE;

		    separate_obj( target );
		    obj_from_char( target );
		    extract_obj( target );
		}
	    }
	}

	if ( ch->pcdata->quest_type == QT_MOB_GIVE_COMPLETE )
	{
	    ch->pcdata->quest_type = QT_COMPLETE;
	}

	if ( ch->pcdata->quest_type == QT_FAILED )
	{
	    if ( ch->pcdata->quest_obj )
	    {
		separate_obj( ch->pcdata->quest_obj );
		obj_from_char( ch->pcdata->quest_obj );
		extract_obj( ch->pcdata->quest_obj );
	    }

	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    ch_printf( ch, "'What was I thinking... I didn't complete my quest from %s, I completely botched it!'\n\r", NAME( ch->pcdata->questmaster ) );
	    clear_questdata( ch, FALSE );
	    ch->pcdata->quest_timer[QTIMER_NEXT] = number_fuzzy( qdata.qtime_next_fail * 2 );
	    sprintf( quest_buf, "0.%s %s, I am SORELY disappointed in you. I gave you a simple task and you completely destroyed it. Begone!", ch->name, ch->name );
	    do_whisper( qm, quest_buf );
	    sprintf( quest_buf, "0.%s I will have no further use for you for a LONG while!", ch->name );
	    do_whisper( qm, quest_buf );
	    return;
	}

	if ( ch->pcdata->quest_type != QT_COMPLETE )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    ch_printf( ch, "'What was I thinking... I didn't complete my quest from %s yet!'\n\r", NAME( ch->pcdata->questmaster ) );
	    return;
	}

	sprintf( quest_buf, "0.%s %s, I thank you. The realm is indeed a better place thanks to you!", ch->name, ch->name );
	do_whisper( qm, quest_buf );

	reward = number_range( qdata.minqp_gain, qdata.maxqp_gain );
	ch->pcdata->quest_curr += reward;
	ch->pcdata->quest_accum += reward;
//	adjust_hiscore( "glory", ch, ch->pcdata->quest_accum, "intro" );
	sprintf( quest_buf, "0.%s All the realms shall know of this great deed you have done!", ch->name );
	do_whisper( qm, quest_buf );

	if ( quest_chance( qdata.prac_chance ) )
	{
	    reward = number_range( qdata.minprac_gain, qdata.maxprac_gain );
	    ch->practice += reward;
	    sprintf( quest_buf, "0.%s Don't tell anyone, but I also gave you a small extra gift.", ch->name );
	    do_whisper( qm, quest_buf );
	}

	if ( quest_chance( 50 ) )
	{
	    reward = number_range( qdata.mingold_gain, qdata.maxgold_gain );
	    ch->gold += reward;
	    sprintf( quest_buf, "0.%s Here, there are %s coins here... please take this as a token of my gratitude.", ch->name, num_punct( reward ) );
	    do_whisper( qm, quest_buf );
	}

	clear_questdata( ch, FALSE );
	reward = number_range( qdata.minqt_comp, qdata.maxqt_comp );
	ch->pcdata->quest_timer[QTIMER_NEXT] = reward;
	sprintf( quest_buf, "0.%s Come see me later, and I'll see if I have anything more for you.", ch->name );
	do_whisper( qm, quest_buf );
    }
    else if ( !str_cmp( arg1, "info" ) )
    {
	char value[MAX_STRING_LENGTH];
	CHAR_DATA *victim = NULL;

	argument = one_argument( argument, value );
	if ( !value || value[0] == '\0' )
	{
	    do_quest( ch, "info self" );
	    return;
	}

	if ( !IS_IMMORTAL( ch ) && str_cmp( value, "self" ) )
	{
	    do_quest( ch, "info self" );
	    return;
	}

	victim = get_char_world( ch, value );
	
	if ( !victim )
	{
	    set_char_color( AT_BLUE, ch );
	    ch_printf( ch, "%s doesn't seem to be here.\n\r", value );
	    return;
	}
	else if ( IS_NPC( victim ) )
	{
	    set_char_color( AT_BLUE, ch );
	    send_to_char( "Mobs can not have quest data.\n\r", ch );
	    return;
	}
	else if ( !IS_QUESTING( victim ) )
	{
	    set_char_color( AT_BLUE, ch );
	    ch_printf( ch, "%s %s not questing at this time.\n\r", ch == victim ? "You" : victim->name, ch == victim ? "are" : "is" );
	    if ( IS_IMMORTAL( ch ) )
	    {
		if ( victim->pcdata->quest_timer[QTIMER_NEXT] > 0 )
		    ch_printf( ch, "%s %s %s minutes left until they can quest again.\n\r", ch == victim ? "You" : victim->name, ch == victim ? "have" : "has", num_punct( victim->pcdata->quest_timer[QTIMER_NEXT] ) );
		else
		    ch_printf( ch, "%s can quest again at any time.\n\r", ch == victim ? "You" : victim->name );
	    }
	    return;
	}
	else
	{
	    bool sTarg = ( ( ch == victim ) ? TRUE : FALSE );
	    set_char_color( AT_BLUE, ch );
	    ch_printf_color( ch, "&r&G&Y&w%s &G&B%s currently performing a quest for &G&w%s&G&B.\n\r", sTarg ? "You" : victim->name, sTarg ? "are" : "is", NAME( victim->pcdata->questmaster ) );
	    if ( victim->pcdata->quest_type == QT_FAILED || victim->pcdata->quest_type == QT_COMPLETE )
	    {
		bool qFailed, qComplete;

		qFailed = victim->pcdata->quest_type == QT_FAILED ? TRUE : FALSE;
		qComplete = victim->pcdata->quest_type == QT_COMPLETE ? TRUE : FALSE;

		ch_printf_color( ch, "&r&G&Y&w%s &G&Bhas %s said quest &G&w%s&G&B!\n\r", sTarg ? "You" : victim->name, victim->pcdata->quest_type == qFailed ? "FAILED" : qComplete ? "COMPLETED" : "done something to", qFailed ? "miserably" : qComplete ? "flawlessly" : "somehow" );
		ch_printf_color( ch, "&r&G&Y&w%s &G&Bhas %s minutes left before %s %s %s!\n\r", sTarg ? "You" : victim->name, num_punct( ch->pcdata->quest_timer[QTIMER_CURR] ), NAME( victim->pcdata->questmaster ), qComplete ? "rewards" : qFailed ? "reams" : "does something to", sTarg ? "you" : victim->sex == SEX_MALE ? "him" : victim->sex == SEX_FEMALE ? "her" : "it" );
		return;
	    }
	    ch_printf_color( ch, "&r&G&Y&w%s &G&Basked %s to ", NAME( victim->pcdata->questmaster ), sTarg ? "you" : victim->name );
	    if ( victim->pcdata->quest_type == QT_OBJ )
	    {
		ch_printf_color( ch, "&G&Bbring %s &G&w%s&G&B.\n\r", victim->pcdata->questmaster->sex == SEX_MALE ? "him" : victim->pcdata->questmaster->sex == SEX_FEMALE ? "her" : "it", victim->pcdata->quest_obj->short_descr );
		ch_printf_color( ch, "&r&G&Y&w%s &G&Bwas last seen in the area of &G&w%s&G&B.\n\r", victim->pcdata->quest_obj->short_descr, victim->pcdata->quest_area->name );
	    }
	    else if ( victim->pcdata->quest_type == QT_MOB_KILL )
	    {
		MOB_INDEX_DATA *target = get_mob_index( victim->pcdata->quest_mob );

		ch_printf_color( ch, "&G&Bkill %s for %s crimes!\n\r", target->short_descr, target->sex == SEX_MALE ? "his" : target->sex == SEX_FEMALE ? "her" : "its" );
		ch_printf_color( ch, "&r&G&Y&w%s &G&Bwas last seen in the area of &G&w%s&G&B.\n\r", target->short_descr, victim->pcdata->quest_area->name );
	    }
	    else if ( victim->pcdata->quest_type == QT_MOB_GIVE || victim->pcdata->quest_type == QT_MOB_GIVE_COMPLETE )
	    {
		bool qComp = victim->pcdata->quest_type == QT_MOB_GIVE_COMPLETE ? TRUE : FALSE;
		MOB_INDEX_DATA *target = get_mob_index( victim->pcdata->quest_mob );

		ch_printf_color( ch, "&G&Btake %s to %s", victim->pcdata->quest_obj->short_descr, target->short_descr );
		if ( qComp )
		{
		    ch_printf_color( ch, "&G&Band %s %s so.\n\r", sTarg ? "you have" : victim->name, sTarg ? "done" : "has done" );
		}
		else
		{
		    send_to_char_color( "&G&B.\n\r", ch );
		}
		ch_printf_color( ch, "&r&G&Y&w%s &G&Bwas last seen in the area of &G&w%s&G&B.\n\r", target->short_descr, victim->pcdata->quest_area->name );
	    }
	    ch_printf_color( ch, "&r&G&Y&w%s &G&B%s %s minutes left to complete %s quest!\n\r", sTarg ? "You" : victim->name, sTarg ? "have" : "has", num_punct( victim->pcdata->quest_timer[QTIMER_CURR] ), sTarg ? "your" : victim->sex == SEX_MALE ? "his" : victim->sex == SEX_FEMALE ? "her" : "its" );
	    return;
	}
    }
    else if ( !str_cmp( arg1, "list" ) )
    {
	int list_count = 1;
	CHAR_DATA *qm = find_questmaster( ch, FALSE );
	QPRIZE_DATA *qprize = NULL;
	OBJ_INDEX_DATA *object = NULL;

	if ( !qm )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "A voice whispers in your ear, 'Nice try, buddy!'\n\r", ch );
	    send_to_char( "A voice whispers in your ear, 'Right... where is the questmaster? Oh yeah... NOT HERE!'\n\r", ch );
	    return;
	}

	send_to_char_color( "\n\r&r&G&Y&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );

	for( qprize = first_qprize; qprize != NULL; qprize = qprize->next )
	{
	    object = get_obj_index( qprize->vnum );

	    ch_printf_color( ch, " &r&Y&G&Y[&G&w%2d&G&Y] &G&W%6s&G&Y..........&G&W%s&Y&G\n\r", list_count, num_punct( qprize->cost ), object->short_descr );
	    list_count++;
	}

	send_to_char_color( "&r&G&Y&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );
    }
    else if ( !str_cmp( arg1, "request" ) )
    {
	CHAR_DATA *qm = find_questmaster( ch, FALSE );

	if ( !qm )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    send_to_char( "'What was I thinking... there is no Questmaster here...'", ch );
	    return;
	}

	if ( IS_QUESTING( ch ) )
	{
	    set_char_color( AT_GREY, ch );
	    send_to_char( "You think to yourself about how much of a _DOPE_ you are.\n\r", ch );
	    ch_printf( ch, "'What was I thinking... %s already gave me a quest!'", NAME( ch->pcdata->questmaster ) );
	    return;
	}

	if ( ch->pcdata->quest_timer[QTIMER_NEXT] != 0 )
	{
	    char quest_buf[MAX_STRING_LENGTH];

	    sprintf( quest_buf, "0.%s %s, there is a magical aura about you that says you just aren't ready yet.", ch->name, ch->name );
	    do_whisper( qm, quest_buf );
	    sprintf( quest_buf, "0.%s Try back in a little while... you may be ready then.", ch->name );
	    do_whisper( qm, quest_buf );
	    return;
	}

	/*
	 * The questmaster whispers the quests, so they wouldn't hear it if he
	 * can't see them. Meaning REALLY hard quest to complete. -Orion
	 */
	if ( !can_see( qm, ch ) )
	{
	    char quest_buf[MAX_STRING_LENGTH];

	    sprintf( quest_buf, "Very funny. I can't see you, %s, and you expect me to send you on a quest? Child.", PERS( ch, qm ) );
	    do_say( qm, quest_buf );
	    return;
	}

	generate_quest( ch, qm );
	return;
    }
    else if ( !str_cmp( arg1, "set" ) )
    {
	char target[MAX_STRING_LENGTH];
	char field[MAX_STRING_LENGTH];
	char value[MAX_STRING_LENGTH];
	CHAR_DATA *victim = NULL;

	if ( !IS_IMMORTAL( ch ) )
	{
	    do_quest( ch, "" );
	    return;
	}

	argument = one_argument( argument, target );

	if ( !target || target[0] == '\0' )
	{
	    do_quest( ch, "" );
	    return;
	}

	victim = get_char_world( ch, target );

	if ( !victim )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "They're nowhere to be found.\n\r", ch );
	    return;
	}

	if ( IS_NPC( victim ) )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "Mobs can not have quest data.\n\r", ch );
	    return;
	}

	if ( ch == victim )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "You can't set your own quest data... nice try though.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, field );
	argument = one_argument( argument, value );

	if ( ( !field || field[0] == '\0' ) || ( !value || value[0] == '\0' )  )
	{
	    do_quest( ch, "" );
	    return;
	}
	else if ( !str_cmp( field, "qtn" ) )
	{
	    int time = 0;

	    if ( !is_number( value ) )
	    {
		set_char_color( AT_BLUE, ch );
		ch_printf( ch, "%s is not a valid value.\n\r", value );
		return;
	    }

	    if ( IS_QUESTING( victim ) )
	    {
		set_char_color( AT_BLUE, ch );
		ch_printf( ch, "%s is questing at this time.\n\r", victim->name );
		return;
	    }

	    time = atoi( value );
	    if ( time <= 0 )
	    {
		time = 1;
	    }

	    victim->pcdata->quest_timer[QTIMER_NEXT] = time;
	    ch_printf( ch, "%s's time left until they can quest again has been set to %s.\n\r", victim->name, num_punct( time ) );
	    return;
	}
	else if ( !str_cmp( field, "qtc" ) )
	{
	    int time = 0;

	    if ( !is_number( value ) )
	    {
		set_char_color( AT_BLUE, ch );
		ch_printf( ch, "%s is not a valid value.\n\r", value );
		return;
	    }

	    if ( !IS_QUESTING( victim ) )
	    {
		set_char_color( AT_BLUE, ch );
		ch_printf( ch, "%s is not questing at this time.\n\r", victim->name );
		return;
	    }

	    time = atoi( value );
	    if ( time <= 0 )
	    {
		time = 1;
	    }

	    victim->pcdata->quest_timer[QTIMER_CURR] = time;
	    ch_printf( ch, "%s's time left for this quest has been set to %s.\n\r", victim->name, num_punct( time ) );
	    return;
	}
	else
	    do_quest( ch, "" );

	return;
    }
    else
	do_quest( ch, "" );
}


/*
 * Allows the changing of Quest Parameters online
 */
void do_qset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );

    if ( !arg1 || arg1[0] == '\0' )
    {
	send_to_pager_color( "&r&Y&G&O___&G&Y==&G&w-----&G&Y=====&G&w-----&G&Y=====&G&w-----&G&Y=&G&W[&G&YO&G&W]&G&Y=&G&w-----&G&Y=====&G&w-----&G&Y=====&G&w-----&G&Y==&G&O___\n\r", ch );
	pager_printf_color( ch, " &G&YMinimum Practices          &G&w%3d &G&W| &G&YMaximum Practices       &G&w%3d\n\r", qdata.minprac_gain, qdata.maxprac_gain );
	pager_printf_color( ch, " &G&YMinimum Gold            &G&w%6d &G&W| &G&YMaximum Gold         &G&w%6d\n\r", qdata.mingold_gain, qdata.maxgold_gain );
	pager_printf_color( ch, " &G&YMinimum Questpoints        &G&w%3d &G&W| &G&YMaximum Questpoints     &G&w%3d\n\r", qdata.minqp_gain, qdata.maxqp_gain );
	pager_printf_color( ch, " &G&YMinimum Quest Time          &G&w%2d &G&W| &G&YMaximum Quest Time       &G&w%2d\n\r", qdata.minq_time, qdata.maxq_time );
	pager_printf_color( ch, " &G&YMinimum Completed Wait      &G&w%2d &G&W| &G&YMaximum Completed Wait   &G&w%2d\n\r", qdata.minqt_comp, qdata.maxqt_comp );
	send_to_pager_color( "   &r&Y&G&W=============================&G&w+&G&W=============================\n\r", ch );
	pager_printf_color( ch, " &G&YQuit Wait                   &G&w%2d &G&W| &G&YFailed Wait              &G&w%2d\n\r", qdata.qtime_next_quit, qdata.qtime_next_fail );
	pager_printf_color( ch, " &G&YCleared Wait                &G&w%2d &G&W| &G&YQuestpoint Loss          &G&w%2d\n\r", qdata.qtime_next_clear, qdata.qp_loss );
	send_to_pager_color( "   &r&Y&G&W=============================&G&w+&G&W=============================\n\r", ch );
	pager_printf_color( ch, " &G&YQuest Level Difference     &G&w%3d &G&W| &G&YGive Quest VNum &G&w%11d\n\r", qdata.qlevel_diff, qdata.give_quest );
	pager_printf_color( ch, " &G&YQuest Chance               &G&w%3d &G&W| &G&YObject Chance           &G&w%3d\n\r", qdata.quest_chance, qdata.object_chance );
	pager_printf_color( ch, " &G&YPractices Chance           &G&w%3d &G&W|\n\r", qdata.prac_chance );
	send_to_pager_color( "&r&Y&G&O___&G&Y==&G&w-----&G&Y=====&G&w-----&G&Y=====&G&w-----&G&Y=&G&W[&G&YO&G&W]&G&Y=&G&w-----&G&Y=====&G&w-----&G&Y=====&G&w-----&G&Y==&G&O___\n\r", ch );
	return;
    }
    else if ( !str_cmp( arg1, "qdata" ) )
    {
	int value = 0;
	char arg2[MAX_STRING_LENGTH];
	char val[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, val );

	if ( !arg2 || arg2[0] == '\0' )
	{
	    do_qset( ch, "" );
	    return;
	}
	else if ( !str_cmp( arg2, "save" ) )
	{
	    set_char_color( AT_GREEN, ch );
	    send_to_char_color( "The quest data has been written.\n\r", ch );
	    write_qdata( );
	    return;
	}

	if ( !val || val[0] == '\0' )
	{
	    do_qset( ch, "" );
	    return;
	}

	if ( !is_number( val ) )
	{
	    if ( !str_cmp( val, "none" ) )
	    {
		sprintf( val, "%d", -1 );
	    }
	    else
	    {
		do_qset( ch, "" );
		return;
	    }
	}
	else
	{
	    value = atoi( val );
	}

	if ( !str_cmp( arg2, "minprac" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.minprac_gain = value;
	}
	else if ( !str_cmp( arg2, "maxprac" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.maxprac_gain = value;
	}
	else if ( !str_cmp( arg2, "mingold" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.mingold_gain = value;
	}
	else if ( !str_cmp( arg2, "maxgold" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.maxgold_gain = value;
	}
	else if ( !str_cmp( arg2, "minqp" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.minqp_gain = value;
	}
	else if ( !str_cmp( arg2, "maxqp" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.maxqp_gain = value;
	}
	else if ( !str_cmp( arg2, "minqtcomp" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.minqt_comp = value;
	}
	else if ( !str_cmp( arg2, "maxqtcomp" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.maxqt_comp = value;
	}
	else if ( !str_cmp( arg2, "minqtime" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.minq_time = value;
	}
	else if ( !str_cmp( arg2, "maxqtime" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.maxq_time = value;
	}
	else if ( !str_cmp( arg2, "qploss" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.qp_loss = value;
	}
	else if ( !str_cmp( arg2, "qtnquit" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.qtime_next_quit = value;
	}
	else if ( !str_cmp( arg2, "qtnfail" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.qtime_next_fail = value;
	}
	else if ( !str_cmp( arg2, "qtnclear" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.qtime_next_clear = value;
	}
	else if ( !str_cmp( arg2, "qleveldiff" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.qlevel_diff = value;
	}
	else if ( !str_cmp( arg2, "qchance" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.quest_chance = value;
	}
	else if ( !str_cmp( arg2, "objchance" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.object_chance = value;
	}
	else if ( !str_cmp( arg2, "givechance" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.give_quest = value;
	}
	else if ( !str_cmp( arg2, "pracchance" ) )
	{
	    if ( value <= 0 )
	    {
		set_char_color( AT_GREEN, ch );
		send_to_char( "Negative values are not allowed in this field.\n\r", ch );
		return;
	    }

	    qdata.prac_chance = value;
	}

	ch_printf( ch, "The %s is set to %s.\n\r", arg2, value < 0 ? "nothing" : num_punct( value ) );
	return;
    }
    else if ( !str_cmp( arg1, "qprize" ) )
    {
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    QPRIZE_DATA *qprize = NULL;

	    send_to_char_color( "\n\r&r&Y&G&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );
	    for( qprize = first_qprize; qprize != NULL; qprize = qprize->next )
	    {
		char min_lev[MAX_STRING_LENGTH], max_lev[MAX_STRING_LENGTH];

		OBJ_INDEX_DATA *prize = get_obj_index( qprize->vnum );

		sprintf( min_lev, "%s", num_punct( qprize->min_level ) );
		sprintf( max_lev, "%s", num_punct( qprize->max_level ) );

		ch_printf_color( ch, "\n\r&r&Y&G&YQPrize&G&W: &G&w%s\n\r", prize->short_descr );
		ch_printf_color( ch, "&r&Y&G&YVNum  &G&W: &G&w%-10d   &G&YMinLevel&G&W: &G&w%-4s  &G&YMaxLevel&G&W: &G&w%-4s  &G&YCost&G&W: &G&W%-11d\n\r", qprize->vnum, qprize->min_level < 0 ? "none" : min_lev, qprize->max_level < 0 ? "none" : max_lev, qprize->cost );
	    }
	    send_to_char_color( "\n\r&r&Y&G&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );
	}
	else if ( !str_cmp( arg2, "" ) )
	{
	}
    }
    else if ( !str_cmp( arg1, "qtarget" ) )
    {
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    QTARG_DATA *qtarg = NULL;

	    send_to_char_color( "\n\r&r&Y&G&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );
	    for( qtarg = first_qtarg; qtarg != NULL; qtarg = qtarg->next )
	    {
		OBJ_INDEX_DATA *targ = get_obj_index( qtarg->vnum );

		ch_printf_color( ch, "\n\r&r&Y&G&YQTarget&G&W: &G&w%s\n\r", targ->short_descr );
		ch_printf_color( ch, "&r&Y&G&YVNum   &G&W: &G&w%-10d\n\r", qtarg->vnum );
	    }
	    send_to_char_color( "\n\r&r&Y&G&O_____&G&Y---&G&w=&G&W[&G&YO&G&W]&G&w=&G&Y---&G&O_____\n\r", ch );
	}
	else if ( !str_cmp( arg2, "" ) )
	{
	}
    }
    else
    {
	do_qset( ch, "" );
	return;
    }
}


/*
 * Checks the room of 'ch' to see if a usable questmaster exists
 */
CHAR_DATA *find_questmaster( CHAR_DATA *ch, bool canBeImm )
{
    CHAR_DATA *questmaster;

    for ( questmaster = ch->in_room->first_person;
	  questmaster != NULL;
	  questmaster = questmaster->next_in_room )
    {
	if ( !IS_NPC( questmaster ) && !canBeImm )
	    continue;

	if ( xIS_SET( questmaster->act, ACT_QUESTMASTER )
	  || questmaster->spec_fun == spec_lookup( "spec_questmaster" )
	  || ( IS_IMMORTAL( questmaster ) && canBeImm ) )
	    break;
    }

    if ( questmaster == NULL )
    {
	send_to_char("You can't do that here.\n\r", ch);
	return NULL;
    }

    if ( ( questmaster->position == POS_FIGHTING ) || questmaster->fighting )
    {
	ch_printf( ch, "Wait until %s stops fighting!\n\r",
	    NAME( questmaster ) );
	return NULL;
    }

    return questmaster;
}


/*
 * Updates quest information for every PC in the game
 */
void quest_update( void )
{
    CHAR_DATA *vch;

    for( vch = first_char; vch != NULL; vch = vch->next )
    {
	if ( IS_NPC( vch ) )
	    continue;

	if ( IS_QUESTING( vch ) )
	{
	    if ( vch->pcdata->quest_timer[QTIMER_CURR] > 1 )
	    {
		vch->pcdata->quest_timer[QTIMER_CURR]--;

		if ( vch->pcdata->quest_timer[QTIMER_CURR] >= 1
		  && vch->pcdata->quest_timer[QTIMER_CURR] <= 5 )
		{
		    set_char_color( AT_GREY, vch );
		    send_to_char( "A voice whispers in your ear, 'Better hurry!'\n\r", vch );
		}
	    }
	    else
	    {
		int qploss, divider;
		bool botchedQuest = FALSE;

		if ( vch->pcdata->quest_obj )
		{
		    separate_obj( vch->pcdata->quest_obj );
		    obj_from_char( vch->pcdata->quest_obj );
		    extract_obj( vch->pcdata->quest_obj );
		}

		if ( vch->pcdata->quest_type == QT_FAILED )
		{
		    botchedQuest = TRUE;
		}

		clear_questdata( vch, FALSE );

		divider	= get_curr_lck( vch ) > 15 ? number_range( 1, 5 ) :
						     number_range( 1, 3 );
		qploss	= (qdata.qp_loss/divider);

		vch->pcdata->quest_curr -= qploss;
		if ( !botchedQuest )
		    vch->pcdata->quest_timer[QTIMER_NEXT] = number_fuzzy( qdata.qtime_next_fail );
		else
		    vch->pcdata->quest_timer[QTIMER_NEXT] = number_fuzzy( qdata.qtime_next_fail * 2 );

		set_char_color( AT_GREY, vch );
                ch_printf( vch, "A voice whispers in your ear, 'Your time is up... try again in about %d minutes.'\n\r", number_fuzzy( vch->pcdata->quest_timer[QTIMER_NEXT] ) );
		send_to_char( "A voice whispers in your ear, 'I almost forgot! For failing your quest, the people now think less of you, good job!'\n\r", vch );
		send_to_char( "A voice whispers in your ear, 'Oh, and one last thing... you totally screwed the pooch on that one. We'll have no need of you for a LONG while.'\n\r", vch );
		xREMOVE_BIT( vch->act, PLR_QUESTING );
	    }
	}
	else
	{
	    if ( vch->pcdata->quest_timer[QTIMER_NEXT] > 1 )
	    {
		vch->pcdata->quest_timer[QTIMER_NEXT]--;
	    }
	    else if ( vch->pcdata->quest_timer[QTIMER_NEXT] != 0 )
	    {
		vch->pcdata->quest_timer[QTIMER_NEXT] = 0;
		set_char_color( AT_GREY, vch );
		send_to_char( "A voice whispers in your ear, 'A new adventure awaits!'\n\r", vch );
	    }
	}
    }
}


/*
 * Clears the quest data of 'ch'
 */
void clear_questdata( CHAR_DATA *ch, bool quit )
{
    if ( IS_NPC( ch ) )
    {
	bug( "clear_questdata: attempting to clear quest data for NPC", 0 );
	return;
    }

    if ( quit == TRUE )
    {
	ch->pcdata->quest_timer[QTIMER_NEXT]	= number_fuzzy( qdata.qtime_next_quit );
	ch->pcdata->quest_curr		-= qdata.qp_loss;
    }
    else
    {
	ch->pcdata->quest_timer[QTIMER_NEXT]	= qdata.qtime_next_clear;
    }

    ch->pcdata->questmaster			= NULL;
    ch->pcdata->quest_timer[QTIMER_CURR]	= 0;
    ch->pcdata->quest_type			= QT_NONE;
    ch->pcdata->quest_mob			= 0;
    ch->pcdata->quest_obj			= NULL;
    ch->pcdata->quest_area			= NULL;
    xREMOVE_BIT( ch->act, PLR_QUESTING );
}


/*
 * Called by do_quest to generate a valid quest for 'ch' from 'questmaster'
 */
void generate_quest( CHAR_DATA *ch, CHAR_DATA *questmaster )
{
    int i = 0, choice = 0, tries = 500, timer = 0;
    bool qMobFound = FALSE;
    char quest_buf[MAX_STRING_LENGTH];
    CHAR_DATA *target = NULL;
    AREA_DATA *area = NULL;

    if ( !quest_chance( qdata.quest_chance ) )
    {
	char buf[MAX_STRING_LENGTH];
	int timer;

	sprintf( buf, "0.%s I'm sorry, %s, but I have nothing for you at this time.", ch->name, ch->name );
	do_whisper( questmaster, buf );
	timer = number_range( qdata.minqt_comp, qdata.maxqt_comp );
	ch->pcdata->quest_timer[QTIMER_NEXT] = timer;
	return;
    }

    for ( i = 0; i < tries; i++ )
    {
	int x = 0;
	choice = number_range( 1, nummobsloaded );

	if ( choice >= ( nummobsloaded/2 ) )
	{
	    target = last_char;

	    for ( x = nummobsloaded; x > 0; x-- )
	    {
		while ( !IS_NPC( target ) )
		{
		    target = target->prev;
		}

		if ( x == choice )
		    break;

		target = target->prev;
	    }
	}
	else
	{
	    target = first_char;

	    for ( x = 0; x < nummobsloaded; x++ )
	    {
		while ( !IS_NPC( target ) )
		{
		    target = target->next;
		}

		if ( x == choice )
		    break;

		target = target->next;
	    }
	}

	if ( target == NULL
	  || target == questmaster
	  || target == supermob
	  || !IS_NPC( target )
	  || xIS_SET( target->act, ACT_NOQUEST )
	  || xIS_SET( target->act, ACT_PACIFIST )
/****************************************
 * Keep out aggressive mobs? Set to no. *
 ****************************************
	  || xIS_SET( target->act, ACT_AGGRESSIVE )
	  || xIS_SET( target->act, ACT_META_AGGR )
 ****************************************
 * Keep out aggressive mobs? Set to no. *
 ****************************************/
	  || !str_cmp( target->in_room->area->filename, "limbo.are" )
	  || IS_SET( target->in_room->area->flags, AFLAG_NOQUEST )
	  || IS_SET( target->in_room->room_flags, ROOM_PET_SHOP )
	  || IS_SET( target->in_room->room_flags, ROOM_SAFE )
	  || target->pIndexData->pShop != NULL
	  || xIS_SET( target->act, ACT_PRACTICE )
	  || xIS_SET( target->act, ACT_SCHOLAR )
	  || xIS_SET( target->act, ACT_IMMORTAL )
	  || xIS_SET( target->act, ACT_PET )
	  || xIS_SET( target->affected_by, AFF_CHARM )
	  || abs( ch->level - target->level ) > qdata.qlevel_diff
	  || !in_hard_range( ch, target->in_room->area ) )
	    continue;
	else
	{
	    qMobFound = TRUE;
	    break;
	}
    }

    if ( !qMobFound )
    {
	char buf[MAX_STRING_LENGTH];
	int timer;

	sprintf( buf, "0.%s I'm sorry, %s, but it seems that I have nothing available for someone of your talents at this time.", ch->name, ch->name );
	do_whisper( questmaster, buf );
	timer = number_range( qdata.minqt_comp, qdata.maxqt_comp );
	ch->pcdata->quest_timer[QTIMER_NEXT] = timer;
	return;
    }

    area = target->in_room->area;

    if ( quest_chance( qdata.object_chance ) && ( first_qtarg != NULL ) )
    {
	int targ_count = 0, chosen_targ = 0;
	QTARG_DATA *obj_targ = NULL;
	OBJ_DATA *quest_object = NULL;

	for( obj_targ = first_qtarg; obj_targ != NULL; obj_targ = obj_targ->next )
	    targ_count++;

	chosen_targ = number_range( 1, targ_count );

	targ_count = 1;

	for( obj_targ = first_qtarg; obj_targ != NULL; obj_targ = obj_targ->next )
	{
	    if ( chosen_targ == targ_count )
		break;

	    targ_count++;
	}

	quest_object = create_object( get_obj_index( obj_targ->vnum ), ch->level );
	ch->pcdata->quest_obj = quest_object;
	ch->pcdata->quest_type = QT_OBJ;
	if ( quest_chance( 60 ) )
	{
	    ROOM_INDEX_DATA *quest_room = target->in_room;

	    sprintf( quest_buf, "0.%s %s, I need for you to find %s for me. I seem to have misplaced it.", ch->name, ch->name, quest_object->short_descr );
	    do_whisper( questmaster, quest_buf );
	    sprintf( quest_buf, "0.%s %s, I believe it was lost somewhere in the area of %s.", ch->name, ch->name, area->name );
	    do_whisper( questmaster, quest_buf );
	    obj_to_room( quest_object, quest_room );
	}
	else
	{
	    sprintf( quest_buf, "0.%s %s, I need for you to recover %s for me! %s stole it from my chambers, while I was sleeping!", ch->name, ch->name, quest_object->short_descr, target->short_descr );
	    do_whisper( questmaster, quest_buf );
	    sprintf( quest_buf, "0.%s %s, I believe %s to be hiding in the area of %s.", ch->name, ch->name, target->short_descr, area->name );
	    do_whisper( questmaster, quest_buf );
	    obj_to_char( quest_object, target );
	}
    }
    else
    {
	OBJ_DATA *object = NULL;

	ch->pcdata->quest_mob = target->pIndexData->vnum;
	if ( quest_chance( 30 ) && qdata.give_quest != -1 )
	    ch->pcdata->quest_type = QT_MOB_GIVE;
	else
	    ch->pcdata->quest_type = QT_MOB_KILL;

	if ( ch->pcdata->quest_type == QT_MOB_GIVE && 
	   ( !xIS_SET( target->act, ACT_AGGRESSIVE ) &&
	     !xIS_SET( target->act, ACT_META_AGGR ) ) )
	{
	    object = create_object( get_obj_index( qdata.give_quest ), ch->level );
	    obj_to_char( object, ch );

	    ch->pcdata->quest_obj = object;

	    sprintf( quest_buf, "0.%s %s, I must ask you to get this %s to %s as soon as possible!", ch->name, ch->name, object->short_descr, target->short_descr );
	    do_whisper( questmaster, quest_buf );
	    sprintf( quest_buf, "0.%s %s, This is of the utmost of import. The last I heard, %s was in the area of %s.", ch->name, ch->name, target->short_descr, area->name );
	    do_whisper( questmaster, quest_buf );
	}
	else
	{
	    if ( ch->pcdata->quest_type != QT_MOB_KILL )
		ch->pcdata->quest_type = QT_MOB_KILL;

	    if ( quest_chance( 50 ) )
	    {
		sprintf( quest_buf, "0.%s %s, %s escaped from the local constable's office today. Since doing so, he's murdered %s citizens of our fair town!", ch->name, ch->name, target->short_descr, num_punct( number_range( 1, 9 ) ) );
		do_whisper( questmaster, quest_buf );
		sprintf( quest_buf, "0.%s %s, %s must be brought to justice immediately! The last I heard, %s was in the area of %s.", ch->name, ch->name, target->short_descr, target->short_descr, area->name );
		do_whisper( questmaster, quest_buf );
	    }
	    else if ( quest_chance( 50 ) )
	    {
		sprintf( quest_buf, "0.%s %s, it has recently been brought to my attention that %s is plotting against the throne.", ch->name, ch->name, target->short_descr );
		do_whisper( questmaster, quest_buf );
		sprintf( quest_buf, "0.%s %s must be stopped 'fore they pose a threat to the crown! The last I heard, %s is in the area of %s.", ch->name, target->short_descr, target->short_descr, area->name );
		do_whisper( questmaster, quest_buf );
	    }
	    else
	    {
		sprintf( quest_buf, "0.%s %s, I saw %s strolling the streets today. I thought it to be of no consequence at the time... then I saw %s approach an... interest... of mine.", ch->name, ch->name, target->short_descr, target->short_descr );
		do_whisper( questmaster, quest_buf );
		sprintf( quest_buf, "0.%s %s simply walked up and began talking to %s as if %s were %s betrothed! I would like for you to... clear up my problem. Understand?", ch->name, target->short_descr, questmaster->sex == SEX_MALE ? "her" : questmaster->sex == SEX_FEMALE ? "him" : "it", questmaster->sex == SEX_MALE ? "she" : questmaster->sex == SEX_FEMALE ? "he" : "it", target->sex == SEX_MALE ? "his" : target->sex == SEX_FEMALE ? "her" : "its" );
		do_whisper( questmaster, quest_buf );
		sprintf( quest_buf, "0.%s The last I heard, %s was in the area of %s.", ch->name, target->short_descr, area->name );
		do_whisper( questmaster, quest_buf );
	    }
	}
    }

    timer = number_range( qdata.minq_time, qdata.maxq_time );
    ch->pcdata->questmaster = questmaster;
    ch->pcdata->quest_timer[QTIMER_CURR] = timer;
    ch->pcdata->quest_area = area;
    xSET_BIT( ch->act, PLR_QUESTING );
    sprintf( quest_buf, "0.%s You have %s minutes to complete this quest.", ch->name, num_punct( timer ) );
    do_whisper( questmaster, quest_buf );
}


/*
 * Ensures that 'ch' and 'target' are within the proper level range
 */
bool valid_leveldiff( CHAR_DATA *ch, CHAR_DATA *target )
{
    int clev, tlev;
    bool return_val = FALSE;

    if ( IS_NPC( ch ) == IS_NPC( target ) )
	return FALSE;

    clev = IS_NPC( ch ) ? target->level : ch->level;
    tlev = IS_NPC( target ) ? target->level : ch->level;

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( tlev <= clev )
	{
	    if ( ( clev - tlev ) <= ( qdata.qlevel_diff * 2 ) )
		return_val = TRUE;
	    else
		return_val = FALSE;
	}
	else if ( ( tlev - clev ) <= qdata.qlevel_diff )
	    return_val = TRUE;
	else
	    return_val = FALSE;
    }
    else
	return_val = TRUE;

    return return_val;
}


/*
 * Random Percentage Check Used in quest generation
 */
bool quest_chance( int percentage )
{
    int value	= 0;
    int value1	= number_range(1, 100);
    int value2	= number_range(1, 100);
    int choice	= number_range(1, 2);

    if ( choice == 1 )
	value = value1;
    else if ( choice == 2 )
	value = value2;
    else
    {
	bug( "quest_chance: 'choice' value invalid!", 0 );
	return FALSE;
    }

    if ( value <= percentage )
	return TRUE;
    else
	return FALSE;
}


/*
 * Loads 'qdata.dat' file with quest information
 */
bool load_qdata( QUEST_DATA *qd )
{
    FILE        *fpin   = fopen( QDATA_FILE, "r" );

    if ( fpin == NULL )
    {
        bug( "Cannot open %s for reading.", QDATA_FILE );
        perror( QDATA_FILE );
        return FALSE;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = fread_letter( fpin );

        if ( letter == '*' )
        {
            fread_to_eol( fpin );
            continue;
        }

        if ( letter != '#' )
        {
            bug( "Load_qdata: # not found.", 0 );
            break;
        }

        word = fread_word( fpin );

        if ( !str_cmp( word, "QDATA" ) )
        {
            fread_qdata( qd, fpin );
        }
        else if (!str_cmp( word, "End" ) )
        {
            FCLOSE( fpin );
            break;
        }
        else
        {
            bug( "Load_qdata: bad section encountered.", 0 );
            break;
        }
    }
    return TRUE;
}


/*
 * Loads 'quest.dat' file with quest information
 */
void load_questdata( void )
{
    FILE        *fpin   = fopen( QUESTDATA_FILE, "r" );
    QPRIZE_DATA *qp = NULL;
    QTARG_DATA *qt = NULL;


    if ( fpin == NULL )
    {
	bug( "Cannot open %s for reading.", QUESTDATA_FILE );
	perror( QUESTDATA_FILE );
	return;
    }

    for (;;)
    {
	char letter;
	char *word;

	letter = fread_letter( fpin );

	if ( letter == '*' )
	{
	    fread_to_eol( fpin );
	    continue;
	}

	if ( letter != '#' )
	{
	    bug( "Load_questdata: # not found.", 0 );
	    break;
	}

	word = fread_word( fpin );

	if ( !str_cmp( word, "QuestPrize" ) )
	{
	    qp = fread_qprize( fpin );

	    if ( qp != NULL )
	    {
		LINK( qp, first_qprize, last_qprize, next, prev );
	    }
	}
	else if ( !str_cmp( word, "QuestTarget" ) )
	{
	    qt = fread_qtarg( fpin );

	    if ( qt != NULL )
		LINK( qt, first_qtarg, last_qtarg, next, prev );
	}
	else if (!str_cmp( word, "End" ) )
	{
	    FCLOSE( fpin );
	    break;
	}
	else
	{
	    bug( "Load_questdata: bad section encountered.", 0 );
	    break;
	}
    }
    return;
}


/*
 * Write out the qdata.dat file
 */
void write_qdata( void )
{
    FILE *fpout;

    if ( ( fpout = fopen( QDATA_FILE, "w" ) ) == NULL )
    {
        bug( "Cannot open %s for writing.", QDATA_FILE );
        perror( QDATA_FILE );
        return;
    }

    fprintf( fpout, "#QDATA\n" );
    fprintf( fpout, "MinPrac        %d\n", qdata.minprac_gain		);
    fprintf( fpout, "MaxPrac        %d\n", qdata.maxprac_gain		);
    fprintf( fpout, "MinGold        %d\n", qdata.mingold_gain		);
    fprintf( fpout, "MaxGold        %d\n", qdata.maxgold_gain		);
    fprintf( fpout, "MinQp          %d\n", qdata.minqp_gain		);
    fprintf( fpout, "MaxQp          %d\n", qdata.maxqp_gain		);
    fprintf( fpout, "MinQTComplete  %d\n", qdata.minqt_comp		);
    fprintf( fpout, "MaxQTComplete  %d\n", qdata.maxqt_comp		);
    fprintf( fpout, "MinQTime       %d\n", qdata.minq_time		);
    fprintf( fpout, "MaxQTime       %d\n", qdata.maxq_time		);
    fprintf( fpout, "QpLoss         %d\n", qdata.qp_loss		);
    fprintf( fpout, "QTNQuit        %d\n", qdata.qtime_next_quit	);
    fprintf( fpout, "QTNFail        %d\n", qdata.qtime_next_fail	);
    fprintf( fpout, "QTNClear       %d\n", qdata.qtime_next_clear	);
    fprintf( fpout, "QLevelDiff     %d\n", qdata.qlevel_diff		);
    fprintf( fpout, "QuestChance    %d\n", qdata.quest_chance		);
    fprintf( fpout, "ObjectChance   %d\n", qdata.object_chance		);
    fprintf( fpout, "GiveQuest      %d\n", qdata.give_quest		);
    fprintf( fpout, "PracChance     %d\n", qdata.prac_chance		);
    fprintf( fpout, "End\n" );
    fprintf( fpout, "#End\n" );
    FCLOSE( fpout );
}


/*
 * Write out the quest.dat file
 */
void write_questdata( void )
{
    FILE *fpout;
    QPRIZE_DATA *qprize;
    QTARG_DATA *qtarg;

    if ( ( fpout = fopen( QUESTDATA_FILE, "w" ) ) == NULL )
    {
        bug( "Cannot open %s for writing.", QUESTDATA_FILE );
        perror( QUESTDATA_FILE );
        return;
    }

    for ( qprize = first_qprize; qprize != NULL; qprize = qprize->next )
    {
        fprintf( fpout, "#QuestPrize\n" );
        fprintf( fpout, "VNum      %d\n", qprize->vnum );
        fprintf( fpout, "Cost      %d\n", qprize->cost );
        fprintf( fpout, "MinLevel  %d\n", qprize->min_level );
        fprintf( fpout, "MaxLevel  %d\n", qprize->max_level );
        fprintf( fpout, "End\n\n" );
    }

    for ( qtarg = first_qtarg; qtarg != NULL; qtarg = qtarg->next )
    {
        fprintf( fpout, "#QuestTarget\n" );
        fprintf( fpout, "VNum     %d\n", qtarg->vnum );
        fprintf( fpout, "End\n\n" );
    }
    fprintf( fpout, "#End\n" );

    FCLOSE( fpout );
}


/*
 * Remove a qprize or qtarg object from the linked-lists
 */
void remove_questdata( bool qPrize, int nCount )
{
    int nCurrent = 1;

    if ( qPrize == TRUE )
    {
	QPRIZE_DATA *quest;

        for( quest = first_qprize; quest != NULL; quest = quest->next )
        {
            if ( nCount == nCurrent )
            {
                UNLINK( quest, first_qprize, last_qprize, next, prev );
                DISPOSE( quest );
                break;
            }
            else
                nCurrent++;
        }
    }
    else
    {
	QTARG_DATA *quest;

        for( quest = first_qtarg; quest != NULL; quest = quest->next )
        {
            if ( nCount == nCurrent )
            {
                UNLINK( quest, first_qtarg, last_qtarg, next, prev );
                DISPOSE( quest );
                break;
            }
            else
                nCurrent++;
        }
    }
}


/*
 * Key Macro... just in case
 */
#if defined(KEY)
    #undef KEY
#endif

#define KEY( literal, field, value )	\
	if ( !strcmp( word, literal ) )	\
	{				\
	    field = value;		\
	    fMatch = TRUE;		\
	    break;			\
	}


/*
 * Read in qdata information for quest setup
 */
void fread_qdata( QUEST_DATA *qd, FILE *fpin )
{
    char *word;
    bool fMatch;

    for(;;)
    {
        word    = feof( fpin ) ? "End" : fread_word( fpin );
        fMatch  = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fpin );
                break;

	    case 'E':
		if ( !str_cmp( word, "End" ) )
		{
		    return;
		}

	    case 'G':
		KEY( "GiveQuest", 	qd->give_quest,		fread_number( fpin ) );

	    case 'M':
		KEY( "MinPrac",		qd->minprac_gain,	fread_number( fpin ) );
		KEY( "MaxPrac",		qd->maxprac_gain,	fread_number( fpin ) );
		KEY( "MinGold",		qd->mingold_gain,	fread_number( fpin ) );
		KEY( "MaxGold",		qd->maxgold_gain,	fread_number( fpin ) );
		KEY( "MinQp",		qd->minqp_gain,		fread_number( fpin ) );
		KEY( "MaxQp",		qd->maxqp_gain,		fread_number( fpin ) );
		KEY( "MinQTComplete",	qd->minqt_comp, 	fread_number( fpin ) );
		KEY( "MaxQTComplete",	qd->maxqt_comp, 	fread_number( fpin ) );
		KEY( "MinQTime",	qd->minq_time,		fread_number( fpin ) );
		KEY( "MaxQTime",	qd->maxq_time,		fread_number( fpin ) );

	    case 'O':
		KEY( "ObjectChance",	qd->object_chance,	fread_number( fpin ) );

	    case 'P':
		KEY( "PracChance",	qd->prac_chance,	fread_number( fpin ) );

	    case 'Q':
		KEY( "QpLoss",		qd->qp_loss,		fread_number( fpin ) );
		KEY( "QTNQuit",		qd->qtime_next_quit,	fread_number( fpin ) );
		KEY( "QTNFail",		qd->qtime_next_fail,	fread_number( fpin ) );
		KEY( "QTNClear",	qd->qtime_next_clear,	fread_number( fpin ) );
		KEY( "QLevelDiff",	qd->qlevel_diff,	fread_number( fpin ) );
		KEY( "QuestChance",	qd->quest_chance,	fread_number( fpin ) );
        }

        if ( !fMatch )
        {
            sprintf( bug_buf, "Fread_qdata: no match: %s", word );
            bug( bug_buf, 0 );
        }
    }

    return;
}


/*
 * Read in a qprize object for the quest prize linked-list
 */
QPRIZE_DATA *fread_qprize( FILE *fpin )
{
    char *word;
    bool fMatch;
    QPRIZE_DATA *quest = NULL;

    CREATE( quest, QPRIZE_DATA, 1 );

    for(;;)
    {
        word    = feof( fpin ) ? "End" : fread_word( fpin );
        fMatch  = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fpin );
                break;

            case 'C':
		KEY( "Cost",		quest->cost,		fread_number( fpin ) );

            case 'E':
                if ( !str_cmp( word, "End" ) )
                {
                    return quest;
                }

            case 'M':
		KEY( "MaxLevel",	quest->max_level,	fread_number( fpin ) );
		KEY( "MinLevel",	quest->min_level,	fread_number( fpin ) );

            case 'V':
		KEY( "VNum",		quest->vnum,		fread_number( fpin ) );
        }

        if ( !fMatch )
        {
            sprintf( bug_buf, "Fread_qprize: no match: %s", word );
            bug( bug_buf, 0 );
        }
    }

    return NULL;
}


/*
 * Load in a qtarg object for the quest target linked-list
 */
QTARG_DATA *fread_qtarg( FILE *fpin )
{
    char *word;
    bool fMatch;
    QTARG_DATA *quest = NULL;

    CREATE( quest, QTARG_DATA, 1 );

    for(;;)
    {
        word    = feof( fpin ) ? "End" : fread_word( fpin );
        fMatch  = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fpin );
                break;

            case 'E':
                if ( !str_cmp( word, "End" ) )
                {
                    return quest;
                }

            case 'V':
                if ( !str_cmp( word, "VNum" ) )
                {
                    quest->vnum = fread_number( fpin );
                    fMatch = TRUE;
                    break;
                }
        }

        if ( !fMatch )
        {
            sprintf( bug_buf, "Fread_qtarg: no match: %s", word );
            bug( bug_buf, 0 );
        }
    }
    return NULL;
}

void check_quest_obj( OBJ_DATA *obj )
{
    CHAR_DATA *vch = NULL;
    bool oMatch = FALSE;

    for( vch = first_char; vch != NULL; vch = vch->next )
    {
	if ( IS_NPC( vch ) )
	    continue;
	else if ( !IS_QUESTING( vch ) || !vch->pcdata->quest_obj || vch->pcdata->quest_obj != obj )
	    continue;
	else
	{
	    if ( vch->pcdata->quest_type != QT_COMPLETE &&
		 vch->pcdata->quest_type != QT_MOB_GIVE_COMPLETE )
	    {
		oMatch = TRUE;
	    }
	    break;
	}
    }

    if ( oMatch == TRUE )
    {
	int reward = number_range( -25, -5 );

	set_char_color( AT_GREY, vch );
	send_to_char( "A voice whispers in your ear, 'The object of your quest has been dealt with... your services are no longer needed.'\n\r", vch );
	clear_questdata( vch, FALSE );

	vch->pcdata->quest_timer[QTIMER_NEXT] = (number_fuzzy( qdata.qtime_next_fail )/2);
	vch->pcdata->quest_curr += reward;
//	adjust_hiscore( "glory", vch, vch->pcdata->quest_accum, "intro" );
    }

    return;
}
