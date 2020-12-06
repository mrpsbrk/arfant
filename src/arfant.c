/* ARF astrology research framework
 *     # a kind of wrapper for the Swiss Ephemeris
 * (by) Marcio Baraco <marciorps@gmail.com>
 */
/** @file arfant.c is the GUI for the ARF
 *    it has basically 3 tabs: charts db, drawing, txt info */

#include "arfc.h"
#include <gtk/gtk.h>
int BUILD_NUMBER =
#include "BUILD_NUMBER"
   ;

//---- GLOBALS -------------------------------------------------------//
char buildtag[16];
// GUI pointers should be seen by everyone
GtkApplication* app;
// those upper cases appears on the UI
GtkWidget* ui_Name;
GtkWidget* ui_Date;
GtkWidget* ui_Time;
GtkWidget* ui_Location;
// cairo display stuff
GtkWidget* ui_display;
cairo_surface_t* imgbuf = NULL;
Figure arf_context = { };
Figure* F = &arf_context;

//---- HELPER FUNCTIONS ----------------------------------------------//
intern
void
refresh()
   {
   gtk_widget_queue_draw_area( ui_display,0,0,F->w,F->h);
   }

intern
void
spit( char* s )
   {
   puts( s );
   free( s );
   }

intern
void
paint_chart()
   {
   // clear
   prep_gray( F, 1 );
   cairo_paint( F->t );
   prep_gray( F, 0.5 );
   draw_text( F, -3.0, -3.0, 0.5, buildtag );
   //
   if ( F->c == NULL)
      {
      prep_gray( F, 0.25 );
      draw_placeholder( F );
      draw_text( F, 20.0,  30.0, 4.0, "Astrology" );
      draw_text( F, 20.0, 130.0, 4.0, "Research" );
      draw_text( F, 20.0, 230.0, 4.0, "Framework" );
      }
   else
      {
      draw_chart_details( F, 20.0, 20.0 );
      Stripe mys[] =
         {
            { fleurons,  .width=0.05 },
            { axis,  .width=0.05 },
            { multi_tics, .width=0.065 },
            { extra_house_sys, .width=0.04 },
            { noop, .width=0.02 },
            { house_slabs, .width=0.3 },
            { basic_aspects, .width=0.3 },
            { dot_dot_points, .as=-2 },
            { point_image, .begin=0.81, .end=0.94 },
            {}
         };
      paint_stripes( F, mys );
      //---------- tables
      //prep_gray( F, 0.1 );
      //char* txt1 = make_point_table(F->c, "|$Y| $N | $z |$s|");
      //draw_paragraph( F, 8, -8, 0.35, txt1 );
      //free( txt1 );
      //char* txt2 = make_house_table(F->c);
      //draw_paragraph( F, F->w - (F->sz * 9) , -8, 0.27, txt2 );
      //free( txt2 );
      }
   }

//---- CALLBACKS -----------------------------------------------------//

//-- A main callback that processes (string) commands ----------------//
void
callback( GtkWidget* w, gpointer data )
   {
   #define strcase(C) if( 0 == strcmp( data, C ) )
      strcase( "Now" )
         {
         if( F->c ) { dump_chart( F->c ); }
         double jdnnow = jdn_of_now();
         F->c = make_chart( "Now", jdnnow, -23.0, -43.0 );
         F->asc = F->c->ascendant;
         paint_chart();
         refresh();
         }
      else
      strcase( "Calculate" )
         {
         char* nam = strdup( gtk_entry_get_text( GTK_ENTRY(ui_Name) ));
         char* dat = strdup( gtk_entry_get_text( GTK_ENTRY(ui_Date) ));
         char* tim = strdup( gtk_entry_get_text( GTK_ENTRY(ui_Time) ));
         char* plc = strdup( gtk_entry_get_text( GTK_ENTRY(ui_Location) ));
         double jdn = jdn_of_datestring( dat );
         double tmd = jdn_of_timestring( tim );
         Datum geo = coords_of_string( plc );
         if( ! isnan(jdn) )
            {
            if( F->c ) { dump_chart( F->c ); }
            F->c = make_chart( nam, jdn+tmd, geo.lat, geo.lon );
            F->asc = F->c->ascendant;
            paint_chart();
            refresh();
            }
         free(nam);
         free(dat);
         free(tim);
         free(plc);
         }
      else
      strcase( "Export PNG" )
         {
         cairo_status_t ret;
         cairo_surface_t* out =
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, F->w, F->h );
         cairo_t* outX = cairo_create( out );
         cairo_set_source_surface( outX, imgbuf, 0, 0 );
         cairo_paint( outX );
         ret = cairo_surface_write_to_png( out, "temp.png" );
         printf( "status of cairo_surface_write_to PNG: %i %s\n",
                 ret, cairo_status_to_string( ret ) );
         cairo_destroy( outX );
         cairo_surface_destroy( out );
         }
      else
      strcase( "Report" )
         {
         if( F->c )
            {
            printf( "\nName:  %s\n", F->c->ev->name );
            printf( "Time:  %f\n", F->c->ev->jdn );
            spit( make_point_table(F->c, "|$Y| $N | $U |$S |$d |$C|") );
            spit( make_house_table( F->c ) );
            }
         }
      else
      strcase( "Quit" )
         { g_application_quit( G_APPLICATION(app) ); }
      else
         { printf( "Unprocessed command: %s\n", (char*) data ); }
   #undef strcase
   }


//-- Callbacks to paint the main figure ------------------------------//
gboolean
config_display( GtkWidget* wid, GdkEventConfigure* ev, gpointer data )
   {
   // prep the buffer
   if( imgbuf ) { cairo_surface_destroy( imgbuf ); }
   imgbuf = gdk_window_create_similar_surface
               (
               gtk_widget_get_window(wid),
               CAIRO_CONTENT_COLOR,
               gtk_widget_get_allocated_width( wid ),
               gtk_widget_get_allocated_height( wid )
               );
   // prep the Figure struct
   if( F->t ) { cairo_destroy( F->t ); }
   F->t = cairo_create( imgbuf );
   F->w = gtk_widget_get_allocated_width( wid );
   F->h = gtk_widget_get_allocated_height( wid );
   F->r = MIN( F->w, F->h )/2.0;
   F->x = F->w/2.0;
   F->y = F->h/2.0;
   F->sz = F->r*0.075;
   //
   paint_chart();
   return FALSE;
   }

gboolean
update_display( GtkWidget* wid, cairo_t* cr, gpointer data )
   {
   cairo_set_source_surface( cr, imgbuf, 0, 0 );
   cairo_paint( cr );
   return FALSE;
   }

//-- A callback to configure the app ---------------------------------//
/*
+--------------------------+
| row                      |
| +--------+-------------+ |
| | box    | display     | |
| |+------+|             | |
| ||entrys||             | |
| ||  &   ||             | |
| ||butons||             | |
| |+------+|             | |
| +--------+-------------+ |
+--------------------------+
*/
static void
build_gui( GtkApplication* app, gpointer d)
   {
   // BASIC UI STRUCTURE
   GtkWidget* g; // reused for many
   GtkWidget* win = gtk_application_window_new( GTK_APPLICATION(app) );
   GtkWidget* row = gtk_box_new( GTK_ORIENTATION_HORIZONTAL,0 );
   GtkWidget* box = gtk_box_new( GTK_ORIENTATION_VERTICAL,0 );
   ui_display = gtk_drawing_area_new();
   //
   // HIGH-LEVEL PACKING
   gtk_container_add( GTK_CONTAINER(win), row );
   gtk_container_add( GTK_CONTAINER(row), box );
   gtk_container_add( GTK_CONTAINER(row), ui_display );
   gtk_box_set_child_packing( GTK_BOX(row),ui_display,TRUE,TRUE,0,GTK_PACK_START );
   //
   // BUTTONS and ENTRIES
   #define BUTTON( CMD ) \
      g = gtk_button_new_with_label ( CMD ); \
      g_signal_connect( \
         g, "clicked",G_CALLBACK(callback),(gpointer)CMD ); \
      gtk_box_pack_start( GTK_BOX(box),g,FALSE,TRUE,2 );
   #define ENTRY( TXT ) \
      ui_##TXT = gtk_entry_new(); \
      gtk_box_pack_start( GTK_BOX(box),ui_##TXT,FALSE,TRUE,2 ); \
      gtk_entry_set_placeholder_text( GTK_ENTRY(ui_##TXT), #TXT );
   //
   BUTTON( "Now" );
   ENTRY( Name );
   ENTRY( Date );
   ENTRY( Time );
   ENTRY( Location );
   BUTTON( "Calculate" );
   BUTTON( "Export PNG" );
   BUTTON( "Export PDF" );
   BUTTON( "Report" );
   BUTTON( "Test" );
   BUTTON( "Quit" );
   #undef ENTRY
   #undef BUTTON
   //
   // PREPARE DISPLAY WIDGET
   g_signal_connect( G_OBJECT( ui_display ), "configure-event",
                     G_CALLBACK( config_display ), NULL );
   g_signal_connect( G_OBJECT( ui_display ), "draw",
                     G_CALLBACK( update_display ), NULL );
   //
   // WINDOW PROPERTIES
   gtk_window_set_title( GTK_WINDOW(win), "arfant" );
   gtk_window_set_default_size( GTK_WINDOW(win), 960, 768 );
   gtk_container_set_border_width( GTK_CONTAINER(win),5 );
   gtk_widget_show_all( win );
   }

//---- MAIN PROGRAM --------------------------------------------------//
int
main( int count, char* args[] )
   {
   int status = 0;
   sprintf( buildtag, "ARF v0.0:%i", BUILD_NUMBER );
   int pts[] = { 0,1,2,3,4,17,5,6,7,8,9, SE_END };
   init_swiss_ephemeris( "UPROC", pts );
   //
   app = gtk_application_new( "br.art.doxa.arfant", G_APPLICATION_FLAGS_NONE);
   g_signal_connect (app, "activate", G_CALLBACK (build_gui), NULL);
   status = g_application_run (G_APPLICATION (app), count, args);
   g_object_unref (app);
   //
   end_swiss_ephemeris();
   return status;
   }



