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
 * - Quest module header                                                   *
 ***************************************************************************/

/*
 * Local Function Routines
 */
CHAR_DATA	*find_questmaster	args(( CHAR_DATA *ch, bool canBeImm ));
void		quest_update		args(( void ));
void		clear_questdata		args(( CHAR_DATA *ch, bool quit ));
void		generate_quest		args(( CHAR_DATA *ch, CHAR_DATA *questmaster ));
bool		valid_leveldiff		args(( CHAR_DATA *ch, CHAR_DATA *target ));
bool		quest_chance		args(( int percentage ));
void		check_quest_obj		args(( OBJ_DATA *obj ));

bool		load_qdata		args(( QUEST_DATA *qd ));
void		load_questdata		args(( void ));
void		write_qdata		args(( void ));
void		write_questdata		args(( void ));
void		remove_questdata	args(( bool qPrize, int nCount ));
void		fread_qdata		args(( QUEST_DATA *qd, FILE *fpin ));
QPRIZE_DATA	*fread_qprize		args(( FILE *fpin ));
QTARG_DATA	*fread_qtarg		args(( FILE *fpin ));


/*
 * Quest Data File
 */
#define QUEST_DIR		"../quest/"
#define QDATA_FILE		QUEST_DIR "qdata.dat"
#define QUESTDATA_FILE		QUEST_DIR "quest.dat"
