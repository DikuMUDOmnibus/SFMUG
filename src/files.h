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
 * - Files header                                                          *
 ***************************************************************************/

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files                 */
#define BACKUP_DIR	"../player/backup/"	/* Backup Player files      */
#define GOD_DIR		"../gods/"	/* God Info Dir                 */
#define BOARD_DIR	"../boards/"	/* Board data dir               */
#define CLAN_DIR	"../clans/"	/* Clan data dir                */
#define SHIP_DIR        "../spacecraft/"
#define PROTOTYPE_DIR   "../prototypes/"
#define SPACE_DIR       "../planets/"    
#define PLANET_DIR      "../planets/"    
#define GUARD_DIR       "../planets/"    
#define COUNCIL_DIR	"../councils/"	/* Council data dir             */
#define DEITY_DIR	"../deity/"	/* Deity data dir               */
#define BUILD_DIR	"../building/"	/* Online building save dir     */
#define SYSTEM_DIR	"../system/"	/* Main system files            */
#define PROG_DIR	"mudprogs/"	/* MUDProg files                */
#define CORPSE_DIR	"../corpses/"	/* Corpses                      */

#ifdef WIN32
  #define NULL_FILE	"nul"		/* To reserve one stream        */
#else
  #define NULL_FILE	"/dev/null"	/* To reserve one stream        */
#endif

#define CLASS_DIR	"../classes/"	/* Classes                      */
#define WATCH_DIR	"../watch/"	/* Imm watch files --Gorog      */
/*
 * The watch directory contains a maximum of one file for each immortal
 * that contains output from "player watches". The name of each file
 * in this directory is the name of the immortal who requested the watch
 */

#define AREA_LIST	"area.lst"      /* List of areas                */
#define WATCH_LIST	"watch.lst"     /* List of watches              */
#define BAN_LIST	"ban.lst"       /* List of bans                 */
#define RESERVED_LIST	"reserved.lst"  /* List of reserved names       */
#define CLAN_LIST	"clan.lst"      /* List of clans                */
#define SHIP_LIST       "ship.lst"
#define PROTOTYPE_LIST  "prototype.lst"
#define PLANET_LIST      "planet.lst"
#define SPACE_LIST      "space.lst"
#define COUNCIL_LIST	"council.lst"   /* List of councils             */
#define GUILD_LIST	"guild.lst"     /* List of guilds               */
#define GOD_LIST	"gods.lst"      /* List of gods                 */
#define DEITY_LIST	"deity.lst"     /* List of deities              */
#define CLASS_LIST	"class.lst"     /* List of classes              */
#define RACE_LIST	"race.lst"      /* List of races                */

#define MORPH_FILE	"morph.dat"     /* For morph data */
#define BOARD_FILE	"boards.txt"            /* For bulletin boards   */
#define SHUTDOWN_FILE	"shutdown.txt"          /* For 'shutdown'        */
#define IMM_HOST_FILE	SYSTEM_DIR "immortal.host" /* For stoping hackers */

#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"     /* Boot up error file  */
#define BUG_FILE	SYSTEM_DIR "bugs.txt"     /* For bug( )          */
#define PBUG_FILE	SYSTEM_DIR "pbugs.txt"    /* For 'bug' command   */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"    /* For 'idea'          */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"    /* For 'typo'          */
#define FIXED_FILE	SYSTEM_DIR "fixed.txt"    /* For 'fixed' command */
#define LOG_FILE	SYSTEM_DIR "log.txt"      /* For talking in logged rooms */
#define MOBLOG_FILE	SYSTEM_DIR "moblog.txt"   /* For mplog messages  */
#define PLEVEL_FILE	SYSTEM_DIR "plevel.txt"   /* Char level info */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"      /* Wizlist             */
#define SKILL_FILE	SYSTEM_DIR "skills.dat"   /* Skill table         */
#define HERB_FILE       SYSTEM_DIR "herbs.dat"    /* Herb table          */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat"  /* Tongue tables       */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"  /* Socials             */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands            */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"    /* How many people are on
                                                     every half hour - trying to
                                                     determine best reboot time */
#define ECONOMY_FILE	SYSTEM_DIR "economy.txt"  /* Gold looted, value of
                                                     used potions/pills  */
#define PROJECTS_FILE	SYSTEM_DIR "projects.txt" /* For projects        */
#define COLOR_FILE	SYSTEM_DIR "colors.dat"   /* User-definable color*/
#define TEMP_FILE	SYSTEM_DIR "charsave.tmp" /* More char save protect */
#define CLASSDIR	"../classes/"
#define RACEDIR		"../races/"
