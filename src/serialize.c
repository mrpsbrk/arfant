/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file serialize.c
 * Allocates and fills long char-arrays (strings) representing Chart
 * data. */

#include "arfc.h"

/** make_csv_list() allocates a string with all the information from
 *     a chart.
 * @param a chart from where to take information
 *
 * @return a pointer to a string (char array) that must be freed
 */
char*
make_csv_list( Chart* c )
   {
   // allocate large string
   int linelen = 75;
   int ptlen   = c->pt_count + c->cp_count;
   char* rets = malloc( ptlen * linelen );
   char* sp = rets;
   // fill it with values from Chart *c
   Point* el = (Point*) c->cusps;
   for ( int i = 0; i < (c->pt_count+c->cp_count) ; i++ )
      {
      sp += sprintf( sp, "%4d", el[i].code );
      sp += sprintf( sp,
                     ", #%02hhX%02hhX%02hhX%02hhX, ",
                     el[i].symbol[0],
                     el[i].symbol[1],
                     el[i].symbol[2],
                     el[i].symbol[3] );
      sp += to_zodiac_ascii( sp, el[i].lon );
      //-- (to avoid full lon precision) sp -= 4;
      sp += sprintf( sp,
                     ", %.5f, %.5f, %.5f",
                     //", %+g, %+g, %+g",
                     //", %+.4e, %+.4e, %+.4e",
                     el[i].lat,
                     el[i].dist,
                     el[i].speed );
      ///@todo ???print char symbol[SYMB_SIZE];???
      sp += sprintf( sp, ", %s\n", el[i].name );
      }
   // realloc to final size (ie free rest)
   char* tmp = realloc( rets, sp - rets + 1 );
   enforce( "reallocate memory", tmp == rets );
   // return pointer to free-able string
   return rets;
   }

/** make_c_lieral() allocates a string with a c literal representation
 *    of chart data.
 * @param c: Pointer to a Chart structure.
 *
 * @return pointer to a char-array that must be freed.
 */
char*
make_c_literal( Chart* c )
   {
   // allocate large string
   int linelen = 200;
   int count = c->pt_count + c->cp_count;
   char* rets = calloc( sizeof(char), linelen * count );
   char* sp = rets;
   // fill it with values from Chart *c
   sp += sprintf( sp, "Point ps[] =\n{\n\t{\n" );
   for ( int i = 0; i<count; )
      {
      sp += sprintf( sp, "\t.data = {%8.8f,%8.8f,%8.8f,%8.8f},\n",
               c->cusps[i].data[0],
               c->cusps[i].data[1],
               c->cusps[i].data[2],
               c->cusps[i].data[3] );
      sp += sprintf( sp, "\t.code = %i,", c->cusps[i].code );
      sp += sprintf( sp, ".symbol = {%hhu,%hhu,%hhu,%hhu},",
               c->cusps[i].symbol[0],
               c->cusps[i].symbol[1],
               c->cusps[i].symbol[2],
               c->cusps[i].symbol[3] );
      sp += sprintf( sp, ".name = \"%s\"\n", c->cusps[i].name );
      i++;
      if ( i == count ) { sp += sprintf( sp, "\t}\n" ); }
      else { sp += sprintf( sp, "\t}, {\n" ); }
      }
   sp += sprintf( sp, "};\n" );
   sp += 4;
   // realloc to final size (ie free rest)
   enforce( "allocation was wide enoug", (sp-rets+1)<(linelen*count) );
   char* tmp = realloc( rets, sp - rets + 1 );
   enforce( "reallocate memory", tmp == rets );
   // return pointer to free-able string
   return rets;
   }

/** make_point_table() allocates a string with data about chart points.
 * @param c: Pointer to a Chart structure.
 *
 * @return pointer to a char-array that must be freed.
 */
char* make_point_table( Chart* c, char* fmt )
   {
   // this X_MACRO is the basic API for this func
   #define X_VARS \
   X('N',"Planet    ","%-*s", 0,namesz, each->name, ) \
   X('C',"SEph      ","%*i",  0,codesz, each->code, ) \
   X('O',"Sky longit","%*.3f",0,lonsz,  each->lon, ) \
   X('o',"Sky longit","%*.1f",0,lonsz-2,each->lon, ) \
   X('L',"Sky latitu","%*.3f",0,latsz,  each->lat, ) \
   X('l',"Sky latitu","%*.1f",0,latsz-2,each->lat, ) \
   X('S',"Speed long","%*.3f",0,spdsz,  each->speed, ) \
   X('s',"Speed long","%*.1f",0,spdsz-2,each->speed, ) \
   X('D',"Distance  ","%*.3f",0,distsz,  each->dist, ) \
   X('d',"Distance  ","%*.1f",0,distsz-2,each->dist, ) \
   X('Z',"Zodiac pos","%.*s", 0,10, buff, to_zodiac_ascii( buff, each->lon ) ) \
   X('z',"Zodiac pos","%.*s", 0,6,  buff, to_zodiac_ascii( buff, each->lon ) ) \
   X('U',"Zodiac pos","%.*s", 1,11, buff, to_zodiac_utf( buff, each->lon ) ) \
   X('u',"Zodiac pos","%.*s", 1,7,  buff, to_zodiac_utf( buff, each->lon ) ) \
   X('Y',"Symbol    ","%-*s", 2,symbsz, buff, snprintf(buff,4,"%s",each->symbol) )
   //(A, T, B,      H,C,  D,    E )
   // find out field sizes
   char buff[NAME_MAX];
   int i,l;
   char* sp;
   int latsz = 0;
   int lonsz = 0;
   int namesz = 0;
   int codesz = 0;
   int spdsz = 0;
   int distsz = 0;
   int symbsz = 0;
   for_each_point( c )
      {
      int temp;
      #define SZ(var,exp) temp=(exp);var=var>temp?var:temp;
      SZ( namesz, strlen( each->name ) );
      SZ( codesz, sprintf( buff, "%i", each->code ) );
      SZ( latsz, sprintf( buff, "%.3f", each->lat ) );
      SZ( lonsz, sprintf( buff, "%.3f", each->lon ) );
      SZ( spdsz, sprintf( buff, "%.3f", each->speed ) );
      SZ( distsz, sprintf( buff, "%.3f", each->dist ) );
      SZ( symbsz, sprintf( buff, "%s", each->symbol ) );
      #undef SZ
      }
   // find out line length
   int len = 0;
   for( i = 0; fmt[i]; i++ )
      {
      if( fmt[i]=='$' )
         {
         i++;
         #define X(A,T,B,H,C,D,E) case A: len += C; break;
         switch( fmt[i] )
            {
            X_VARS
            }
         #undef X
         }
      else { len++; }
      }
   // synth a separator
   char* separator = malloc( len+1 );
   sp = separator;
   for( i = 0; fmt[i]; i++ )
      {
      if( fmt[i]=='$' )
         {
         i++;
         #define X(A,T,B,H,C,D,E) case A: for(l=H;l<C;l++) *sp++='-'; break;
         switch( fmt[i] )
            {
            X_VARS
            }
         #undef X
         }
      else
      if ( fmt[i]=='|' )
         { *sp++ = '+'; }
      else
         { *sp++ = '-'; }
      }
   //*sp++ = '\0';
   sp[len] = '\0';
   // allocate large string
   int totalsz = (len+10) * (c->pt_count+4);
   char* rets = calloc( sizeof(char), totalsz );
   sp = rets;
   // fill header
   sp += sprintf( sp, "%s\n", separator );
   for( i = 0; fmt[i]; i++ )
      {
      if( fmt[i]=='$' )
         {
         i++;
         #define X(A,T,B,H,C,D,E) case A: sp+=sprintf(sp,"%*.*s",C-H,C-H,T); break;
         switch( fmt[i] )
            {
            X_VARS
            }
         #undef X
         }
      else
         { *sp++ = fmt[i]; }
      }
   sp += sprintf( sp, "\n%s\n", separator );
   // fill it with values from Chart *c
   for_each_point( c )
      {
      for( i = 0; fmt[i]; i++ )
         {
         if( fmt[i]=='$' )
            {
            i++;
            #define X(A,T,B,H,C,D,E) case A: E; sp+=sprintf(sp,B,C,D); break;
            switch( fmt[i] )
               {
               default: break;
               X_VARS
               }
            #undef X
            }
         else
            {
            *sp++ = fmt[i];
            }
         }
      sp += sprintf( sp, "\n" );
      }
   sp += sprintf( sp, "%s", separator );
   // realloc to final size (ie free rest)
   enforce( "allocation was wide enoug", (sp-rets+1)<(totalsz) );
   char* tmp = realloc( rets, sp - rets + 1 );
   enforce( "reallocate memory", tmp == rets );
   free( separator );
   // return pointer to free-able string
   return rets;
   #undef X_VARS
   }

/** make_house_table() allocates a string with data about house cusps.
 * @param c: Pointer to a Chart structure.
 *
 * @return pointer to a char-array that must be freed.
 */
char* make_house_table( Chart* c )
   {
   #define SPACER() sp += sprintf( sp, "+-------" ); \
      for( int s = 0; c->sys_count>s; s++ ) \
         { sp += sprintf( sp, "+----------" ); } \
      sp += sprintf( sp, "+\n" );
   char buf[NAME_SIZE];
   char* rets = malloc( (12 + 12*c->sys_count) * 16 );
   char* sp = rets;
   SPACER();
   sp += sprintf( sp, "| house |");
   for( int s = 0; c->sys_count>s; s++ )
      {
      sp += sprintf( sp, " %-8.8s |", swe_house_name( c->syscode(s) ) );
      }
   sp += sprintf( sp, "\n");
   SPACER();
   for( int i = 1; i<= 12; i++ )
      {
      to_roman( buf, i );
      sp += sprintf( sp, "| %-5s |", buf );
      for( int s = 0; c->sys_count>s; s ++ )
         {
         //to_zodiac_utf( buf, c->housealt(i,s) );
         to_zodiac_ascii( buf, c->housealt(i,s) );
         sp += sprintf( sp, " %.8s |", buf );
         }
      sp += sprintf( sp, "\n" );
      }
   SPACER();
   //--printf("strlen = %li; calc = %i\n", strlen( rets ), (12 + 12*syscount) * 16 );
   // realloc to final size (ie free rest)
   char* tmp = realloc( rets, sp - rets + 1 );
   enforce( "reallocate memory", tmp == rets );
   // return pointer to free-able string
   return rets;
   #undef SPACER
   }

/** make_aspect_table() allocates a string with data about aspects.
 * @param c: Pointer to a Chart structure.
 *
 * @return pointer to a char-array that must be freed.
 */
char* make_aspect_table( Chart* c )
   {
   return strdup("###NICE TABLE OF ASPECTS###\n");
   }
/* -- TWO OLD VERSIONS -------------------------------------------------
void
aspect_table( Chart* c )
   {
   int e = c->pt_count;
   for ( int i = 1; i < e; i++ )
      {
      printf( "%s\t", c->points[i].symbol );
      for ( int j = 0; j < i; j++ )
         {
         Aspect a = to_aspect( c->points[j].lon, c->points[i].lon );
         printf( "%s%4.0f\t", a.symbol, a.score );
         }
      printf( "\n" );
      }
   for ( int i=0; i<e-1; i++ )
      {
      printf( "\t%s", c->points[i].symbol );
      }
   printf( "\n" );
   }
------------------------------------------------------------------------
void
aspect_list( Chart* c )
   {
   char buf1[16], buf2[16];
   Aspect* list = make_aspects( c );
   for ( Aspect* asp = list; asp->kind!=0; asp++ )
      {
      to_zodiac_utf( buf1, c->points[asp->point1].lon );
      to_zodiac_utf( buf2, c->points[asp->point2].lon );
      printf( "%.7s %s %s %s %.7s (%4.2f)\n",
              buf1,
              c->points[asp->point1].symbol,
              asp->symbol,
              c->points[asp->point2].symbol,
              buf2,
              asp->score );
      }
   dump_aspects( list );
   }
----------------------------------------------------------------------*/

//####################################################################//
//---- TEST ----------------------------------------------------------//
//####################################################################//
#ifdef TEST
// DATA TO FEED THE TESTS -- longer than the program itself :P
///@todo transfer test data to a test file
Event ev =
   {
   .lon = 0.0,
   .lat = 0.0,
   .jdn = 2450722.0833333335,
   .name = "Swiss Ephemeris Birth date"
   };
Point ps[] =
   {  {
      .data = {66.89071887,66.89071887,66.89071887,0.0},
      .code = -12,.symbol= {80,84,75,0},  .name  = "XII"
   }, {
      .data = {37.40820635,37.40820635,37.40820635,0.0},
      .code = -11,.symbol= {80,84,75,0},  .name  = "XI"
   }, {
      .data = {5.50894333,5.50894333,5.50894333,5.50894333},
      .code = -10,.symbol= {80,84,75,0},  .name  = "X"
   }, {
      .data = {333.11860420,333.11860420,333.11860420,0.0},
      .code = -9,.symbol= {80,84,75,0},   .name  = "VIIII"
   }, {
      .data = {302.77336908,302.77336908,302.77336908,0.0},
      .code = -8,.symbol= {80,84,75,0},   .name  = "VIII"
   }, {
      .data = {274.64161015,274.64161015,274.64161015,0.0},
      .code = -7,.symbol= {80,84,75,0},   .name  = "VII"
   }, {
      .data = {246.89071887,246.89071887,246.89071887,274.64161015},
      .code = -6,.symbol= {80,84,75,0},   .name  = "VI"
   }, {
      .data = {217.40820635,217.40820635,217.40820635,94.64161015},
      .code = -5,.symbol= {80,84,75,0},   .name  = "V"
   }, {
      .data = {185.50894333,185.50894333,185.50894333,5.05690861},
      .code = -4,.symbol= {80,84,75,0},   .name  = "IIII"
   }, {
      .data = {153.11860420,153.11860420,153.11860420,0.0},
      .code = -3,.symbol= {80,84,75,0},   .name  = "III"
   }, {
      .data = {122.77336908,122.77336908,122.77336908,94.64161015},
      .code = -2,.symbol= {80,84,75,0},   .name  = "II"
   }, {
      .data = {94.64161015,94.64161015,94.64161015,94.64161015},
      .code = -1,.symbol= {80,84,75,0},   .name  = "I"
   }, {
      .data = {223.91622326,-0.00001732,0.99126426,1.00276546},
      .code = 0,.symbol= {226,152,137,0}, .name  = "Sun"
   }, {
      .data = {102.09461377,1.93300136,0.00263551,12.44023785},
      .code = 1,.symbol= {226,152,189,0}, .name  = "Moon"
   }, {
      .data = {206.31411842,2.01032526,0.86449770,0.38043693},
      .code = 2,.symbol= {226,152,191,0}, .name  = "Mercury"
   }, {
      .data = {190.72698637,1.69709458,1.29945752,1.22212534},
      .code = 3,.symbol= {226,153,128,0}, .name  = "Venus"
   }, {
      .data = {15.67825466,-1.41465331,0.48799564,-0.10900015},
      .code = 4,.symbol= {226,153,130,0}, .name  = "Mars"
   }, {
      .data = {291.75403306,-0.45007735,5.40596535,0.15097420},
      .code = 5,.symbol= {226,153,131,0}, .name  = "Jupiter"
   }, {
      .data = {296.49275459,-0.34167462,10.24766234,0.05982047},
      .code = 6,.symbol= {226,153,132,0}, .name  = "Saturn"
   }, {
      .data = {38.46903587,-0.47054990,18.79253346,-0.04092120},
      .code = 7,.symbol= {226,153,133,0}, .name  = "Uranus"
   }, {
      .data = {348.31212908,-1.10698054,29.35695242,-0.01258900},
      .code = 8,.symbol= {226,153,134,0}, .name  = "Neptune"
   }, {
      .data = {292.74323199,-1.13236781,34.50007677,0.01566892},
      .code = 9,.symbol= {226,153,135,0}, .name  = "Pluto"
   }  };
Chart tc = 
   {
   .ev = &ev,
   .cusps = ps,
   .points = ps + 12,
   .pt_count = 10,
   .cp_count = 12
   };

BEGIN_TESTS
//---- MAKE_POINT_TABLE()
   TRIAL( "bounds check make_house_table()",
      BOUND(
         char* buf = make_point_table(&tc, "||$C$Z|$N|");
         CLOBBER( buf );
         PROBE( buf );
         free(buf);
         );
      );
   ///@todo test with 4-full chart->symbol
//---- MAKE_HOUSE_TABLE()
   TRIAL( "bounds check make_house_table()",
      BOUND(
         char* buf = make_house_table(&tc);
         CLOBBER( buf );
         PROBE( buf );
         free(buf);
         );
      );
//---- MAKE_CSV_LIST()
   TRIAL( "bounds check make_csv_list()",
      BOUND(
         char* buf = make_csv_list(&tc);
         CLOBBER( buf );
         PROBE( buf );
         free(buf);
         );
      );
//---- MAKE_C_LITERAL()
   TRIAL( "bounds check make_c_literal()",
      BOUND(
         char* buf = make_c_literal(&tc);
         CLOBBER( buf );
         PROBE( buf );
         free(buf);
         );
      );
END_TESTS
#endif //TEST


