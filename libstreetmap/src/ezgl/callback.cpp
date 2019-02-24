#include "ezgl/callback.hpp"
#include "map_db.h"
#include <time.h>       //used for timing
#include <iostream>

namespace ezgl {

// File wide static variables to track whether the pan
// button is currently pressed AND the old x and y positions of the mouse pointer
bool pan_button_pressed = false;
clock_t time_at_click;
int last_panning_event_time = 0;
double prev_x = 0, prev_y = 0;
double prev_dx = 0, prev_dy = 0;

gboolean press_key(GtkWidget *, GdkEventKey *event, gpointer data)
{
  auto application = static_cast<ezgl::application *>(data);

  // Call the user-defined key press callback if defined
  if(application->key_press_callback != nullptr) {
    // see: https://developer.gnome.org/gdk3/stable/gdk3-Keyboard-Handling.html
    application->key_press_callback(application, event, gdk_keyval_name(event->keyval));
  }

  return FALSE; // propagate the event
}

gboolean press_mouse(GtkWidget *, GdkEventButton *event, gpointer)
{
    if(event->type == GDK_BUTTON_PRESS) {

    // Check for pan button press to support dragging
    if(event->button == 1) {
      time_at_click = clock();
      pan_button_pressed = true;
      prev_x = event->x;
      prev_y = event->y;
    }

    }

    return TRUE; // consume the event
}

gboolean release_mouse(GtkWidget *, GdkEventButton *event, gpointer data )
{
  if(event->type == GDK_BUTTON_RELEASE) {
    auto application = static_cast<ezgl::application *>(data);
      
    bool was_panning = false;
    
    // Check for pan button release to support dragging
    if(event->button == 1) {
      was_panning = (double)(clock() - time_at_click) >= 10000;
      time_at_click = 0;
      pan_button_pressed = false;
    }
    
    // Call the user-defined mouse press callback if defined
    // Its being called on release instead of on press so we can support panning with
    // a click and drag of the left mouse button, 
    if(!was_panning && application->mouse_press_callback != nullptr) {
      ezgl::point2d const widget_coordinates(event->x, event->y);

      std::string main_canvas_id = application->get_main_canvas_id();
      ezgl::canvas *canvas = application->get_canvas(main_canvas_id);

      ezgl::point2d const world = canvas->get_camera().widget_to_world(widget_coordinates);
      application->mouse_press_callback(application, event, world.x, world.y);
    }
    
//    //implement inertial scrolling
//    if(was_panning) {
//        std::string main_canvas_id = application->get_main_canvas_id();
//        auto canvas = application->get_canvas(main_canvas_id);
//        
//        
//        //translate by 1/10th prev_dx/dy each time for a max of 5 times
//        //or when very small
//        int counter = 0;
//        while((prev_dx > 0.00000001 && prev_dy > 0.00000001) || counter < 5) {
//            translate(canvas, -prev_dx, -prev_dy);
//            application->refresh_drawing();
//            
//            prev_dx = prev_dx/10.0;
//            prev_dy = prev_dy/10.0;
//            counter ++;
//        }
//    }
    
  }

  return TRUE; // consume the event
}

gboolean move_mouse(GtkWidget *, GdkEventButton *event, gpointer data)
{
  auto application = static_cast<ezgl::application *>(data);

  if(event->type == GDK_MOTION_NOTIFY) {

    // Check if the pan button is pressed to support dragging
    if(pan_button_pressed) {
      // drop this panning event if we have just served another one
      if(gtk_get_current_event_time() - last_panning_event_time < 100)
        return true;

      last_panning_event_time = gtk_get_current_event_time();

      GdkEventMotion *motion_event = (GdkEventMotion *)event;

      std::string main_canvas_id = application->get_main_canvas_id();
      auto canvas = application->get_canvas(main_canvas_id);

      point2d curr_trans = canvas->get_camera().widget_to_world({motion_event->x, motion_event->y});
      point2d prev_trans = canvas->get_camera().widget_to_world({prev_x, prev_y});

      double dx = curr_trans.x - prev_trans.x;
      double dy = curr_trans.y - prev_trans.y;

      prev_x = motion_event->x;
      prev_y = motion_event->y;
      
      //set these for inertial panning
      prev_dx = dx;
      prev_dy = dy;

      // Flip the delta x to avoid inverted dragging
      translate(canvas, -dx, -dy);
    }
    // Else call the user-defined mouse move callback if defined
    else if(application->mouse_move_callback != nullptr) {
      ezgl::point2d const widget_coordinates(event->x, event->y);

      std::string main_canvas_id = application->get_main_canvas_id();
      ezgl::canvas *canvas = application->get_canvas(main_canvas_id);

      ezgl::point2d const world = canvas->get_camera().widget_to_world(widget_coordinates);
      application->mouse_move_callback(application, event, world.x, world.y);
    }
  }

  return TRUE; // consume the event
}

gboolean scroll_mouse(GtkWidget *, GdkEvent *event, gpointer data)
{

  if(event->type == GDK_SCROLL) {
    auto application = static_cast<ezgl::application *>(data);

    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);

    GdkEventScroll *scroll_event = (GdkEventScroll *)event;

    ezgl::point2d scroll_point(scroll_event->x, scroll_event->y);

    if(scroll_event->direction == GDK_SCROLL_UP) {
      // Zoom in at the scroll point
      ezgl::zoom_in(canvas, scroll_point, 5.0 / 3.0);
    } else if(scroll_event->direction == GDK_SCROLL_DOWN) {
      // Zoom out at the scroll point
      ezgl::zoom_out(canvas, scroll_point, 5.0 / 3.0);
    } else if(scroll_event->direction == GDK_SCROLL_SMOOTH) {
      // Doesn't seem to be happening
    } // NOTE: We ignore scroll GDK_SCROLL_LEFT and GDK_SCROLL_RIGHT
  }
  return TRUE;
}

gboolean press_zoom_fit(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::zoom_fit(canvas, canvas->get_camera().get_initial_world());

  return TRUE;
}

gboolean press_zoom_in(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::zoom_in(canvas, 5.0 / 3.0);

  return TRUE;
}

gboolean press_zoom_out(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::zoom_out(canvas, 5.0 / 3.0);

  return TRUE;
}

gboolean press_up(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_up(canvas, 5.0);

  return TRUE;
}

gboolean press_down(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_down(canvas, 5.0);

  return TRUE;
}

gboolean press_left(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_left(canvas, 5.0);

  return TRUE;
}

gboolean press_right(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_right(canvas, 5.0);

  return TRUE;
}

gboolean press_proceed(GtkWidget *, gpointer data)
{
  auto ezgl_app = static_cast<ezgl::application *>(data);
  ezgl_app->quit();

  return TRUE;
}

//Help dialog, much of this code was used from the ezgl quickstart guide on the ECE297 website
//http://www.eecg.toronto.edu/~vaughn/ece297/ECE297/assignments/ezgl/ezgl.pdf
gboolean press_help(GtkWidget *, gpointer data)
{
    GObject *window;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    auto application = static_cast<ezgl::application *>(data);
    
    //get pointer to main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    //create the help dialog window
    dialog = gtk_dialog_new_with_buttons(
        "Help Window",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("GOT IT"),
        GTK_RESPONSE_DELETE_EVENT,
        NULL
    );
    
    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("This is where we will have a tutorial, hopefully we can include images");
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    //show the dialog
    gtk_widget_show_all(dialog);
    
    //Connect a response to the callback function
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL
    );
    
}

gboolean transit_toggled(GtkToggleButton *toggle_button, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    // Call the user-defined transit toggle callback if defined
    if(application->transit_toggled_callback != nullptr) {
      bool isToggled = gtk_toggle_button_get_active(toggle_button);

      application->transit_toggled_callback(application, isToggled);
    }
}

gboolean bikes_toggled(GtkToggleButton *toggle_button, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    // Call the user-defined bikes toggle callback if defined
    if(application->bikes_toggled_callback != nullptr) {
      bool isToggled = gtk_toggle_button_get_active(toggle_button);

      application->bikes_toggled_callback(application, isToggled);
    }
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    gtk_widget_destroy(GTK_WIDGET (dialog));
}

gboolean search_entry_handle_event (GtkSearchEntry *entry, GdkEvent *event) {
    MAP.state.search_changed = true;
}

}

