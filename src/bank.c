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
 * - Banking module                                                        *
 ***************************************************************************/

#include "mud.h"
#include "bank.h"

// The character command to deposit money into an "account."
void do_deposit			args( (CHAR_DATA *ch, char *argument) );

// The character command to withdraw money from an "account."
void do_withdraw		args( (CHAR_DATA *ch, char *argument) );

// The character command to see their current funding status.
void do_balance			args( (CHAR_DATA *ch, char *argument) );

// Puts gold onto a character, and check to see if it's taken from the bank.
void gold_to_char		args( (CHAR_DATA *ch, int amount, bool fromBank) );

// Puts gold into an "account," and check to see if it's taken from the character.
void gold_to_bank		args( (CHAR_DATA *ch, int amount, bool fromChar) );

// Checks to see if there is a banker mob at the character's location.
CHAR_DATA *find_banker_mob	args( (CHAR_DATA *ch) );

/*
 * Take a certain amount of money from a player and place them into
 * their "bank account." -Orion
 */
void do_deposit( CHAR_DATA *ch, char *argument )
{
    int amount;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *banker;

    amount = 0;
    banker = find_banker_mob( ch );

    if ( banker == NULL )
    {
	send_to_char( "There is no banker at this location.\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) )
    {
	sprintf( buf, "Why, I'm sorry, %s, we don't do business with YOUR kind.", ch->short_descr );
	do_say( banker, buf );
    }

    if ( !argument || argument[0] == '\0' )
    {
	sprintf( buf, "Pardon me, %s, but what exactly do you want to deposit?", ch->name );
	do_say( banker, buf );
    }

    if ( !is_number( argument ) )
    {
	if ( !str_cmp( argument, "all" ) )
	{
	    amount = ch->gold;

	    if ( amount < 0 || amount > ch->gold )
	    {
		sprintf( buf, "Oh, %s, you're such a great comedian!", ch->name );
		do_say( banker, buf );
		return;
	    }

	    gold_to_bank( ch, amount, TRUE );
	}
	else
	{
	    sprintf( buf, "I'm sorry, %s, I don't understand what you want to do.", ch->name );
	    do_say( banker, buf );
	}
    }
    else
    {
	amount = atoi( argument );

	if ( amount < 0 || amount > ch->gold )
	{
	    sprintf( buf, "Oh, %s, you're such a great comedian!", ch->name );
	    do_say( banker, buf );
	    return;
	}

	gold_to_bank( ch, amount, TRUE );
    }
}


/*
 * Take a certain amount of money from a player's "bank account"
 * and place them onto the player. -Orion
 */
void do_withdraw( CHAR_DATA *ch, char *argument )
{
    int amount;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *banker;

    amount = 0;
    banker = find_banker_mob( ch );

    if ( banker == NULL )
    {
	send_to_char( "There is no banker at this location.\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) )
    {
	sprintf( buf, "Why, I'm sorry, %s, we don't do business with YOUR kind.", ch->short_descr );
	do_say( banker, buf );
    }

    if ( !argument || argument[0] == '\0' )
    {
	sprintf( buf, "Pardon me, %s, but what exactly do you want to withdraw?", ch->name );
	do_say( banker, buf );
    }

    if ( !is_number( argument ) )
    {
	if ( !str_cmp( argument, "all" ) )
	{
	    amount = ch->pcdata->balance;

	    if ( amount < 0 || amount > ch->pcdata->balance )
	    {
		sprintf( buf, "Oh, %s, you're such a great comedian!", ch->name );
		do_say( banker, buf );
		return;
	    }

	    gold_to_char( ch, amount, TRUE );
	}
	else
	{
	    sprintf( buf, "I'm sorry, %s, I don't understand what you want to do.", ch->name );
	    do_say( banker, buf );
	}
    }
    else
    {
	amount = atoi( argument );

	if ( amount < 0 || amount > ch->pcdata->balance )
	{
	    sprintf( buf, "Oh, %s, you're such a great comedian!", ch->name );
	    do_say( banker, buf );
	    return;
	}

	gold_to_char( ch, amount, TRUE );
    }
}


/*
 * Give the player their "account" status, listing all money in
 * their "bank account," in their inventory, and all money that
 * is in the process of being transferred between any player's
 * "accounts." -Orion
 */
void do_balance( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC( ch ) )
    {
	send_to_pager_color( "\n\r\n\r", ch );
	send_to_pager_color( "&r&Y&G&w@&G&W=============================================================================&G&w@\n\r", ch );
	send_to_pager_color( "&r&Y&G&W|                              &G&cStatus of Account                              &G&W|\n\r", ch );
	send_to_pager_color( "&r&Y&G&w@&G&W=============================================================================&G&w@\n\r", ch );
	pager_printf_color( ch, "&r&Y&G&W| &G&CGold&G&c: &G&w%-13.13s %17.17s| ", num_punct( ch->gold ), "" );
	pager_printf_color( ch, "&G&CBank&G&c: &G&w%-13s %17.17s&G&W|\n\r", num_punct( ch->pcdata->balance ), "" );
	send_to_pager_color( "&r&Y&G&w@&G&W=============================================================================&G&w@\n\r", ch );
	send_to_pager_color( "&r&Y&G&w@&G&W=============================================================================&G&w@\n\r", ch );
    }
    return;
}


/*
 * Puts gold onto a character, and checks to see if said gold
 * should be taken from the bank. If not it just adds gold to
 * the character. -Orion
 */
void gold_to_char( CHAR_DATA *ch, int amount, bool fromBank )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *banker = find_banker_mob( ch );

    if ( IS_NPC( ch ) )
	return;

    if ( fromBank == TRUE )
    {
	if ( banker == NULL )
	{
	    bug( "%s", "gold_to_char: called from bank, but no banker found." );
	}
	else
	{
	    if ( amount > ch->pcdata->balance )
	    {
		sprintf( buf, "I'm sorry, %s, but you don't have that much in your account.", ch->name );
		do_say( banker, buf );
	    }
	    else
	    {
		int gold_check = (ch->gold + amount);

		sprintf( buf, "Thank you for your business, %s.", ch->name );
		do_say( banker, buf );

		if ( gold_check < 0 || gold_check > MAX_GOLD )
		{
		    sprintf( buf, "Damnit! I'm sorry, %s, I can't carry that much right now.", ch->short_descr );
		    do_say( ch, buf );
		    return;
		}
		else
		{
		    ch->pcdata->balance -= amount;
		    ch->gold += amount;
		}
	    }
	}

	return;
    }
    else
    {
	int gold_check = (ch->gold + amount);

	if ( gold_check < 0 )
	    return;
	else if ( gold_check > MAX_GOLD )
	    ch->gold = MAX_GOLD;
	else
	    ch->gold += amount;
    }
}


/*
 * Puts gold into a character's "account," and checks to see if
 * said gold should be taken from the character. If not it just
 * adds gold to the character. -Orion
 */
void gold_to_bank( CHAR_DATA *ch, int amount, bool fromChar )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *banker = find_banker_mob( ch );

    if ( IS_NPC( ch ) )
	return;

    if ( fromChar == TRUE )
    {
	if ( banker == NULL )
	{
	    bug( "%s", "gold_to_char: called from bank, but no banker found." );
	}
	else
	{
	    if ( amount > ch->gold )
	    {
		sprintf( buf, "I'm sorry, %s, but you don't have that much money, apparently.", ch->name );
		do_say( banker, buf );
	    }
	    else
	    {
		int gold_check = (ch->pcdata->balance + amount);

		sprintf( buf, "Thank you very much, %s.", banker->short_descr );
		do_say( ch, buf );

		if ( gold_check < 0 || gold_check > MAX_GOLD )
		{
		    sprintf( buf, "I'm sorry, %s, you can't store that much right now.", ch->name );
		    do_say( banker, buf );
		    return;
		}
		else
		{
		    ch->gold -= amount;
		    ch->pcdata->balance += amount;
		}
	    }
	}

	return;
    }
    else
    {
	int gold_check = (ch->pcdata->balance + amount);

	if ( gold_check < 0 )
	    return;
	else if ( gold_check > MAX_GOLD )
	    ch->pcdata->balance = MAX_GOLD;
	else
	    ch->pcdata->balance += amount;
    }
}


/*
 * Search the room the player is in to see if there is a banker anywhere
 * to be found, and return the result. -Orion
 */
CHAR_DATA *find_banker_mob( CHAR_DATA *ch )
{
    /*
     * Initialize the variables. The "banker" is the CHAR_DATA being
     * tested, and "room" is the variable that stores the room "ch"
     * was in at the calling of the function. -Orion
     */
    CHAR_DATA *banker = NULL;
    ROOM_INDEX_DATA *room = ch->in_room;

    /*
     * Loop through all the characters in "room" and see if any are
     * valid bankers. If so, break out and return the result, else
     * loop until there is no other option, then return NULL. -Orion
     */
    for ( banker = room->first_person; banker != NULL;
	banker = banker->next_in_room )
    {
	/*
	 * If the character isn't an NPC, or they they don't have an
	 * ACT_BANKER flag, then they aren't a banker. -Orion
	 */
	if ( IS_NPC( banker ) && xIS_SET( banker->act, ACT_BANKER ) )
	    break;
	else
	    continue;
    }

    return banker;
}
