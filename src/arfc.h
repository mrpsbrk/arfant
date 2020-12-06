/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file arfc.h
 *  @brief is included in all ARF files, but not exported.
 *
 * This file has a lot of C-related infrastructure, like other includes,
 * preprocessor definitions and testing framework.
 **/

#ifndef ARFC_H
#define ARFC_H

//---- INCLUDES ------------------------------------------------------//
//-- messy to include'em here, but i need glib's types, so...
#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include "swe/swephexp.h"

#include "arf.h"

//---- C STUFF -------------------------------------------------------//
#define intern static
#define complain(...) fprintf(stderr, __VA_ARGS__ )
#define enforce( action_description, exp ) \
if(!( exp )) { fputs("failed to " action_description, stderr); exit(1); }
#define attempt( action_description, exp ) \
if(!( exp )) { fputs("failed to " action_description, stderr); }

//--> for debuging (I KNOW IT IS UGLY)
#define SHOUT() printf("at line %d\n", __LINE__)

//---- C testing -----------------------------------------------------//
#ifdef TEST
#include <mcheck.h>

int unfreed_mallocs = 0;
#define malloc( x ) malloc( x ); unfreed_mallocs++;
#define calloc(...) calloc(__VA_ARGS__); unfreed_mallocs++;
#define strdup( x ) strdup( x ); unfreed_mallocs++;
#define free( x )   free( x );   unfreed_mallocs--;

#define BEGIN_TESTS int main( int arg_count, char* args[] ) \
   { \
   if( 0!=mcheck( NULL ) ) exit(171); \
   puts( "Running tests for " __FILE__ ); \
   int num_failures = 0;
#define END_TESTS return num_failures; }
#define MUST(d,op) \
   printf( "   test: %-40s: ", d ); \
   if ( op ) { printf( "OK\n" ); } \
   else { printf( "FAIL\n" ); num_failures++; }
#define NEAR(exp,ref) ( fabs(exp-ref)<EPSILON )
// for stuff that actually borks the program
#define TRIAL(d,op) \
   printf( "   test: %-40s: ", d ); \
   do { int errors = 0; \
      do { op } while (0); \
      if (0==errors) { printf( "OK\n" ); } \
      else { num_failures++; printf( "FAIL %d errors\n", errors );} \
   } while (0)
#define AVOID(op) if( (op) ) { errors++; complain( #op "\n" ); }
#define ENSURE(op) if( !(op) ) { errors++; complain( "! " #op "\n" ); }

#define BOUND(op) \
   unfreed_mallocs = 0; \
   do { op } while (0); \
   if(0 != unfreed_mallocs) \
      { errors++;printf("(%d unfreed)",unfreed_mallocs); };
#define PROBE( ptr ) if( MCHECK_OK != mprobe( ptr ) ) \
   { errors++; complain( "mprobe error for" #ptr ); }
#define CLOBBER(S) do { char* ptr = S; \
   while ( *ptr != '\0' ) { *ptr='X'; ptr++; } \
   } while (0);

#endif //TEST

//Doxygen template
/* * fun() is
 * @param
 *
 * @return
 */

/*---- UNICODE code points for
 * --- astrological symbols
Aries       \u2648
Taurus      \u2649
Gemini      \u264A
Cancer      \u264B
Leo         \u264C
Virgo       \u264D
Libra       \u264E
Scorpio     \u264F
Sagittarius \u2650
Capricorn   \u2651
Aquarius    \u2652
Pisces      \u2653

Ascending Node   \u260A
Descending Node  \u260B

Earth     \u2295  or  \u2641

Sun       \u2609
Moon      \u263D
Mercury   \u263F
Venus     \u2640
Mars      \u2642
Jupiter   \u2643
Saturn    \u2644
Uranus    \u2645  or  \u26E2
Neptune   \u2646

Ceres     \u26B3
Pluto     \u2BD3  or  \u2647
Eris      \u2BF0
Comet (g) \u2604

Retrograde   \u211E
Conjunct     \u260C
Semisextile  \u26BA
Sextile      \u26B9
Square       \u25A1
Trine        \u25B3
Quincunx     \u26BB
Opposition   \u260D

Moon phases
New      \U0001F311
Crescent \U0001F312
1st qtr  \U0001F313
Gibbous  \U0001F314
Full     \U0001F315
Dissem   \U0001F316
Last qtr \U0001F317
Balsamic \U0001F318

 * --- alchemical symbols
 * --- geometrical shapes
Black CIRCLE \u25CF
White circle \u25CB
Bullseye     \u25CE
Fisheye      \u25C9
Black Diamond \u25C6
White diamond \u25C7
Lozenge      \u25CA
Black square \u25A0
White square \u25A1
Doubl square \u25A3
Down pointing Triangle \u25BC
" but white            \u25BD
Up pointing Triangle   \u25B2



*/

#endif //ARFC_H
