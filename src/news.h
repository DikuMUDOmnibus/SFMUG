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
 * - News module header                                                    *
 ***************************************************************************/

/*
 * Version - 1    - Original code. Simplistic.
 * Version - 2    - Coverted to use a linked list, and allow news removal.
 * Version - 3    - Converted time slots to use a timestamp. Made it more
 *                  functional, as such.
 * Version - 4    - Added html functionality.
 * Version - 5    - Advanced somewhat html functionality.
 * Version - 6    - Completed HTML functionality.
 * Version - 6a   - Cleaned up the installation instructions, and fixed
 *                  minor bugs (unused variables).
 * Version - 6b   - Changed e-mail addresses to reflect e-mail change,
 *                  Also added a version announcement.
 * Version - 6c   - Updated the install.txt file to reflect all news
 *                  news options.
 * Version - 6d   - Updated the license.txt file to clarify a licensing
 *                  issue brought to my attention.
 * Version - 6e   - Updated the snippet information, and e-mail address.
 * Version - 7    - Updated the snippet to correct faulty fprintf calls.
 * Version - 8    - Added a parameter to news.h to make news.html creation
 *                  an option.
 * Version - 8a   - Added '<string.h>' to the news.c includes.
 * Version - 8b   - Added a news.help file to the .zip.
 * Version - 8.1a - Added the fptof function.
 * Version - 8.1b - Got bored and changed the default news colors.
 * Version - 8.2a - Removed an unnecessary #include.
 * Version - 8.2b - Moved the directory defines for the news into the
 *                  news.h file.
 * Version - 8.3a - Moved the news definitions and information into the
 *                  news.h file.
 * Version - 8.3b - Added the ability to turn on and off the HTML news
 *                  inside the MUD.
 * Version - 8.3c - Added a fix to make the news system compile under the
 *                  Linux OS properly.
 *
 * Used for identification, and with the news 'version' call.
 */
#define NEWS_VERSION		"8.30c"

/*
 * News data file. -Orion
 */
#define NEWS_DIR		"../news/"
#define HTML_NEWS_DIR		"../news/"
#define NEWS_FILE		NEWS_DIR "news.dat"
#define HTML_NEWS_FILE		HTML_NEWS_DIR "news.html"
#define HTML_NEWS_IMAGES	"./news_img"

/*
 * Four times default MAX_STRING_LENGTH, to ensure enough space. -Orion
 */
#define MAX_NEWS_LENGTH		(MAX_STRING_LENGTH * 3)
#define MAX_HTML_LENGTH		(MAX_STRING_LENGTH * 6)

/*
 * News color definitions. -Orion
 */
#define AT_BLADE		"&G&w"
#define HTML_BLADE		HTML_GREY
#define AT_BASE			"&G&W"
#define HTML_BASE		HTML_WHITE
#define AT_HILT			"&G&b"
#define HTML_HILT		HTML_DBLUE
#define AT_BALL			"&G&W"
#define HTML_BALL		HTML_WHITE
#define AT_TITLE		"&G&W"
#define HTML_TITLE		HTML_WHITE

#define AT_DATE			"&G&W"
#define HTML_DATE		HTML_WHITE
#define AT_SEPARATOR		"&G&b"
#define HTML_SEPARATOR		HTML_DBLUE
#define AT_NEWS			"&G&w"
#define HTML_NEWS		HTML_GREY


/*
 * Type definition of the news_data struct. -Orion
 */
typedef struct  news_data               NEWS_DATA;

/*
 * News show types. -Orion
 */
typedef enum
{
  TYPE_ALL,		TYPE_NORMAL,		TYPE_LIST_FIRST,
  TYPE_LIST_LAST,	TYPE_IMM_LIST,
  MAX_SHOW_TYPE
} news_show_types;

/*
 * News types. -Orion
 */
typedef enum
{
  NEWS_NORM,		NEWS_GROUP,		NEWS_COUNCIL,
  NEWS_CODE,		NEWS_IMM,
  MAX_NEWS_TYPE
} news_types;

/*
 * News structure. -Orion Elder
 */
struct  news_data
{
    NEWS_DATA *         next;
    NEWS_DATA *         prev;
    int                 day;
    int                 month;
    int                 year;
    char *              news_data;
    time_t              time_stamp;
};

/*
 * The first and last news pointers for the NEWS_DATA linked list. -Orion
 */
extern          NEWS_DATA         *     first_news; /* Orion Elder */
extern          NEWS_DATA         *     last_news; /* Orion Elder */

/*
 * HTML color codes. -Orion
 */
#define HTML_RED	"#CC0000"
#define HTML_DRED	"#880000"

#define HTML_BLUE	"#0000CC"
#define HTML_DBLUE	"#000088"

#define HTML_GREEN	"#33FF33"
#define HTML_DGREEN	"#339933"

#define HTML_PURPLE	"#CC00FF"
#define HTML_DPURPLE	"#9900CC"

#define HTML_CYAN	"#00FFFF"
#define HTML_DCYAN	"#009999"

#define HTML_WHITE	"#FFFFFF"
#define HTML_GREY	"#CCCCCC"
#define HTML_DGREY	"#888888"
#define HTML_BLACK	"#000000"

#define HTML_COLOR_OPEN1	"<font color=\""
#define HTML_COLOR_OPEN2	"\">"
#define HTML_COLOR_CLOSE	"</font>"

