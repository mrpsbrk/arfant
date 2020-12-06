/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file convert.c
 * Date conversions. Maybe some string comprehension attempts.
 * Also formating.
 */

#include "arfc.h"

/** jdn_of_gregorian() returns the astronomical date
 * @param (y)ear (m)onth (d)ay (h)our and (min)utes
 *
 * @return date in swiss ephemeris format or NaN
 */
double
jdn_of_gregorian( int y, int m, int d, int h, int min )
   {
   double jdn = 0.0;
   if ( m < 1 || m > 12 || d < 1 || d > 31 || h > 23 || min>60 )
      { return NAN; }
   double h_d = (double)h + ( (double)min / 60.0 );
   jdn = swe_julday( y,m,d,h_d,SE_GREG_CAL );
   return jdn;
   }

/** jdn_of_gdate() turns a glib GDate into astronomical date
 * @param a valid GDate
 *
 * @return date in swiss ephemeris format or NaN
 */
double
jdn_of_gdate( GDate* d )
   {
   guint32 num_of_days;
   const guint32 glib_jdn_delta = 1721425;
   /*difference between glib's jdn and the official
    *  -- it is the jdn of 1/1/1 */
   num_of_days = g_date_get_julian( d ) + glib_jdn_delta;
   return num_of_days - 0.5; //glib uses midnight, jdn midday
   }

/** jdn_of_gdatetime() turns a glib GDate into astronomical date
 * @param a valid GDateTime, must be in UTC
 *
 * @return date in swiss ephemeris format or NaN
 */
double
jdn_of_gdatetime( GDateTime* ut )
   {
   double n = jdn_of_gregorian( g_date_time_get_year( ut ),
                                g_date_time_get_month( ut ),
                                g_date_time_get_day_of_month( ut ),
                                g_date_time_get_hour( ut ),
                                g_date_time_get_minute( ut )
                              );
   return n;
   }

/** jdn_of_datestring() parses the string representation of a date.
 * @param a string representing a date, format following locale
 *
 * @return date in swiss ephemeris format or NaN
 */
double
jdn_of_datestring( char* str )
   {
   GDate a;
   g_date_set_parse( &a,str );
   if ( !g_date_valid( &a ) ) { return NAN;}
   //
   return jdn_of_gdate( &a );
   }

/** jdn_of_isotag() parses a string in ISO date tag format
 * The format is very strict.
 * @param string in format YYYY-MM-DDtHH-MM-SS:+TZ
 *
 * @return date in swiss ephemeris format or NaN
 */
double
jdn_of_isotag( char* tag )
   {
   GDateTime* lt = g_date_time_new_from_iso8601( tag, NULL );
   if ( lt == NULL ) { return NAN; };
   GDateTime* ut = g_date_time_to_utc( lt );
   double jdn = jdn_of_gdatetime( ut );
   g_date_time_unref( lt );
   g_date_time_unref( ut );
   return jdn;
   }

/** jdn_of_now() current time in astronomical date format
 * @param nothin
 *
 * @return current time in swiss ephemeris format
 */
double
jdn_of_now()
   {
   GDateTime* now = g_date_time_new_now_utc();
   double jdn = jdn_of_gdatetime( now );
   g_date_time_unref( now );
   return jdn;
   }

/** make_gdatetime_of_jdn() transforms the date-time into GLib format.
 * @param jdn is a double
 *
 * @return GDateTime struct. Must be freed with g_date_time_unref() or
 *         dump_gdatetime()
 */
GDateTime*
make_gdatetime_of_jdn( double jdn )
   {
   const double epoch = 2440587.500; //1970-01-01T00:00:00Z
   gint64 dif = ( jdn - epoch ) * 24*60*60;
   GDateTime * ut = g_date_time_new_from_unix_utc( dif );
   //--> does not UNREF!!!! must be unrefed later
   return ut;
   }

/** coords_of_string() parses a string into a set of geodesic coords.
 * @param a string like "latitude[|;,]longitude"
 *
 * @return a struct with two doubles that can be slapd into a Event
 */
Datum
coords_of_string( char* str )
   {
   Datum r = {};
   if( str == NULL ) { return r; }
   gboolean neg = FALSE;
   char * ptr = str;
   while ( *ptr != '\0' )
      {
      if( *ptr == ',' || *ptr == ';' ) { ptr++; break; }
      if( *ptr == 'S' || *ptr == 's' ) { neg = TRUE; }
      if( *ptr == '-' || isdigit(*ptr) )
         { r.lat = strtod(ptr,&ptr); continue; }
      ptr++;
      }
   if( neg ){ r.lat = fabs(r.lat)*-1.0; neg = FALSE; }
   while ( *ptr != '\0' )
      {
      if( *ptr == 'W' || *ptr == 'w' ) { neg = TRUE; }
      if( *ptr == '-' || isdigit(*ptr) )
         { r.lon = strtod(ptr,&ptr); continue; }
      ptr++;
      }
   if( neg ){ r.lon = fabs(r.lon)*-1.0; }
   return r;
   }

/** make_event_of_string( A ) tries to interpret a string into a set of
 * {name,date,time,lat,lon}. It then allocates a Event struct to fit
 * this data.
 * @param A: an array of char.
 *
 * @return pointer to Event structure, that must be dumped.
 */
extern Event* make_event_of_string( char* a )
   {
   const char delims[] = ",";
   double jdn = NAN;
   Event* ret = NULL;
   // copy string so we can chop it with strtok
   char* cpy = strdup( a );
   // make one token for each thing
   char* p_name = strtok( cpy, delims );
   char* p_date = strtok( NULL,delims );
   char* p_time = strtok( NULL,delims );
   char* p_geo  = strtok( NULL,"" );
   // try to process the things
   if( p_name && p_date && p_time )
      {
      jdn = jdn_of_datestring( p_date );
      jdn += jdn_of_timestring( p_time );
      }
   Datum d = coords_of_string( p_geo );
   // if we get some result, allocate the structu
   if( !isnan( jdn ) )
      {
      ret = calloc( sizeof(Event), 1 );
      ret->jdn = jdn;
      ret->name = strdup( p_name );
      ret->lat = d.lat;
      ret->lon = d.lon;
      }
   free( cpy );
   return ret;
   }

/** jdn_of_timestring() parses the string representation of a time and
 * optionally a timezone.
 *
 * Editorial note: there is probably a saner way to do this, ideally
 * using glib or some other library. The present version parses digits
 * one by one, so that we can do something with timezone tags. At least
 * this should be very permissive.
 * 
 * @param a string representing hours, minutes, and possibly seconds,
 *        optionally followed by a timezone identifier.
 *
 * @return a double from 0.0 to 1.0 representing the fraction of day
 */
double
jdn_of_timestring( char* str )
   {
   ///@todo bork on errors
   double pm = 0.0;
   double tzsign = -1.0;
   if( strstr(str,"PM") || strstr(str,"pm") || strstr(str,"p.m") || strstr(str,"P.M") )
      { pm=0.5; }
   int digits[10] = {}; // [ HH MM SS TZ TZ ]
   int index = 0;
   while ( *str!='\0' )
      {
      if( index<10 )
         {
         if( isdigit(*str) )
            {
            digits[index]=*str-48;
            index++;
            }
         if( (*str=='h'||*str=='H'||*str==':')&&(index%2==1) )
            {
            //expand single-digit numbers
            digits[index] = digits[index-1];
            digits[index-1] = 0;
            index++;
            }
         if (*str=='+' || *str=='-')
            {
            if ( *str == '-' ) { tzsign = +1.0; }
            if( index%2==1 )
               {
               //expand single-digit numbers
               digits[index] = digits[index-1];
               digits[index-1] = 0;
               }
            index=6;
            }
         str++;
         }
      }
   if( index%2==1 )
      {
      //expand single-digit numbers
      digits[index] = digits[index-1];
      digits[index-1] = 0;
      }
   //and then calculate based on the parsed digits
   double ret = pm;
   ret += digits[0] * ( 10.0 / 24.0 ); //hours, tens
   ret += digits[1] * (  1.0 / 24.0 ); //hours, units
   ret += digits[2] * ( 10.0 / (24.0*60.0) ); //minutes, tens
   ret += digits[3] * (  1.0 / (24.0*60.0) ); //minutes, units
   ret += digits[4] * ( 10.0 / (24.0*60.0*60.0) ); //seconds, tens
   ret += digits[5] * (  1.0 / (24.0*60.0*60.0) ); //seconds, units
   ret += digits[6] * tzsign * ( 10.0 / 24.0 ); //TZ, hours, tens
   ret += digits[7] * tzsign * (  1.0 / 24.0 ); //TZ, hours, units
   ret += digits[8] * tzsign * ( 10.0 / (24.0*60.0) ); //TZ, minutes, tens
   ret += digits[9] * tzsign * (  1.0 / (24.0*60.0) ); //TZ, minutes, units
   /*TESTIN
   printf( "%f -- %d %d %d %d %d %d %d %d %d %d\n", ret,
            digits[0], digits[1], digits[2],
            digits[3], digits[4], digits[5],
            digits[6], digits[7], digits[8],
            digits[9] );// */
   return ret;
   }

//####################################################################//
//---- TEST ----------------------------------------------------------//
//####################################################################//
#ifdef TEST
BEGIN_TESTS
   // The birthday for the Swiss Ephemeris is 1997-09-30:16:00:00+02
   const double swebday = 2450722.0833333335;
   const double ltz = 0.0; //local time zone
   //
   // TEST jdn_of_timestring()
   TRIAL("jdn_of_timestring() of known values",
      ENSURE( NEAR(jdn_of_timestring("10:23:45"), (37425.0/(24*60*60))) );
      ENSURE( NEAR(jdn_of_timestring("06:00PM"), 0.75+ltz) );
      ENSURE( NEAR(jdn_of_timestring("06:00pm"), 0.75+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "06h00" ), 0.25+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "6h" ), 0.25+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "6h18" ), 0.2625+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "6h9" ), 0.25625+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "6h9:00" ), 0.25625+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "6h00" ), 0.25+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "06:00" ), 0.25+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "18:00" ), 0.75+ltz) );
      ENSURE( NEAR(jdn_of_timestring( "21:00+03:00" ), 0.75) );
      ENSURE( NEAR(jdn_of_timestring( "15:00-03:00" ), 0.75) );
      );
   //
   // TEST: make_event_of_string( );
   MUST("make_event_of_string(invalid) borks",
      ( make_event_of_string( "aksjddfadshf" ) == NULL ) &&
      ( make_event_of_string( "name,Sept 30 1997" ) == NULL ) );
   TRIAL("make_event_of_string(swebday)",
      Event* ev = make_event_of_string( "sweph,9/30/1997,14:00,47.34N,8.57E" );
      AVOID( strcmp( ev->name, "sweph") );
      ENSURE( NEAR( ev->jdn, swebday ) );
      ENSURE( NEAR( ev->lat, 47.34 ) );
      ENSURE( NEAR( ev->lon, 8.57 ) );
      //printf( " lat: %f lon: %f\n", ev->lat, ev->lon );
      if(ev) { dump_event(ev); };
      );
   //
   // TEST: jdn_of_gregorian()
   MUST("jdn_of_gregorian(invalid) borks",
         isnan(   jdn_of_gregorian(0, 0, 0, 0, 0))
         && isnan(jdn_of_gregorian(1, 1, 1, 1,61))
         && isnan(jdn_of_gregorian(1, 1, 1,25, 1))
         && isnan(jdn_of_gregorian(1, 1, 1,24, 1))
         && isnan(jdn_of_gregorian(1, 1,32, 1, 1))
         && isnan(jdn_of_gregorian(1, 1, 0, 1, 1))
         && isnan(jdn_of_gregorian(1,13, 1, 1, 1))
         && isnan(jdn_of_gregorian(1, 0, 1, 1, 1)) );
   MUST("jdn_of_gregorian(swebday)",
         NEAR(jdn_of_gregorian(1997,9,30,14,0),swebday) );
   MUST("jdn_of_gregorian(n+1)-(n) is 1.0",
         NEAR(jdn_of_gregorian(1997,9,30,14,0) -
            jdn_of_gregorian(1997,9,29,14,0),
            1.0) );
   //
   // TEST: jdn_of_gdate( );
   TRIAL("jdn_of_gdate(swebday)",
      GDate* gd = g_date_new_dmy( 30, 9, 1997);
      ENSURE( !isnan( jdn_of_gdate( gd ) ) );
      ENSURE( NEAR( jdn_of_gdate( gd ), 2450721.5 ) );
      g_date_free( gd );
      );
   //
   // TEST: jdn_of_gdatetime(swebday);
   TRIAL("jdn_of_gdatetime(swebday)",
      GDateTime* gdt = g_date_time_new_utc(1997,9,30,14,0,0);
      ENSURE( !isnan( jdn_of_gdatetime( gdt ) ) );
      ENSURE( NEAR( jdn_of_gdatetime( gdt ), 2450722.0833333335 ) );
      dump_gdatetime( gdt );
      );
   //
   // TEST jdn_of_datestring()
   MUST("jdn_of_datestring(Invalid) borks",
         isnan(jdn_of_datestring("Invalid")));
   MUST("jdn_of_datestring(swebday)",
         jdn_of_datestring("1997-09-30") == 2450721.5 );
   //
   // TEST jdn_of_isotag()
   MUST("jdn_of_isotag(Invalid) borks",
         isnan(jdn_of_isotag("Invalid")));
   MUST("jdn_of_isotag(swebday)",
         NEAR(jdn_of_isotag("1997-09-30T16:00:00+02"),swebday));
   //
   // TEST jdn_of_now()
   MUST("jdn_of_now() > test's jdn",
         2459141.539 < jdn_of_now() );
   //
   // TEST: make_gdatetime_of_jdn( );
   TRIAL("make_gdatetime_of_jdn()",
      GDateTime* gdt = make_gdatetime_of_jdn( swebday );
      AVOID( g_date_time_get_year(gdt) != 1997 );
      AVOID( g_date_time_get_month(gdt) != 9 );
      AVOID( g_date_time_get_day_of_month(gdt) != 30 );
      AVOID( g_date_time_get_hour(gdt) != 14 );
      g_date_time_unref( gdt );
      );
   //
   // TEST: coords_of_string( char* );
   Datum dt;
   TRIAL("coords_of_string(invalid) is 0,0",
      dt = coords_of_string("invalid");
      AVOID( dt.lat != 0.0 );
      AVOID( dt.lon != 0.0 );
      dt = coords_of_string("");
      AVOID( dt.lat != 0.0 );
      AVOID( dt.lon != 0.0 );
      );
   TRIAL("coords_of_string(Rio de Janeiro)",
      dt = coords_of_string("-23,-43");
      //printf("\n\t\t\tdt.lat = %f dt.lon = %f\n\t\t\t\t\t\t", dt.lat, dt.lon);
      AVOID( dt.lat != -23.0 );
      AVOID( dt.lon != -43.0 );
      );
   TRIAL("coords_of_string(SW)",
      dt = coords_of_string("23S,43W");
      AVOID( dt.lat != -23.0 );
      AVOID( dt.lon != -43.0 );
      dt = coords_of_string("-24S,44W");
      AVOID( dt.lat != -24.0 );
      AVOID( dt.lon != -44.0 );
      dt = coords_of_string("25S,-45W");
      AVOID( dt.lat != -25.0 );
      AVOID( dt.lon != -45.0 );
      dt = coords_of_string("-26S,-46W");
      AVOID( dt.lat != -26.0 );
      AVOID( dt.lon != -46.0 );
      //printf("\n\t\t\tdt.lat = %f dt.lon = %f\n\t\t\t\t\t\t", dt.lat, dt.lon);
      );
   TRIAL("coords_of_string( with spaces )",
      dt = coords_of_string("  -23  ,  -43  ");
      //printf("\n\t\t\tdt.lat = %f dt.lon = %f\n\t\t\t\t\t\t", dt.lat, dt.lon);
      AVOID( dt.lat != -23.0 );
      AVOID( dt.lon != -43.0 );
      );
END_TESTS
#endif //TEST


