/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file ar.c is (a)strology (r)eporter
 *    a command line utility that spews all sorts of useful info */

#include "arfc.h"
int BUILD_NUMBER =
#include "BUILD_NUMBER"
   ;

char the_summary[] =
   "Astrology Reporter\n\n"
   "Outputs astrology data about an event. If no event is specified,\n"
   "uses the current time. The event should be specified according to\n"
   "ISO standard dates. Available house systems are:\n\n"
   "P Placidus     K Koch           T Topocentric\n"
   "C Campanus     M Morinus        U Krusinski-Pisa-Goelzer\n"
   "O Porphyrius   L Pullen SD      Q Pullen SR\n"
   "B Alcabitus    S Sripati        R Regiomontanus\n"
   "E Equal        D Equal from MC  V Vehlow equal\n"
   "I Sunshine     i Sunshine alt   Z Axial rotation\n"
   "Y APC houses   H Horizontal     F Carter poli-equatorial\n";

Event** events;

int pts[64] = { 0,1,2,3,4,5,6,7,8,9,11, SE_END };
static char* opt_sys = "PTK"; //in order of popularity
static char* opt_geo = "0,0";
static char* opt_fmt = NULL;
static Datum opt_geo_d;
static char* geo_rio = "-23,-43"; //UGLY to hardcode this

#define OPTIONS \
FMT( 0 , rio,   "Use geo coords for Rio de Janeiro") \
FMT('q', quiet, "Don't output unrequested information") \
FMT('j', jdn,   "Show Julian Day Number") \
FMT('u', houses,"Show house cusps") \
FMT('a', asp,   "Show aspect table") \
FMT('i', lit,   "Output C language literal") \
FMT('c', csv,   "Output comma sepparated values") \
FMT('C', ceres, "Include dwarf planet Ceres") \
FMT('E', eris,  "Include dwarf planet Eris") \
FMT('K', chiron,"Include comet Chiron") \
FMT('A', asts,  "Include main asteroids") \
FMT('U', ura,   "Include Uranian fictious planets") \
FMT('t', tst,   "temporary - run test()")
///@todo include opt for NO Inners
///@todo include opt for NO Pluto
///@todo include opt for NO Transaturnians

#define FMT(a,b,c) \
static gboolean opt_ ## b = FALSE;
// declare opt_* global variables
OPTIONS
#undef FMT

intern
void
parse_arguments( int* num_of_args, char** args[] )
   {
   #define FMT(a,b,c) { #b,a,0,G_OPTION_ARG_NONE,&opt_##b,c,NULL },
   static GOptionEntry main_opts[] =
      {
         {
         "geo", 'g', 0, G_OPTION_ARG_STRING, &opt_geo,
         "Set geographical coordinates", NULL
         },
         {
         "sys", 's', 0, G_OPTION_ARG_STRING,  &opt_sys,
         "Set House System", NULL
         },
         {
         "fmt", 'f', 0, G_OPTION_ARG_STRING,  &opt_fmt,
         "Point table format", NULL
         },
      OPTIONS
         { NULL }
      };
   #undef FMT
   GOptionContext* cx;
   cx = g_option_context_new( " YYYYMMDD.HHMMSS+TZ" );
   g_option_context_set_summary( cx, the_summary );
   g_option_context_add_main_entries( cx, main_opts, NULL );
   GError* error = NULL;
   if ( !g_option_context_parse( cx, num_of_args, args, &error ) )
      {
      g_print( "option parsing failed: %s\n", error->message );
      exit( 1 );
      }
   g_option_context_free( cx );
   //
   // edit pts[] based on command line opts
   int* pp = pts;
   *pp++ = SE_SUN;
   *pp++ = SE_MOON;
   *pp++ = SE_MERCURY;
   *pp++ = SE_VENUS;
   *pp++ = SE_MARS;
   if ( opt_ceres )
      {
      *pp++ = 17;
      }
   *pp++ = SE_JUPITER;
   *pp++ = SE_SATURN;
   if ( opt_chiron )
      {
      *pp++ = SE_CHIRON;
      }
   *pp++ = SE_URANUS;
   *pp++ = SE_NEPTUNE;
   *pp++ = SE_PLUTO;
   if ( opt_eris )
      {
      *pp++ = SE_AST_OFFSET + 136199;
      }
   if ( opt_ura )
      {
      *pp++ = 40;
      *pp++ = 41;
      *pp++ = 42;
      *pp++ = 43;
      *pp++ = 44;
      *pp++ = 45;
      *pp++ = 46;
      *pp++ = 47;
      }
   *pp = SE_END;
   //
   // latitude and longitude
   if ( opt_rio ) { opt_geo = geo_rio; }
   opt_geo_d = coords_of_string( opt_geo );
   //
   // parse all other arguments as event descriptions
   events = calloc( sizeof(Event*), *num_of_args );
   int c = 0;
   for ( int i=1; i < *num_of_args; i++ )
      {
      events[c] = make_event_of_string( (*args)[i] );
      if( events[c] )
         { c++; }
      else
         { printf("failed to parse: %s\n", (*args)[i] ); }
      }
   if ( c == 0 ) //???SHOULD THIS BE if num_of_args == 1?????
      {
      events[0] = calloc( 1, sizeof(Event) );
      events[0]->name = "now";
      events[0]->jdn = jdn_of_now();
      events[0]->lat = opt_geo_d.lat;
      events[0]->lon = opt_geo_d.lon;
      }
   }

//*//
intern
void
test( Chart* c )
   {
   puts( "running test() func" );
   printf( "aspect count: %i\n", c->asp_count );
   for( int i = 0; i < c->asp_count; i++ )
      {
      printf( "%s %s %s kind: %i score: %f\n",
         c->points[c->aspects[i].point1].symbol,
         c->aspects[i].symbol,
         c->points[c->aspects[i].point2].symbol,
         c->aspects[i].kind,
         c->aspects[i].score
         );
      }
   //printf("zlon we are looking for: %f\n", c->points[0].lon );
   //double sunret = nearest_return( c, 0, NAN );
   //printf( "jdn for next return: %f\n", sunret );
   //Chart* src;
   //src = make_chart( "Return", sunret, c->ev->lat, c->ev->lon );
   //report_chart( src );
   //dump_chart( src );
   }
//*/

intern
void
spit( char* s )
   {
   puts( s );
   free( s );
   }

/** process_event( ev ) reports on each of the events provided on the
 * command line according to what was requested.
 * 
 * @param ev An Event* structure.
 */
intern
void
process_event( Event* ev )
   {
   char buff[NAME_SIZE];
   Chart* c;
   /* -- do stuff according to what the user requests
   if( opt_ )
      {
      }
   */
   if ( !opt_quiet )
      {
      printf( "\nname:  %s\n", ev->name );
      to_datetag( buff, ev->jdn );
      printf( "time:  %s\n", buff );
      to_coords(buff,ev);
      printf( "place: %s\n", buff );
      }
   if( opt_jdn )
      {
      printf( "day number: %.3f\n", ev->jdn );
      }
   ///@maybe avoid calculating chart if no astro data requested
   c = make_chart_of_event( ev );
   if( !opt_quiet )
      {
      char* s;
      if( opt_fmt )
         { s = make_point_table(c, opt_fmt); }
      else
         { s = make_point_table(c, "|$Y| $N | $U |$S |$d |$C|"); }
      puts(s);
      free(s);
      }
   if( opt_houses )
      {
      spit( make_house_table(c) );
      }
   if( opt_asp )
      {
      spit(  make_aspect_table(c) );
      }
   if( opt_csv )
      {
      spit(  make_csv_list(c) );
      }
   if( opt_lit )
      {
      spit(  make_c_literal(c) );
      }
   if( opt_tst ) /* -- TESTING ---------------------------------------*/
      {
      test( c );
      }
   dump_chart( c );
   }

int
main( int num_of_args, char* args[] )
   {
   // initialization
   parse_arguments( &num_of_args, &args );
   init_swiss_ephemeris( opt_sys, pts );
   // banner
   if ( !opt_quiet )
      {
      printf( "Astrology Research Framework v0.0:%d\n", BUILD_NUMBER );
      }
   // process events
   if( events[0] == NULL) { puts("no events"); }
   for( int i = 0; events[i]; i++ )
      {
      process_event( events[i] );
      }
   // termination
   end_swiss_ephemeris();
   }
