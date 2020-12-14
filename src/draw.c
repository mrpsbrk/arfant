/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file draw.c
 *    contains drawing functions, based on cairo.
 * 
 * Almost everything here passes around a pointer to a @ref Figure
 * struct, which has all sorts of useful values for drawing.
 *
 * The idea is that the @a draw_* functions take arguments in the same
 * format of Chart structures, so you can pass these values directly.
 **/

#include "arfc.h"

//-- COORDINATES TRANSFORMATION -- try to enforce use of those -------//
#define ANG( x ) M_PI+((x) - F->asc) * (-M_PI/180)
#define COORDX(r,a) (r) * cos( ANG( (a) )) + F->x
#define COORDY(r,a) (r) * sin( ANG( (a) )) + F->y
#define RECT(r,a) COORDX(r,a), COORDY(r,a)
//-- Radians and Degrees ---------------------------------------------//
#define R2D( a ) ( (a)*(180.0/M_PI) )
#define D2R( a ) ( (a)*(M_PI/180.0) )
//-- Distance between two angles -------------------------------------//
#define ADIST(M,N) 180.0-fabs( fabs( (M)-(N) ) - 180.0 )

//-- law of COSINES!!!!! ---------------------------------------------//
#define deg_of_l_at_r(L,R) R2D( acos( 1-(L)*(L)/(2*(R)*(R)) ) )
#define rad_of_l_at_r(L,R) acos( 1-(L)*(L)/(2*(R)*(R)) )
#define l_of_deg_at_r(A,R) sqrt( 2.0*(R)*(R)-2.0*(R)*(R)*cos(D2R(A)) )
#define l_of_rad_at_r(A,R) sqrt( 2.0*(R)*(R)-2.0*(R)*(R)*cos(A) )

//-- DEFINES for cosmetic reasons ------------------------------------//
#define cairo( command, ... ) cairo_ ## command( F->t ,## __VA_ARGS__ )
#define prep( command, ... ) prep_ ## command( F ,## __VA_ARGS__ )
#define draw( command, ... ) draw_ ## command( F ,## __VA_ARGS__ )

//-- UTILITIES -------------------------------------------------------//
static char* sign_utf[] =
   {
   "\u2648","\u2649","\u264A","\u264B","\u264C","\u264D",
   "\u264E","\u264F","\u2650","\u2651","\u2652","\u2653"
   };

double*
make_distrib( Figure* F, double dist )
   {
   double * ret = malloc( F->c->pt_count * sizeof (double) );
   enforce( "get space for point distribution", (ret) );
   for_point_i( F->c )
      {
      double d, delta;
      ret[i] = F->c->points[i].lon;
      for ( int k=0; F->c->pt_count>k; k++ )
         {
         if ( k==i ) { continue; }
         d = ADIST( F->c->points[i].lon , F->c->points[k].lon );
         if ( d<dist )
            {
            delta = ( dist-d )/2;
            if ( ( F->c->points[i].lon < F->c->points[k].lon ) ) { delta *= -1; }
            ret[i] += delta;
            }
         }
      }
   return ret;
   }

void
dump_distrib( double* distrib )
   {
   if( distrib ) { free( distrib ); }
   }


//-- PREPARATIONS ----------------------------------------------------//
//---- are setting stuff that do not cause pixels to paint

/** prep_code() adjusts many settings from a single @p code.
 * We assume positive codes to be swiss ephemeris numbers, to make it
 * easy to use in point iterations.
 *
 * @param F Pointer to Figure struct.
 * @param code Integer code. */
void
prep_code( Figure* F, int code )
   {
   //cairo( set_font_size, F->sz );
   prep_line( F, 0, 2.0 );
   switch ( code )
      {
      case -3:
         cairo( set_source_rgb, 0.75, 0.75, 0.75 );
         break;
      case -2:
         cairo( set_source_rgb, 0.25, 0.25, 0.25 );
         break;
      case -1:
         cairo( set_source_rgb, 0.0, 0.0, 0.0 );
         break;
      case SE_SUN:
         cairo( set_source_rgb, 1.0, 0.8, 0.0 );
         break;
      case SE_MOON:
         cairo( set_source_rgb, 0.82, 0.56, 0.15 ); //FLICTS
         break;
      case SE_MERCURY:
         cairo( set_source_rgb, 0.8, 0.4, 0.0 );
         break;
      case SE_VENUS:
         cairo( set_source_rgb, 0.9, 0.3, 0.7 );
         break;
      case SE_MARS:
         cairo( set_source_rgb, 0.8, 0.0, 0.1 );
         break;
      case SE_JUPITER:
         cairo( set_source_rgb, 0.7, 0.4, 0.1 );
         break;
      case SE_SATURN:
         cairo( set_source_rgb, 0.3, 0.5, 0.3 );
         break;
      case SE_URANUS:
         cairo( set_source_rgb, 0.0, 0.8, 0.8 );
         break;
      case SE_NEPTUNE:
         cairo( set_source_rgb, 0.2, 0.35, 0.7 );
         break;
      case SE_PLUTO:
         cairo( set_source_rgb, 0.6, 0.3, 0.1 );
         break;
      case SE_CERES:
         cairo( set_source_rgb, 0.0, 0.6, 0.2 );
         break;
      default:
         cairo( set_source_rgb, 0.0, 0.0, 0.0 );
      }
   }

void
prep_gray( Figure* F, double l )
   {
   cairo( set_source_rgb, l, l, l );
   }

void
prep_rgb( Figure* F, double r, double g, double b )
   {
   cairo( set_source_rgb, r, g, b );
   }

void
prep_transp( Figure* F, double r, double g, double b, double a )
   {
   cairo( set_source_rgba, r, g, b, a );
   }

void
prep_font( Figure* F, double s )
   {
   cairo( select_font_face,"sans-serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL );
   cairo( set_font_size, s );
   }

void
prep_line( Figure* F, int type, double wid )
   {
   double dashes[] = { 2.0, 8.0, 2.0, 2.0, 2.0, 2.0, 2.0 };
   double f = wid * (0.75+(type%100)/50.0);
   for ( int i=0; i<7; i++) { dashes[i] *= f; }
   F->l = type;
   /** LINE TYPES:
    * [0] normal (straight, full)
    * [1] dashed
    * [2] dotted
    * [100] wavy
    * [200] sawy
    * [300] zigy
    * [400] dipy
    */
   switch( type%10 )
      {
      case 1: /// --- --- --- --- --- ---
         cairo( set_dash, dashes+1, 2, 0.0 );
         break;
      case 2: /// ---   ---   ---   ---   ---
         cairo( set_dash, dashes+1, 1, 0.0 );
         break;
      case 3: /// - - - - - - - - - - - - -
         cairo( set_dash, dashes+2, 1, 0.0 );
         break;
      case 4: /// - -    - -    - -    - -
         cairo( set_dash, dashes, 4, 0.0 );
         break;
      case 5: /// -    -     -    -    -
         cairo( set_dash, dashes, 2, 0.0 );
         break;
      case 6: /// ---- - ---- - ---- - ----
         cairo( set_dash, dashes+1, 4, 0.0 );
         break;
      case 7: /// ---- - - ---- - - ---- - -
         cairo( set_dash, dashes+1, 6, 0.0 );
         break;
      default:
         cairo( set_dash, dashes, 0, 0.0 );
      }
   cairo( set_line_width, wid );
   }

//-- TRACE -----------------------------------------------------------//
//---- is like drawing, but does not call stroke() or paint()
//### ATTENTION THOSE USE RECTANGULAR COORDINATES (should NOT be extern)
intern
void
trace_line( Figure* F, double x1, double y1, double x2, double y2 )
   {
   cairo( move_to, x1, y1 );
   cairo( line_to, x2, y2 );
   }

intern
void
trace_dipy_line( Figure* F, double x1, double y1, double x2, double y2 )
   {
   double pr = (F->l%100)/10.0;
   cairo( move_to, x1, y1 );
   cairo( curve_to,
      (x1*pr+F->x)/(1.0+pr), (y1*pr+F->y)/(1.0+pr),
      (x2*pr+F->x)/(1.0+pr), (y2*pr+F->y)/(1.0+pr),
      x2, y2 );
   }

intern
void
trace_wavy_line( Figure* F, double xA, double yA, double xB, double yB )
   {
   //deltas
   double dist = sqrt( pow( xA-xB,2.0 ) + pow( yA-yB,2.0 ) );
   double f = dist / F->sz;
   double Dx = 0.5 * ( yB - yA ) / f;
   double Dy = 0.5 * ( xB - xA ) / f;
   // amount of waves to make
   f = ceil( f * (0.75+(F->l%100)/50.0) );
   //
   cairo( move_to, xA, yA );
   for ( double i = 1.0, k = f-1.0; i <= f; i++, k-- )
      {
      // find (N)ext point
      double xN = ( xA*k + xB*i ) / f;
      double yN = ( yA*k + yB*i ) / f;
      //find (M)idpoint
      double xM = ( xA*( 2*k+1 ) + xB*( 2*i-1 ) ) / (2*f);
      double yM = ( yA*( 2*k+1 ) + yB*( 2*i-1 ) ) / (2*f);
      // trace a bit over midpoint, a bit bellow
      cairo( curve_to, xM + Dx, yM - Dy, xM - Dx, yM + Dy, xN, yN );
      }
   }

intern
void
trace_sawy_line( Figure* F, double xA, double yA, double xB, double yB )
   {
   //deltas
   double dist = sqrt( pow( xA-xB,2.0 ) + pow( yA-yB,2.0 ) );
   double f = dist / F->sz;
   double Dx = 0.1 * ( yB - yA ) / f;
   double Dy = 0.1 * ( xB - xA ) / f;
   //amount of zigs to zag
   f = ceil( f * (0.75+(F->l%100)/50.0) ) * 2.0;
   //
   cairo( move_to, xA, yA );
   for ( double i = 1.0, k = f-1.0; i < f; i+=2, k-=2 )
      {
      // find (M)idpoint
      double xM = ( xA*k + xB*i ) / f;
      double yM = ( yA*k + yB*i ) / f;
      // trace a bit over midpoint, a bit bellow
      cairo( line_to, xM + Dx, yM - Dy );
      cairo( line_to, xM - Dx, yM + Dy );
      }
   cairo( line_to, xB, yB );
   }

intern
void
trace_zigy_line( Figure* F, double xA, double yA, double xB, double yB )
   {
   //deltas
   double dist = sqrt( pow( xA-xB,2.0 ) + pow( yA-yB,2.0 ) );
   double f = 2.5 * dist / F->sz;
   double Dx = 0.33 * ( yB - yA ) / f;
   double Dy = 0.33 * ( xB - xA ) / f;
   //amount of zigs to zag
   f = ceil( f * (0.75+(F->l%100)/50.0) ) * 2.0;
   //
   cairo( move_to, xA, yA );
   double i = 1.0, k = f-1.0;
   int b = 0;
   while ( i < f )
      {
      // find (M)idpoint
      double xM = ( xA*k + xB*i ) / f;
      double yM = ( yA*k + yB*i ) / f;
      // trace a bit over midpoint, a bit bellow
      if(b) { cairo( line_to, xM + Dx, yM - Dy ); }
      else  { cairo( line_to, xM - Dx, yM + Dy ); }
      i+=2;
      k-=2;
      b=!b;
      }
   cairo( line_to, xB, yB );
   }

//-- DRAW functions --------------------------------------------------//
//----push pixels to screen
void
draw_placeholder( Figure* F )
   {
   double r = ( F->w + F->h ) / 5;
   cairo( move_to, 0,    0 );
   cairo( line_to, F->w, F->h );
   cairo( line_to, F->w, 0 );
   cairo( line_to, 0,    F->h );
   cairo( move_to, F->x + r, F->y );
   cairo( arc,     F->x, F->y, r, 0, 2*M_PI );
   cairo( stroke );
   }

void
draw_line (Figure* F, double r1, double z1, double r2, double z2 )
   {
   // convert to rectangular coords
   double a = ANG( z1 );
   double b = ANG( z2 );
   double xA = F->x + r1 * cos(a);
   double yA = F->y + r1 * sin(a);
   double xB = F->x + r2 * cos(b);
   double yB = F->y + r2 * sin(b);
   // call tracing func depending on line type
   if ( F->l < 100 )
      { trace_line( F, xA, yA, xB, yB ); }
   else if ( F->l < 200 )
      { trace_wavy_line( F, xA, yA, xB, yB ); }
   else if ( F->l < 300 )
      { trace_sawy_line( F, xA, yA, xB, yB ); }
   else if ( F->l < 400 )
      { trace_zigy_line( F, xA, yA, xB, yB ); }
   else if ( F->l < 500 )
      { trace_dipy_line( F, xA, yA, xB, yB ); }
   cairo( stroke );
   }

void
draw_spoke(Figure* F, double r1, double r2, double a )
   {
   double ar = ANG( a );
   cairo( move_to, F->x+r1*cos( ar ), F->y+r1*sin( ar ) );
   cairo( line_to, F->x+r2*cos( ar ), F->y+r2*sin( ar ) );
   cairo( stroke );
   }

void
draw_slab(Figure* F, double r1, double z1, double r2, double z2 )
   {
   //double rx = r1 * 0.9651; // sin( 2 degree )
   double rx = r1 - l_of_deg_at_r( 2.0 , r1 );
   cairo( move_to, RECT(rx,z1) );
   cairo( line_to, RECT(r2,z1) );
   cairo( arc, F->x, F->y, r2, ANG(z1), ANG(z2) );
   cairo( line_to, RECT(rx,z2) );
   cairo( arc, F->x, F->y, r1, ANG(z2-2), ANG(z1+2) );
   cairo( close_path );
   cairo( stroke );
   }

void
draw_edge (Figure* F, double r, double a1, double a2 )
   {
   double ar1 = ANG( a1 );
   double ar2 = ANG( a2 );
   cairo( move_to, F->x + r * cos(ar1), F->y + r * sin(ar1) );
   cairo( line_to, F->x + r * cos(ar2), F->y + r * sin(ar2) );
   cairo( stroke );
   }

void
draw_circle(Figure* F, double r )
   {
   cairo( move_to, F->x + r, F->y );
   cairo( arc,     F->x, F->y, r, 0, 2*M_PI );
   cairo( stroke );
   }

void
draw_glyph(Figure* F, double r, double a, char* txt )
   {
   double ar = ANG( a );
   cairo_text_extents_t exts;
   cairo_text_extents( F->t, txt, &exts );
   cairo( move_to,
          F->x + r * cos(ar) - exts.x_bearing - exts.width/2,
          F->y + r * sin(ar) - exts.y_bearing - exts.height/2 );
   cairo( show_text, txt );
   }

void
draw_glyph_turned(Figure* F, double r, double a, char* txt )
   {
   double ar = ANG( a );
   // housekeeping
   cairo_matrix_t save;
   cairo(get_matrix, &save );
   cairo_text_extents_t exts;
   cairo(text_extents, txt, &exts );
   //
   cairo( translate, F->x + r * cos(ar), F->y + r * sin(ar) );
   cairo( rotate, 0.5 * M_PI + ar );
   cairo( move_to, 0 - exts.x_bearing - exts.width/2, 0 - exts.y_bearing - exts.height/2);
   cairo( show_text, txt );
   cairo( set_matrix, &save );
   }

void
draw_image( Figure* F, double r, double z, double l, char* src )
   {
   //check if the file exists
   if ( g_file_test( src, G_FILE_TEST_EXISTS ) )
      {
      //preserve transformation
      cairo_matrix_t save;
      cairo(get_matrix, &save );
      //
      cairo_surface_t * img1;
      img1 = cairo_image_surface_create_from_png ( src );
      int iw = cairo_image_surface_get_width(  img1 );
      int ih = cairo_image_surface_get_height( img1 );
      cairo( translate, RECT( r, z ) );
      cairo( scale, l/iw, l/ih);
      cairo( translate, -0.5*iw, -0.5*ih );
      cairo_set_source_surface ( F->t, img1, 0, 0);
      cairo_paint ( F->t );
      cairo_surface_destroy ( img1 );
      //preserve transformation
      cairo( set_matrix, &save );
      }
   }

intern void
draw_arc_2pt_r( Figure* F, double r1, double a1, double r2, double a2, double r )
   {
   double x1 = COORDX(r1,a1);
   double y1 = COORDY(r1,a1);
   double x2 = COORDX(r2,a2);
   double y2 = COORDY(r2,a2);
   // if r too small, make r = dist
   double dist = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
   if ( r < (dist/2.0) ) { r = dist/1.9; }
   //
   double xM = ( x1 + x2 ) / 2.0;
   double yM = ( y1 + y2 ) / 2.0;
   double q = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
   double xC = xM + sqrt(r*r-(q/2.0)*(q/2.0))*(y1-y2)/q;
   double yC = yM + sqrt(r*r-(q/2.0)*(q/2.0))*(x2-x1)/q;
   cairo( arc, xC, yC, r, 0, 2*M_PI );
   //cairo( arc, xC, yC, r, asin( (x2-xC)/r ), asin( (x1-xC)/r ) );
   cairo( stroke );
   }

void draw_fleuron( Figure* F, double r, double z, double sz )
   {
   const double ds[][6] =
      {
         {   0,   0,   0,   0,  80,   0 },
         { 110,   0, 110,  20, 110,  30 },
         { 110,  40, 110,  50, 100,  50 },
         {  90,  50,  80,  40,  90,  30 },
         {   0,   0,   0,   0,  90,  30 }, //0
         {  80,  40,  90,  50, 100,  50 }, //1
         { 110,  50, 120,  40, 120,  30 }, //2
         { 120,  10, 110,   0,  80,   0 }, //3
         {  20,   0,  30,  60,   0,  60 }, //4
         {  30,  60,  20, 120,  80, 120 }, //5
         { 110, 120, 120, 110, 120,  90 }, //6
         { 120,  80, 110,  70, 100,  70 }, //7
         {  90,  70,  80,  80,  90,  90 }, //8
         {  80,  80,  90,  70, 100,  70 }, //7'
         { 110,  70, 110,  80, 110,  90 }, //9
         { 110, 100, 110, 120,  80, 120 }, //5'
         {  60, 120,  50, 110,  50,  90 }, //10
         {  50,  70,  60,  60,  80,  60 }, //11
         {  60,  60,  50,  50,  50,  30 }, //12
         {  50,  10,  60,   0,  80,   0 }, //3'
         {  40,   0,  30,  40,  50,  60 }, //13
         {  30,  80,  40, 120,  80, 120 }
      };
   // housekeeping
   cairo_matrix_t save;
   cairo( get_matrix, &save );
   cairo( translate, F->x, F->y );
   cairo( rotate, ANG((z+180)) );
   cairo( translate, r * -1, 0 );
   cairo( scale, sz/100.0, sz/100.0 );
   //
   cairo( move_to, ds[0][4], ds[0][5]-60 );
   for( int i = 1; i<22; i++ )
      {
      cairo( curve_to,
         ds[i][0], ds[i][1]-60,
         ds[i][2], ds[i][3]-60,
         ds[i][4], ds[i][5]-60 );
      }
   //
   // housekeeping
   cairo( set_matrix, &save );
   cairo( stroke );
   };


void draw_node_head( Figure* F, double r, double tn, double sz )
   {
   double rP = sz / 2.0;
   double rF = rP * 0.2;
   
   double xP = COORDX( r-rP, tn );
   double yP = COORDY( r-rP, tn );
   double xF1 = cos( ANG( tn+135 ) ) * (rP+rF) + xP;
   double yF1 = sin( ANG( tn+135 ) ) * (rP+rF) + yP;
   double xF2 = cos( ANG( tn-135 ) ) * (rP+rF) + xP;
   double yF2 = sin( ANG( tn-135 ) ) * (rP+rF) + yP;
   
   prep( line, 0, 2 );
   cairo( arc_negative, xF1, yF1, rF, ANG(tn), ANG( tn+315 ) );
   cairo( arc, xP, yP, rP, ANG(tn+135), ANG(tn-135) );
   cairo( arc_negative, xF2, yF2, rF, ANG(tn-315), ANG( tn ) );
   cairo( stroke );
   }



//---- DRAW funcs NOT ON POLAR COORDS!!!!!! --------------------------//
///@todo change name --- maybe place_text place_parag etc?
void
draw_text(Figure* F, double x, double y, double siz, char* txt )
   {
   prep_font( F, siz * F->sz );
   cairo_text_extents_t exts;
   cairo_text_extents( F->t, txt, &exts );
   if ( x < 0.0 ) { x = x + F->w - exts.width; }
   if ( y < 0.0 ) { y = y + F->h - exts.height; }
   cairo( move_to, x - exts.x_bearing, y - exts.y_bearing );
   cairo( show_text, txt );
   }

void
draw_paragraph(Figure* F, double x, double y, double sz, char* txt )
   {
   char * cp = strdup( txt );
   char * ln = cp;
   // find out size
   int count = 1;
   while ( *ln ) { if (*ln == '\n') { count++; } ln++; }
   if ( y < 0 ) { y = y + F->h - (F->sz*sz*count); }
   ///@todo accept negative x values
   //
   cairo( select_font_face,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL );
   cairo( set_font_size, sz * F->sz );
   //
   double off=0.0;
   ln = strtok( cp, "\n" );
   while ( ln )
      {
      cairo( move_to, x, y + off );
      cairo( show_text, ln );
      off += sz * F->sz;
      ln = strtok( NULL, "\n" );
      }
   free( cp );
   }

void
draw_chart_details( Figure* F, double x, double y )
   {
   char txt[32];
   draw_text( F, x, y, 1.5, F->c->ev->name );
   to_datetag( txt, F->c->ev->jdn );
   draw_text( F, x, y+(1.85*F->sz), 0.75, txt );
   to_coords( txt, F->c->ev );
   draw_text( F, x, y+(2.75*F->sz), 0.75, txt );
   }

//-- STRIPES ---------------------------------------------------------//
//---- are bands that run around the chart
//---- should have format:
//----                    void function(Figure*, radius1, radius2)

//---- BASIC STRIPES -------------------------------------------------//
void spacer( Figure* F, double r1, double r2 )
   {
   static int useless = 0;
   useless++;
   //puts("dis func does nada");
   }

void demarcador( Figure* F, double r1, double r2 )
   {
   prep( code, -1 );
   prep( transp, 1.0, 0.0, 0.0, 0.5 );
   draw( circle, r1 );
   prep( transp, 0.0, 1.0, 0.0, 0.5 );
   draw( circle, r2 );
   }

void border( Figure* F, double r1, double r2 )
   {
   draw( circle, r1 );
   draw( circle, r2 );
   }

void axis( Figure* F, double r1, double r2 )
   {
   double r = ( r1 + r2 ) / 2.0;
   double as = F->c->house(1).def;
   double mc = F->c->house(10).def;
   double tn = F->c->house(12).def;
   prep( code, -1 );
   prep( line, 36, 1.5 );
   draw( line, r, as, r, as+180 );
   draw( line, r, mc, r, mc+180 );
   draw( line, r, tn, r, tn+180 );
   }

void axis_decor( Figure* F, double r1, double r2 )
   {
   //demarcador( F, r1, r2 );
   double sz = fabs( r1-r2 );
   prep( code, -1 );
   draw( fleuron, r1, F->c->ascendant, sz );
   draw( fleuron, r1, F->c->midheaven, sz );
   draw( node_head, r1, F->c->house(12).def, sz );
   }

void arrows( Figure* F, double r1, double r2 )
   {
   // ugly, ugly arrows
   draw( line, r1, F->c->house(1).def+2.0, r2, F->c->house(1).def );
   draw( line, r1, F->c->house(1).def-2.0, r2, F->c->house(1).def );
   draw( line, r1, F->c->house(10).def +2.0, r2, F->c->house(10).def );
   draw( line, r1, F->c->house(10).def -2.0, r2, F->c->house(10).def );
   }

void tics2( Figure* F, double r1, double r2 )
   {
   for ( double i = 0.0; i < 360.0; i += 2.0 )
      {
      trace_line( F, RECT(r1, i), RECT(r2, i) );
      }
   cairo( stroke );
   }

void tics5( Figure* F, double r1, double r2 )
   {
   for ( double i = 0.0; i < 360.0; i += 5.0 )
      {
      trace_line( F, RECT(r1, i), RECT(r2, i) );
      }
   cairo( stroke );
   }

void tics10( Figure* F, double r1, double r2 )
   {
   for ( double i = 0.0; i < 360.0; i += 10.0 )
      {
      trace_line( F, RECT(r1, i), RECT(r2, i) );
      }
   cairo( stroke );
   }

void multi_tics( Figure* F, double r1, double r2 )
   {
   double m1 = ( r1 + r1 + r1 + r2)/4.0;
   double m2 = ( r1 + r2 + r2 + r2)/4.0;
   double p1 = ( m1 + m1 + m2)/3.0;
   double p2 = ( m1 + m2 + m2)/3.0;
   for ( int i = 0; i < 360; i += 1 )
      {
      switch ( i%30 )
         {
         case 0:
            draw( spoke, r1, r2, i );
            break;
         case 10:
         case 20:
            draw( spoke, m1, m2, i );
            break;
         case 5:
         case 15:
         case 26:
            break;
         default:
            draw( spoke, p1, p2, i );
         }
      }
   }

void sign_divs( Figure* F, double r1, double r2 )
   {
   for ( double i = 0.0; i < 360.0; i += 30.0 )
      {
      trace_line( F, RECT(r1, i), RECT(r2, i) );
      }
   cairo( stroke );
   }

void sign_glyphs( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2)*0.85 );
   for ( int i = 0; i < 12; i++ )
      {
      draw( glyph, (r1+r2)/2, 30.0 * i + 15.0, sign_utf[i] );
      }
   }

void sign_glyphs_turned( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2)*0.85 );
   for ( int i = 0; i < 12; i++ )
      {
      draw( glyph_turned, (r1+r2)/2, 30.0 * i + 15.0, sign_utf[i] );
      }
   }

void house_divs( Figure* F, double r1, double r2 )
   {
   for_each_house( F->c )
      {
      draw( spoke, r1, r2, each->cusp );
      }
   cairo( stroke );
   }

void point_glyphs( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2)*0.85 );
   for_each_point( F->c )
      {
      draw( glyph, (r1+r2)/2, each->lon, each->symbol );
      }
   }

void point_balls( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2)*0.85 );
   for_each_point( F->c )
      {
      draw( glyph, (r1+r2)/2, each->lon, "\u25CF" );
      }
   }

/*//

void point_glyphs_distrib( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2)*0.85 );
   for_point_i( F->c )
      {
      draw( glyph, (r1+r2)/2, F->distrib[i], F->c->points[i].symbol );
      }
   }

void point_pos_distrib( Figure* F, double r1, double r2 )
   {
   char buff[4];
   double rA = (r1+r1+r1+r2)/4.0;
   double rB = (r1+r2+r2+r2)/4.0;
   for_point_i( F->c )
      {
      int l = (int) F->c->points[i].lon;
      prep( font, fabs(r1-r2)*0.333 );
      sprintf(buff, "%2.2i", l%30 );
      draw( glyph, rA, F->distrib[i], buff );
      prep( font, fabs(r1-r2)*0.425 );
      draw( glyph, rB, F->distrib[i], sign_utf[l/30] );
      }
   }

void point_balls_distrib( Figure* F, double r1, double r2 )
   {
   prep( font, fabs(r1-r2) );
   for_point_i( F->c )
      {
      draw( glyph, (r1+r2)/2, F->distrib[i], "\u25CF" );
      }
   }

//*/

///@todo order by planet size (for cool superpositions on conjunction)
void point_image( Figure* F, double r1, double r2 )
   {
   char basedir[] = "img/";
   char filename[32] = "";
   double s = fabs(r1-r2);
   //for_each_point( F->c )
      //{
      //sprintf( filename, "%s%i.png", basedir, each->code );
      //draw_image( F, (r1+r2)/2.0, each->lon, s, filename );
      //s *= 0.9;
      //}

   //----THIS IS VERY WRONG BECAUSE VERY HARDCODED
   //----int pts[] = { 0,1,2,3,4,17,5,6,7,8,9,11, SE_END };
   //----int index = { 0,1,2,3,4, 5,6,7,8,9,X,11, SE_END };
   int order[] = {7,0,1,6,8,9,3,4,2,10,5};
   for ( int i = 0; i<11; i++ )
      {
      Point * each = F->c->points + order[i];
      sprintf( filename, "%s%i.png", basedir, each->code );
      draw_image( F, (r1+r2)/2.0, each->lon, s, filename );
      s *= 0.88;
      }
   }

//---- FANCIER STRIPES -----------------------------------------------//

void zodiac_open( Figure* F, double r1, double r2 )
   {
   prep( code, -1 );
   prep( line, 0, 1.25 );
   double m1 = ( r1 + r1 + r1 + r2)/4.0;
   double m2 = ( r1 + r2 + r2 + r2)/4.0;
   double p1 = ( m1 + m1 + m2)/3.0;
   double p2 = ( m1 + m2 + m2)/3.0;
   for ( int i = 0; i < 360; i += 1 )
      {
      switch ( i%30 )
         {
         case 0:
            draw( spoke, r1, r2, i );
            break;
         case 10:
         case 20:
            draw( spoke, m1, m2, i );
            break;
         case 14:
         case 15:
         case 16:
            break;
         default:
            draw( spoke, p1, p2, i );
         }
      }
   ///@todo calculate r1' = 3 deg @ (r1+r2)/2
   double rM = (r1+r2)/2.0;
   double gw = l_of_deg_at_r( 2.2, rM );
   sign_glyphs_turned(F, rM+gw, rM-gw);
   }

void house_slabs( Figure* F, double r1, double r2 )
   {
   #define Hlon(n) F->c->house( (n) ).lon
   double aM;
   double rM = (r1+r2)/2.0;
   prep( font, fabs(r1-r2)/2.8 );
   cairo( new_path );
   for( int i=1; i<12; i++ )
      {
      prep( gray, 0.65 );
      draw( slab, r1, Hlon(i), r2, Hlon(i+1) );
      prep( gray, 0.90 );
      aM = ( Hlon(i)+Hlon(i+1) )/2.0;
      if ( Hlon(i)>Hlon(i+1) ) aM += 180.0;
      draw( glyph, rM, aM, F->c->house(i).name );
      }
   prep( gray, 0.65 );
   draw( slab, r1, Hlon(12), r2, Hlon(1) );
   //
   prep( gray, 0.90 );
   aM = ( Hlon(12)+Hlon(1) )/2.0;
   if ( Hlon(12)>Hlon(1) ) aM += 180.0;
   draw( glyph, rM, aM, F->c->house(12).name );
   //
   cairo( stroke );
   #undef Hlon
   }

void dot_dot_points( Figure* F, double r1, double r2 )
   {
   // set places for things
   double r2A = 0.85 * r2 + 0.15 * r1;
   double rP =  0.68 * r2 + 0.32 * r1;
   double rD =  0.40 * r2 + 0.60 * r1;
   double rS =  0.24 * r2 + 0.76 * r1;
   double r1A = 0.15 * r2 + 0.85 * r1;
   // ensure we have a distrib
   double psz = fabs(r1-r2)/3;
   double* ds =  make_distrib( F, deg_of_l_at_r(psz,rP) );
   //
   Point* ps = F->c->points;
   char buf[4];
   //
   for_point_i( F->c )
      {
      // colorify
      prep( code, ps[i].code );
      // dots
      prep( font, F->sz );
      draw( glyph, r1, ps[i].lon, "\u25CF" );
      draw( glyph, r2, ps[i].lon, "\u25CF" );
      // lines
      draw( line, r1, ps[i].lon, r1A, ds[i] );
      draw( line, r2, ps[i].lon, r2A, ds[i] );
      // info
      int l = (int) ps[i].lon;
      sprintf(buf, "%2.2i", l%30 );
      draw( glyph, rD, ds[i], buf );
      draw( glyph, rS, ds[i], sign_utf[l/30] );
      // planet symbol
      prep( font, psz );
      draw( glyph, rP, ds[i], ps[i].symbol );
      }
   dump_distrib( ds );
   }

void extra_house_sys( Figure* F, double r1, double r2 )
   {
   cairo( set_source_rgba, 0.55, 0.25, 0.75, 0.25 );
   prep( line, 0, 1.5 );
   //
   double dt = deg_of_l_at_r( fabs(r1-r2)/2.0,r1 );
   double rB = r1<r2? r1 : r2;
   double rT = (r1+r2)/2.0;
   double up = fabs(r1-r2)/F->c->sys_count;
   //
   for( int i = 1; i<F->c->sys_count; i++)
      {
      cairo( move_to, RECT(rT,F->c->housealt(1,i)) );
      for( int h=1; h<12; h++ )
         {
         cairo( arc_negative,
            F->x, F->y, rB,
            ANG( F->c->housealt(h,i)+dt ),
            ANG( F->c->housealt(h+1,i)-dt )
            );
         cairo( line_to, RECT(rT,F->c->housealt(h+1,i)) );
         }
      cairo( arc_negative,
         F->x, F->y, rB,
         ANG( F->c->housealt(12,i)+dt ),
         ANG( F->c->housealt(1,i)-dt )
         );
      cairo( close_path );
      cairo( stroke );
      rB += up;
      rT += up;
      }
   }

void basic_aspects( Figure* F, double r1, double r2 )
   {
   prep( code, -3 );
   prep( line, 3, 1.5 );
   for ( Aspect* each = F->c->aspects;
      each < (F->c->aspects + F->c->asp_count);
      each++
      )
      {
      Point* pt1 = &( F->c->points[ each->point1 ] );
      Point* pt2 = &( F->c->points[ each->point2 ] );
      draw( line, r1, pt1->lon, r1, pt2->lon );
      }
   }

void fancy_aspects( Figure* F, double r1, double r2 )
   {
   double conj_r = l_of_deg_at_r(4.0,r1);
   for ( Aspect* each = F->c->aspects;
      each < (F->c->aspects + F->c->asp_count);
      each++
      )
      {
      Point* pt1 = &( F->c->points[ each->point1 ] );
      Point* pt2 = &( F->c->points[ each->point2 ] );
      switch ( each->kind )
         {
         default:
         prep( line, 0, 1 );
         cairo( set_source_rgba, 0.0, 0.0, 0.0, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 1: // conjunction
         prep( line, 0, (each->score/25.0) );
         cairo( set_source_rgba, 0.99, 0.6, 0.1, (each->score/120.0) );
         if ( pt1->lon > pt2-> lon )
         draw( arc_2pt_r, r1, pt1->lon, r1, pt2->lon, conj_r );
         else
         draw( arc_2pt_r, r1, pt2->lon, r1, pt1->lon, conj_r );
         break;
         case 2: // opposition
         prep( line, 220, (each->score/25.0) );
         cairo( set_source_rgba, 0.85, 0.25, 0.25, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 4: // square
         prep( line, 320, (each->score/25.0) );
         cairo( set_source_rgba, 0.8, 0.3, 0.0, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 3: // trine
         prep( line, 110, (each->score/25.0) );
         cairo( set_source_rgba, 0.4, 0.7, 0.0, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 6: // sextile
         prep( line, 180, (each->score/25.0) );
         cairo( set_source_rgba, 0.2, 0.7, 0.2, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 12: // inconjunct
         prep( line, 420, (each->score/25.0) );
         cairo( set_source_rgba, 0.8, 0.7, 0.3, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 5: // pentagons
         prep( line, 4, (each->score/30.0) );
         cairo( set_source_rgba, 0.5, 0.7, 0.9, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         case 7: // septile
         prep( line, 7, (each->score/30.0) );
         cairo( set_source_rgba, 0.8, 0.3, 0.9, (each->score/120.0) );
         draw( line, r1, pt1->lon, r1, pt2->lon );
         break;
         }
      }
   }




//-- THE PAINTER LOOP ------------------------------------------------//
/** paint_stripes() is the main chart-making function.
 * It takes an array @p bs of @ref Stripe structures, each representing
 * a band that runs around the chart, like the zodiac or the planets or
 * midpoints.
 *
 * The Stripe structures are @a Painter function pointers together with
 * information about where this should be placed and how. Usually it
 * would be a name and a width, like:
 @code
   Stripe* my_stripes =
      (Stripe)[]
         {
            { basic_zodiac, .width=0.2 },
            { basic_points, .width=0.2 },
            { basic_houses, .width=0.2 },
            {}
         };
   paint_stripes( c, my_stripes );
 @endcode
 * 
 * The parameters that can be specified are .begin, .end, .width and
 * .over.
 * 
 * @param F Default pointer to Figure structure.
 * @param bs Array of Stripes.
 *
 * @return Nothing.
 * 
 * @todo double check this logic
 */
void
paint_stripes( Figure* F, Stripe* bs )
   {
   for ( int i = 0; bs[i].func; i++ )
      {
      //first, skip out-of-bounds references
      if ( abs(bs[i].over)>i )
         { continue; }
      //set dimensions from other stripes
      if ( bs[i].over < 0 )
         {
         bs[i].begin += bs[ i+bs[i].over ].begin;
         bs[i].end += bs[ i+bs[i].over ].end;
         }
      if ( bs[i].over > 0 )
         {
         bs[i].begin += bs[ bs[i].over-1 ].begin;
         bs[i].end += bs[ bs[i].over-1 ].end;
         }
      //pack stripes that are not relative
      if ( !(bs[i].begin>0.0) )
         {
         if ( bs[i].width && bs[i].end )
            { bs[i].begin = bs[i].end + bs[i].width; }
         else
            { bs[i].begin = i? bs[i-1].end : 1.0; }
         }
      //set widths
      if ( bs[i].width>0.0 )
         {
         bs[i].end = bs[i].begin - bs[i].width;
         }
      //call the Stripe->Painter function
      bs[i].func( F, bs[i].begin * F->r, bs[i].end * F->r );
      }
   }

//---- TEMPORARY STUFF -----------------------------------------------//
void noop( Figure* F, double r1, double r2 )
   {
   static int useless = 0;
   useless++;
   //puts("dis func does nada");
   }

void test_lines( Figure* F, double r1, double r2 )
   {
   prep( rgb, .9, .4, .7 );
   for ( int i=0; i<20; i++)
      {
      prep( line, i, 2.0 );
      draw( spoke, r1, r2, i*3.0);
      }
   for ( int i=0; i<20; i++)
      {
      prep( line, i*10+6, 2.0 );
      draw( spoke, r1, r2, (i*3.0)+270 );
      }
   for ( int i=100; i<400; i+=10)
      {
      prep( line, i, 2.0 );
      draw( line, r1, i*0.65, r2, i*0.65);
      }
   prep( gray, 0.0 );
   }

/*//
void paint_stripes_test( Figure* F )
   {
   Stripe mys[] =
      {
         { test_lines, .begin=0.25, .end=0.65 },
         { point_image, .begin=0.55, .end=0.89 },
         { axis,  .begin=0.95, .end=0.99 },
         { tics2,  .width=0.01 },
         { tics10, .width=0.02 },
         { sign_divs, .width=0.04 },
         { border, .from= -1, .to=-3 },
         { sign_glyphs, .from=-1, .to=-3 },
         { house_divs, .width=0.3 },
         { noop,   .as=-1000 },
         {}
      };
   paint_stripes( F, mys );
   }
//*/


//####################################################################//
//---- TEST ----------------------------------------------------------//
//####################################################################//
#ifdef TEST
#define for_each_stripe for( Stripe* each = ts; each->func; each++ )
BEGIN_TESTS
   Figure fig = {};
   Stripe ts[] =
      {
      /* 00 */ { noop, .width = .1 },
      /* 01 */ { noop, .width = .2 },
      /* 02 */ { noop, .width = .05 },
      /* 03 */ { noop, .width = .01 },
      /* 04 */ { noop, .width = .2 },
      /* 05 */ { noop, .over = 1 },
      /* 06 */ { noop, .over = -1 },
      /* 07 */ { noop, .begin = 0.9, .end = 0.8 },
      /* 08 */ { noop, .begin = 0.9, .width = 0.1 },
      /* 09 */ { noop, .over = 8, .width = 0.1 },
      /* 10 */ { noop, .end = 0.8, .width = 0.1 },
      /* XX */ //{ noop, .over = -1 },
      /* XX */ {}
      };
   paint_stripes( &fig, ts );
   TRIAL("Stripes dont end up with 0 dimensions",
      for_each_stripe
         {
         ENSURE( each->begin != 0.0 );
         ENSURE( each->end != 0.0 );
         }
      );
   TRIAL("Stripes with .width get consistent size",
      for_each_stripe
         {
         if( each->width != 0 )
            { ENSURE( NEAR( each->begin - each->end, each->width ) ); }
         }
      ENSURE( ts[8].end == ts[7].end );
      );
   MUST("Combine .width with .over", NEAR( ts[9].end , ts[7].end ) );
   MUST("Combine .width with .end", NEAR( ts[10].begin , 0.9 ) );
   TRIAL(".over copies dimensions",
      ENSURE( ts[5].begin == ts[0].begin );
      ENSURE( ts[5].end == ts[0].end );
      ENSURE( ts[6].begin == ts[0].begin );
      ENSURE( ts[6].end == ts[0].end );
      );
END_TESTS
#endif //TEST

