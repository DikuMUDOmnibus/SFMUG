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
 * - Ban module header                                                     *
 ***************************************************************************/

typedef struct  ban_data                BAN_DATA;

/*
 * Ban Types --- Shaddai
 */
typedef enum
{
  BAN_WARN = -1, BAN_SITE = 1, BAN_CLASS, BAN_RACE
} ban_types;

/*
 * Site ban structure.
 */
struct  ban_data
{
    BAN_DATA *next;
    BAN_DATA *prev;
    char     *name;     /* Name of site/class/race banned */
    char     *user;     /* Name of user from site */
    char     *note;     /* Why it was banned */
    char     *ban_by;   /* Who banned this site */
    char     *ban_time; /* Time it was banned */
    int      flag;      /* Class or Race number */
    int      unban_date;/* When ban expires */
    sh_int   duration;  /* How long it is banned for */
    sh_int   level;     /* Level that is banned */
    bool     warn;      /* Echo on warn channel */
    bool     prefix;    /* Use of *site */
    bool     suffix;    /* Use of site* */
};

extern          BAN_DATA          *     first_ban;
extern          BAN_DATA          *     last_ban;
extern          BAN_DATA          *     first_ban_class;
extern          BAN_DATA          *     last_ban_class;
extern          BAN_DATA          *     first_ban_race;
extern          BAN_DATA          *     last_ban_race;
