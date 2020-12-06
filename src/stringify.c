/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file stringify.c
 * Fill out character arrays with Hopefully) meaningful stuff.
 */

#include "arfc.h"

/** to_datetag() formats a return string as Y-M-D H-M.
 * @param (char *) ret is the string to write in.
 * @param (double) jdn is the date in jdn format.
 *
 * @return number of characters written
 */
size_t
to_datetag( char* ret, double jdn )
   {
   GDateTime* gdt = make_gdatetime_of_jdn( jdn );
   size_t len = sprintf(
                     ret,
                     "%04d-%d-%d:%02d-%02d UCT",
                     g_date_time_get_year(gdt),
                     g_date_time_get_month(gdt),
                     g_date_time_get_day_of_month(gdt),
                     g_date_time_get_hour(gdt),
                     g_date_time_get_minute(gdt)
                     );
   dump_gdatetime( gdt );
   return len;
   }

/** to_roman() renders a number in roman notation, additive
 * @param a return string, an integer
 *
 * @return the number of chars written
 */
size_t
to_roman( char* ret, int num )
   {
   const char digit[] = { 'M', 'D', 'C', 'L', 'X', 'V', 'I' };
   const int value[] = { 1000, 500, 100,  50,  10,   5,  1, 0  };
   int i = 0;
   size_t len = 0;
   while ( num > 0 )
      {
      if ( num < value[i] ) {i++; continue;}
      *ret++ = digit[i];
      len++;
      num = num - value[i];
      }
   *ret='\0';
   return len;
   }

/** to_zodiac_utf() does a sprintf of a sky longitude, using utf-8 chars
 * for the signs.
 * @param a return string, a longitude in degrees
 *
 * @return the number of chars written
 */
size_t
to_zodiac_utf( char* str, double lon )
   {
   if ( isnan( lon ) ) { return 0; }
   static char* abbr[] =
      {
      "\u2648","\u2649","\u264A","\u264B","\u264C","\u264D","\u264E",
      "\u264F","\u2650","\u2651","\u2652","\u2653"
      };
   int sign;
   double deg, min;
   sign= ( int ) lon/30; //FIXME this would break on lon > 360
   min= modf( ( lon-( sign*30 ) ),&deg )*60;
   size_t len = sprintf( str,"%02.0f%s%06.3f",deg,abbr[sign],min );
   return len;
   }

/** to_zodiac_ascii() does a sprintf of a sky longitude, using
 * abbreviations for the signs.
 * @param a return string, a longitude in degrees
 *
 * @return the number of chars written
 */
size_t
to_zodiac_ascii( char* str, double lon )
   {
   if ( isnan( lon ) ) { return 0; }
   static char* abbr[] =
      {"Ar","Ta","Gm","Cn","Le","Vi","Lb","Sc","Sg","Cp","Aq","Pi" };
   int sign;
   double deg, min;
   sign= ( int ) lon/30; //FIXME this would break on lon > 360
   min= modf( ( lon-( sign*30 ) ),&deg )*60;
   size_t len = sprintf( str,"%02.0f%s%06.3f",deg,abbr[sign],min );
   return len;
   }

/** to_coords() does a sprintf of the geo coords of an Event
 * @param a return string, and an Event
 *
 * @return the number of chars written
 */
size_t
to_coords( char* str, Event* ev )
   {
   char latTAG = (ev->lat < 0.0) ? 'S' : 'N';
   char lonTAG = (ev->lon < 0.0) ? 'W' : 'E';
   size_t len = sprintf( str,
                   "%.2f%c, %.2f%c",
                   fabs(ev->lat),
                   latTAG,
                   fabs(ev->lon),
                   lonTAG );
   return len;
   }

/** to_symbol_utf() copies the UTF symbol into a string.
 * @param ar Array that is copied into.
 * @param code SwissEphemeris point number.
 *
 * @return Number of bytes copied.
 */
size_t
to_symbol_utf( char* ar, int code )
   {
   size_t len = 3;
   #define cp_symbol( S ) g_strlcpy(ar, S , SYMB_SIZE); break
   switch ( code )
      {
      case SE_SUN:      cp_symbol( "\u2609" ) ; 
      case SE_MOON:     cp_symbol( "\u263D" ) ;
      case SE_MERCURY:  cp_symbol( "\u263F" ) ;
      case SE_VENUS:    cp_symbol( "\u2640" ) ;
      case SE_MARS:     cp_symbol( "\u2642" ) ;
      case SE_JUPITER:  cp_symbol( "\u2643" ) ;
      case SE_SATURN:   cp_symbol( "\u2644" ) ;
      case SE_URANUS:   cp_symbol( "\u2645" ) ; //also "\u2645"
      case SE_NEPTUNE:  cp_symbol( "\u2646" ) ;
      case SE_PLUTO:    cp_symbol( "\u2647" ) ;
      case SE_EARTH:    cp_symbol( "\u2295" ) ;
      case SE_MEAN_NODE:
      case SE_TRUE_NODE:cp_symbol( "\u260A" ) ;
      //descending node is "\u260B"
      case SE_AST(1):
      case SE_CERES:    cp_symbol( "\u26B3" ) ;
      case SE_AST(2):
      case SE_PALLAS:   cp_symbol( "\u26B4" ) ;
      case SE_AST(3):
      case SE_JUNO:     cp_symbol( "\u26B5" ) ;
      case SE_AST(4):
      case SE_VESTA:    cp_symbol( "\u26B6" ) ;
      case SE_ERIS:     cp_symbol( "\u2BF0" ) ; //also "\u2BF1"
      case SE_AST(2060):
      case SE_CHIRON:   cp_symbol( "\u26B7" ) ;
      case SE_AST(5145):
      case SE_PHOLUS:   cp_symbol( "\u2BDB" ) ;
      // URANIAN BODIES
      case SE_CUPIDO:   cp_symbol( "\u2BE0" ) ;
      case SE_HADES:    cp_symbol( "\u2BE1" ) ;
      case SE_ZEUS:     cp_symbol( "\u2BE2" ) ;
      case SE_KRONOS:   cp_symbol( "\u2BE3" ) ;
      case SE_APOLLON:  cp_symbol( "\u2BE4" ) ;
      case SE_ADMETOS:  cp_symbol( "\u2BE5" ) ;
      case SE_VULKANUS: cp_symbol( "\u2BE6" ) ;
      case SE_POSEIDON: cp_symbol( "\u2BE7" ) ;
      //comet: "\u2604"
      //Lot of fortune: "\u2297"
      //Transpluto: "\u2BD7"
      //Lilith: "\u26B8"
      //Nessus: "\u2BDC"
      default: cp_symbol( "\u2605" ); //star?
      }
   #undef cp_symbol
   return len;
   }

//####################################################################//
//---- TEST ----------------------------------------------------------//
//####################################################################//
#ifdef TEST
#define BUFSIZE 64
#define MATCH(func,arg,ret) func(bf, arg );AVOID(strcmp(ret,bf));
BEGIN_TESTS
   //
   // The birthday for the Swiss Ephemeris is 1997-09-30:14:00:00+02
   const double swebday = 2450722.0833333335;
   //
   // TEST: to_symbol_utf( char*, int );
   TRIAL("to_symbol_utf()",
      char bf[BUFSIZE];
      MATCH( to_symbol_utf, SE_SUN,     "\u2609" );
      MATCH( to_symbol_utf, SE_VENUS,   "\u2640" );
      MATCH( to_symbol_utf, SE_MARS,    "\u2642" );
      MATCH( to_symbol_utf, SE_SATURN,  "\u2644" );
      MATCH( to_symbol_utf, SE_JUPITER, "\u2643" );
      MATCH( to_symbol_utf, SE_MERCURY, "\u263F" );
      MATCH( to_symbol_utf, SE_NEPTUNE, "\u2646" );
      //MATCH( to_symbol_utf, SE_, "" );
      );
   //
   // TEST: to_zodiac_utf( char*, double );
   TRIAL("to_zodiac_utf()",
      char bf[BUFSIZE];
      MATCH( to_zodiac_utf, 15.50, "15\u264830.000" );
      MATCH( to_zodiac_utf, 42.25, "12\u264915.000" );
      MATCH( to_zodiac_utf, 73.25, "13\u264A15.000" );
      MATCH( to_zodiac_utf, 94.50, "04\u264B30.000" );
      );
   //
   // TEST: to_zodiac_ascii( char*, double );
   TRIAL("to_zodiac_ascii()",
      char bf[BUFSIZE];
      MATCH( to_zodiac_ascii, 15.5, "15Ar30.000" );
      MATCH( to_zodiac_ascii, 45.5, "15Ta30.000" );
      );
   //
   // TEST: to_coords( char*, Event* );
   TRIAL("to_coords()",
      char bf[BUFSIZE];
      #define EVX(LA,LO) & (Event) {.lat=LA,.lon=LO}
      MATCH( to_coords, EVX( 42.25, 59.75), "42.25N, 59.75E" );
      MATCH( to_coords, EVX(-42.25, 59.75), "42.25S, 59.75E" );
      MATCH( to_coords, EVX( 42.25,-59.75), "42.25N, 59.75W" );
      MATCH( to_coords, EVX(-42.25,-59.75), "42.25S, 59.75W" );
      );
   //
   // TEST: to_datetag( char*, int );
   TRIAL("to_datetag() against known vals",
      char bf[BUFSIZE];
      MATCH( to_datetag, swebday, "1997-9-30:14-00 UCT" );
      );
   //
   // TEST: to_roman( char*, int );
   TRIAL("to_roman() against known vals",
      char bf[BUFSIZE];
      MATCH( to_roman, 1997, "MDCCCCLXXXXVII" );
      MATCH( to_roman,  500, "D" );
      MATCH( to_roman, 2020, "MMXX" );
      );
   //
END_TESTS
#endif //TEST
