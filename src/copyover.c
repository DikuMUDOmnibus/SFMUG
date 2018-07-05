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
 * - Copyover module                                                       *
 ***************************************************************************/

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "mud.h"
#include "copyover.h"
#include "files.h"

#ifdef I3
   bool I3_is_connected( void );
#endif

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];
ROOM_INDEX_DATA *  room_index_hash         [MAX_KEY_HASH];

bool     write_to_descriptor     args( ( int desc, char *txt, int length ) );

/*
 * Save the world's objects and mobs in their current positions -- Scion
 */
void save_mobile( FILE *fp, CHAR_DATA *mob )
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill = NULL;

   if ( !IS_NPC( mob ) || !fp )
	return;
   fprintf( fp, "#MOBILE\n" );
   fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
   fprintf( fp, "Level   %d\n", mob->level );
   fprintf( fp, "Gold	%d\n", mob->gold );
   if ( mob->in_room ) 
   {
      if( xIS_SET( mob->act, ACT_SENTINEL ) )
      {
		  /* Sentinel mobs get stamped with a "home room" when they are created
		  by create_mobile(), so we need to save them in their home room regardless
		  of where they are right now, so they will go to their home room when they
		  enter the game from a reboot or copyover -- Scion */
	   fprintf( fp, "Room	%d\n", mob->home_vnum );
	} 
      else
	   fprintf( fp, "Room	%d\n", mob->in_room->vnum );
   }
   else
	fprintf( fp, "Room	%d\n", ROOM_VNUM_LIMBO );
#ifdef OVERLANDCODE
   fprintf( fp, "Coordinates  %d %d %d\n", mob->x, mob->y, mob->map );
#endif
   if ( QUICKMATCH( mob->name, mob->pIndexData->player_name) == 0 )
        fprintf( fp, "Name     %s~\n", mob->name );
   if ( QUICKMATCH( mob->short_descr, mob->pIndexData->short_descr) == 0 )
  	fprintf( fp, "Short	%s~\n", mob->short_descr );
   if ( QUICKMATCH( mob->long_descr, mob->pIndexData->long_descr) == 0 )
  	fprintf( fp, "Long	%s~\n", mob->long_descr );
   if ( QUICKMATCH( mob->description, mob->pIndexData->description) == 0 )
  	fprintf( fp, "Description %s~\n", mob->description );
   fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	mob->hit, mob->max_hit, mob->mana, mob->max_mana, mob->move, mob->max_move );
   fprintf( fp, "Position %d\n", mob->position );
   fprintf( fp, "Flags %s\n",   print_bitvector(&mob->act) );
   if ( !xIS_EMPTY( mob->affected_by ) )
      fprintf( fp, "AffectedBy   %s\n",	print_bitvector(&mob->affected_by) );

   for ( paf = mob->first_affect; paf; paf = paf->next )
   {
     	if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
	    continue;

	if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
	  fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
	    skill->name, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector) );
	else
	  fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
	    paf->type, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector) );
   }

   de_equip_char( mob );

   if ( mob->first_carrying )
	fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY, TRUE );

   re_equip_char( mob );

   fprintf( fp, "EndMobile\n\n" );
   return;
}

void save_world( CHAR_DATA *ch )
{
   FILE *mobfp;
   FILE *objfp;
   int mobfile=0;
   char filename[256];
   CHAR_DATA *rch;
   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;

   log_string( "Preserving world state...." );

   sprintf( filename, "%s%s", SYSTEM_DIR, MOB_FILE );
   if ( ( mobfp = fopen( filename, "w" ) ) == NULL )
   {
	bug( "save_world: fopen mob file", 0 );
	perror( filename );
   }
   else
	mobfile++;
	
   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
	for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
	{
	   if( pRoomIndex )
	   {
	      if( !pRoomIndex->first_content                      /* Skip room if nothing in it */
	        || IS_SET( pRoomIndex->room_flags, ROOM_CLANSTOREROOM ) /* These rooms save on their own */
	        )
	         continue;

	      sprintf( filename, "%s%d", COPYOVER_DIR, pRoomIndex->vnum );
	      if( ( objfp = fopen( filename, "w" ) ) == NULL )
	      {
		   bug( "save_world: fopen %d", pRoomIndex->vnum );
		   perror( filename );
		   continue;
	      }
            fwrite_obj( NULL, pRoomIndex->last_content, objfp, 0, OS_CARRY, TRUE );
	      fprintf( objfp, "#END\n" );
	      FCLOSE( objfp );
	   }
	}
   }
	
   if( mobfile )
   {
	for( rch = first_char; rch; rch = rch->next )
      {
	   if( !IS_NPC(rch) || rch == supermob || xIS_SET( rch->act, ACT_PROTOTYPE ) || xIS_SET( rch->act, ACT_PET ) )
	      continue;
	   else
		save_mobile( mobfp, rch );
	}
	fprintf( mobfp, "#END\n" );
	FCLOSE( mobfp );
   }
   return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

CHAR_DATA *load_mobile( FILE *fp )
{
   CHAR_DATA *mob = NULL;
   char *word;
   bool fMatch;
   int inroom = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;

  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
  if ( !strcmp( word, "Vnum" ) )
  {
    int vnum;
    
    vnum = fread_number( fp );
    if( get_mob_index( vnum ) == NULL )
    {
	bug( "fread_mobile: Unable to create mobile for vnum %d", vnum );
	return NULL;
    }
    mob = create_mobile( get_mob_index( vnum ) );
    if ( !mob )
    {
	for ( ; ; ) 
      {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai 
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
	}
	bug( "Fread_mobile: No index data for vnum %d", vnum );
	return NULL;
	}
  }
  else
  {
	for ( ; ; ) 
      {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai 
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
      }
	extract_char( mob, TRUE );
	bug( "Fread_mobile: Vnum not found", 0 );
	return NULL;
  }
  for( ; ; ) 
  {
      word   = feof( fp ) ? "EndMobile" : fread_word( fp );
      fMatch = FALSE; 
      switch ( UPPER(word[0]) ) 
	{
	   case '*':
           fMatch = TRUE;
           fread_to_eol( fp );
           break;  
	   case '#':
		if ( !strcmp( word, "#OBJECT" ) )
			fread_obj( mob, fp, OS_CARRY );
         case 'A':
	     if ( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
	     {
		  AFFECT_DATA *paf;

		  CREATE( paf, AFFECT_DATA, 1 );
		  if ( !strcmp( word, "Affect" ) )
		  {
		    paf->type	= fread_number( fp );
		  }
		  else
		  {
		    int sn;
		    char *sname = fread_word(fp);

		    if ( (sn=skill_lookup(sname)) < 0 )
		    {
			if ( (sn=herb_lookup(sname)) < 0 )
			    bug( "load_mobile: unknown skill.", 0 );
			else
			    sn += TYPE_HERB;
		    }
		    paf->type = sn;
		  }

		  paf->duration	= fread_number( fp );
		  paf->modifier	= fread_number( fp );
		  paf->location	= fread_number( fp );
		  if ( paf->location == APPLY_WEAPONSPELL
		  ||   paf->location == APPLY_WEARSPELL
		  ||   paf->location == APPLY_REMOVESPELL
		  ||   paf->location == APPLY_STRIPSN
		  ||   paf->location == APPLY_RECURRINGSPELL )
		    paf->modifier	= slot_lookup( paf->modifier );
		  paf->bitvector	= fread_bitvector( fp );
		  LINK(paf, mob->first_affect, mob->last_affect, next, prev );
		  fMatch = TRUE;
		  break;
	      }
	      KEY( "AffectedBy",	mob->affected_by,	fread_bitvector( fp ) );
	    	break;
#ifdef OVERLANDCODE
	   case 'C':
	      if ( !str_cmp( word, "Coordinates" ) )
	      {
		  mob->x = fread_number( fp );
		  mob->y = fread_number( fp );
		  mob->map	 = fread_number( fp );

		  fMatch = TRUE;
		  break;
	      }
		break;
#endif
	   case 'D':
		KEY( "Description", mob->description, fread_string(fp));
		break;
	   case 'E':
		if ( !strcmp( word, "EndMobile" ) )
		{
		   if ( inroom == 0 )
			inroom = ROOM_VNUM_LIMBO;
		   pRoomIndex = get_room_index( inroom );
		   if ( !pRoomIndex )
			pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
		   char_to_room( mob, pRoomIndex );
		   return mob;
		} 
		if( !str_cmp( word, "End" ) ) /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
		   fMatch = TRUE; /* Trick the system into thinking it matched something */
		break;
 	   case 'F':
		KEY( "Flags", mob->act, fread_bitvector( fp ) );
         case 'G':
		KEY( "Gold",	mob->gold,	fread_number( fp ) );
	 	break;
	   case 'H':
	      if ( !strcmp( word, "HpManaMove" ) )
	      {
		   mob->hit		= fread_number( fp );
		   mob->max_hit	= fread_number( fp );
		   mob->mana	= fread_number( fp );
		   mob->max_mana	= fread_number( fp );
		   mob->move	= fread_number( fp );
		   mob->max_move	= fread_number( fp );

		   if ( mob->max_move <= 0 )
		      mob->max_move = 150;

		   fMatch = TRUE;
		   break;
	      }
		break;
	   case 'L':
		KEY( "Long", mob->long_descr, fread_string( fp ) );
	      KEY( "Level", mob->level,	fread_number( fp ) );
		break;
	   case 'N':
		KEY( "Name", mob->name, fread_string( fp ) );
		break;
	   case 'P':
		KEY( "Position", mob->position, fread_number( fp ) );
		break;
	   case 'R':
		KEY( "Room",  inroom, fread_number(fp));
		break;
	   case 'S':
		KEY( "Short", mob->short_descr, fread_string(fp));
		break;
	}
	if ( !fMatch && str_cmp( word, "End" ) )
	{
	   bug ( "load_mobile: no match.", 0 );
	   bug ( word, 0 );
	}
  }
  return NULL;
}

void read_obj_file( char *dirname, char *filename )
{
   ROOM_INDEX_DATA *room;
   FILE *fp;
   char fname[MSL];
   int vnum;

   vnum = atoi( filename );

   if ( ( room = get_room_index( vnum ) ) == NULL )
   {
	bug( "read_obj_file: ARGH! Missing room index for %d!", vnum );
	return;
   }
   
   sprintf( fname, "%s%s", dirname, filename );
   if ( ( fp = fopen( fname, "r" ) ) != NULL )
   {
	sh_int iNest;
	bool found;
	OBJ_DATA *tobj, *tobj_next;

	rset_supermob( room );
      for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	  rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	   char letter;
	   char *word;

	   letter = fread_letter( fp );
	   if ( letter == '*' )
	   {
		fread_to_eol( fp );
		continue;
	   }

	   if ( letter != '#' )
	   {
		bug( "read_obj_file: # not found.", 0 );
		bug( filename, 0 );
		break;
	   }

	   word = fread_word( fp );
	   if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
		fread_obj( supermob, fp, OS_CARRY );
	   else
		if ( !str_cmp( word, "END"    ) )	/* Done		*/
		  break;
	      else
		{
		    bug( "read_obj_file: bad section.", 0 );
		    bug( filename, 0 );
		    break;
		}
	}
	FCLOSE( fp );
        unlink( fname );
	for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	{
	   tobj_next = tobj->next_content;
#ifdef OVERLANDCODE
	   if( IS_OBJ_STAT( tobj, ITEM_ONMAP ) )
	   {
		SET_ACT_FLAG( supermob, ACT_ONMAP );
		supermob->map = tobj->map;
		supermob->x = tobj->x;
		supermob->y = tobj->y;
	   }
#endif
         obj_from_char( tobj );
#ifndef OVERLANDCODE
	   obj_to_room( tobj, room );
#else
	   obj_to_room( tobj, room, supermob );
	   REMOVE_ACT_FLAG( supermob, ACT_ONMAP );
	   supermob->map = -1;
	   supermob->x = -1;
	   supermob->y = -1;
#endif
	}
	release_supermob();
   }
   else
	log_string( "Cannot open obj file" );

   return;
}

void load_obj_files( void )
{
   DIR *dp;
   struct dirent *dentry;
   char directory_name[100];

   log_string( "World state: loading objs" );
   sprintf( directory_name, "%s", COPYOVER_DIR );
   dp = opendir( directory_name );
   dentry = readdir( dp );
   while ( dentry )
   {
      if ( dentry->d_name[0] != '.' )
      {
	   read_obj_file( directory_name, dentry->d_name );
      }
      dentry = readdir( dp );
   }
   closedir( dp );
   return;
}

void load_world( CHAR_DATA *ch )
{
   FILE *mobfp;
   char file1[256];
   char *word;
   int done = 0;
   bool mobfile = FALSE;

   sprintf( file1, "%s%s", SYSTEM_DIR, MOB_FILE );
   if( ( mobfp = fopen( file1, "r" ) ) == NULL )
   {
	bug( "load_world: fopen mob file", 0 );
	perror( file1 );
   } 
   else
	mobfile = TRUE;

   if( mobfile )
   {
	log_string( "World state: loading mobs" );
	while( done == 0 )
      {
	   if( feof( mobfp ) )
	      done++;
	   else 
	   {
		word = fread_word( mobfp );
		if( strcmp( word, "#END" ) )
		   load_mobile( mobfp );
		else
		   done++;
	   }	
	}
	FCLOSE( mobfp );
   }
	
   load_obj_files();

   /* Once loaded, the data needs to be purged in the event it causes a crash so that it won't try to reload */
   unlink( file1 );
   return;
}

/* Used by copyover code */
void save_areas( void )
{
   AREA_DATA   *tarea;
   char         filename[256];

   for( tarea = first_build; tarea; tarea = tarea->next )
   {
      if( !IS_SET( tarea->status, AREA_LOADED ) )
         continue;
      sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
      fold_area( tarea, filename, FALSE );
   }
   return;
}

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
void do_copyover( CHAR_DATA *ch, char *argument )
{
    FILE *fp;
    DESCRIPTOR_DATA *d, *de_next;
    char buf [100], buf2[100], buf3[100];
    extern int control;

    sprintf( log_buf, "Copyover initiated by %s.", ch->name );
    log_string( log_buf );

    fp = fopen( COPYOVER_FILE, "w" );
    
    if( !fp )
    {
      send_to_char( "Copyover file not writeable, aborted.\n\r", ch );
	sprintf( log_buf, "Could not write to copyover file: %s. Copyover aborted.", COPYOVER_FILE );
	log_string( log_buf );
      perror( "do_copyover:fopen" );
      return;
    }
    
    /* Consider changing all loaded prototype areas here, if you use OLC */
    /*
    log_string( "Saving modified area files..." );
    save_areas();
    */

    /* And this one here will save the status of all objects and mobs in the game.
     * This really should ONLY ever be used here. The less we do stuff like this the better.
     */
    save_world( ch );

    log_string( "Saving player files and connection states...." );
    if( ch && ch->desc )
        write_to_descriptor( ch->desc->descriptor, "\e[0m", 0 );
    sprintf( buf, "\n\rThe flow of time is halted momentarily as the world is reshaped!\n\r" );
    /* For each playing descriptor, save its state */
    for( d = first_descriptor; d ; d = de_next )
    {
        CHAR_DATA *och = CH(d);
        de_next = d->next; /* We delete from the list , so need to save this */
        if( !d->character || d->connected < CON_PLAYING ) /* drop those logging on */
        {
            write_to_descriptor( d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0 );
            close_socket( d, FALSE ); /* throw'em out */
        }
        else
        {
            fprintf( fp, "%d %d %d %d %d %s %s\n",
                     d->descriptor,
                     0,
                     och->in_room->vnum, d->port, d->idle, och->name, d->host );
            /* One of two places this gets changed */
            och->pcdata->copyover = TRUE;
            save_char_obj( och );
            write_to_descriptor( d->descriptor, buf, 0 );
        }
    }

    fprintf( fp, "0 0 0 0 %d maxp maxp\n", sysdata.maxplayers );
    fprintf( fp, "-1\n" );
    FCLOSE( fp );

#ifdef I3
   if( I3_is_connected() )
   {
	I3_savechanlist();
	I3_savemudlist();
   }
#endif

   /* added this in case there's a need to debug the contents of the various files */
   if( argument && !str_cmp( argument, "debug" ) )
   {
	log_string( "Copyover debug - Aborting before execl" );
	return;
   }

    log_string( "Executing copyover...." );
    /* Close reserve and other always-open files and release other resources */
    FCLOSE( fpReserve );
    FCLOSE( fpLOG );
    
    /* exec - descriptors are inherited */
    sprintf( buf,  "%d", port );
    sprintf( buf2, "%d", control );
#ifdef I3
    sprintf( buf3, "%d", I3_socket );
#else
    strcpy ( buf3, "-1" );
#endif

    /* Uncomment this bfd_close line if you've installed the BFD snippet, you'll need it. */
    // bfd_close( abfd );
    execl( EXE_FILE, "13C", buf, "copyover",  buf2, buf3, (char *)NULL );
    
    /* Failed - sucessful exec will not return */
    perror( "do_copyover: execl" );

    /* Here you might want to reopen fpReserve */
    /* Since I'm a neophyte type guy, I'll assume this is a good idea and cut and past from main()  */
    
    if( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }
    if( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }
    log_string( "Copyover execution failed!!" );
    send_to_char( "Copyover FAILED!\n\r", ch );
}

/* Recover from a copyover - load players */
void copyover_recover()
{
    DESCRIPTOR_DATA *d = NULL;
    FILE *fp;
    char name[100];
    char host[MSL];
    int desc, dcompress, room, dport, idle, maxp = 0;
    bool fOld;
    
    fp = fopen( COPYOVER_FILE, "r" );
    
    if (!fp) /* there are some descriptors open which will hang forever then ? */
    {
        perror( "copyover_recover: fopen" );
        log_string( "Copyover file not found. Exitting.\n\r" );
        exit( 1 );
    }
    
    unlink( COPYOVER_FILE ); /* In case something crashes - doesn't prevent reading */
    for( ; ; )
    {
        d = NULL;
        
        fscanf( fp, "%d %d %d %d %d %s %s\n", &desc, &dcompress, &room, &dport, &idle, name, host );

        if (desc == -1 || feof(fp) )
            break;

	if( !str_cmp( name, "maxp" ) || !str_cmp( host, "maxp" ) )
	{
	    maxp = idle;
	    continue;
	}

        /* Write something, and check if it goes error-free */
	if ( !write_to_descriptor( desc, "\n\rThe ether swirls in chaos.\n\r", 0 ) )
        {
            close( desc ); /* nope */
            continue;
        }
        
        CREATE( d, DESCRIPTOR_DATA, 1 );

        d->next		= NULL;
        d->descriptor	= desc;
        d->connected	= CON_GET_NAME;
        d->outsize	= 2000;
        d->idle		= 0;
        d->lines		= 0;
        d->scrlen		= 24;
        d->newstate	= 0;
        d->prevcolor	= 0x07;

        CREATE( d->outbuf, char, d->outsize );

        d->user = STRALLOC( "Unknown" );
        d->host = STRALLOC( host );
        d->port = dport;
        d->idle = idle;
        LINK( d, first_descriptor, last_descriptor, next, prev );
        d->connected = CON_COPYOVER_RECOVER; /* negative so close_socket will cut them off */

        /* Now, find the pfile */
        fOld = load_char_obj( d, name, FALSE, TRUE );
        
        if( !fOld ) /* Player file not found?! */
        {
            write_to_descriptor( d->descriptor, "\n\rSomehow, your character was lost during copyover. Contact the immortals ASAP.\n\r", 0 );
            close_socket( d, FALSE );
        }
        else /* ok! */
        {
            write_to_descriptor( d->descriptor, "\n\rTime resumes its normal flow.\n\r", 0 );
            d->character->in_room = get_room_index( room );
            if( !d->character->in_room )
                d->character->in_room = get_room_index( ROOM_VNUM_TEMPLE );
            
            /* Insert in the char_list */
            add_char( d->character );
            
            char_to_room( d->character, d->character->in_room );
	      act( AT_MAGIC, "A puff of ethereal smoke dissipates around you!", d->character, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n appears in a puff of ethereal smoke!", d->character, NULL, NULL, TO_ROOM );
            d->connected = CON_PLAYING;
            if ( ++num_descriptors > sysdata.maxplayers )
	         sysdata.maxplayers = num_descriptors;
        }
    }
    FCLOSE( fp );
    if( maxp > sysdata.maxplayers )
	sysdata.maxplayers = maxp;
    log_string( "Copyover recovery complete." );
    return;
}
