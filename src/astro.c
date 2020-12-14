/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file astro.c
 *    contains astrology related functions.
 *    Much of this is handling the Swiss Ephemeris, and data types.
 **/

#include "arfc.h"
intern void fill_points( Chart* ); //--> used to be extern in arf.h
intern void fill_cusps( Chart* ); //--> used to be extern in arf.h
intern void fill_aspects( Chart* );

//---- CONFIGURATION DATA --------------------------------------------//
static int def_pts[] = { 0,1,2,3,4,5,6,7,8, SE_END };
static int * dyn_pts = NULL;
static int * the_pts = def_pts;
static char def_systems[] = "PTK";
static char * the_systems = def_systems;
static char * dyn_systems = NULL;
// systems_by_popularity[] = "PTKEUORWCB";
// all_swiss_eph_systems[] = "BYXHCFEDNIiKUMPTOLQRSVW";

//FIX_ME should this be in arf.h??????????
static struct
   {
   int kind;
   double angle;
   double orb;
   char symbol[4];
   } aspect_defs[] =
   {
      {   1,   0.0, 10.0, "\u260C" }, //conjunction
      {   2, 180.0,  8.0, "\u260D" }, //opposition
      {   3, 120.0,  7.0, "\u25B3" }, //trine
      {   4,  90.0,  6.0, "\u25A1" }, //square
      {   6,  60.0,  3.0, "\u26B9" }, //sextile
      { 121,  30.0,  2.0, "\u26BA" }, //inconjunct
      { 125, 150.0,  2.0, "\u26BB" }, //quincunx
   //char* symbs[] = {" ",,,"\u25A1","\u2155","\u26B9","\u2150"};
      { 0,  NAN,  NAN, "" }
   };

//---- FUNCTIONS -----------------------------------------------------//
/** Makes the ephemeris ready to use
 * @param systems is a string, where each letter is a house-system
 * @param ptlist is an array of point codes, terminated by SE_END
 */
extern
void
//FIXME the configs should be passed as a CONF_STRUCT...
init_swiss_ephemeris( char* systems, int* ptlist )
   {
   char* d = NULL;
   char* paths[] = { "./ephe", "~/sweph", "/usr/share/sweph" };
   // maybe add?? $XDG_CONFIG/arf/ephemeris:/usr/share/sweph:/usr/local/share/sweph
   for ( int f = 0; f < ( sizeof( paths )/sizeof( *paths ) ); f++ )
      {
      if ( g_file_test( paths[f], G_FILE_TEST_EXISTS ) ) { d = paths[f]; }
      }
   swe_set_ephe_path( d );
   if ( ptlist!=NULL )
      {
      int len = 0;
      while( SE_END != ptlist[len] ) { len++; }
      dyn_pts = malloc( sizeof(int) * (len + 1) );
      int i = 0;
      while( SE_END != ptlist[i] )
         {
         dyn_pts[i] = ptlist[i];
         i++;
         }
      dyn_pts[i] = SE_END;
      the_pts = dyn_pts;
      }
   if ( systems!=NULL )
      {
      dyn_systems = strdup( systems );
      the_systems = dyn_systems;
      }
   }

/** end_swiss_ephemeris() Does any cleanup needed after using ephemeris.
 */
extern
void
end_swiss_ephemeris()
   {
   if(dyn_pts)
      {
      free(dyn_pts);
      dyn_pts = NULL;
      the_pts = def_pts;
      }
   if(dyn_systems)
      {
      free(dyn_systems);
      dyn_systems = NULL;
      the_systems = def_systems;
      }
   swe_close();
   }

/** make_chart() allocates and fills a Chart from some event data.
 * @param name String describing the event, usually a name.
 * @param jdn A precise moment in time, in the Julian Day Number format.
 * @param lat,lon Latitude and Longitude.
 *
 * @return a pointer to a Chart structure.
 */
extern
Chart*
make_chart( char* name, double jdn, double lat, double lon )
   {
   Chart* c;
   c = malloc( sizeof( Chart ) );
   // populate Chart->Event
   c->ev = malloc( sizeof( Event ) );
   c->ev->name = strdup( name );
   c->ev->jdn = jdn;
   c->ev->lat = lat;
   c->ev->lon = lon;
   // count point length
   int len=0;
   while( the_pts[len]!=SE_END ) { len++; }
   c->pt_count = len;
   c->sys_count = strlen(the_systems);
   //at least 12 houses, but more if many alt housesystems requested
   c->cp_count = 12 + 12 * (c->sys_count/4);
   // alloc big array which is both Cusps and Points
   c->cusps = calloc( sizeof( Point ), (c->pt_count + c->cp_count) );
   c->points = c->cusps + c->cp_count;
   // populate arrays
   fill_points( c );
   fill_cusps( c );
   fill_aspects( c );
   return c;
   }

/** make_chart_of_event() calls make_chart() with data from an event.
 * @param ev An Event* structure.
 *
 * @return a pointer to a Chart structure.
 */
extern
Chart*
make_chart_of_event( Event* ev )
   {
   return make_chart( ev->name, ev->jdn, ev->lat, ev->lon );
   }

/** dump_chart() deallocates memory of a Chart struct.
 * @param c Pointer to a Chart structure.
 */
extern
void
dump_chart( Chart* c )
   {
   free( c->ev->name );
   free( c->ev );
   free( c->cusps ); // also frees c->points
   free( c->aspects );
   free( c );
   }

/** fill_points() calculates the points in a Chart.
 * @param c Pointer to a Chart structure.
 */
intern
void
fill_points( Chart* c )
   {
   //--- swiss ephemeris vars
   long stat;
   long opts= SEFLG_SPEED;
   double ret[6];
   char buff[50]; /* SWEPH address for planet name, at least 20 char */
   char err[AS_MAXCH];
   //---
   for ( int i=0; i < c->pt_count; i++ )
      {
      stat = swe_calc_ut( c->ev->jdn,the_pts[i],opts,ret,err );
      if ( stat<0 )
         {
         (*c).points[i].code  =the_pts[i];
         g_strlcpy( (*c).points[i].name, "Swiss Ephemeris error", NAME_SIZE );
         g_strlcpy( (*c).points[i].symbol, "?", SYMB_SIZE );
         }
      else
         {
         (*c).points[i].code  =the_pts[i];
         (*c).points[i].lon   =ret[0];
         (*c).points[i].lat   =ret[1];
         (*c).points[i].dist  =ret[2];
         (*c).points[i].speed =ret[3];
         swe_get_planet_name( the_pts[i], buff );
         g_strlcpy( (*c).points[i].name, buff, NAME_SIZE );
         to_symbol_utf( (*c).points[i].symbol,the_pts[i] );
         }
      }
   stat = swe_calc_ut( c->ev->jdn,SE_TRUE_NODE,opts,ret,err );
   (*c).points[-12].def = ret[0];
   }

/** fill_cusps() calculates house cusps in a Chart.
 * @param c A pointer to a Chart structure.
 */
intern
void
fill_cusps( Chart* c )
   {
   // swiss ephemeris stuff
   double ret[13] = {};
   double extra[10] = {};
   long stat;
   //char err[AS_MAXCH];
   //long opts= SEFLG_SPEED;
   //
   if ( 'G' == the_systems[0] ) { puts( "Gauquelin not implemented" ); exit( 'g' ); }
   int len = strlen( the_systems );
   //
   for( int s=0; s<len; s++ ) // s indexing the _S_ystems
      {
      int base = -12 * ((s+1)/4); // for .points[base-x]
      char tag = the_systems[s];
      stat = swe_houses(c->ev->jdn,c->ev->lat,c->ev->lon,tag,ret,extra );
      if( stat==-1 ) { tag = '?'; }
      for( int i=1; i<=12; i++ ) //ATTENTION! counting from 1!
         {
         int place = s<3 ? s : (s+1)%4;
         (*c).points[base-i].data[place] = ret[i];
         (*c).points[base-i].symbol[place] = tag;
         if( !(s&&((s+1)%4)) ) //this is true everytime we are in a new 12-cluster
            {
            (*c).points[base-i].code = -i;
            to_roman( (*c).points[base-i].name, i);
            }
         }
      }
   (*c).points[-1].def = extra[0]; // Ascendant
   (*c).points[-2].def = extra[5]; // "co-ascendant Kock"
   (*c).points[-3].def = extra[6]; // "co-ascendant Munkasey"
   (*c).points[-4].def = extra[2]; // Right ascension ARMC
   (*c).points[-5].def = extra[4]; // "equatorial ascendant"
   (*c).points[-6].def = extra[7]; // "polar ascendant"
   (*c).points[-7].def = extra[3]; // West Point (Vertex)
   //(*c).points[-8].def = extra[?];
   //(*c).points[-9].def = extra[?];
   (*c).points[-10].def = extra[1];// Midheaven MC
   //(*c).points[-11].def = extra[?];
   /// @todo add missing (*c).points[-8,-9,-11,-12].def
   //(*c).points[-12].def = ret[0];
   }

/** to_aspect() determines whether there is an aspect between to angles.
 * @param ang1, ang2 Two sky positions, in degrees.
 *
 * @return An Aspect structure, that can be .kind=0 for no aspect.
 */
Aspect
to_aspect( double ang1, double ang2 )
   {
   const char* symbs[] =
      {" ","\u260C","\u260D","\u25B3","\u25A1","\u2155","\u26B9","\u2150"};
      // Semisextile  \u26BA
      // Quincunx     \u26BB
   Aspect a = {.kind=0,.diff=0.0,.score=0.0,.symbol={'.','\0','\0','\0'} };
   double m, s;
   a.diff = fabs( ang1 - ang2 );
   if ( a.diff > 180.0 ) { a.diff = ( a.diff - 360.0 ) * -1.0; }
   #define RANK(n) m = fabs( fmod( a.diff, 360.0/n ) - (180/n) ) / (180.0/n); \
      s = pow( m, 18 ) * 110.0 - 10.0;
   for ( int i = 1; i<8; i++ )
      {
      RANK( (double)i );
      if ( s>a.score ) { a.score = s; a.kind = i; }
      }
   RANK( 12.0 );
   if ( s>a.score ) { a.score = s; a.kind = 12; }
   g_strlcpy( a.symbol, symbs[a.kind], SYMB_SIZE );
   return a;
#undef RANK
   }

intern
void
fill_aspects( Chart* c )
   {
   int count = 0;
   #define pts c->points
   c->aspects = malloc( sizeof(Aspect) * c->pt_count*(c->pt_count-1) );
   for ( int i = 0; i < c->pt_count; i++)
      {
      for ( int k = i+1; k < c->pt_count; k++ )
         {
         c->aspects[count] = to_aspect( pts[i].lon, pts[k].lon );
         if( c->aspects[count].kind != 0 )
            {
            c->aspects[count].point1 = i;
            c->aspects[count].point2 = k;
            count++;
            }
         }
      }
   c->asp_count = count;
   #undef pts
   }

/** make_aspects() allocates and calculates aspects (using aspect_defs[])
 * @param Pointer to a Chart structure.
 *
 * @return Pointer to an array of Aspects.
 */
Aspect*
make_aspects( Chart* c )
   {
   Aspect* list = malloc( sizeof(Aspect) * c->pt_count*(c->pt_count-1) );
   int ac = 0;
   double da, d;
   for ( int i = 0; c->pt_count > i; i++ )
      {
      for ( int k=i+1; c->pt_count > k; k++ )
         {
         d = c->points[i].lon - c->points[k].lon;
         d = d < 180.0 ? d : d - 360.0;
         d = d >-180.0 ? d : d + 360.0;
         da = fabs( d );
         for ( int m=0; aspect_defs[m].kind!=0; m++ )
            {
            if ( fabs( da-aspect_defs[m].angle )<aspect_defs[m].orb )
               {
               list[ac].kind = aspect_defs[m].kind;
               list[ac].score = aspect_defs[m].orb-fabs( da-aspect_defs[m].angle );
               list[ac].point1 = i;
               list[ac].point2 = k;
               list[ac].diff = d;
               g_strlcpy( list[ac].symbol, aspect_defs[m].symbol, SYMB_SIZE );
               ac++;
               }
            }
         }
      }
   list[ac++] = ( Aspect ) { 0, NAN, 0, 0, {}, NAN };
   list = realloc( list, sizeof( Aspect ) * ac );
   return list;
   }

/** dump_aspect() deallocates a naked array of aspects.
 * @param Pointer to an array of Aspects.
 */
void
dump_aspects( Aspect* ptr )
   {
   free( ptr );
   }

/** nearest_return() 
 * @param c Pointer to a Chart.
 * @param i Index of the point we want (in Chart.points[i] ).
 * @param date A reference date to search. If NAN use now().
 *
 * @return A moment in JDN format.
 */
double
nearest_return( Chart* c, int code, double ref_time )
   {
   // swiss ephemeris stuff
   long stat, opts= SEFLG_SPEED;
   double ret[6] = {};
   char err[AS_MAXCH];
   // precision limit of thousandth of arcsecond
   const double epsilon = 1.0 / 3600000.0;
   // calculating zlon of sun now
   double orig_zlon = c->points[code].lon;
   double next_zlon = 10000.0;
   double next = ref_time;
   if (isnan(ref_time)) { next = jdn_of_now(); }
   while ( fabs( orig_zlon - next_zlon ) > epsilon )
      {
      stat = swe_calc_ut( next, code, opts, ret, err );
      if (stat<0) {puts("swiss ephemeris error"); return NAN; }
      next_zlon = ret[0];
      next = ( orig_zlon - next_zlon ) / ret[3] + next;
      printf("target: %f calc: %f\n", orig_zlon, next_zlon );
      }
   return next;
   }

/** nearest_ingress() 
 * @param int code Code of a point (sweph format).
 * @param double zlon Where exactly is it ingressing into.
 * @param date A reference date to search. If 0 uses now().
 *
 * @return A moment in JDN format.
 */
double
nearest_ingress( int code, double zlon, double ref_time )
   {
   // this should FAIL...
   return NAN;
   }


//####################################################################//
//---- TEST ----------------------------------------------------------//
//####################################################################//
#ifdef TEST
#include <locale.h> //--> needed to make mbtowc() work

#define SANITY_TEST_CHART( c ) AVOID( NULL == c ) \
   AVOID( c->ev->jdn < 1500000.0 ); \
   AVOID( c->ev->jdn > 2650000.0 ); \
   for( int i = 1; i <= 12; i++) \
      { \
      AVOID( c->points[0-i].lon <= 0.0 ); \
      AVOID( c->points[0-i].lon > 360.0 ); \
      AVOID( c->points[0-i].code != -i ); \
      } \
   for_each_point(c) \
      { \
      AVOID( each->lon <= 0.0 || each->lon > 360.0 ); \
      AVOID( each->lat < -180.0 || each->lat > 180.0 ); \
      AVOID( each->dist < 0.0 || each->dist > 200.0 ); \
      AVOID( fabs(each->speed) > 30.0 ); \
      AVOID( strlen(each->symbol) == 0 ); \
      AVOID( strlen(each->symbol) > 4 ); \
      AVOID( strlen(each->name) == 0 ); \
      AVOID( mbtowc(NULL,each->symbol,10) < 1 ); \
      }

//---> TODO: Verify with some known data
//---> TODO: give weird list of points and check pt_count
//---> TODO: give weird list of house systems and check cp_count
//---> TODO: check invalid house systems
//---> TODO: try out all ITERATOR DEFINES
//---> TODO: try out all ACESSOR DEFINES
//---> TODO: try mprobe()
//---> TODO: write first and last bytes of all arrays
//---> TODO: 
/*
   TRIAL("condition",
      AVOID( TRUE );
      );
*/

BEGIN_TESTS
   g_test_init(&arg_count, &args, NULL);
   //
   Chart * tc;
   double jdn = 0;
   double lat = 0;
   double lon = 0;
   setlocale(LC_ALL, "");
   //
   // TEST the initialization process
   TRIAL("no leaks on make_chart (no arf init)",
      BOUND(
         swe_set_ephe_path( NULL );
         tc = make_chart( "test_alloc", 2457555.0, 120.0, 20.0 );
         SANITY_TEST_CHART( tc );
         PROBE( tc );
         dump_chart( tc );
         swe_close();
         );
      );
   TRIAL("init_swiss_ephemeris()",
      BOUND(
         init_swiss_ephemeris( NULL, NULL );
         tc = make_chart( "test_alloc", 2457555.0, 120.0, 20.0 );
         SANITY_TEST_CHART( tc );
         PROBE( tc );
         ENSURE( NULL == dyn_pts );
         ENSURE( NULL == dyn_systems );
         dump_chart( tc );
         end_swiss_ephemeris();
         //
         init_swiss_ephemeris( NULL, (int[]) {0,3,5,7,SE_END} );
         tc = make_chart( "test_alloc", 2457555.0, 60.0, 70.0 );
         SANITY_TEST_CHART( tc );
         PROBE( tc );
         PROBE( the_pts );
         ENSURE( NULL == dyn_systems );
         dump_chart( tc );
         end_swiss_ephemeris();
         //
         init_swiss_ephemeris( "PT", NULL );
         tc = make_chart( "test_alloc", 2457555.0, 60.0, 70.0 );
         SANITY_TEST_CHART( tc );
         PROBE( tc );
         ENSURE( NULL == dyn_pts );
         PROBE( the_systems );
         dump_chart( tc );
         end_swiss_ephemeris();
         );
      );
   TRIAL("Koch and Placidus at weid coords",
      init_swiss_ephemeris( "TPK", NULL );
      tc = make_chart( "test_alloc", 2457555.0, 120.0, 20.0 );
      SANITY_TEST_CHART( tc );
      dump_chart( tc );
      end_swiss_ephemeris();
      );
   TRIAL("check equal houses are 30 deg appart",
      init_swiss_ephemeris( "E", NULL );
      tc = make_chart( "testEQUALhouses", 2457555.0, 120.0, 20.0 );
      double diff;
      for(int i=-1;i>-12;i--)
         {
         AVOID( tc->points[i].code != i );
         diff = tc->points[i-1].lon - tc->points[i].lon;
         AVOID( !( NEAR(diff, 30.0) || NEAR(diff, -330.0) ) );
         }
      AVOID( tc->points[-12].code != -12 );
      diff = tc->points[-1].lon - tc->points[-12].lon;
      AVOID( !( NEAR(diff, 30.0) || NEAR(diff, -330.0) ) );
      dump_chart( tc );
      end_swiss_ephemeris();
      );
   //
   init_swiss_ephemeris( "TURC", NULL );
   //
   TRIAL( "mem before = after",
      BOUND(
         tc = make_chart( "te2", 2457355.0, 120.0, 20.0 );
         dump_chart( tc );
         )
      );
   TRIAL("basic chart calculation",
      tc = make_chart( "test_alloc", 2457555.0, 120.0, 20.0 );
      SANITY_TEST_CHART( tc );
      dump_chart( tc );
      );
   TRIAL("ramdomized chart",
      jdn = g_test_rand_double_range( 2455000.0, 2459000.0 );
      lat = g_test_rand_double_range( -180.0, 180.0 );
      lon = g_test_rand_double_range( -180.0, 180.0 );
      tc = make_chart( "test_alloc", jdn, lat, lon );
      SANITY_TEST_CHART( tc );
      dump_chart( tc );
      );
   TRIAL("Accessor macros",
      jdn = g_test_rand_double_range( 2455000.0, 2459000.0 );
      lat = g_test_rand_double_range( -180.0, 180.0 );
      lon = g_test_rand_double_range( -180.0, 180.0 );
      tc = make_chart( "test_alloc", jdn, lat, lon );
      int count = 0;
      for_each_house(tc)
         {
         count++;
         AVOID( each->cusp < 0.0 || each->cusp > 360.0 );
         }
      AVOID( count != 12);
      for_each_point(tc)
         {
         AVOID( each->lon < 0.0 || each->lon > 360.0 );
         }
      for(int n=1; n<12; n++)
         {
         AVOID(tc->housealt(n,0) != tc->house(n).cusp);
         AVOID(tc->housealt(n,0) != tc->points[-n].data[0]);
         AVOID(tc->housealt(n,1) != tc->points[-n].data[1]);
         AVOID(tc->housealt(n,2) != tc->points[-n].data[2]);
         AVOID(tc->housealt(n,3) != tc->points[-12-n].data[0]);
         AVOID(tc->housealt(n,4) != tc->points[-12-n].data[1]);
         AVOID(tc->housealt(n,5) != tc->points[-12-n].data[2]);
         AVOID(tc->housealt(n,6) != tc->points[-12-n].data[3]);
         }
      //for(int i = 0; i<6; i++)
         //{
         //printf("\n%f\t%f\t%f", tc->housealt(2,i), tc->housealt(3,i), tc->housealt(4,i));
         //}
      dump_chart( tc );
      );
   TRIAL("make many charts at the same time",
      BOUND(
         Chart* ca[10];
         for (int i=0;i<10;i++)
            {
            jdn = g_test_rand_double_range( 2435000.0, 2460000.0 );
            lat = g_test_rand_double_range( -180.0, 180.0 );
            lon = g_test_rand_double_range( -180.0, 180.0 );
            ca[i] = make_chart( "test",jdn,lat,lon );
            SANITY_TEST_CHART( tc );
            }
         for (int i=0;i<10;i++) { dump_chart( ca[i] ); }
         );
      );
/*
/ulb/swetest -b30.9.1997 -n1 -s1 -fpPlbRs -pd -eswe -utc14.11:45 -geopos47.3412,8.5772 
date (dmy) 30.9.1997 greg.   14:00:00.379 UT    version 2.09.03
UT:  2450722.083337721     delta t: 62.804908 sec
TT:  2450722.084064630
geo. long 47.341200, lat 8.577200, alt 0.000000
Epsilon (t/m)     23°26'13.9871   23°26'22.4612
Nutation          -0° 0' 3.9503   -0° 0' 8.4741
0 Sun              187.4420610  -0.0000329    1.001340491   0.9832472
1 Moon             175.3105808   0.5096621    0.002715225  11.7943297
2 Mercury          177.0345580   1.8715475    1.274859543   1.7908944
3 Venus            231.2516712  -1.9136587    0.944007358   1.1342469
4 Mars             241.1545547  -1.0663118    1.790740596   0.7004553
5 Jupiter          312.1890164  -0.9824110    4.408857093  -0.0253421
6 Saturn            17.6599719  -2.7562113    8.406475107  -0.0770806
7 Uranus           304.8163321  -0.6355571   19.352420040  -0.0114528
8 Neptune          297.2003221   0.3958303   29.795011104  -0.0047181
9 Pluto            243.4607600  12.1247212   30.543026115   0.0253703
10 mean Node       168.6199570   0.0000000    0.002569555  -0.0529937
11 true Node       169.7795649   0.0000000    0.002715947  -0.0070778
12 mean Apogee     171.6641150   0.2739842    0.002710625   0.1107051
*/
   TRIAL("Sweph birth chart = astro.com/swetest",
      tc = make_chart( "sweph", 2450722.083337721, 47.341200, 8.5772 );
//0 Sun
      ENSURE( NEAR( tc->points[0].lon,187.4420610  ) );
      ENSURE( NEAR( tc->points[0].lat, -0.0000329  ) );
      ENSURE( NEAR( tc->points[0].dist, 1.001340491) );
      ENSURE( NEAR( tc->points[0].speed,0.9832472  ) );
//1 Moon --> this longitude is fake, site is 175.3105808
      ENSURE( NEAR( tc->points[1].lon,  175.3105812457) );
      ENSURE( NEAR( tc->points[1].lat,  0.5096621  ) );
      ENSURE( NEAR( tc->points[1].dist, 0.002715225) );
      ENSURE( NEAR( tc->points[1].speed,11.7943297 ) );
//2 Mercury
      ENSURE( NEAR( tc->points[2].lon,  177.0345580) );
      ENSURE( NEAR( tc->points[2].lat,  1.8715475  ) );
      ENSURE( NEAR( tc->points[2].dist, 1.274859543) );
      ENSURE( NEAR( tc->points[2].speed,1.7908944  ) );
//3 Venus
      ENSURE( NEAR( tc->points[3].lon,  231.2516712) );
      ENSURE( NEAR( tc->points[3].lat,  -1.9136587 ) );
      ENSURE( NEAR( tc->points[3].dist, 0.944007358) );
      ENSURE( NEAR( tc->points[3].speed,1.1342469  ) );
//4 Mars
      ENSURE( NEAR( tc->points[4].lon,  241.1545547) );
      ENSURE( NEAR( tc->points[4].lat,  -1.0663118 ) );
      ENSURE( NEAR( tc->points[4].dist, 1.790740596) );
      ENSURE( NEAR( tc->points[4].speed,0.7004553  ) );
//5 Jupiter
      ENSURE( NEAR( tc->points[5].lon,  312.1890164) );
      ENSURE( NEAR( tc->points[5].lat,  -0.9824110 ) );
      ENSURE( NEAR( tc->points[5].dist, 4.408857093) );
      ENSURE( NEAR( tc->points[5].speed,-0.0253421 ) );
//6 Saturn
      ENSURE( NEAR( tc->points[6].lon,  17.6599719 ) );
      ENSURE( NEAR( tc->points[6].lat,  -2.7562113 ) );
      ENSURE( NEAR( tc->points[6].dist, 8.406475107) );
      ENSURE( NEAR( tc->points[6].speed,-0.0770806 ) );
//7 Uranus
      ENSURE( NEAR( tc->points[7].lon,304.8163321  ) );
      ENSURE( NEAR( tc->points[7].lat,-0.6355571   ) );
      ENSURE( NEAR( tc->points[7].dist,19.352420040) );
      ENSURE( NEAR( tc->points[7].speed,-0.0114528 ) );
//8 Neptune
      ENSURE( NEAR( tc->points[8].lon,297.2003221  ) );
      ENSURE( NEAR( tc->points[8].lat,0.3958303    ) );
      ENSURE( NEAR( tc->points[8].dist,29.795011104) );
      ENSURE( NEAR( tc->points[8].speed,-0.0047181 ) );
//9 Pluto            243.4607600  12.1247212   30.543026115   0.0253703
      //
      dump_chart( tc );
      );
   //
   end_swiss_ephemeris();
END_TESTS
#endif //TEST
