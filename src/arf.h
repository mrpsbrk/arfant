/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file arf.h
 *    main include for ARF
 *    defines the API
 *    most important are DATA TYPES
 **/

/** @mainpage Astrology Research Framework
 * Utility for astrology calculations and reports.
 *
 * There are many "frontends" for the Swiss Ephemeris (as there should
 * most certainly be, as it is awesome), but some things that should be
 * very simple imho were not, really, in any of them. My idea at first
 * was just to expoort chart calculations in a text format suitable to
 * be used in PostScript, to make fancy charts. I ended up incorporating
 * the Cairo graphics library, so it became an app in itself.
 *
 * Usually you'll get a Chart structure that contains almost all the
 * information you'll need, like point position, cusps and so on.
 **/

#ifndef ARF_H
#define ARF_H

//---- CONSTANTS -----------------------------------------------------//
#define SE_AST(X) ( SE_AST_OFFSET + X )
#define ARF_ERIS (SE_AST_OFFSET+136199)
#define SE_ERIS ARF_ERIS
#define SE_END -11
#define NAME_SIZE 24
#define SYMB_SIZE 4
#define EPSILON (1.0/3600000.0)

//---- DATA TYPES ----------------------------------------------------//
/** struct Datum represents a place in spherical coordinates.
 * It might be both geographical coords or sky coords, according to
 * where it is used. Many other structs begin exactly the same
 * to allow casting.
 **/
typedef struct Datum
   {
   double lon;
   double lat;
   }
Datum;

/** struct Event is a place, a time, and a name. */
typedef struct Event
   {
   double lon;
   double lat;
   double jdn;
   char* name;
   }
Event;

/** struct Point is everything that can show up in a chart, planets,
 * asteroids, fictional planets, etc.
 *
 * Houses are also represented by a Point structure. The fisrt value
 * @a data[0] is the house cusp. The next two values are available for
 * alternative house systems. The last position, which can also be
 * accessed as @a .def, has special values depending on the house.
 * 
 * house(1).def is the ascendant, even on systems where the first house
 *              cusp is something different.
 * house(10).def is the Midheaven.
 **/
typedef struct Point
   {
   union
      {
      double data[4];
      struct
         {
         double lon;
         double lat;
         double dist;
         double speed;
         };
      struct
         {
         double cusp;
         double alt1;
         double alt2;
         double def;
         };
      };
   int code;
   char symbol[SYMB_SIZE]; // a single UTF-8 char, should fit 3 bytes+\0
   char name[NAME_SIZE];
   }
Point;

/** struct Aspect has the angular diff & aspect code & strength */
typedef struct Aspect
   {
   int kind;
   float score;
   int point1;
   int point2;
   char symbol[SYMB_SIZE];
   double diff;
   }
Aspect;

/** struct Chart has an event and an array of cusps and points */
typedef struct Chart
   {
   Event* ev;
   Point* cusps;
   Point* points; //array of points
   Aspect* aspects;
   int pt_count;
   int cp_count;
   int sys_count;
   int asp_count;
   }
Chart;


//---- ITERATORS and ACCESSORS ---------------------------------------//
#define for_each_point(c) \
        for(Point * each = c->points; each<(c->points+c->pt_count);each++)
#define for_point_i(c) \
        for(int i = 0; i<(c->pt_count);i++)
#define for_each_house(c) \
        for(Point * each = (c->points-1); each>(c->points-13); each--)
#define house(x) points[ -(x) ]
#define housealt(n,sys) points[(-12*((sys+1)/4))-(n)].data[sys<3?sys:(sys+1)%4]
#define ascendant points[-1].def
#define midheaven points[-10].def
#define syscode(sys) points[(-12*((sys+1)/4))-1].symbol[sys<3?sys:(sys+1)%4]

//---- DATE & UNIT CONVERSIONS (in convert.c) ------------------------//
extern double jdn_of_gregorian( int y, int m, int d, int h, int min );
///@todo other calendars? jdn_of_arabic() _chinese() _hebrew() _etc()
extern double jdn_of_gdate  ( GDate* );
extern double jdn_of_gdatetime( GDateTime* d );
extern double jdn_of_datestring ( char* );
extern double jdn_of_timestring ( char* );
extern double jdn_of_isotag ( char* );
extern double jdn_of_now ();
extern Datum coords_of_string( char* );
extern Event* make_event_of_string( char* );
#define dump_event( e ) free(e->name); free( e );
// to interop with Glib
extern GDateTime* make_gdatetime_of_jdn( double );
#define dump_gdatetime( X ) g_date_time_unref( X )

//---- STRING FORMATTING (in stringify.c) ----------------------------//
extern size_t to_datetag( char*, double );
extern size_t to_roman( char*, int );
extern size_t to_symbol_utf( char*, int );
extern size_t to_zodiac_utf( char*, double );
extern size_t to_zodiac_ascii( char*, double );
extern size_t to_coords( char*, Event* );

//---- ASTROLOGY CALC FUNCS (in astro.c) -----------------------------//
extern void init_swiss_ephemeris(char* systems, int* points);
extern void set_housesystem( int );
extern void end_swiss_ephemeris();
extern Chart* make_chart( char* name, double jdn, double lat, double lon );
extern Chart* make_chart_of_event( Event* ev );
extern void dump_chart( Chart* );
extern Aspect to_aspect( double, double );
extern Aspect* make_aspects( Chart* );
extern void dump_aspects( Aspect* );
extern double nearest_return( Chart* c, int code, double ref_time );
extern double nearest_ingress( int code, double zlon, double ref_time );

//---- SERIALIZATION (in serialize.c) --------------------------------//
extern char* make_csv_list( Chart* );
extern char* make_c_literal( Chart* );
extern char* make_point_table( Chart*, char* );
extern char* make_house_table( Chart* );
extern char* make_aspect_table( Chart* );
//extern char* make_( Chart* );

//---- DRAWINGS (in draw.c) ------------------------------------------//
/** struct Figure contains all config needed for drawing */
typedef struct Figure
   {
   cairo_t* t; // should allow casting
   Chart* c;
   double asc;
   guint h;
   guint w;
   double r;
   double x; // of center point
   double y; // of center point
   double sz; // font size
   int l; //line type
   }
Figure;

extern void prep_code( Figure*, int );
extern void prep_gray( Figure*, double );
extern void prep_rgb ( Figure*, double, double, double );
extern void prep_transp ( Figure*, double, double, double,double );
extern void prep_font( Figure*, double );
extern void prep_line( Figure*, int type, double w );
// Drawing functions
extern void draw_placeholder(  Figure* );
extern void draw_line(  Figure*, double, double, double, double );
extern void draw_spoke( Figure*, double, double, double );
extern void draw_edge(  Figure*, double, double, double );
extern void draw_circle(Figure*, double );
extern void draw_glyph( Figure*, double, double, char* );
extern void draw_turn_glyph(Figure*, double, double, char* );
//extern void draw_ (Figure*, );
//-- rectangular coords:
extern void draw_text( Figure* F, double x, double y, double sz, char* txt );
extern void draw_paragraph(Figure* F, double x, double y, double sz, char* txt );
extern void draw_chart_details( Figure* F, double x, double y );
//
extern void draw_fleuron( Figure*, double, double, double sz );
extern void draw_node_head( Figure*, double, double, double sz );
///@todo draw_map() using **Natural Earth Projection**

//---- STRIPES (in draw.c) -------------------------------------------//
//extern void paint_stripes_test( Figure* );
///Painter is a function pointer that paints a Stripe of a chart.
typedef void Painter( Figure*, double, double);
///Stripe is a structure representing parts of a chart drawing.
typedef struct Stripe
   {
   Painter* func;
   double begin, end, width;
   int    over;
   } Stripe;
extern Painter spacer;
extern Painter border;
extern Painter axis;
extern Painter axis_decor;
extern Painter tics2;
extern Painter tics10;
extern Painter multi_tics;
extern Painter sign_divs;
extern Painter sign_glyphs;
extern Painter sign_glyphs_turned;
extern Painter house_divs;
extern Painter house_slabs;
extern Painter point_glyphs;
extern Painter point_balls;
extern Painter point_glyphs_distrib;
extern Painter point_pos_distrib;
extern Painter point_balls_distrib;
extern Painter point_image;
//
extern Painter noop;
extern Painter demarcador;
// new ones
extern Painter dot_dot_points;
extern Painter extra_house_sys;
extern Painter basic_aspects;
extern Painter fancy_aspects;
extern Painter zodiac_open;
//main stripe painter
extern void paint_stripes( Figure*, Stripe* bs );

#endif //ARF_H
