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
 *                      More Skills Moduals                                 *
 ****************************************************************************/


#include <math.h> 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void    add_reinforcements  args( ( CHAR_DATA *ch ) );
ch_ret  one_hit             args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int     xp_compute                ( CHAR_DATA *ch , CHAR_DATA *victim );
int ris_save( CHAR_DATA *ch, int chance, int ris );
CHAR_DATA *get_char_room_mp( CHAR_DATA *ch, char *argument );
void  clear_roomtype( ROOM_INDEX_DATA * room );

extern int      top_affect;
extern int top_r_vnum;

const	char *	sector_name	[SECT_MAX]	=
{
    "inside", "city", "field", "forest", "hills", "mountain", "water swim", "water noswim", 
    "underwater", "air", "desert", "unknown", "ocean floor", "underground",
    "scrub", "rocky", "savanna", "tundra", "glacial", "rainforest", "jungle", 
    "swamp", "wetlands", "brush", "steppe", "farmland", "volcanic"
};

/* I deleted alot of things out of here from SWR2.0 -Lews */

void do_reinforcements( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance, credits;
    
    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;
    	
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYour reinforcements are already on the way.\n\r", ch );
    	            return;
    	        }
    	        
    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
    	            return;
    	        }    
    	        
    	        if ( ch->gold < 5000 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough credits to send for reinforcements.\n\r" );
    	            return;
    	        }    
    	        
    	        chance = (int) (ch->pcdata->learned[gsn_reinforcements]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_reinforcements , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for reinforcements but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_reinforcements );
    	   	return;	
    	
    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    send_to_char( "&GYour reinforcements are on the way.\n\r", ch);
    credits = 5000;
    ch_printf( ch, "It cost you %d credits.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );
             
    learn_from_success( ch, gsn_reinforcements );
    learn_from_success( ch, gsn_reinforcements );
    
    
    ch->backup_mob = MOB_VNUM_SOLDIER;

    ch->backup_wait = 1;
    
}

void do_postguard( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance, credits;
    
    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;
    	
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYou already have backup coming.\n\r", ch );
    	            return;
    	        }
    	        
    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
    	            return;
    	        }    
                
                if ( !ch->in_room || !ch->in_room->area || 
                ( ch->in_room->area->planet && ch->in_room->area->planet->governed_by != ch->pcdata->clan ) )
    	        {
    	            send_to_char( "&RYou cannot post guards on enemy planets. Try calling for reinforcements instead.\n\r", ch );
    	            return;
    	        }    
                    	        
    	        if ( ch->gold < 5000 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough credits.\n\r", ch );
    	            return;
    	        }    
    	        
    	        chance = (int) (ch->pcdata->learned[gsn_postguard]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_postguard , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for a guard but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_postguard );
    	   	return;	
    	
    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    send_to_char( "&GYour guard is on the way.\n\r", ch);
    
    credits = 5000;
    ch_printf( ch, "It cost you %d credits.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_postguard );
    learn_from_success( ch, gsn_postguard );
    
    ch->backup_mob = MOB_VNUM_GUARD;

    ch->backup_wait = 1;
    add_reinforcements( ch );

}

void add_reinforcements( CHAR_DATA *ch )
{
     MOB_INDEX_DATA  * pMobIndex;
     OBJ_DATA        * blaster;
     OBJ_INDEX_DATA  * pObjIndex;
       
     
     if ( !ch->in_room )
        return;
     
     if ( ( pMobIndex = get_mob_index( ch->backup_mob ) ) == NULL )
        return;         

     if ( ch->backup_mob == MOB_VNUM_SOLDIER       )  
     {
        CHAR_DATA * mob[3];
        int         mob_cnt;
        
        send_to_char( "Your reinforcements have arrived.\n\r", ch );
        for ( mob_cnt = 0 ; mob_cnt < 3 ; mob_cnt++ )
        {
            mob[mob_cnt] = create_mobile( pMobIndex );
            if ( !mob[mob_cnt] )
                return;
            char_to_room( mob[mob_cnt], ch->in_room );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[mob_cnt], TO_ROOM );
            mob[mob_cnt]->level = 50;
            mob[mob_cnt]->hit = 100;
            mob[mob_cnt]->max_hit = 100;
            mob[mob_cnt]->armor = 50;
            mob[mob_cnt]->damroll = 0;
            mob[mob_cnt]->hitroll = 10;
            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
            {
                 blaster = create_object( pObjIndex, mob[mob_cnt]->level );
                 obj_to_char( blaster, mob[mob_cnt] );
                 equip_char( mob[mob_cnt], blaster, WEAR_WIELD );                        
            } 
            
            if ( mob[mob_cnt]->master )
	       stop_follower( mob[mob_cnt] );
	    add_follower( mob[mob_cnt], ch );
            xSET_BIT( mob[mob_cnt]->affected_by, AFF_CHARM );

            /* no blasters -Lews
            do_setblaster( mob[mob_cnt] , "full" );
             */
            if ( ch->pcdata && ch->pcdata->clan )   
               mob[mob_cnt]->mob_clan = ch->pcdata->clan;
        }
     }
     else
     {
        CHAR_DATA *mob;
        
        mob = create_mobile( pMobIndex );
        char_to_room( mob, ch->in_room );
        if ( ch->pcdata && ch->pcdata->clan )
        {
          char tmpbuf[MAX_STRING_LENGTH];
        
          sprintf( tmpbuf , "A guard stands at attention. (%s)\n\r" , ch->pcdata->clan->name );
          STRFREE( mob->long_descr );
          mob->long_descr = STRALLOC( tmpbuf );
        }
        act( AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM );
        send_to_char( "Your guard has arrived.\n\r", ch );
        mob->level = 75;
        mob->hit = 200;
        mob->max_hit = 200;
        mob->armor = 0;
        mob->damroll = 5;
        mob->hitroll = 20;
        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
        {
            blaster = create_object( pObjIndex, mob->level );
            obj_to_char( blaster, mob );
            equip_char( mob, blaster, WEAR_WIELD );                        
        }
        /* No blasters -Lews
        do_setblaster( mob , "full" );
         */
        if ( ch->pcdata && ch->pcdata->clan )   
               mob->mob_clan = ch->pcdata->clan;
     }
           ch->backup_wait = 0; /* no need to keep waitng -Lews 10/01 */                    
}

void do_pickshiplock( CHAR_DATA *ch, char *argument )
{
   do_pick( ch, argument );
}

void do_hijack( CHAR_DATA *ch, char *argument )
{
    int chance; 
    SHIP_DATA *ship;
    char buf[MAX_STRING_LENGTH];
    bool uhoh = FALSE;
    CHAR_DATA *guard;
    ROOM_INDEX_DATA *room;
            
    	        if ( (ship = ship_from_cockpit(ch->in_room)) == NULL )  
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( ship->class > SPACE_STATION )
    	        {
    	            send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( check_pilot( ch , ship ) )
    	        {
    	            send_to_char("&RWhat would be the point of that!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( ship->type == MOB_SHIP && get_trust(ch) < 102 )
    	        {
    	            send_to_char("&RThis ship isn't pilotable by mortals at this point in time...\n\r",ch);
    	            return;
    	        }
    	        
                if  ( ship->class == SPACE_STATION )
                {
                   send_to_char( "You can't do that here.\n\r" , ch );
                   return;
                }   
    
    	        if ( ship->lastdoc != ship->location )
                {
                     send_to_char("&rYou don't seem to be docked right now.\n\r",ch);
                     return;
                }
    
    	        if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    	        {
    	            send_to_char("The ship is not docked right now.\n\r",ch);
    	            return;
    	        }
                
                if ( ship->shipstate == SHIP_DISABLED )
    	        {
    	            send_to_char("The ships drive is disabled .\n\r",ch);
    	            return;
    	        }
                
                for ( room = ship->first_room ; room ; room = room->next_in_ship )
		{
		   for ( guard = room->first_person; guard ; guard = guard->next_in_room )
		      if ( IS_NPC(guard) && guard->pIndexData && guard->pIndexData->vnum == MOB_VNUM_SHIP_GUARD 
		      && guard->position > POS_SLEEPING && !guard->fighting )
                      {
                         start_hating( guard, ch );
                         start_hunting( guard , ch );
                         uhoh = TRUE;
                      }   
		}
		
                if ( uhoh )
    	        {
    	            send_to_char("Uh oh....\n\r",ch);
    	            return;
    	        }
		                
                chance = IS_NPC(ch) ? 0
	                 : (int)  (ch->pcdata->learned[gsn_hijack]) ;
                if ( number_percent( ) > chance )
    		{  
    		    send_to_char("You fail to figure out the correct launch code.\n\r",ch);
    	            return;
                }
                
                chance = IS_NPC(ch) ? 0
	                 : (int)  (ch->pcdata->learned[gsn_spacecraft]) ;
                if ( number_percent( ) < chance )
    		{  
                
    		   if (ship->hatchopen)
    		   {
    		     ship->hatchopen = FALSE;
    		     sprintf( buf , "The hatch on %s closes." , ship->name);  
       	             echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
       	             echo_to_room( AT_YELLOW , ship->entrance , "The hatch slides shut." );
       	           }
    		   set_char_color( AT_GREEN, ch );
    		   send_to_char( "Launch sequence initiated.\n\r", ch);
    		   act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch,
		        NULL, argument , TO_ROOM );
		   echo_to_ship( AT_YELLOW , ship , "The ship hums as it lifts off the ground.");
    		   sprintf( buf, "%s begins to launch.", ship->name );
    		   echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    		   ship->shipstate = SHIP_LAUNCH;
    		   ship->currspeed = ship->realspeed;
                   learn_from_success( ch, gsn_spacecraft );
                   learn_from_success( ch, gsn_hijack );
                   sprintf( buf, "%s has been hijacked!", ship->name );
    		   echo_to_all( AT_RED , buf, 0 );
    		   
                   return;   	   	
                }
                set_char_color( AT_RED, ch );
	        send_to_char("You fail to work the controls properly!\n\r",ch);
    	   	return;	
    	
}


void do_propeganda ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;
    int percent = 0;
    
   if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
       send_to_char( "What would be the point of that.\n\r", ch );
       return;
   }
    
    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Spread propeganda to who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( !xIS_SET( victim->act , ACT_CITIZEN ) )
    {
        send_to_char( "I don't think diplomacy will work on them...\n\r" , ch );
        return;    
    }
    
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }
    
    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }
    

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "In your dreams or what?\n\r" , ch );
        return;
    }
    
    if ( victim->position <= POS_SLEEPING )
    {
        send_to_char( "You might want to wake them first...\n\r" , ch );
        return;
    }

    clan = ch->pcdata->clan;
       
    planet = ch->in_room->area->planet;
        
    sprintf( buf, ", and the evils of %s" , planet->governed_by ? planet->governed_by->name : "their current leaders" );
    ch_printf( ch, "You speak to them about the benifits of the %s%s.\n\r", ch->pcdata->clan->name,
        planet->governed_by == clan ? "" : buf );
    act( AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n tells $N about their organization.\n\r",  ch, NULL, victim, TO_NOTVICT );

    WAIT_STATE( ch, skill_table[gsn_propeganda]->beats );

    if ( percent - get_curr_cha(ch) + victim->level > ch->pcdata->learned[gsn_propeganda]  ) 
    {

        if ( planet->governed_by != clan )
	{
	  sprintf( buf, "%s is a traitor!" , ch->name);
	  do_yell( victim, buf );
          global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}
	
	return;
    }
    
    if ( planet->governed_by == clan )
    { 
       planet->pop_support += 1;
       send_to_char( "Popular support for your organization increases.\n\r", ch );
    }     
    else
    {
       planet->pop_support -= .1;
       send_to_char( "Popular support for the current government decreases.\n\r", ch );
    }
    
    if ( number_percent() == 23 )
    {
	send_to_char( "You feel a bit more charming than you used to...\n\r", ch );
        ch->perm_cha++;
        ch->perm_cha = UMIN( ch->perm_cha , 25 );
    }

    learn_from_success( ch, gsn_propeganda );
        
    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
    if ( planet->pop_support < -100 )
        planet->pop_support = -100;

}

void  clear_roomtype( ROOM_INDEX_DATA * location )
{
      if ( location->area && location->area->planet )
      {
          if ( location->sector_type <= SECT_CITY )
              location->area->planet->citysize--;
          else if ( location->sector_type == SECT_FARMLAND )
              location->area->planet->farmland--;
          else if ( location->sector_type != SECT_DUNNO )
              location->area->planet->wilderness--;

          if ( IS_SET( location->room_flags , ROOM_BARRACKS ) )
              location->area->planet->barracks--;
          if ( IS_SET( location->room_flags , ROOM_CONTROL ) )
              location->area->planet->controls--;
              
      }

      REMOVE_BIT( location->room_flags , ROOM_NO_MOB );
      REMOVE_BIT( location->room_flags , ROOM_SAFE );
      REMOVE_BIT( location->room_flags , ROOM_CAN_LAND );
      REMOVE_BIT( location->room_flags , ROOM_SHIPYARD );
      REMOVE_BIT( location->room_flags , ROOM_DARK );
      REMOVE_BIT( location->room_flags , ROOM_CONTROL );
      REMOVE_BIT( location->room_flags , ROOM_BARRACKS );
      REMOVE_BIT( location->room_flags , ROOM_GARAGE );
}


