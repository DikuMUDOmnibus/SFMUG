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
 * - Online reset editing module                                           *
 ***************************************************************************
 * This file relies heavily on your linked lists being correct, and that   *
 * pArea->reset_first is the first reset in pArea. Likewise, the           *
 * pArea->reset_last *MUST* be the last reset in pArea. Weird and not so   *
 * good things will happen if any of your lists are messed up. The most    *
 * important are your pRoom->contents, pRoom->people, rch->carrying,       *
 * obj->contains, and pArea->reset_first .. pArea->reset_last.  -- Altrag  *
 *                                                                         *
 * Reworded by Orion Elder to shorten.                                     *
 ***************************************************************************/
 
#include <string.h>
#if defined(macintosh)
	#include <types.h>
#endif
#include "mud.h"

/* Externals */
extern	int	top_reset;
char *		sprint_reset	args( ( CHAR_DATA *ch, RESET_DATA *pReset,
					sh_int num, bool rlist ) );
RESET_DATA *	parse_reset	args( ( AREA_DATA *tarea, char *argument,
					CHAR_DATA *ch ) );
int		get_wearloc	args( ( char *type ) );
int		get_trapflag	args( ( char *flag ) );
int		get_exflag	args( ( char *flag ) );
int		get_rflag	args( ( char *flag ) );
extern	char *	const		wear_locs[];
extern	char *	const		ex_flags[];

bool is_room_reset  args( ( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                            AREA_DATA *pArea ) );
void add_obj_reset  args( ( AREA_DATA *pArea, char cm, OBJ_DATA *obj,
                            int v2, int v3 ) );
void delete_reset   args( ( AREA_DATA *pArea, RESET_DATA *pReset ) );
void instaroom      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    bool dodoors ) );
OBJ_DATA * make_trap	args( ( int v0, int v1, int v2, int v3 ) );
#define RID ROOM_INDEX_DATA
RID *find_room      args( ( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom ) );
#undef RID
void edit_reset     args( ( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
                            ROOM_INDEX_DATA *aRoom ) );
#define RD RESET_DATA
RD *find_reset      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    int num ) );
#undef RD
void list_resets    args( ( CHAR_DATA *ch, AREA_DATA *pArea,
			    ROOM_INDEX_DATA *pRoom, int start, int end ) );

ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH];

SHIP_DATA * make_mob_ship( PLANET_DATA *planet , int model );
void    resetship args( ( SHIP_DATA *ship ) );


RESET_DATA *find_reset(AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, int numb)
{
  RESET_DATA *pReset;
  int num = 0;
  
 /* Resets Ships and planetes -Lews */
  reset_all();

  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    if ( is_room_reset(pReset, pRoom, pArea) && ++num >= numb )
      return pReset;
  return NULL;
}

/* This is one loopy function.  Ugh. -- Altrag */
bool is_room_reset( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                    AREA_DATA *pArea )
{
  ROOM_INDEX_DATA *pRoom;
  RESET_DATA *reset;
  int pr;
  
  if ( !aRoom )
    return TRUE;
  switch( pReset->command )
  {
  case 'M':
  case 'O':
    pRoom = get_room_index( pReset->arg3 );
    if ( !pRoom || pRoom != aRoom )
      return FALSE;
    return TRUE;
  case 'P':
  case 'T':
  case 'H':
    if ( pReset->command == 'H' )
      pr = pReset->arg1;
    else
      pr = pReset->arg3;
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           (!pr || pr == reset->arg1) && get_obj_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'B':
    switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
    {
    case BIT_RESET_DOOR:
    case BIT_RESET_ROOM:
      return (aRoom->vnum == pReset->arg1);
    case BIT_RESET_MOBILE:
      for ( reset = pReset->prev; reset; reset = reset->prev )
        if ( reset->command == 'M' && get_mob_index(reset->arg1) )
          break;
      if ( reset && is_room_reset(reset, aRoom, pArea) )
        return TRUE;
      return FALSE;
    case BIT_RESET_OBJECT:
      for ( reset = pReset->prev; reset; reset = reset->prev )
        if ( (reset->command == 'O' || reset->command == 'P' ||
              reset->command == 'G' || reset->command == 'E') &&
             (!pReset->arg1 || pReset->arg1 == reset->arg1) &&
              get_obj_index(reset->arg1) )
          break;
      if ( reset && is_room_reset(reset, aRoom, pArea) )
        return TRUE;
      return FALSE;
    }
    return FALSE;
  case 'G':
  case 'E':
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( reset->command == 'M' && get_mob_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'D':
  case 'R':
    pRoom = get_room_index( pReset->arg1 );
    if ( !pRoom || pRoom->area != pArea || (aRoom && pRoom != aRoom) )
      return FALSE;
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

ROOM_INDEX_DATA *find_room( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( pRoom )
    return pRoom;
  one_argument(argument, arg);
  if ( !is_number(arg) && arg[0] != '\0' )
  {
    send_to_char( "Reset to which room?\n\r", ch );
    return NULL;
  }
  if ( arg[0] == '\0' )
    pRoom = ch->in_room;
  else
    pRoom = get_room_index(atoi(arg));
  if ( !pRoom )
  {
    send_to_char( "Room does not exist.\n\r", ch );
    return NULL;
  }
  return pRoom;
}

/* Separate function for recursive purposes */
#define DEL_RESET(area, reset, rprev) \
do { \
  rprev = reset->prev; \
  delete_reset(area, reset); \
  reset = rprev; \
  continue; \
} while(0)
void delete_reset( AREA_DATA *pArea, RESET_DATA *pReset )
{
  RESET_DATA *reset;
  RESET_DATA *reset_prev;

  if ( pReset->command == 'M' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      /* Break when a new mob found */
      if ( reset->command == 'M' )
        break;
      /* Delete anything mob is holding */
      if ( reset->command == 'G' || reset->command == 'E' )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'B' &&
          (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_MOBILE &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
    }
  }
  else if ( pReset->command == 'O' || pReset->command == 'P' ||
            pReset->command == 'G' || pReset->command == 'E' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      if ( reset->command == 'T' &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'H' &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      /* Delete nested objects, even if they are the same object. */
      if ( reset->command == 'P' && (reset->arg3 > 0 ||
           pReset->command != 'P' || reset->extra-1 == pReset->extra) &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'B' &&
          (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);

      /* Break when a new object of same type is found */
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           reset->arg1 == pReset->arg1 )
        break;
    }
  }
  if ( pReset == pArea->last_mob_reset )
    pArea->last_mob_reset = NULL;
  if ( pReset == pArea->last_obj_reset )
    pArea->last_obj_reset = NULL;
  UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
  DISPOSE(pReset);
  return;
}
#undef DEL_RESET

RESET_DATA *find_oreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No object resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    OBJ_INDEX_DATA *pObjTo = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      if ( (pObjTo = get_obj_index(reset->arg1)) &&
            is_name(arg, pObjTo->name) && ++cnt == num )
        break;
    }
    if ( !pObjTo || !reset )
    {
      send_to_char( "To object not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

RESET_DATA *find_mreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No mobile resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    MOB_INDEX_DATA *pMob = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      if ( (pMob = get_mob_index(reset->arg1)) &&
            is_name(arg, pMob->player_name) && ++cnt == num )
        break;
    }
    if ( !pMob || !reset )
    {
      send_to_char( "Mobile not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

void edit_reset( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
		 ROOM_INDEX_DATA *aRoom )
{
  char arg[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL;
  RESET_DATA *reset = NULL;
  MOB_INDEX_DATA *pMob = NULL;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  int num = 0;
  int vnum;
  char *origarg = argument;
  
  argument = one_argument(argument, arg);
  if ( !*arg || !str_cmp(arg, "?") )
  {
    char *nm = (ch->substate == SUB_REPEATCMD ? "" : (aRoom ? "rreset "
    		: "reset "));
    char *rn = (aRoom ? "" : " [room#]");
    ch_printf(ch, "Syntax: %s<list|edit|delete|add|insert|place%s>\n\r",
        nm, (aRoom ? "" : "|area"));
    ch_printf( ch, "Syntax: %sremove <#>\n\r", nm );
    ch_printf( ch, "Syntax: %smobile <mob#> [limit]%s\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> [limit [room%s]]\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> give <mob name> [limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> equip <mob name> <location> "
        "[limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> put <to_obj name> [limit]\n\r",
        nm );
    ch_printf( ch, "Syntax: %shide <obj name>\n\r", nm );
    ch_printf( ch, "Syntax: %strap <obj name> <type> <charges> <flags>\n\r",
        nm );
    ch_printf( ch, "Syntax: %strap room <type> <charges> <flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> door%s <dir> "
        "<exit flags>\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> object <obj name> "
        "<extra flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> mobile <mob name> "
        "<affect flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> room%s <room flags>"
        "\n\r", nm, rn );
    ch_printf( ch, "Syntax: %srandom <last dir>%s\n\r", nm, rn );
    if ( !aRoom )
    {
      send_to_char( "\n\r[room#] will default to the room you are in, "
          "if unspecified.\n\r", ch );
    }
    return;
  }
  if ( !str_cmp(arg, "on") )
  {
    ch->substate = SUB_REPEATCMD;
    ch->dest_buf = (aRoom ? (void *)aRoom : (void *)pArea);
    send_to_char( "Reset mode on.\n\r", ch );
    return;
  }
  if ( !aRoom && !str_cmp(arg, "area") )
  {
    if ( !pArea->first_reset )
    {
      send_to_char( "You don't have any resets defined.\n\r", ch );
      return;
    }
    num = pArea->nplayer;
    pArea->nplayer = 0;
    reset_area(pArea);
    pArea->nplayer = num;
    send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !str_cmp(arg, "list") )
  {
    int start, end;
    
    argument = one_argument(argument, arg);
    start = is_number(arg) ? atoi(arg) : -1;
    argument = one_argument(argument, arg);
    end   = is_number(arg) ? atoi(arg) : -1;
    list_resets(ch, pArea, aRoom, start, end);
    return;
  }
  
  if ( !str_cmp(arg, "edit") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset edit <number> <command>\n\r", ch );
      return;
    }
    num = atoi(arg);
    if ( !(pReset = find_reset(pArea, aRoom, num)) )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( !(reset = parse_reset(pArea, argument, ch)) )
    {
      send_to_char( "Error in reset.  Reset not changed.\n\r", ch );
      return;
    }
    reset->prev = pReset->prev;
    reset->next = pReset->next;
    if ( !pReset->prev )
      pArea->first_reset = reset;
    else
      pReset->prev->next = reset;
    if ( !pReset->next )
      pArea->last_reset  = reset;
    else
      pReset->next->prev = reset;
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "add") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    add_reset(pArea, pReset->command, pReset->extra, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "place") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    place_reset(pArea, pReset->command, pReset->extra, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "insert") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset insert <number> <command>\n\r", ch );
      return;
    }
    num = atoi(arg);
    if ( (reset = find_reset(pArea, aRoom, num)) == NULL )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not inserted.\n\r", ch );
      return;
    }
    INSERT(pReset, reset, pArea->first_reset, next, prev);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "delete") )
  {
    int start, end;
    bool found;
    
    if ( !*argument )
    {
      send_to_char( "Usage: reset delete <start> [end]\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    start = is_number(arg) ? atoi(arg) : -1;
    end   = is_number(arg) ? atoi(arg) : -1;
    num = 0; found = FALSE;
    for ( pReset = pArea->first_reset; pReset; pReset = reset )
    {
      reset = pReset->next;
      if ( !is_room_reset(pReset, aRoom, pArea) )
        continue;
      if ( start > ++num )
        continue;
      if ( (end != -1 && num > end) || (end == -1 && found) )
        return;
      UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      if ( pReset == pArea->last_mob_reset )
        pArea->last_mob_reset = NULL;
      DISPOSE(pReset);
      top_reset--;
      found = TRUE;
    }
    if ( !found )
      send_to_char( "Reset not found.\n\r", ch );
    else
      send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !str_cmp(arg, "remove") )
  {
    int iarg;
    
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Delete which reset?\n\r", ch );
      return;
    }
    iarg = atoi(arg);
    for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    {
      if ( is_room_reset( pReset, aRoom, pArea ) && ++num == iarg )
        break;
    }
    if ( !pReset )
    {
      send_to_char( "Reset does not exist.\n\r", ch );
      return;
    }
    delete_reset( pArea, pReset );
    send_to_char( "Reset deleted.\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "mobile" ) )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which mobile vnum?\n\r", ch );
      return;
    }
    if ( !(pMob = get_mob_index(atoi(arg))) )
    {
      send_to_char( "Mobile does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      num = 1;
    else if ( !is_number(arg) )
    {
      send_to_char( "Reset how many mobiles?\n\r", ch );
      return;
    }
    else
      num = atoi(arg);
    if ( !(pRoom = find_room(ch, argument, aRoom)) )
      return;
    pReset = make_reset('M', 0, pMob->vnum, num, pRoom->vnum);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Mobile reset added.\n\r", ch );
    return;
  }
  if ( !str_prefix(arg, "object") )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which object vnum?\n\r", ch );
      return;
    }
    if ( !(pObj = get_obj_index(atoi(arg))) )
    {
      send_to_char( "Object does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      strcpy(arg, "room");
    if ( !str_prefix( arg, "put" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      /* Put in_objects after hide and trap resets */
      while ( reset->next && (reset->next->command == 'H' ||
              reset->next->command == 'T' ||
             (reset->next->command == 'B' &&
              (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
              (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))) )
        reset = reset->next;
/*      pReset = make_reset('P', 1, pObj->vnum, num, reset->arg1);*/
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('P', reset->extra+1, pObj->vnum, vnum, 0);
      /* Grumble.. insert puts pReset before reset, and we need it after,
         so we make a hackup and reverse all the list params.. :P.. */
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset in object created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "give" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      while ( reset->next && reset->next->command == 'B' &&
      	     (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
      	     (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
        reset = reset->next;
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('G', 1, pObj->vnum, vnum, 0);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset to mobile created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "equip" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      while ( reset->next && reset->next->command == 'B' &&
      	     (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
      	     (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
        reset = reset->next;
      num = get_wearloc(argument);
      if ( num < 0 )
      {
        send_to_char( "Reset object to which location?\n\r", ch );
        return;
      }
      for ( pReset = reset->next; pReset; pReset = pReset->next )
      {
        if ( pReset->command == 'M' )
          break;
        if ( pReset->command == 'E' && pReset->arg3 == num )
        {
          send_to_char( "Mobile already has an item equipped there.\n\r", ch);
          return;
        }
      }
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('E', 1, pObj->vnum, vnum, num);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset equipped by mobile created.\n\r", ch );
      return;
    }
    if ( arg[0] == '\0' || !(num = (int)str_cmp(arg, "room")) ||
         is_number(arg) )
    {
      if ( !(bool)num )
        argument = one_argument(argument, arg);
      if ( !(pRoom = find_room(ch, argument, aRoom)) )
        return;
      if ( pRoom->area != pArea )
      {
        send_to_char( "Cannot reset objects to other areas.\n\r", ch );
        return;
      }
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('O', 0, pObj->vnum, vnum, pRoom->vnum);
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char( "Object reset added.\n\r", ch );
      return;
    }
    send_to_char( "Reset object to where?\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "random") )
  {
    argument = one_argument(argument, arg);
    vnum = get_dir( arg );
    if ( vnum < 0 || vnum > 9 )
    {
      send_to_char( "Reset which random doors?\n\r", ch );
      return;
    }
    if ( vnum == 0 )
    {
      send_to_char( "There is no point in randomizing one door.\n\r", ch );
      return;
    }
    pRoom = find_room(ch, argument, aRoom);
    if ( pRoom->area != pArea )
    {
      send_to_char( "Cannot randomize doors in other areas.\n\r", ch );
      return;
    }
    pReset = make_reset('R', 0, pRoom->vnum, vnum, 0);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Reset random doors created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "trap") )
  {
    char oname[MAX_INPUT_LENGTH];
    int chrg, value, extra = 0;
    bool isobj;
    
    argument = one_argument(argument, oname);
    argument = one_argument(argument, arg);
    num = is_number(arg) ? atoi(arg) : -1;
    argument = one_argument(argument, arg);
    chrg = is_number(arg) ? atoi(arg) : -1;
    isobj = is_name(argument, "obj");
    if ( isobj == is_name(argument, "room") )
    {
      send_to_char( "Reset: TRAP: Must specify ROOM or OBJECT\n\r", ch );
      return;
    }
    if ( !str_cmp(oname, "room") && !isobj )
    {
      vnum = (aRoom ? aRoom->vnum : ch->in_room->vnum);
      extra = TRAP_ROOM;
    }
    else
    {
      if ( is_number(oname) && !isobj )
      {
        vnum = atoi(oname);
        if ( !get_room_index(vnum) )
        {
          send_to_char( "Reset: TRAP: no such room\n\r", ch );
          return;
        }
        reset = NULL;
        extra = TRAP_ROOM;
      }
      else
      {
        if ( !(reset = find_oreset(ch, pArea, aRoom, oname)) )
          return;
/*        vnum = reset->arg1;*/
        vnum = 0;
        extra = TRAP_OBJ;
      }
    }
    if ( num < 1 || num >= MAX_TRAPTYPE )
    {
      send_to_char( "Reset: TRAP: invalid trap type\n\r", ch );
      return;
    }
    if ( chrg < 0 || chrg > 10000 )
    {
      send_to_char( "Reset: TRAP: invalid trap charges\n\r", ch );
      return;
    }
    while ( *argument )
    {
      argument = one_argument(argument, arg);
      value = get_trapflag(arg);
      if ( value < 0 || value > 31 )
      {
        send_to_char( "Reset: TRAP: bad flag\n\r", ch );
        return;
      }
      SET_BIT(extra, 1 << value);
    }
    pReset = make_reset('T', extra, num, chrg, vnum);
    if ( reset )
      INSERT(pReset, reset, pArea->last_reset, prev, next);
    else
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Trap created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "bit") )
  {
    int (*flfunc)(char *type);
    int flags = 0;
    char option[MAX_INPUT_LENGTH];
    char *parg;
    bool ext_bv = FALSE;
    
    argument = one_argument(argument, option);
    if ( !*option )
    {
      send_to_char( "You must specify SET, REMOVE, or TOGGLE.\n\r", ch );
      return;
    }
    num = 0;
    if ( !str_prefix(option, "set") )
      SET_BIT(num, BIT_RESET_SET);
    else if ( !str_prefix(option, "toggle") )
      SET_BIT(num, BIT_RESET_TOGGLE);
    else if ( str_prefix(option, "remove") )
    {
      send_to_char( "You must specify SET, REMOVE, or TOGGLE.\n\r", ch );
      return;
    }
    argument = one_argument(argument, option);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !*option )
    {
      send_to_char( "Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch );
      return;
    }
    if ( !str_prefix(option, "door") )
    {
      SET_BIT(num, BIT_RESET_DOOR);
      if ( aRoom )
      {
        pRoom = aRoom;
        argument = parg;
      }
      else if ( !is_number(arg) )
      {
        pRoom = ch->in_room;
        argument = parg;
      }
      else if ( !(pRoom = find_room(ch, arg, aRoom)) )
        return;
      argument = one_argument(argument, arg);
      if ( !*arg )
      {
        send_to_char( "Must specify direction.\n\r", ch );
        return;
      }
      vnum = get_dir(arg);
      SET_BIT(num, vnum << BIT_RESET_DOOR_THRESHOLD);
      vnum = pRoom->vnum;
      flfunc = &get_exflag;
      reset = NULL;
    }
    else if ( !str_prefix(option, "object") )
    {
      SET_BIT(num, BIT_RESET_OBJECT);
      vnum = 0;
      flfunc = &get_oflag;
      if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
        return;
      ext_bv = TRUE;
    }
    else if ( !str_prefix(option, "mobile") )
    {
      SET_BIT(num, BIT_RESET_MOBILE);
      vnum = 0;
      flfunc = &get_aflag;
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      ext_bv = TRUE;
    }
    else if ( !str_prefix(option, "room") )
    {
      SET_BIT(num, BIT_RESET_ROOM);
      if ( aRoom )
      {
        pRoom = aRoom;
        argument = parg;
      }
      else if ( !is_number(arg) )
      {
        pRoom = ch->in_room;
        argument = parg;
      }
      else if ( !(pRoom = find_room(ch, arg, aRoom)) )
        return;
      vnum = pRoom->vnum;
      flfunc = &get_rflag;
      reset = NULL;
    }
    else
    {
      send_to_char( "Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch );
      return;
    }
    while ( *argument )
    {
      int value;
      argument = one_argument(argument, arg);
      value = (*flfunc)(arg);
      if ( value < 0 || (!ext_bv && value > 31) )
      {
        send_to_char( "Reset: BIT: bad flag\n\r", ch );
        return;
      }
      if (ext_bv)	/* one per flag for extendeds */
      {
        pReset = make_reset('B', 1, vnum, num, flags);
        if (reset)
        {
          INSERT(pReset, reset, pArea->last_reset, prev, next);
          reset = pReset;
        }
        else
          LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      }
      else
        SET_BIT(flags, 1 << value);
    }
    if ( !flags )
    {
      send_to_char( "Set which flags?\n\r", ch );
      return;
    }
    if (!ext_bv)
    {
      pReset = make_reset('B', 1, vnum, num, flags);
      if ( reset )
        INSERT(pReset, reset, pArea->last_reset, prev, next);
      else
        LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    }
    send_to_char( "Bitvector reset created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "hide") )
  {
    argument = one_argument(argument, arg);
    if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
      return;
/*    pReset = make_reset('H', 1, reset->arg1, 0, 0);*/
    pReset = make_reset('H', 1, 0, 0, 0);
    INSERT(pReset, reset, pArea->last_reset, prev, next);
    send_to_char( "Object hide reset created.\n\r", ch );
    return;
  }
  if ( ch->substate == SUB_REPEATCMD )
  {
    ch->substate = SUB_NONE;
    interpret(ch, origarg);
    ch->substate = SUB_REPEATCMD;
    ch->last_cmd = (aRoom ? do_rreset : do_reset);
  }
  else
    edit_reset(ch, "", pArea, aRoom);
  return;
}

void do_reset( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea = NULL;
  char arg[MAX_INPUT_LENGTH];
  char *parg;
  
  /*
   * Can't have NPC's doing this.  Bug report sent in by Cronel
   * -- Shaddai
   */
  if ( IS_NPC( ch ) )
	return;

  parg = one_argument(argument, arg);
  if ( ch->substate == SUB_REPEATCMD )
  {
    pArea = ch->dest_buf;
    if ( pArea && pArea != ch->pcdata->area && pArea != ch->in_room->area )
    {
      AREA_DATA *tmp;
      
      for ( tmp = first_build; tmp; tmp = tmp->next )
        if ( tmp == pArea )
          break;
      if ( !tmp )
        for ( tmp = first_area; tmp; tmp = tmp->next )
          if ( tmp == pArea )
            break;
      if ( !tmp )
      {
        send_to_char("Your area pointer got lost.  Reset mode off.\n\r", ch);
        bug("do_reset: %s's dest_buf points to invalid area",
		ch->name);	/* why was this cast to an int? */
        ch->substate = SUB_NONE;
        ch->dest_buf = NULL;
        return;
      }
    }
    if ( !*arg )
    {
      ch_printf(ch, "Editing resets for area: %s\n\r", pArea->name);
      return;
    }
    if ( !str_cmp(arg, "done") || !str_cmp(arg, "off") )
    {
      send_to_char( "Reset mode off.\n\r", ch );
      ch->substate = SUB_NONE;
      ch->dest_buf = NULL;
      return;
    }
  }
  if ( !pArea && get_trust(ch) > LEVEL_WORKER )
  {
    char fname[80];
    
    sprintf(fname, "%s.are", capitalize(arg));
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !str_cmp(fname, pArea->filename) )
      {
        argument = parg;
        break;
      }
    if ( !pArea )
      pArea = ch->pcdata->area;
    if ( !pArea )
      pArea = ch->in_room->area;
  }
  else
    pArea = ch->pcdata->area;
  if ( !pArea )
  {
    send_to_char( "You do not have an assigned area.\n\r", ch );
    return;
  }
  edit_reset(ch, argument, pArea, NULL);
  return;
}

void do_rreset( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  
  if ( ch->substate == SUB_REPEATCMD )
  {
    pRoom = ch->dest_buf;
    if ( !pRoom )
    {
      send_to_char( "Your room pointer got lost.  Reset mode off.\n\r", ch);
      bug("do_rreset: %s's dest_buf points to invalid room", (int)ch->name);
    }
    ch->substate = SUB_NONE;
    ch->dest_buf = NULL;
    return;
  }
  else
    pRoom = ch->in_room;
  if ( !can_rmodify(ch, pRoom) )
    return;
  edit_reset(ch, argument, pRoom->area, pRoom);
  return;
}

void add_obj_reset( AREA_DATA *pArea, char cm, OBJ_DATA *obj, int v2, int v3 )
{
  OBJ_DATA *inobj;
  static int iNest;
  
  if ( (cm == 'O' || cm == 'P') && obj->pIndexData->vnum == OBJ_VNUM_TRAP )
  {
    if ( cm == 'O' )
      add_reset(pArea, 'T', obj->value[3], obj->value[1], obj->value[0], v3);
    return;
  }
  add_reset( pArea, cm, (cm == 'P' ? iNest : 1), obj->pIndexData->vnum,
  	     v2, v3 );
  /* Only add hide for in-room objects that are hidden and cant be moved, as
     hide is an update reset, not a load-only reset. */
  if ( cm == 'O' && IS_OBJ_STAT(obj, ITEM_HIDDEN) &&
      !IS_SET(obj->wear_flags, ITEM_TAKE) )
    add_reset(pArea, 'H', 1, 0, 0, 0);
  for ( inobj = obj->first_content; inobj; inobj = inobj->next_content )
    if ( inobj->pIndexData->vnum == OBJ_VNUM_TRAP )
      add_obj_reset(pArea, 'O', inobj, 0, 0);
  if ( cm == 'P' )
    iNest++;
  for ( inobj = obj->first_content; inobj; inobj = inobj->next_content )
    add_obj_reset( pArea, 'P', inobj, 1, 0 );
  if ( cm == 'P' )
    iNest--;
  return;
}

void instaroom( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, bool dodoors )
{
  CHAR_DATA *rch;
  OBJ_DATA *obj;
  
  for ( rch = pRoom->first_person; rch; rch = rch->next_in_room )
  {
    if ( !IS_NPC(rch) )
      continue;
    add_reset( pArea, 'M', 1, rch->pIndexData->vnum, rch->pIndexData->count,
               pRoom->vnum );
    for ( obj = rch->first_carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE )
        add_obj_reset( pArea, 'G', obj, 1, 0 );
      else
        add_obj_reset( pArea, 'E', obj, 1, obj->wear_loc );
    }
  }
  for ( obj = pRoom->first_content; obj; obj = obj->next_content )
  {
    add_obj_reset( pArea, 'O', obj, 1, pRoom->vnum );
  }
  if ( dodoors )
  {
    EXIT_DATA *pexit;

    for ( pexit = pRoom->first_exit; pexit; pexit = pexit->next )
    {
      int state = 0;
    
      if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
        continue;
      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
        if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
          state = 2;
        else
          state = 1;
      }
      add_reset( pArea, 'D', 0, pRoom->vnum, pexit->vdir, state );
    }
  }
  return;
}

void wipe_resets( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA *pReset;
  
  for ( pReset = pArea->first_reset; pReset; )
  {
    if ( pReset->command != 'R' && is_room_reset( pReset, pRoom, pArea ) )
    {
      /* Resets always go forward, so we can safely use the previous reset,
         providing it exists, or first_reset if it doesnt.  -- Altrag */
      RESET_DATA *prev = pReset->prev;
      
      delete_reset(pArea, pReset);
      pReset = (prev ? prev->next : pArea->first_reset);
    }
    else
      pReset = pReset->next;
  }
  return;
}

void do_instaroom( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) || get_trust(ch) < LEVEL_SLAVE || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  argument = one_argument(argument, arg);
  if ( !str_cmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( !(pRoom = find_room(ch, arg, NULL)) )
  {
    send_to_char( "Room doesn't exist.\n\r", ch );
    return;
  }
  if ( !can_rmodify(ch, pRoom) )
    return;
  if ( pRoom->area != pArea && get_trust(ch) < LEVEL_MASTER )
  {
    send_to_char( "You cannot reset that room.\n\r", ch );
    return;
  }
  if ( pArea->first_reset )
    wipe_resets(pArea, pRoom);
  instaroom(pArea, pRoom, dodoors);
  send_to_char( "Room resets installed.\n\r", ch );
}

void do_instazone( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  int vnum;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;

  if ( IS_NPC(ch) || get_trust(ch) < LEVEL_SLAVE || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  if ( !str_cmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( pArea->first_reset )
    wipe_resets(pArea, NULL);
  for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
  {
    if ( !(pRoom = get_room_index(vnum)) || pRoom->area != pArea )
      continue;
    instaroom( pArea, pRoom, dodoors );
  }
  send_to_char( "Area resets installed.\n\r", ch );
  return;
}

int generate_itemlevel( AREA_DATA *pArea, OBJ_INDEX_DATA *pObjIndex )
{
    int olevel;
    int min = UMAX(pArea->low_soft_range, 1);
    int max = UMIN(pArea->hi_soft_range, min + 15);

    if ( pObjIndex->level > 0 )
	olevel = UMIN(pObjIndex->level, MAX_LEVEL);
    else
	switch ( pObjIndex->item_type )
	{
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  min, max );	break;
	    case ITEM_POTION:	olevel = number_range(  min, max );	break;
	    case ITEM_SCROLL:	olevel = pObjIndex->value[0];		break;
	    case ITEM_WAND:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_STAFF:	olevel = number_range( min+9, max+5 );	break;
	    case ITEM_ARMOR:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_WEAPON:	olevel = number_range( min+4, max+1 );	break;
	}
    return olevel;
}

/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
  RESET_DATA *pReset;
  CHAR_DATA *mob;
  OBJ_DATA *obj;
  OBJ_DATA *lastobj;
  ROOM_INDEX_DATA *pRoomIndex;
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_INDEX_DATA *pObjToIndex;
  EXIT_DATA *pexit;
  OBJ_DATA *to_obj;
  char buf[MAX_STRING_LENGTH];
  int level = 0;
  void *plc = NULL;
  bool ext_bv = FALSE;
  
  if ( !pArea )
  {
    bug( "reset_area: NULL pArea", 0 );
    return;
  }
  
  mob = NULL;
  obj = NULL;
  lastobj = NULL;
  if ( !pArea->first_reset )
  {
    bug( "%s: reset_area: no resets", (int)pArea->filename );
    return;
  }
  level = 0;
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    switch( pReset->command )
    {
    default:
      sprintf( buf, "%s Reset_area: bad command %c.", pArea->filename,
	pReset->command );
      bug ( buf, 0 );
      break;
    
    case 'M':
      if ( !(pMobIndex = get_mob_index(pReset->arg1)) )
      {
	sprintf( buf, "%s Reset_area: 'M': bad mob vnum %d.", pArea->filename,
	  pReset->arg1 );
	bug( buf, 0 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
	sprintf( buf, "%s Reset_area: 'M': bad room vnum %d.", pArea->filename,
	  pReset->arg3 );
	bug ( buf, 0 );
        continue;
      }
      if ( pMobIndex->count >= pReset->arg2 )
      {
        mob = NULL;
        break;
      }
      mob = create_mobile(pMobIndex);
      {
	ROOM_INDEX_DATA *pRoomPrev = get_room_index(pReset->arg3 - 1);
        
	if ( pRoomPrev && IS_SET(pRoomPrev->room_flags, ROOM_PET_SHOP) )
	  xSET_BIT(mob->act, ACT_PET);
      }
      if ( room_is_dark(pRoomIndex) )
	  xSET_BIT(mob->affected_by, AFF_INFRARED);
      char_to_room(mob, pRoomIndex);
      economize_mobgold(mob);
      level = URANGE(0, mob->level - 2, LEVEL_DEMIGOD);
      break;
    
    case 'G':
    case 'E':
	if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
	{
	    sprintf (buf, "%s Reset_area: 'E' or 'G': bad obj vnum %d.", pArea->filename, pReset->arg1 );
	    bug ( buf, 0 );
	    continue;
	}
	if ( !mob )
	{
	    lastobj = NULL;
	    break;
	}
	if ( mob->pIndexData->pShop )
	{
	    int olevel = generate_itemlevel( pArea, pObjIndex );
	    obj = create_object(pObjIndex, olevel);
	    xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
	}
	else
	    obj = create_object(pObjIndex, number_fuzzy(level));
	obj->level = URANGE(0, obj->level, LEVEL_DEMIGOD);
	obj = obj_to_char(obj, mob);
	if ( pReset->command == 'E' )
	{
	    if( obj->carried_by != mob )
	    {
		bug( "'E' reset: can't give object %d to mob %d.", obj->pIndexData->vnum, mob->pIndexData->vnum );
		break;
	    }
	    equip_char(mob, obj, pReset->arg3);
	}
	lastobj = obj;
	break;
    
    case 'O':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
/*
	sprintf (buf, "%s Reset_area: 'O': bad obj vnum %d.",
		pArea->filename, pReset->arg1 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
/*
	sprintf ( buf, "%s Reset_area: 'O': bad room vnum %d.", pArea->filename,
	   pReset->arg3 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( pArea->nplayer > 0 ||
           count_obj_list(pObjIndex, pRoomIndex->first_content) >= pReset->arg2 )
      {
        obj = NULL;
        lastobj = NULL;
        break;
      }
      obj = create_object(pObjIndex, number_fuzzy(generate_itemlevel(pArea, pObjIndex)));
      obj->level = UMIN(obj->level, LEVEL_DEMIGOD);
      obj->cost = 0;
      obj_to_room(obj, pRoomIndex);
      lastobj = obj;
      break;
    
    case 'P':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
/*
	sprintf ( buf, "%s Reset_area: 'P': bad obj vnum %d.", pArea->filename,
	   pReset->arg1 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( pReset->arg3 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'P': bad objto vnum %d.",pArea->filename,
		pReset->arg3 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             count_obj_list(pObjIndex, to_obj->first_content) > 0 )
        {
          obj = NULL;
          break;
        }
        lastobj = to_obj;
      }
      else
      {
        int iNest;
        
        if ( !lastobj )
          break;
        to_obj = lastobj;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
          if ( !(to_obj = to_obj->last_content) )
          {
/*
	    sprintf(buf,"%s Reset_area: 'P': Invalid nesting obj %d."
		,pArea->filename, pReset->arg1 );
	    bug( buf, 0 );
*/
            iNest = -1;
            break;
          }
        if ( iNest < 0 )
          continue;
      }
      obj = create_object(pObjIndex, number_fuzzy(UMAX(generate_itemlevel(pArea, pObjIndex),to_obj->level)));
      obj->level = UMIN(obj->level, LEVEL_DEMIGOD);
      obj_to_obj(obj, to_obj);
      break;
    
    case 'T':
      if ( IS_SET(pReset->extra, TRAP_OBJ) )
      {
        /* We need to preserve obj for future 'T' and 'H' checks */
        OBJ_DATA *pobj;
        
        if ( pReset->arg3 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
          {
/*
	    sprintf (buf,"%s Reset_area: 'T': bad objto vnum %d."
		,pArea->filename, pReset->arg3 );
	    bug ( buf, 0 );
*/
            continue;
          }
          if ( pArea->nplayer > 0 ||
             !(to_obj = get_obj_type(pObjToIndex)) ||
              (to_obj->carried_by && !IS_NPC(to_obj->carried_by)) ||
               is_trapped(to_obj) )
            break;
        }
        else
        {
          if ( !lastobj || !obj )
            break;
          to_obj = obj;
        }
        pobj = make_trap( pReset->arg2, pReset->arg1,
        		  number_fuzzy(to_obj->level), pReset->extra );
        obj_to_obj(pobj, to_obj);
      }
      else
      {
        if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'T': bad room %d.", pArea->filename,
		pReset->arg3 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
             count_obj_list(get_obj_index(OBJ_VNUM_TRAP),
               pRoomIndex->first_content) > 0 )
          break;
        to_obj = make_trap(pReset->arg1, pReset->arg1, 10, pReset->extra);
        obj_to_room(to_obj, pRoomIndex);
      }
      break;
    
    case 'H':
      if ( pReset->arg1 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'H': bad objto vnum %d.",pArea->filename,
		pReset->arg1 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             to_obj->in_room->area != pArea ||
             IS_OBJ_STAT(to_obj, ITEM_HIDDEN) )
          break;
      }
      else
      {
        if ( !lastobj || !obj )
          break;
        to_obj = obj;
      }
      xSET_BIT(to_obj->extra_flags, ITEM_HIDDEN);
      break;
    
    case 'B':
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int doornum;
        
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'B': door: bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	  bug( buf, 0 );
*/
          continue;
        }
        doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK)
                >> BIT_RESET_DOOR_THRESHOLD;
        if ( !(pexit = get_exit(pRoomIndex, doornum)) )
          break;
        plc = &pexit->exit_info;
        }
        break;
      case BIT_RESET_ROOM:
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'B': room: bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	  bug(buf, 0);
*/
          continue;
        }
        plc = &pRoomIndex->room_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
          {
/*
	    sprintf(buf,"%s Reset_area: 'B': object: bad objto vnum %d.",
		pArea->filename, pReset->arg1 );
	    bug( buf, 0 );
*/
            continue;
          }
          if ( !(to_obj = get_obj_type(pObjToIndex)) ||
                !to_obj->in_room ||
                 to_obj->in_room->area != pArea )
            continue;
        }
        else
        {
          if ( !lastobj || !obj )
            continue;
          to_obj = obj;
        }
        plc = &to_obj->extra_flags;
	ext_bv = TRUE;
        break;
      case BIT_RESET_MOBILE:
        if ( !mob )
          continue;
        plc = &mob->affected_by;
	ext_bv = TRUE;
        break;
      default:
/*
	sprintf(buf, "%s Reset_area: 'B': bad options %d.",
		pArea->filename, pReset->arg2 );
	bug( buf, 0 );
*/
        continue;
      }
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) )
      {
	if ( ext_bv )
	    xSET_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    SET_BIT(*(int *)plc, pReset->arg3);
      }
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
      {
	if ( ext_bv )
	    xTOGGLE_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    TOGGLE_BIT(*(int *)plc, pReset->arg3);
      }
      else
      {
	if ( ext_bv )
	    xREMOVE_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    REMOVE_BIT(*(int *)plc, pReset->arg3);
      }
      break;
    
    case 'D':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
/*
	sprintf(buf, "%s Reset_area: 'D': bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	bug(buf, 0);
*/
        continue;
      }
      if ( !(pexit = get_exit(pRoomIndex, pReset->arg2)) )
        break;
      switch( pReset->arg3 )
      {
      case 0:
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        break;
      case 1:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      case 2:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        SET_BIT(   pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      }
      break;
    
    case 'R':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
/*
	sprintf(buf,"%s Reset_area: 'R': bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	bug(buf, 0);
*/
        continue;
      }
      randomize_exits(pRoomIndex, pReset->arg2-1);
      break;
    }
  }
  return;
}

void list_resets( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
		  int start, int end )
{
  RESET_DATA *pReset;
  ROOM_INDEX_DATA *room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj, *obj2;
  OBJ_INDEX_DATA *lastobj;
  RESET_DATA *lo_reset;
  bool found;
  int num = 0;
  const char *rname = "???", *mname = "???", *oname = "???";
  char buf[256];
  char *pbuf;
  
  if ( !ch || !pArea )
    return;
  room = NULL;
  mob = NULL;
  obj = NULL;
  lastobj = NULL;
  lo_reset = NULL;
  found = FALSE;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    if ( !is_room_reset(pReset, pRoom, pArea) )
      continue;
    ++num;
    sprintf(buf, "%2d) ", num);
    pbuf = buf+strlen(buf);
    switch( pReset->command )
    {
    default:
      sprintf(pbuf, "*** BAD RESET: %c %d %d %d %d ***\n\r",
          pReset->command, pReset->extra, pReset->arg1, pReset->arg2,
          pReset->arg3);
      break;
    case 'M':
      if ( !(mob = get_mob_index(pReset->arg1)) )
        mname = "Mobile: *BAD VNUM*";
      else
        mname = mob->player_name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "%s (%d) -> %s (%d) [%d]", mname, pReset->arg1, rname,
          pReset->arg3, pReset->arg2 );
      if ( !room )
        mob = NULL;
      if ( (room = get_room_index(pReset->arg3-1)) &&
            IS_SET(room->room_flags, ROOM_PET_SHOP) )
        strcat( buf, " (pet)\n\r" );
      else
        strcat( buf, "\n\r" );
      break;
    case 'G':
    case 'E':
      if ( !mob )
        mname = "* ERROR: NO MOBILE! *";
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      sprintf( pbuf, "%s (%d) -> %s (%s) [%d]", oname, pReset->arg1, mname,
          (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]),
          pReset->arg2 );
      if ( mob && mob->pShop )
        strcat( buf, " (shop)\n\r" );
      else
        strcat( buf, "\n\r" );
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'O':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "(object) %s (%d) -> %s (%d) [%d]\n\r", oname,
          pReset->arg1, rname, pReset->arg3, pReset->arg2 );
      if ( !room )
        obj = NULL;
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'P':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object1: *BAD VNUM*";
      else
        oname = obj->name;
      obj2 = NULL;
      if ( pReset->arg3 > 0 )
      {
        obj2 = get_obj_index(pReset->arg3);
        rname = (obj2 ? obj2->name : "Object2: *BAD VNUM*");
        lastobj = obj2;
      }
      else if ( !lastobj )
        rname = "Object2: *NULL obj*";
      else if ( pReset->extra == 0 )
      {
        rname = lastobj->name;
        obj2 = lastobj;
      }
      else
      {
        int iNest;
        RESET_DATA *reset;
        
        reset = lo_reset->next;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
        {
          for ( ; reset; reset = reset->next )
            if ( reset->command == 'O' || reset->command == 'G' ||
                 reset->command == 'E' || (reset->command == 'P' &&
                !reset->arg3 && reset->extra == iNest) )
              break;
          if ( !reset || reset->command != 'P' )
            break;
        }
        if ( !reset )
          rname = "Object2: *BAD NESTING*";
        else if ( !(obj2 = get_obj_index(reset->arg1)) )
          rname = "Object2: *NESTED BAD VNUM*";
        else
          rname = obj2->name;
      }
      sprintf( pbuf, "(Put) %s (%d) -> %s (%d) [%d] {nest %d}\n\r", oname,
          pReset->arg1, rname, (obj2 ? obj2->vnum : pReset->arg3),
          pReset->arg2, pReset->extra );
      break;
    case 'T':
      sprintf(pbuf, "TRAP: %d %d %d %d (%s)\n\r", pReset->extra, pReset->arg1,
          pReset->arg2, pReset->arg3, flag_string(pReset->extra, trap_flags));
      break;
    case 'H':
      if ( pReset->arg1 > 0 )
        if ( !(obj2 = get_obj_index(pReset->arg1)) )
          rname = "Object: *BAD VNUM*";
        else
          rname = obj2->name;
      else if ( !obj )
        rname = "Object: *NULL obj*";
      else
        rname = oname;
      sprintf(pbuf, "Hide %s (%d)\n\r", rname,
          (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
      break;
    case 'B':
      {
      char * const *flagarray;
      bool ext_bv = FALSE;
      
      strcpy(pbuf, "BIT: ");
      pbuf += 5;
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) )
      {
        strcpy(pbuf, "Set: ");
        pbuf += 5;
      }
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
      {
        strcpy(pbuf, "Toggle: ");
        pbuf += 8;
      }
      else
      {
        strcpy(pbuf, "Remove: ");
        pbuf += 8;
      }
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int door;
        
        if ( !(room = get_room_index(pReset->arg1)) )
          rname = "Room: *BAD VNUM*";
        else
          rname = room->name;
        door = (pReset->arg2 & BIT_RESET_DOOR_MASK)
             >> BIT_RESET_DOOR_THRESHOLD;
        door = URANGE(0, door, MAX_DIR+1);
        sprintf(pbuf, "Exit %s%s (%d), Room %s (%d)", dir_name[door],
            (room && get_exit(room, door) ? "" : " (NO EXIT!)"), door,
            rname, pReset->arg1);
        }
        flagarray = ex_flags;
        break;
      case BIT_RESET_ROOM:
        if ( !(room = get_room_index(pReset->arg1)) )
          rname = "Room: *BAD VNUM*";
        else
          rname = room->name;
        sprintf(pbuf, "Room %s (%d)", rname, pReset->arg1);
        flagarray = r_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
          if ( !(obj2 = get_obj_index(pReset->arg1)) )
            rname = "Object: *BAD VNUM*";
          else
            rname = obj2->name;
        else if ( !obj )
          rname = "Object: *NULL obj*";
        else
          rname = oname;
        sprintf(pbuf, "Object %s (%d)", rname,
            (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
        flagarray = o_flags;
        ext_bv = TRUE;
        break;
      case BIT_RESET_MOBILE:
        if ( pReset->arg1 > 0 )
        {
          MOB_INDEX_DATA *mob2;
          
          if ( !(mob2 = get_mob_index(pReset->arg1)) )
            rname = "Mobile: *BAD VNUM*";
          else
            rname = mob2->player_name;
        }
        else if ( !mob )
          rname = "Mobile: *NULL mob*";
        else
          rname = mname;
        sprintf(pbuf, "Mobile %s (%d)", rname,
            (pReset->arg1 > 0 ? pReset->arg1 : mob ? mob->vnum : 0));
        flagarray = a_flags;
        ext_bv = TRUE;
        break;
      default:
        sprintf(pbuf, "bad type %d", pReset->arg2 & BIT_RESET_TYPE_MASK);
        flagarray = NULL;
        break;
      }
      pbuf += strlen(pbuf);
      if ( flagarray )
      {
        if (ext_bv)
        {
          EXT_BV tmp;
          
          tmp = meb(pReset->arg3);
          sprintf(pbuf, "; flags: %s [%d]\n\r",
          	ext_flag_string(&tmp, flagarray), pReset->arg3);
        }
        else
          sprintf(pbuf, "; flags: %s [%d]\n\r",
          	flag_string(pReset->arg3, flagarray), pReset->arg3);
      }
      else
        sprintf(pbuf, "; flags %d\n\r", pReset->arg3);
      }
      break;
    case 'D':
      {
      char *ef_name;
      
      pReset->arg2 = URANGE(0, pReset->arg2, MAX_DIR+1);
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      switch(pReset->arg3)
      {
      default:	ef_name = "(* ERROR *)";	break;
      case 0:	ef_name = "Open";		break;
      case 1:	ef_name = "Close";		break;
      case 2:	ef_name = "Close and lock";	break;
      }
      sprintf(pbuf, "%s [%d] the %s%s [%d] door %s (%d)\n\r", ef_name,
          pReset->arg3, dir_name[pReset->arg2],
          (room && get_exit(room, pReset->arg2) ? "" : " (NO EXIT!)"),
          pReset->arg2, rname, pReset->arg1);
      }
      break;
    case 'R':
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf(pbuf, "Randomize exits 0 to %d -> %s (%d)\n\r", pReset->arg2,
          rname, pReset->arg1);
      break;
    }
    if ( start == -1 || num >= start )
      send_to_char( buf, ch );
    if ( end != -1 && num >= end )
      break;
  }
  if ( num == 0 )
    send_to_char( "You don't have any resets defined.\n\r", ch );
  return;
}

/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets( AREA_DATA *pArea )
{
  RESET_DATA *pReset, *lastobj = NULL;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    switch(pReset->command)
    {
    default:
      break;
    case 'G': case 'E': case 'O':
      lastobj = pReset;
      break;
    case 'P':
      if ( pReset->arg3 == 0 )
      {
        if ( !lastobj )
          pReset->extra = 1000000;
        else if ( lastobj->command != 'P' || lastobj->arg3 > 0 )
          pReset->extra = 0;
        else
          pReset->extra = lastobj->extra+1;
        lastobj = pReset;
      }
    }
  }
  return;
}

/*
 * Create a new reset (for online building)			-Thoric
 */
RESET_DATA *make_reset( char letter, int extra, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;

	CREATE( pReset, RESET_DATA, 1 );
	pReset->command	= letter;
	pReset->extra	= extra;
	pReset->arg1	= arg1;
	pReset->arg2	= arg2;
	pReset->arg3	= arg3;
	top_reset++;	
	return pReset;
}

/*
 * Add a reset to an area				-Thoric
 */
RESET_DATA *add_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset;

    if ( !tarea )
    {
	bug( "add_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, extra, arg1, arg2, arg3 );
    switch( letter )
    {
	case 'M':  tarea->last_mob_reset = pReset;	break;
	case 'H':  if ( arg1 > 0 )			break;
	case 'E':  case 'G':  case 'P':
	case 'O':  tarea->last_obj_reset = pReset;	break;
	case 'T':
	    if ( IS_SET( extra, TRAP_OBJ ) && arg1 == 0)
		tarea->last_obj_reset = pReset;
	    break;
    }

    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}

/*
 * Place a reset into an area, insert sorting it		-Thoric
 */
RESET_DATA *place_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset, *tmp, *tmp2;

    if ( !tarea )
    {
	bug( "place_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, extra, arg1, arg2, arg3 );
    if ( letter == 'M' )
	tarea->last_mob_reset = pReset;

    if ( tarea->first_reset )
    {
	switch( letter )
	{
	    default:
		bug( "place_reset: Bad reset type %c", letter );
		return NULL;
	    case 'D':	case 'R':
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		if ( tmp )	/* organize by location */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 > arg1; tmp = tmp->prev );
		if ( tmp )	/* organize by direction */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 == tmp->arg1 && tmp->arg2 > arg2; tmp = tmp->prev );
		if ( tmp )
		    INSERT( pReset, tmp, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'M':	case 'O':
		/* find last reset of same type */
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		tmp2 = tmp ? tmp->next : NULL;
		/* organize by location */
		for ( ; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter && tmp->arg3 <= arg3 )
		    {
			tmp2 = tmp->next;
			/* organize by vnum */
			if ( tmp->arg3 == arg3 )
			  for ( ; tmp; tmp = tmp->prev )
			    if ( tmp->command == letter
			    &&   tmp->arg3 == tmp->arg3
			    &&   tmp->arg1 <= arg1 )
			    {
				tmp2 = tmp->next;
				break;
			    }
			    break;
			}
		/* skip over E or G for that mob */
		if ( tmp2 && letter == 'M' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'E' && tmp2->command != 'G' )
			    break;
		}
		else
		/* skip over P, T or H for that obj */
		if ( tmp2 && letter == 'O' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'P' && tmp2->command != 'T'
			&&   tmp2->command != 'H' )
			    break;
		}
		if ( tmp2 )
		    INSERT( pReset, tmp2, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'G':	case 'E':
		/* find the last mob */
		if ( (tmp=tarea->last_mob_reset) != NULL )
		{
		    /*
		     * See if there are any resets for this mob yet,
		     * put E before G and organize by vnum
		     */
		    if ( tmp->next )
		    {
			tmp = tmp->next;
			if ( tmp && tmp->command == 'E' )
			{
			    if ( letter == 'E' )
				for ( ; tmp && tmp->command == 'E' && tmp->arg1 < arg1; tmp = tmp->next );
			    else
				for ( ; tmp && tmp->command == 'E'; tmp = tmp->next );
			}
			else
			if ( tmp && tmp->command == 'G' && letter == 'G' )
			    for ( ; tmp && tmp->command == 'G' && tmp->arg1 < arg1; tmp = tmp->next );
			if ( tmp )
			    INSERT( pReset, tmp, tarea->first_reset, next, prev );
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    }
		    else
			LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}
		break;
	    case 'P':	case 'T':   case 'H':
		/* find the object in question */
		if ( ((letter == 'P' && arg3 == 0)
		||    (letter == 'T' && IS_SET(extra, TRAP_OBJ) && arg1 == 0)
		||    (letter == 'H' && arg1 == 0))
		&&    (tmp=tarea->last_obj_reset) != NULL )
		{
		    if ( (tmp=tmp->next) != NULL )
		      INSERT( pReset, tmp, tarea->first_reset, next, prev );
		    else
		      LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}

		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		   if ( (tmp->command == 'O' || tmp->command == 'G'
		   ||    tmp->command == 'E' || tmp->command == 'P')
		   &&    tmp->arg1 == arg3 )
		   {
			/*
			 * See if there are any resets for this object yet,
			 * put P before H before T and organize by vnum
			 */
			if ( tmp->next )
			{
			    tmp = tmp->next;
			    if ( tmp && tmp->command == 'P' )
			    {
				if ( letter == 'P' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'T' )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'H' )
			    {
				if ( letter == 'H' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'H' )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'T' && letter == 'T' )
				for ( ; tmp && tmp->command == 'T' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
			    if ( tmp )
				INSERT( pReset, tmp, tarea->first_reset, next, prev );
			    else
				LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			}
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			return pReset;
		   }
		break;
	}
	/* likely a bad reset if we get here... add it anyways */
    }
    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
    OBJ_DATA *trap;

    trap = create_object( get_obj_index( OBJ_VNUM_TRAP ), 0 );
    trap->timer = 0;
    trap->value[0] = v0;
    trap->value[1] = v1;
    trap->value[2] = v2;
    trap->value[3] = v3;
    return trap;
}


/*
 * Reset everything.
 */
void reset_all( )
{

   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;
   CHAR_DATA * mob = NULL;
   MOB_INDEX_DATA *pMobIndex = NULL;
   OBJ_INDEX_DATA *pObjIndex = NULL;
   OBJ_DATA * obj = NULL;
   EXIT_DATA *xit;
   bool found = FALSE;
   int vnum, anumber, onum;

   /* natural disasters */
   {
       PLANET_DATA * dPlanet = NULL;
       int pCount = 0;
       int rCount;
       CLAN_DATA * dClan = NULL;
       DESCRIPTOR_DATA *d = NULL;
       
       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
           pCount++;
           
       rCount = number_range( 1 , pCount );
       
       pCount = 0;
       
       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
           if ( ++pCount == rCount )
               break;
       
       if ( dPlanet && dPlanet->area && dPlanet->governed_by )
           dClan = dPlanet->governed_by;
           
       if ( dClan )
          for ( d = first_descriptor ; d ; d = d->next )
              if ( d->connected == CON_PLAYING && !d->original && 
              d->character && d->character->pcdata && d->character->pcdata->clan 
              && d->character->pcdata->clan == dPlanet->governed_by )
                      break;
         
      /* it seems that aliens were always attacking, I'm going to make it a little bit more *
       * random, until I can figure it all out. -Lews                                       */
       if( number_range( 0, 100 ) < 90 )
       d = NULL;
                   
       if ( d )
       {
         switch ( number_bits( 2 ) )
         {
         case 0:
         
            for ( pRoomIndex = dPlanet->area->first_room ; pRoomIndex ; pRoomIndex = pRoomIndex->next_in_area )
              if ( pRoomIndex->sector_type == SECT_CITY 
              && !IS_SET( pRoomIndex->room_flags , ROOM_NO_MOB ) )
                 break;
            if( pRoomIndex )
            {
               int mCount;
               char dBuf[MAX_STRING_LENGTH];
               
               if ( (pMobIndex = get_mob_index(MOB_VNUM_ALIEN)) )
               {
                sprintf( dBuf , "(GNET) A Colonist: Help %s is being invaded by alien forces" , dPlanet->name );    
                echo_to_all( AT_LBLUE , dBuf , ECHOTAR_ALL );
                for( mCount = 0 ; mCount < 20 ; mCount++ )
                {
                  mob = create_mobile( pMobIndex );
                  char_to_room( mob, pRoomIndex );
                  mob->hit = 100;
                  mob->max_hit = 100;
                  if ( room_is_dark(pRoomIndex) )
                   xSET_BIT(mob->affected_by, AFF_INFRARED);
                  if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
                  {
                     obj = create_object( pObjIndex, mob->level );
                     obj_to_char( obj, mob );
                     equip_char( mob, obj, WEAR_WIELD );                        
                  }
                  /* no blasters yet -Lews 
                  do_setblaster( mob , "full" );
                   */
                }
               }
              
            }
            break;
         
         default:
            break;
         }
         
       }  
   }
      
   for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {    
     for ( pRoomIndex  = room_index_hash[iHash]; pRoomIndex; pRoomIndex  = pRoomIndex->next )
     {    
        vnum = 0;
        onum = 0;
        
        for ( xit = pRoomIndex->first_exit ; xit ; xit = xit->next )
             if ( IS_SET(xit->exit_info , EX_ISDOOR ) )
             {
                 SET_BIT( xit->exit_info , EX_CLOSED );
                 if ( xit->key >= 0 )
                     SET_BIT( xit->exit_info , EX_LOCKED );
             }
        
        
        if ( IS_SET(pRoomIndex->room_flags, ROOM_CONTROL ) 
        && pRoomIndex->area && pRoomIndex->area->planet  && pRoomIndex->area->planet->starsystem 
        && pRoomIndex->area->planet->governed_by )
        {
           SPACE_DATA * system =  pRoomIndex->area->planet->starsystem;
           SHIP_DATA * ship;
           int numpatrols = 0;
           int numdestroyers = 0;
           int numbattleships = 0;
           int numcruisers = 0;
           int fleetsize = 0;

           for ( ship = system->first_ship ; ship ; ship = ship->next_in_starsystem )
               if ( !str_cmp( ship->owner , pRoomIndex->area->planet->governed_by->name )
               && ship->type == MOB_SHIP )
               {
                  if ( ship->model == MOB_DESTROYER )
                     numdestroyers++;
                  else if ( ship->model == MOB_CRUISER )
                     numcruisers++;
                  else if ( ship->model == MOB_BATTLESHIP )
                     numbattleships++;
                  else 
                     numpatrols++;
               }
                      
           fleetsize = 100*numbattleships + 25*numcruisers + 5*numdestroyers + numpatrols;
                  
           if ( fleetsize + 100 < pRoomIndex->area->planet->controls )
              make_mob_ship( pRoomIndex->area->planet , MOB_BATTLESHIP );
           else if ( fleetsize + 25 < pRoomIndex->area->planet->controls
           && numcruisers < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_CRUISER );
           else if ( fleetsize + 5 < pRoomIndex->area->planet->controls
           && numdestroyers < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_DESTROYER );
           else  if ( fleetsize < pRoomIndex->area->planet->controls
           && numpatrols < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_PATROL );
        }
        
        if ( IS_SET(pRoomIndex->room_flags, ROOM_GARAGE ) )
             vnum = MOB_VNUM_MECHANIC;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_CONTROL ) )
             vnum = MOB_VNUM_CONTROLLER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_SHIPYARD ) )
             vnum = MOB_VNUM_TECHNICIAN;
        
        if ( vnum > 0 )
        {
            found = FALSE;

	    for ( mob = pRoomIndex->first_person; mob ;mob = mob->next_in_room )
	    {
	        if (  IS_NPC( mob ) && mob->pIndexData && mob->pIndexData->vnum == vnum )
		    found = TRUE;
            }
            
            if ( !found  )
            {
            
                if ( !(pMobIndex = get_mob_index(vnum)) )
                {
        	   bug( "Reset_all: Missing mob (%d)", vnum );
        	   return;
      		}
                mob = create_mobile(pMobIndex);
                xSET_BIT ( mob->act , ACT_CITIZEN );           
                if ( room_is_dark(pRoomIndex) )
                   xSET_BIT(mob->affected_by, AFF_INFRARED);
                char_to_room(mob, pRoomIndex);
      		if ( pRoomIndex->area && pRoomIndex->area->planet )
                     pRoomIndex->area->planet->population++;

/* not using the ROOM_NOPEDIT flag -Lews
	    	if ( ( IS_SET(pRoomIndex->room_flags, ROOM_NOPEDIT) && vnum == MOB_VNUM_TRADER ) 
	    	||  vnum == MOB_VNUM_SUPPLIER )
	        {
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
      		     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_LIGHT)) )
      			{
        		   bug( "Reset_all: Missing default light (%d)", OBJ_VNUM_LIGHT );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
      		     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_COMLINK)) )
      			{
        		   bug( "Reset_all: Missing default comlink (%d)", OBJ_VNUM_COMLINK );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
                   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
        	     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_CANTEEN)) )
      			{
        		   bug( "Reset_all: Missing default canteen (%d)", OBJ_VNUM_CANTEEN );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
                     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_SHOVEL)) )
      			{
        		   bug( "Reset_all: Missing default shovel (%d)", OBJ_VNUM_SHOVEL );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
                }
                
                if ( vnum == MOB_VNUM_SUPPLIER )   
	        {
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_BATTERY)) )
                     {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_BACKPACK)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_AMMO)) )
                     {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);	        
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
	             }
	           }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_BLASTER)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
	             }
	           }
	           
	           onum = number_range( OBJ_VNUM_FIRST_PART ,OBJ_VNUM_LAST_PART  );
                   if ( (pObjIndex = get_obj_index(onum)) )
                   {
                     obj = create_object(pObjIndex, 1);
                     obj = obj_to_char(obj, mob);
                     xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
	           }
	        }
*/	    
	    
	       if ( vnum == MOB_VNUM_WAITER )   
	       {
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_APPETIZER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_SALAD)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_LUNCH)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_DINNER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_GLASSOFWATER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_COFFEE)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
	       }

	       if ( vnum == MOB_VNUM_BARTENDER )   
	       {
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_BEER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_WHISKEY)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_GLASSOFWATER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_COFFEE)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
	       }
	    }
	    
	}
        
        /* no mail rooms here -Lews
        if ( IS_SET( pRoomIndex->room_flags, ROOM_MAIL ) )
        {
   	   if ( !(pObjIndex = get_obj_index(OBJ_VNUM_MAIL_TERMINAL)) )
   	   {
             bug( "Reset_all: Missing mail terminal (%d)", OBJ_VNUM_MAIL_TERMINAL );
             return;
   	   }
   	   if ( count_obj_list(pObjIndex, pRoomIndex->first_content) <= 0 )
   	   {
      	     obj = create_object(pObjIndex, 1);
             obj_to_room(obj, pRoomIndex);
   	   }
        }
        

        if ( IS_SET( pRoomIndex->room_flags, ROOM_INFO ) )
        {
   	   if ( !(pObjIndex = get_obj_index(OBJ_VNUM_MESSAGE_TERMINAL)) )
   	   {
             bug( "Reset_all: Missing message terminal (%d)", OBJ_VNUM_MESSAGE_TERMINAL );
             return;
   	   }
   	   if ( count_obj_list(pObjIndex, pRoomIndex->first_content) <= 0 )
   	   {
      	     obj = create_object(pObjIndex, 1);
             obj_to_room(obj, pRoomIndex);
   	   }
        }
        */

        if ( IS_SET( pRoomIndex->room_flags, ROOM_BARRACKS ) 
        && pRoomIndex->area && pRoomIndex->area->planet)
        {
            int guard_count = 0;
            OBJ_DATA * blaster;
            GUARD_DATA * guard;
            char tmpbuf[MAX_STRING_LENGTH];
        
            if ( !(pMobIndex = get_mob_index(MOB_VNUM_PATROL)) )
            {
        	   bug( "Reset_all: Missing default patrol (%d)", vnum );
        	   return;
      	    }
            
            for ( guard = pRoomIndex->area->planet->first_guard ; guard ; guard = guard->next_on_planet )
                guard_count++;
                
            if ( pRoomIndex->area->planet->barracks*5 <= guard_count ) 
                continue;
                
            mob = create_mobile( pMobIndex );
            char_to_room( mob, pRoomIndex );
            mob->level = 10;
            mob->hit = 100;
            mob->max_hit = 100;
            mob->armor = 50;
            mob->damroll = 0;
            mob->hitroll = 20;
            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
            {
                 blaster = create_object( pObjIndex, mob->level );
                 obj_to_char( blaster, mob );
                 equip_char( mob, blaster, WEAR_WIELD );                        
            } 
            /* no blasters -lews
            do_setblaster( mob , "full" );
             */

            CREATE( guard , GUARD_DATA , 1 );
                
            guard->planet = pRoomIndex->area->planet;
            LINK( guard , guard->planet->first_guard, guard->planet->last_guard, next_on_planet, prev_on_planet );
            LINK( guard , first_guard, last_guard, next, prev );
            guard->mob = mob;
            guard->reset_loc = pRoomIndex;
            mob->guard_data = guard;           
            if ( room_is_dark(pRoomIndex) )
                   xSET_BIT(mob->affected_by, AFF_INFRARED);
            if ( pRoomIndex->area->planet->governed_by )
            {
                sprintf( tmpbuf , "A soldier patrols the area. (%s)\n\r" , pRoomIndex->area->planet->governed_by->name );
                STRFREE( mob->long_descr );
                mob->long_descr = STRALLOC( tmpbuf );
                mob->mob_clan  = pRoomIndex->area->planet->governed_by;
            }
            
            continue; 
        }

        if ( ( IS_SET( pRoomIndex->room_flags, ROOM_SHIPYARD ) 
        || IS_SET( pRoomIndex->room_flags, ROOM_CAN_LAND ) )
        && pRoomIndex->area && pRoomIndex->area->planet)
        {
            char tmpbuf[MAX_STRING_LENGTH];
            CHAR_DATA * rch;
            int numguards = 0;
             
            if ( !(pMobIndex = get_mob_index(MOB_VNUM_GUARD)) )
            {
        	   bug( "Reset_all: Missing default guard (%d)", vnum );
        	   return;
      	    }
            
            for( rch = pRoomIndex->first_person ; rch ; rch = rch->next_in_room )
               if ( IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum == MOB_VNUM_GUARD )
                   numguards++;
            
            if ( numguards >= 2 )
               continue;          
            
            mob = create_mobile( pMobIndex );
            char_to_room( mob, pRoomIndex );
            mob->level = 100;
            mob->hit = 500;
            mob->max_hit = 500;
            mob->armor = 0;
            mob->damroll = 0;
            mob->hitroll = 20;

            if ( room_is_dark(pRoomIndex) )
                   xSET_BIT(mob->affected_by, AFF_INFRARED);
            if ( pRoomIndex->area->planet->governed_by )
            {
                sprintf( tmpbuf , "A Platform Security Guard stands alert and ready for trouble. (%s)\n\r" , pRoomIndex->area->planet->governed_by->name );
                STRFREE( mob->long_descr );
                mob->long_descr = STRALLOC( tmpbuf );
                mob->mob_clan  = pRoomIndex->area->planet->governed_by;
            }
            
        }

/* hidden food & resources */

        if ( !pRoomIndex->area || !pRoomIndex->area->planet )
           continue;

        anumber = number_bits( 2 );
        
        if ( pRoomIndex->sector_type != SECT_CITY &&  pRoomIndex->sector_type != SECT_WATER_NOSWIM  
        &&  pRoomIndex->sector_type != SECT_WATER_SWIM &&  pRoomIndex->sector_type != SECT_UNDERWATER
        &&  pRoomIndex->sector_type != SECT_DUNNO  &&  pRoomIndex->sector_type != SECT_AIR        
        &&  pRoomIndex->sector_type != SECT_INSIDE  &&  pRoomIndex->sector_type != SECT_GLACIAL
        && !pRoomIndex->last_content && number_bits(3) == 0 )
        {
            switch ( pRoomIndex->sector_type )
            {
                default:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_PLANT;
                   else
                      vnum = OBJ_VNUM_HEMP;
                   break;

                case SECT_FARMLAND:
                   if ( anumber == 0 )
                      vnum = OBJ_VNUM_FRUIT;
                   else  if ( anumber == 1 )
                      vnum = OBJ_VNUM_ROOT;
                   else  if ( anumber == 2 )
                      vnum = OBJ_VNUM_MUSHROOM;
                   else
                      vnum = OBJ_VNUM_PLANT;
                   break;
                   
                case SECT_FOREST:
                case SECT_BRUSH:
                case SECT_SCRUB:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_ROOT;
                   else
                      vnum = OBJ_VNUM_HEMP;
                   break;
                
                case SECT_RAINFOREST:
                case SECT_JUNGLE:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_FRUIT;
                   else
                      vnum = OBJ_VNUM_RESIN;
                   break;
                                   
                case SECT_MOUNTAIN:
                case SECT_UNDERGROUND: 
                case SECT_ROCKY:
                case SECT_TUNDRA: 
                case SECT_VOLCANIC:            
                   if ( anumber == 0 )
                      vnum = OBJ_VNUM_MUSHROOM;
                   else
                   if ( anumber == 1 )
                      vnum = OBJ_VNUM_CRYSTAL;
                   else 
                   if ( anumber == 2 )
                      vnum = OBJ_VNUM_METAL;
                   else
                      vnum = OBJ_VNUM_GOLD;
                   break;

                case SECT_SWAMP:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_SEAWEED;
                   else
                      vnum = OBJ_VNUM_RESIN;
                   break;
                
                case SECT_OCEANFLOOR: 
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_SEAWEED;
                   else
                      vnum = OBJ_VNUM_CRYSTAL;
                   break;
            }

            if ( !(pObjIndex = get_obj_index(vnum)) )
            {
        	   bug( "Reset_all: Missing obj (%d)", vnum );
        	   return;
      	    }
            obj = create_object(pObjIndex , 1);
            if ( pRoomIndex->sector_type != SECT_FARMLAND )
            {
               if ( vnum == OBJ_VNUM_ROOT || vnum == OBJ_VNUM_CRYSTAL 
               || vnum == OBJ_VNUM_GOLD || vnum == OBJ_VNUM_METAL ) 
                  xSET_BIT ( obj->extra_flags , ITEM_BURIED );           
               else
                  xSET_BIT ( obj->extra_flags , ITEM_HIDDEN );           
            }
            if ( vnum == OBJ_VNUM_GOLD )
            {
               obj->value[0] = number_range(1,10);
               obj->value[1] = obj->value[0];
               obj->cost =  obj->value[0]*10;
            }
            obj_to_room(obj, pRoomIndex);
             
        }
        
/* random mobs start here */        
        

        if ( IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB ) )
           continue;
        
        if ( number_bits(1) != 0 )
           continue;
           
        if ( pRoomIndex->sector_type == SECT_CITY )
        {
           if ( pRoomIndex->area->planet->population >= max_population(pRoomIndex->area->planet) )
              continue;  
           
           if ( pRoomIndex->area->planet->tec_level > 0 )
           {
           if ( number_bits( 5 ) == 0 )
           {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_VENDOR)) )
              {
                  int rep;
                  
                  mob = create_mobile(pMobIndex);
                  xSET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  for ( rep = 0 ; rep < 3 ; rep++ )
                    if ( (pObjIndex = get_obj_index( number_range( OBJ_VNUM_FIRST_FABRIC , OBJ_VNUM_LAST_FABRIC  ) ) ) )
                    {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_SEWKIT ) ) )
                  {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                  }
                  continue;
              }
           }

           if ( number_bits( 6 ) == 0 )
           {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_DEALER)) )
              {
                  mob = create_mobile(pMobIndex);
                  xSET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_BLACK_POWDER ) ) )
                  {
                        obj = create_object(pObjIndex, 1);
                        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                  }
                  continue;
              }
           }
           if ( number_bits( 6 ) == 0 )
           {
              int mnum;
              
              switch ( number_bits( 2 ) )
              {
                   default:
                          mnum = MOB_VNUM_BUM;
                          break;
                   case 0:
                          mnum = MOB_VNUM_THUG;
                          break;
                   case 1:
                          mnum = MOB_VNUM_THIEF;
                          break;
              }
              if ( (pMobIndex = get_mob_index(mnum)) )
              {
                  mob = create_mobile(pMobIndex);
                  xSET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  continue;
              }
           }
           
           }
           
           
           if ( !(pMobIndex = get_mob_index(MOB_VNUM_CITIZEN)) )
           {
        	   bug( "Reset_all: Missing default colonist (%d)", vnum );
        	   return;
           }
           mob = create_mobile(pMobIndex);
           xSET_BIT ( mob->act , ACT_CITIZEN );           
           mob->sex = number_bits( 1 )+1;
           STRFREE( mob->long_descr );
           STRFREE( mob->short_descr );
           STRFREE( mob->name );
           
           switch( pRoomIndex->area->planet->tec_level )
           {
           	case 0:
           	
           	 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	  default:
                	  mob->long_descr = STRALLOC( "A caveman walks by.\n\r" );
                	  mob->short_descr = STRALLOC( "A caveman" );
                	  mob->name = STRALLOC( "caveman" );
                	  if ( (pObjIndex = get_obj_index( OBJ_VNUM_CLUB ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                	  break;

               	  case 0:
                	mob->long_descr = STRALLOC( "A cavewoman walks by.\n\r" );
                	mob->short_descr = STRALLOC( "A cavewoman" );
                	mob->name = STRALLOC( "cavewoman" );
                  	mob->gold = number_range( 1 , 3 );
                  	break;

                	case 1:
                  	mob->long_descr = STRALLOC( "A young cavegirl smiles at you as she walks by.\n\r" );
                	mob->short_descr = STRALLOC( "A cavegirl" );
                	mob->name = STRALLOC( "cavegirl" );
                  	break;

                	case 2:
                  	mob->long_descr = STRALLOC( "A young cavegirl is looking at you.\n\r" );
                	  mob->short_descr = STRALLOC( "A cavegirl" );
                	  mob->name = STRALLOC( "cavegirl" );
           	       break;

             	   case 3:
            	      mob->long_descr = STRALLOC( "A cavewman looks at quizicly at you.\n\r" );
                      mob->short_descr = STRALLOC( "A cavewoman" );
                      mob->name = STRALLOC( "cavewoman" );
            	      mob->gold = number_range( 2 , 4 );
            	      break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An old cavewoman walks slowly by.\n\r" );
                       mob->short_descr = STRALLOC( "A cavewoman" );
                       mob->name = STRALLOC( "cavewoman" );
           	       mob->gold = number_range( 2 , 9 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A caveman is going about his buisness.\n\r" );
                	 mob->short_descr = STRALLOC( "A caveman" );
                	 mob->name = STRALLOC( "caveman" );
                	 if ( (pObjIndex = get_obj_index( OBJ_VNUM_CLUB ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A mean looking caveman is looking at you.\n\r" );
         	         mob->short_descr = STRALLOC( "A caveman" );
         	         mob->name = STRALLOC( "caveman" );
        	         mob->gold = number_range( 1 , 100 );
        	         if ( (pObjIndex = get_obj_index( OBJ_VNUM_CLUB ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young caveman is walking by.\n\r" );
           	       mob->short_descr = STRALLOC( "A caveman" );
           	       mob->name = STRALLOC( "caveman" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young caveman is playing.\n\r" );
                 	 mob->short_descr = STRALLOC( "A caveman" );
                 	 mob->name = STRALLOC( "caveman" );
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A caveman looks to be in a hurry.\n\r" );
                 	 mob->short_descr = STRALLOC( "A caveman" );
                 	 mob->name = STRALLOC( "caveman" );
                 	 mob->gold = number_range( 1 , 10 );
                 	 if ( (pObjIndex = get_obj_index( OBJ_VNUM_CLUB ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An old caveman hobbles by.\n\r" );
                  	mob->short_descr = STRALLOC( "A caveman" );
                  	mob->name = STRALLOC( "caveman" );
                  	mob->gold = number_range( 5 , 15 );
                  	break;
              	}
           	
           	break;
           	
           	case 1:
           	 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	   default:
                	  mob->long_descr = STRALLOC( "A peasent is going about her daily business.\n\r" );
                	  mob->short_descr = STRALLOC( "A peaset" );
                	  mob->name = STRALLOC( "peasent woman" );
                	  break;

               	 case 0:
                	mob->long_descr = STRALLOC( "A wealthy lady is dressed in fine silks.\n\r" );
                	mob->short_descr = STRALLOC( "A lady" );
                	mob->name = STRALLOC( "lady" );
                  	mob->gold = number_range( 5 , 130 );
                  	break;

                	case 1:
                  	mob->long_descr = STRALLOC( "A young maiden smiles at you as she walks by.\n\r" );
                  	mob->short_descr = STRALLOC( "A maiden" );
                  	mob->name = STRALLOC( "maiden" );
                  	break;

                	case 2:
                  	mob->long_descr = STRALLOC( "A young girl is skipping.\n\r" );
                  	mob->short_descr = STRALLOC( "A young girl" );
                  	mob->name = STRALLOC( "girl" );
           	       break;

             	   case 3:
            	      mob->long_descr = STRALLOC( "A merchant is in a fine dress.\n\r" );
            	      mob->short_descr = STRALLOC( "A merchant" );
            	      mob->name = STRALLOC( "merchant woman" );
            	      mob->gold = number_range( 20 , 50 );
            	      break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An old woman strolls by.\n\r" );
           	       mob->short_descr = STRALLOC( "A peaset" );
           	       mob->name = STRALLOC( "peasent woman" );
          	        mob->gold = number_range( 20 , 50 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A peasent is going about his daily business.\n\r" );
           	      	 mob->short_descr = STRALLOC( "A peaset" );
           	      	 mob->name = STRALLOC( "peasent man" );
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A wealthy lord is dressed in fine silk robes.\n\r" );
         	         mob->short_descr = STRALLOC( "A lord" );
         	         mob->name = STRALLOC( "lord" );
        	         mob->gold = number_range( 1 , 100 );
        	         if ( (pObjIndex = get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young peasent is just hanging out.\n\r" );
           	       mob->short_descr = STRALLOC( "A peaset" );
           	       mob->name = STRALLOC( "peasent" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young boy is kicking a small stone around.\n\r" );
                 	 mob->short_descr = STRALLOC( "A boy" );
                 	 mob->name = STRALLOC( "boy" );
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A merchant looks to be in a hurry.\n\r" );
                 	 mob->short_descr = STRALLOC( "A merchant" );
                 	 mob->name = STRALLOC( "merchant man" );
                 	 mob->gold = number_range( 20 , 50 );
                 	 if ( (pObjIndex = get_obj_index( OBJ_VNUM_SHSWORD ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An elderly man strolls by.\n\r" );
                  	mob->short_descr = STRALLOC( "A peaset" );
                  	mob->name = STRALLOC( "peasent man" );
                  	mob->gold = number_range( 20 , 50 );
                  	break;
                  }
           	
           	break;
           	
           	case 2:
           	 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	   default:
                	  mob->long_descr = STRALLOC( "A woman is going about her daily business.\n\r" );
                	  mob->short_descr = STRALLOC( "A woman" );
                	  mob->name = STRALLOC( "woman" );
                	  break;

               	 case 0:
                	  mob->long_descr = STRALLOC( "A woman is jogging by.\n\r" );
                	  mob->short_descr = STRALLOC( "A jogger" );
                	  mob->name = STRALLOC( "woman jogger" );
                	  if ( (pObjIndex = get_obj_index( OBJ_VNUM_MACE ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	  if ( (pObjIndex = get_obj_index( OBJ_VNUM_WALKMAN ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	  mob->gold = number_range( 1 , 50 );
                break;

                	case 1:
                  	mob->long_descr = STRALLOC( "A young girl smiles at you as she walks by.\n\r" );
                  	mob->short_descr = STRALLOC( "A girl" );
                  	mob->name = STRALLOC( "girl" );
                  	if ( (pObjIndex = get_obj_index( OBJ_VNUM_WALKMAN ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	break;

                	case 2:
                  	mob->long_descr = STRALLOC( "A young schoolgirl is skipping.\n\r" );
                  	mob->short_descr = STRALLOC( "A schoolgirl" );
                  	mob->name = STRALLOC( "girl schoolgirl" );
           	       break;

             	   case 3:
            	      mob->long_descr = STRALLOC( "A woman is dressed in a powersuit.\n\r" );
            	      mob->short_descr = STRALLOC( "An executive" );
            	      mob->name = STRALLOC( "executive woman" );
            	      mob->gold = number_range( 20 , 100 );
            	      if ( (pObjIndex = get_obj_index( OBJ_VNUM_MACE ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
            	      break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An elderly woman strolls by.\n\r" );
           	       mob->short_descr = STRALLOC( "A woman" );
           	       mob->name = STRALLOC( "woman" );
          	        mob->gold = number_range( 20 , 50 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A man is going about his daily business.\n\r" );
           	      	 mob->short_descr = STRALLOC( "A man" );
           	      	 mob->name = STRALLOC( "man" );
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A wealthy man is dressed in suit.\n\r" );
         	         mob->short_descr = STRALLOC( "A man" );
         	         mob->name = STRALLOC( "man" );
        	         mob->gold = number_range( 1 , 100 );
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young man is just hanging out.\n\r" );
           	       mob->short_descr = STRALLOC( "A man" );
           	       mob->name = STRALLOC( "man" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young boy is playing with a gameboy.\n\r" );
                 	 mob->short_descr = STRALLOC( "A boy" );
                 	 mob->name = STRALLOC( "boy" );
                 	 if ( (pObjIndex = get_obj_index( OBJ_VNUM_GAMEBOY ) ) )
			  {
                        	obj = create_object(pObjIndex, 1);
                        	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        	obj = obj_to_char(obj, mob);
                  	  }
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A businessman looks to be in a hurry.\n\r" );
                 	 mob->short_descr = STRALLOC( "An executive" );
                 	 mob->name = STRALLOC( "executive man buisnessman" );
                 	 mob->gold = number_range( 20 , 50 );
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An elderly man strolls by.\n\r" );
                  	mob->short_descr = STRALLOC( "An old man" );
                  	mob->name = STRALLOC( "man" );
                  	mob->gold = number_range( 20 , 50 );
                  	break;
                   }
           	
           	break;
           	
           	case 3:
           	mob->short_descr = STRALLOC( "A colonist" );
                 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	   default:
                	  mob->long_descr = STRALLOC( "A colonist is going about her daily business.\n\r" );
                	  mob->name = STRALLOC( "colonist citizen" );
                	  break;

               	  case 0:
                	  mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silks.\n\r" );
                	  mob->name = STRALLOC( "colonist citizen" );
                  	 mob->gold = number_range( 1 , 100 );
                  	break;

                 case 1:
                  	mob->long_descr = STRALLOC( "A young colonist smiles at you as she walks by.\n\r" );
                  	mob->name = STRALLOC( "colonist citizen kid" );
                 break;

                 case 2:
                  	mob->long_descr = STRALLOC( "A young schoolgirl is skipping.\n\r" );
                  	mob->name = STRALLOC( "colonist citizen girl shoolgirl" );
           	 break;

             	 case 3:
            	      mob->long_descr = STRALLOC( "A colonist is dressed in formal business attire.\n\r" );
            	      mob->name = STRALLOC( "colonist citizen" );
            	      mob->gold = number_range( 20 , 50 );
            	 break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
           	       mob->name = STRALLOC( "colonist citizen" );
          	        mob->gold = number_range( 20 , 50 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A colonist is going about his daily business.\n\r" );
           	      	 mob->name = STRALLOC( "colonist citizen" );
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silk robes.\n\r" );
         	         mob->name = STRALLOC( "colonist citizen" );
        	         mob->gold = number_range( 1 , 100 );
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young colonist is just hanging out.\n\r" );
           	       mob->name = STRALLOC( "colonist citizen kid" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young boy is kicking a small stone around.\n\r" );
                 	 mob->name = STRALLOC( "colonist citizen boy" );
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A businessman looks to be in a hurry.\n\r" );
                 	 mob->name = STRALLOC( "colonist citizen man buisnessman" );
                 	 mob->gold = number_range( 20 , 50 );
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
                  	mob->name = STRALLOC( "colonist citizen" );
                  	mob->gold = number_range( 20 , 50 );
                  	break;
           	  }
           	
           	break;
           	
           	case 4:
           	mob->short_descr = STRALLOC( "A colonist" );
           	mob->name = STRALLOC( "colonist citizen" );
           	 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	   default:
                	  mob->long_descr = STRALLOC( "A colonist is going about her daily business.\n\r" );
                	  break;

               	 case 0:
                	  mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silks.\n\r" );
                  	mob->gold = number_range( 1 , 100 );
                  	break;

                	case 1:
                  	mob->long_descr = STRALLOC( "A young colonist smiles at you as she walks by.\n\r" );
                  	break;

                	case 2:
                  	mob->long_descr = STRALLOC( "A young schoolgirl is skipping.\n\r" );
           	       break;

             	   case 3:
            	      mob->long_descr = STRALLOC( "A colonist is dressed in formal business attire.\n\r" );
            	      mob->gold = number_range( 20 , 50 );
            	      break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
          	        mob->gold = number_range( 20 , 50 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A colonist is going about his daily business.\n\r" );
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silk robes.\n\r" );
        	         mob->gold = number_range( 1 , 100 );
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young colonist is just hanging out.\n\r" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young boy is kicking a small stone around.\n\r" );
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A businessman looks to be in a hurry.\n\r" );
                 	 mob->gold = number_range( 20 , 50 );
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
                  	mob->gold = number_range( 20 , 50 );
                  	break;
           	   }
           	
           	break;
           	
           	default:
           	 mob->short_descr = STRALLOC( "A colonist" );
           	 mob->name = STRALLOC( "colonist citizen" );
          	 if ( mob->sex == 2 )
             	 switch( number_bits( 4 ) )
             	 {
             	   default:
                	  mob->long_descr = STRALLOC( "A colonist is going about her daily business.\n\r" );
                	  break;

               	 case 0:
                	  mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silks.\n\r" );
                  	mob->gold = number_range( 1 , 100 );
                  	break;

                	case 1:
                  	mob->long_descr = STRALLOC( "A young colonist smiles at you as she walks by.\n\r" );
                  	break;

                	case 2:
                  	mob->long_descr = STRALLOC( "A young schoolgirl is skipping.\n\r" );
           	       break;

             	   case 3:
            	      mob->long_descr = STRALLOC( "A colonist is dressed in formal business attire.\n\r" );
            	      mob->gold = number_range( 20 , 50 );
            	      break;
                  
            	    case 4:
           	       mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
          	        mob->gold = number_range( 20 , 50 );
          	        break;
          	    }
          	 else
         	     switch( number_bits( 3 ) )
        	      {
        	        default:
           	      	 mob->long_descr = STRALLOC( "A colonist is going about his daily business.\n\r" );
          	        break;

         	       case 0:
         	         mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silk robes.\n\r" );
        	         mob->gold = number_range( 1 , 100 );
          	        break;

            	      case 1:
           	       mob->long_descr = STRALLOC( "A young colonist is just hanging out.\n\r" );
            	      break;

               		case 2:
                 	 mob->long_descr = STRALLOC( "A young boy is kicking a small stone around.\n\r" );
                  	break;

                	case 3:
                 	 mob->long_descr = STRALLOC( "A businessman looks to be in a hurry.\n\r" );
                 	 mob->gold = number_range( 20 , 50 );
                  	break;

                	case 4:
                  	mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
                  	mob->gold = number_range( 20 , 50 );
                  	break;
              	}
              break;
             }
             
           char_to_room(mob, pRoomIndex);
           pRoomIndex->area->planet->population++;
           continue;
        }

        if ( pRoomIndex->area->planet->wildlife >  pRoomIndex->area->planet->wilderness )
              continue;  

        if ( pRoomIndex->sector_type > SECT_CITY && number_bits( 16 ) == 0 )
        {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_DRAGON)) )
              {
                  OBJ_DATA * nest;
                  
                  mob = create_mobile(pMobIndex);
                  xSET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_DRAGON_NEST ) ) )
                  {
                        nest = create_object(pObjIndex, 1);
                        nest = obj_to_room(nest, pRoomIndex);
                        if ( (pObjIndex = get_obj_index( OBJ_VNUM_DRAGON_CRYSTAL ) ) )
                        {
                            obj = create_object(pObjIndex, 1);
                            obj = obj_to_obj(obj, nest);
                        }
                  }
                  continue;
              }
        }

        anumber = number_bits( 3 );
        
        switch ( pRoomIndex->sector_type )
        {
           default:
              continue;
              break;

           case SECT_WATER_SWIM:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else
                vnum = MOB_VNUM_FISH;
              break;
                              
           case SECT_OCEANFLOOR:
           case SECT_UNDERWATER:
              vnum = MOB_VNUM_FISH;
              break;

           case SECT_AIR:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else
                vnum = MOB_VNUM_BIRD;
              break;

           case SECT_VOLCANIC:
           case SECT_UNDERGROUND: 
           case SECT_DESERT:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else
                vnum = MOB_VNUM_SCAVENGER;
              break;
                        
           case SECT_MOUNTAIN:
           case SECT_SCRUB:
           case SECT_ROCKY:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_BIRD;
              break;


           case SECT_FIELD:
           case SECT_FOREST:
           case SECT_HILLS:
           case SECT_SAVANNA:
           case SECT_BRUSH:
           case SECT_STEPPE:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_SMALL_ANIMAL;
              break;
              
           case SECT_TUNDRA:
              if ( anumber == 0 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_BIRD;
              break;

           case SECT_RAINFOREST:
           case SECT_JUNGLE:
           case SECT_SWAMP:
           case SECT_WETLANDS:
              if ( anumber == 0 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_INSECT;
              break;
  
        }
        
        if ( !(pMobIndex = get_mob_index(vnum)) )
        {
        	   bug( "Reset_all: Missing mob (%d)", vnum );
        	   return;
      	}
        mob = create_mobile(pMobIndex);
        xREMOVE_BIT ( mob->act , ACT_CITIZEN );           
        if ( room_is_dark(pRoomIndex) )
                   xSET_BIT(mob->affected_by, AFF_INFRARED);
        char_to_room(mob, pRoomIndex);
        pRoomIndex->area->planet->wildlife++;

     }
   }
                        
/*

  for ( pReset = first_reset; pReset; pReset = pReset->next )
  {
    num++;
    switch( pReset->command )
    {
    default:
      bug( "Reset_area: %d bad command %c.", num,  pReset->command );
      break;
    
    case 'M':
      if ( !(pMobIndex = get_mob_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'M': bad mob vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
        bug( "Reset_area: %d 'M': bad room vnum %ld.", num, pReset->arg3 );
        continue;
      }
      if ( pMobIndex->count >= pReset->arg2 )
      {
        mob = NULL;
        break;
      }
      mob = create_mobile(pMobIndex);
      {
        ROOM_INDEX_DATA *pRoomPrev = get_room_index(pReset->arg3 - 1);
        
        if ( pRoomPrev && IS_SET(pRoomPrev->room_flags, ROOM_PET_SHOP) )
          SET_BIT(mob->act, ACT_PET);
      }
      if ( room_is_dark(pRoomIndex) )
        xSET_BIT(mob->affected_by, AFF_INFRARED);
      char_to_room(mob, pRoomIndex);
      level = URANGE(0, mob->level - 2, LEVEL_AVATAR);
      if ( pRoomIndex->area->planet )
            pRoomIndex->area->planet->population++;
      break;
    
    case 'G':
    case 'E':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'E' or 'G': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !mob )
      {
        lastobj = NULL;
        break;
      }
      if ( mob->pIndexData->pShop )
      {
	int olevel = generate_itemlevel( pObjIndex );
        obj = create_object(pObjIndex, olevel);
        xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
      }
      else
        obj = create_object(pObjIndex, number_fuzzy(level));
      obj->level = URANGE(0, obj->level, LEVEL_AVATAR);
      obj = obj_to_char(obj, mob);
      if ( pReset->command == 'E' )
        equip_char(mob, obj, pReset->arg3);
      lastobj = obj;
      break;
    
    case 'O':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'O': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
        bug( "Reset_area: %d 'O': bad room vnum %ld.", num, pReset->arg3 );
        continue;
      }
      if ( count_obj_list(pObjIndex, pRoomIndex->first_content) > 0 )
      {
        obj = NULL;
        lastobj = NULL;
        break;
      }
      obj = create_object(pObjIndex, number_fuzzy(generate_itemlevel( pObjIndex)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj->cost = 0;
      obj_to_room(obj, pRoomIndex);
      lastobj = obj;
      break;
    
    case 'P':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'P': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( pReset->arg3 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
        {
          bug( "Reset_area: %d 'P': bad objto vnum %ld.", num, pReset->arg3 );
          continue;
        }
        if (
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             count_obj_list(pObjIndex, to_obj->first_content) > 0 )
        {
          obj = NULL;
          break;
        }
        lastobj = to_obj;
      }
      else
      {
        int iNest;
        
        if ( !lastobj )
          break;
        to_obj = lastobj;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
          if ( !(to_obj = to_obj->last_content) )
          {
            bug( "Reset_area: %d 'P': Invalid nesting obj %d.", num, pReset->arg1 );
            iNest = -1;
            break;
          }
        if ( iNest < 0 )
          continue;
      }
      obj = create_object(pObjIndex, number_fuzzy(UMAX(generate_itemlevel( pObjIndex),to_obj->level)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj_to_obj(obj, to_obj);
      break;
    
    case 'H':
      if ( pReset->arg1 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'H': bad objto vnum %ld.", num, pReset->arg1 );
          continue;
        }
        if ( 
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             IS_OBJ_STAT(to_obj, ITEM_HIDDEN) )
          break;
      }
      else
      {
        if ( !lastobj || !obj )
          break;
        to_obj = obj;
      }
      SET_BIT(to_obj->extra_flags, ITEM_HIDDEN);
      break;
    
    case 'B':
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int doornum;
        
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'B': door: bad room vnum %ld.", num, pReset->arg1 );
          continue;
        }
        doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK)
                >> BIT_RESET_DOOR_THRESHOLD;
        if ( !(pexit = get_exit(pRoomIndex, doornum)) )
          break;
        plc = &pexit->exit_info;
        }
        break;
      case BIT_RESET_ROOM:
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'B': room: bad room vnum %ld.", num, pReset->arg1 );
          continue;
        }
        plc = &pRoomIndex->room_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
          {
            bug( "Reset_area: %d 'B': object: bad objto vnum %ld.", num, pReset->arg1 );
            continue;
          }
          if ( !(to_obj = get_obj_type(pObjToIndex)) ||
                !to_obj->in_room  )
            continue;
        }
        else
        {
          if ( !lastobj || !obj )
            continue;
          to_obj = obj;
        }
        plc = &to_obj->extra_flags;
        break;
      case BIT_RESET_MOBILE:
        if ( !mob )
          continue;
        plc = &mob->affected_by;
        break;
      default:
        bug( "Reset_area: %d 'B': bad options %d.", num, pReset->arg2 );
        continue;
      }
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) )
        SET_BIT(*plc, pReset->arg3);
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
        TOGGLE_BIT(*plc, pReset->arg3);
      else
        REMOVE_BIT(*plc, pReset->arg3);
      break;
    
    case 'D':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'D': bad room vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pexit = get_exit(pRoomIndex, pReset->arg2)) )
        break;
      switch( pReset->arg3 )
      {
      case 0:
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        break;
      case 1:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      case 2:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        SET_BIT(   pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      }
      break;
    
    case 'R':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'R': bad room vnum %ld.", num, pReset->arg1 );
        continue;
      }
      randomize_exits(pRoomIndex, pReset->arg2-1);
      break;
    }
  }
  return;

*/

}

SHIP_DATA * make_mob_ship( PLANET_DATA *planet , int model )
{
    SHIP_DATA *ship;
    int shipreg = 0;
    int dIndex = 0;
    char filename[10];
    char shipname[MAX_STRING_LENGTH];
    
    if ( !planet || !planet->governed_by || !planet->starsystem )
       return NULL;
       
    /* mobships are given filenames < 0 and are not saved */   
       
    for ( ship = first_ship ; ship ; ship = ship->next )
        if ( shipreg > atoi( ship->filename ) )
             shipreg = atoi( ship->filename );

    shipreg--;
    sprintf( filename , "%d" , shipreg );
    
    CREATE( ship, SHIP_DATA, 1 );
    LINK( ship, first_ship, last_ship, next, prev );
    
    ship->filename = str_dup( filename ) ;
    
    ship->next_in_starsystem = NULL;
    ship->prev_in_starsystem =NULL;
    ship->next_in_room = NULL;
    ship->prev_in_room = NULL;
    ship->first_turret = NULL;
    ship->last_turret = NULL;
    ship->first_hanger = NULL;
    ship->last_hanger = NULL;
    ship->in_room = NULL;
    ship->starsystem = NULL;
    ship->home = STRALLOC( planet->starsystem->name );
    for ( dIndex = 0 ; dIndex < MAX_SHIP_ROOMS ; dIndex++ )
       ship->description[dIndex] = NULL;
    ship->owner = STRALLOC( planet->governed_by->name );
    ship->pilot = STRALLOC("");
    ship->copilot = STRALLOC("");
    ship->dest = NULL;
    ship->type = MOB_SHIP;
    ship->class = 0;
    ship->model = model;
    ship->hyperspeed = 0;
    ship->laserstate = LASER_READY;
    ship->missilestate = MISSILE_READY;
    ship->tractorbeam = 2;
    ship->hatchopen = FALSE;
    ship->autotrack = FALSE;
    ship->autospeed = FALSE;
    ship->location = 0;
    ship->lastdoc = 0;
    ship->shipyard = 0;
    ship->collision = 0;
    ship->target = NULL;
    ship->currjump = NULL;
    ship->chaff = 0;
    ship->maxchaff = 0;
    ship->chaff_released = FALSE;

    switch( ship->model )
    {
    case MOB_BATTLESHIP:
       ship->realspeed = 25;
       ship->maxmissiles = 50;
       ship->lasers = 10;
       ship->maxenergy = 30000;
       ship->maxshield = 1000;
       ship->maxhull = 30000;
       ship->manuever = 25;
       sprintf( shipname , "Battlecruiser m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    case MOB_CRUISER:
       ship->realspeed = 50;
       ship->maxmissiles = 30;
       ship->lasers = 8;
       ship->maxenergy = 15000;
       ship->maxshield = 350;
       ship->maxhull = 10000;
       ship->manuever = 50;
       sprintf( shipname , "Cruiser m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    case MOB_DESTROYER:
       ship->realspeed = 100;
       ship->maxmissiles = 20;
       ship->lasers = 6;
       ship->maxenergy = 7500;
       ship->maxshield = 200;
       ship->maxhull = 2000;
       ship->manuever = 100;
       ship->hyperspeed = 100;
       sprintf( shipname , "Destroyer m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    default:
       ship->realspeed = 255;
       ship->maxmissiles = 0;
       ship->lasers = 2;
       ship->maxenergy = 2500;
       ship->maxshield = 0;
       ship->maxhull = 100;
       ship->manuever = 100;
       sprintf( shipname , "Patrol craft m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;
    }
    
    ship->name = STRALLOC( shipname );
    ship->hull = ship->maxhull;
    ship->missiles = ship->maxmissiles;
    ship->energy = ship->maxenergy;
    ship->shield = 0;

    ship_to_starsystem(ship, starsystem_from_name(ship->home) );  
    ship->vx = planet->x + number_range( -2000 , 2000 );
    ship->vy = planet->y + number_range( -2000 , 2000 );
    ship->vz = planet->z + number_range( -2000 , 2000 );
    ship->shipstate = SHIP_READY;
    ship->autopilot = TRUE;
    ship->autorecharge = TRUE;
    ship->shield = ship->maxshield;
    
    return ship;
}
