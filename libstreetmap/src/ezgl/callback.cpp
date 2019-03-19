#include "ezgl/callback.hpp"
#include "map_db.h"
#include <time.h>       //used for timing
#include <iostream>
#include "../constants.hpp"

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
    
    //implement inertial scrolling
    if(was_panning) {
        std::string main_canvas_id = application->get_main_canvas_id();
        auto canvas = application->get_canvas(main_canvas_id);
        
        
        //translate by 1/10th prev_dx/dy each time for a max of 5 times
        //or when very small
        int counter = 0;
        while((prev_dx > 0.00000001 && prev_dy > 0.00000001) || counter < 5) {
            translate(canvas, -prev_dx, -prev_dy);
            application->refresh_drawing();
            
            prev_dx = prev_dx/10.0;
            prev_dy = prev_dy/10.0;
            counter ++;
        }
    }
    
  }

  return TRUE; // consume the event
}

gboolean move_mouse(GtkWidget *, GdkEventButton *event, gpointer data)
{
  auto application = static_cast<ezgl::application *>(data);

  if(event->type == GDK_MOTION_NOTIFY) {

    // Check if the pan button is pressed to support dragging
    if(pan_button_pressed) {

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
    GtkLabel *label;
    GtkWidget *dialog;
    auto application = static_cast<ezgl::application *>(data);
    
    //get pointer to main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    //create the help dialog window
    dialog = gtk_dialog_new_with_buttons(
        "Welcome to Go2MAP!",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("GOT IT"),
        GTK_RESPONSE_DELETE_EVENT,
        NULL
    );
    
    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = (GtkLabel *)gtk_label_new(HELP_TEXT.c_str());
    gtk_label_set_max_width_chars(label, 80);
    gtk_container_add(GTK_CONTAINER(content_area), (GtkWidget *)label);
    
    //show the dialog
    gtk_widget_show_all(dialog);
    
    //Connect a response to the callback function
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL
    );
    
    return TRUE;
}

gboolean transit_toggled(GtkToggleButton *toggle_button, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    // Call the user-defined transit toggle callback if defined
    if(application->transit_toggled_callback != nullptr) {
      bool isToggled = gtk_toggle_button_get_active(toggle_button);

      application->transit_toggled_callback(application, isToggled);
    }
    
    return TRUE;
}

gboolean bikes_toggled(GtkToggleButton *toggle_button, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    // Call the user-defined bikes toggle callback if defined
    if(application->bikes_toggled_callback != nullptr) {
      bool isToggled = gtk_toggle_button_get_active(toggle_button);

      application->bikes_toggled_callback(application, isToggled);
    }
    
    return TRUE;
}

gboolean poi_toggled(GtkToggleButton *toggle_button, gpointer data){
    auto application = static_cast<ezgl::application *>(data);
    
    // Call the user-defined bikes toggle callback if defined
    if(application->poi_toggled_callback != nullptr) {
      bool isToggled = gtk_toggle_button_get_active(toggle_button);

      application->poi_toggled_callback(application, isToggled);
    }
    
    return TRUE;
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    (void) response_id;
    (void) user_data;
    
    gtk_widget_destroy(GTK_WIDGET (dialog));
}

gboolean search_entry_handle_event (GtkSearchEntry *entry, GdkEvent *event) {
    (void) entry;
    (void) event;
    
    MAP.state.search_changed = true;
    
    return TRUE;
}

gboolean handle_search_suggestion (GtkMenuItem *menu_item, gpointer data) {
    auto application = static_cast<ezgl::application *>(data);
    
    if(application->search_suggestion_callback != nullptr) {
        std::string suggestion = gtk_menu_item_get_label(menu_item);
    
        application->search_suggestion_callback(application, suggestion);
    }
    
    return TRUE;
}

gboolean press_find(GtkWidget *widget, gpointer data) {   
    auto application = static_cast<ezgl::application *>(data);
    
    if(application->find_callback != nullptr) {
        application->find_callback(widget, data);
    }
    
    return TRUE;
}

gboolean handle_to_from (GtkMenuItem *menu_item, gpointer data) {
    auto application = static_cast<ezgl::application *>(data);
    
    //gives us event: "Directions To" or "Directions From"
    std::string suggestion = gtk_menu_item_get_label(menu_item);
    
    //populate to respective text entry with closest intersection, and set
    //icon to be there through changing MAP.route_data
    if(suggestion == "Directions From") {
       GtkEntry* text_entry = (GtkEntry *) application->get_object("SearchBar"); 
       int id = MAP.state.directions_intersection_id;
       gtk_entry_set_text(text_entry, MAP.intersection_db[id].name.c_str());
       MAP.route_data.start_intersection = (unsigned)id;    
       MAP.state.is_from_set_right_click = true;
    } else if (suggestion == "Directions To") {
       GtkEntry* text_entry = (GtkEntry *) application->get_object("ToBar");
       int id = MAP.state.directions_intersection_id;
       gtk_entry_set_text(text_entry, MAP.intersection_db[id].name.c_str());
       MAP.route_data.end_intersection = (unsigned)id;
       MAP.state.is_to_set_right_click = true;
    }
    //clear the current route so it doesn't look funny
    MAP.route_data.route_segments.clear();
    
    //if both to/from set from right click, automatically search
    if(MAP.state.is_from_set_right_click && MAP.state.is_to_set_right_click) {
        GtkButton * directions_button = (GtkButton* ) application->get_object("DirectionsButton");
        gtk_button_clicked(directions_button);
    }
        
    
    application->refresh_drawing();
    
    return TRUE;
}

gboolean press_directions(GtkWidget *widget, gpointer data) {  
    GObject *window;
    GtkWidget *content_area;
    GtkWidget *dialog;
    auto application = static_cast<ezgl::application *>(data);
    
    //NEED the user-defined callback to generate directions, don't do anything
    //if its not there
    if(application->directions_callback == nullptr) return TRUE;
    
    //generate directions un user-defined callback, if issue in generating instructions
    //just return and don't act on it
    if(not application->directions_callback(widget, data)) return TRUE;
    
    //get pointer to main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    //create the help dialog window
    dialog = gtk_dialog_new_with_buttons(
        "Directions",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("GOT IT"),
        GTK_RESPONSE_DELETE_EVENT,
        NULL
    );
    
    // Create a scrolled window and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget * scroll_window = (GtkWidget *) gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll_window, 400, 400);
    gtk_container_add((GtkContainer *)content_area, scroll_window);
    
    //Create a Viewport and attach it to the scrolled window
    GtkWidget * viewport = (GtkWidget *) gtk_viewport_new(NULL, NULL);
    gtk_container_add((GtkContainer *)scroll_window, viewport);
    
    //create a grid of the directions and place it in the viewport    
    GtkGrid* grid = (GtkGrid* ) gtk_grid_new();
    
    //set label font attributes used in grid
    PangoAttrList * text_attributes = pango_attr_list_new();
    PangoAttribute * text_scale = pango_attr_scale_new(1.2);
    pango_attr_list_insert(text_attributes, text_scale);
    
    int row = 0;
    
    //insert a start row
    gtk_grid_insert_row(grid, row);
    
    //insert starting image
    GtkWidget * start_image = gtk_image_new_from_file("./libstreetmap/resources/GreenLocationMarkerDouble.png");
    gtk_widget_set_size_request(start_image, 50, 50);
    gtk_grid_attach(grid, start_image, 0, row, 1, 1);
    
    //set start text based off if the intersection contains <unkown>
    std::string start_text;
    std::string start_intersection_name = getIntersectionName(MAP.route_data.start_intersection);
    if(start_intersection_name.find("<unknown>") == std::string::npos) {
        start_text = "Start at " + start_intersection_name;
    } else {
        start_text = "Starting your journey";
    }
    
    //create and insert the start label
    GtkWidget * start_label = gtk_label_new(start_text.c_str());
    gtk_label_set_line_wrap((GtkLabel *) start_label, true);
    gtk_label_set_xalign((GtkLabel *)start_label, 0);
    gtk_label_set_attributes((GtkLabel *)start_label, text_attributes);
    gtk_grid_attach(grid, start_label, 1, row, 2, 1);
        
    //add directions
    for(unsigned i = 0; i < MAP.directions_data.size(); i++) {
        row++;
        gtk_grid_insert_row(grid, row);
        std::string directions = "Turn ";
        
        //assign image based on turn type
        GtkWidget * image;
        switch(MAP.directions_data[i].turn_type){
        case TurnType::RIGHT    : 
            directions += "right onto ";
            image = gtk_image_new_from_file("./libstreetmap/resources/right_turn_icon.png");
            break;
        case TurnType::LEFT     : 
            directions += "left onto ";
            image = gtk_image_new_from_file("./libstreetmap/resources/left_turn_icon.png");
            break;
        //default to STRAIGHT
        default: image = gtk_image_new_from_file("./libstreetmap/resources/straight_icon.png");
        }
        //generate label for directions
        directions += MAP.directions_data[i].street;
        GtkWidget * label = gtk_label_new(directions.c_str());
        
        //set label properties
        gtk_label_set_line_wrap((GtkLabel *) label, true);
        gtk_label_set_xalign((GtkLabel *)label, 0);
        gtk_label_set_attributes((GtkLabel *)label, text_attributes);
        
        //set image properties
        gtk_widget_set_size_request(image, 50, 50);
        
        //attach directions and image
        gtk_grid_attach(grid, image, 0, row, 1, 1);
        gtk_grid_attach(grid, label, 1, row, 2, 1);
        
        //generate distance label and set properties
        GtkWidget * label_distance = gtk_label_new(MAP.directions_data[i].path_distance.c_str());
        gtk_label_set_xalign((GtkLabel *)label_distance, 1);
        gtk_label_set_yalign((GtkLabel *)label_distance, 0);
        gtk_widget_set_size_request((GtkWidget *)label_distance, -1, 25);
        
        //generate time label and set properties
        GtkWidget * label_time = gtk_label_new(MAP.directions_data[i].path_time.c_str());
        gtk_label_set_xalign((GtkLabel *)label_time, 0);
        gtk_label_set_yalign((GtkLabel *)label_time, 0);
        gtk_widget_set_size_request((GtkWidget *)label_time, -1, 25);
        
        //insert a row and attach both labels
        row++;
        gtk_grid_insert_row(grid, row);
        
        gtk_grid_attach(grid, label_distance, 2, row, 1, 1);
        gtk_grid_attach(grid, label_time, 1, row, 1, 1);
    }
    
    //insert the final row
    row++;
    gtk_grid_insert_row(grid, row);
    
    //insert end image
    GtkWidget * end_image = gtk_image_new_from_file("./libstreetmap/resources/BlueLocationMarkerDouble.png");
    gtk_widget_set_size_request(end_image, 50, 50);
    gtk_grid_attach(grid, end_image, 0, row, 1, 1);
    
    //set start text based off if the intersection contains <unkown>
    std::string end_text;
    std::string end_intersection_name = getIntersectionName(MAP.route_data.end_intersection);
    if(end_intersection_name.find("<unknown>") == std::string::npos) {
        end_text = "You have arrived at " + end_intersection_name;
    } else {
        end_text = "You have Arrived!";
    }
    
    //create and insert the start label
    GtkWidget * end_label = gtk_label_new(end_text.c_str());
    gtk_label_set_line_wrap((GtkLabel *) end_label, true);
    gtk_label_set_xalign((GtkLabel *)end_label, 0);
    gtk_label_set_attributes((GtkLabel *)end_label, text_attributes);
    gtk_grid_attach(grid, end_label, 1, row, 2, 1);
    
    //generate distance label and set properties
    GtkWidget * label_distance_total = gtk_label_new(MAP.travel_distance.c_str());
    gtk_label_set_xalign((GtkLabel *)label_distance_total, 1);
    gtk_label_set_yalign((GtkLabel *)label_distance_total, 0);
    gtk_widget_set_size_request((GtkWidget *)label_distance_total, -1, 25);

    //generate time label and set properties
    GtkWidget * label_time_total = gtk_label_new(MAP.travel_time.c_str());
    gtk_label_set_xalign((GtkLabel *)label_time_total, 0);
    gtk_label_set_yalign((GtkLabel *)label_time_total, 0);
    gtk_widget_set_size_request((GtkWidget *)label_time_total, -1, 25);

    //insert a row and attach both labels
    row++;
    gtk_grid_insert_row(grid, row);

    gtk_grid_attach(grid, label_distance_total, 2, row, 1, 1);
    gtk_grid_attach(grid, label_time_total, 1, row, 1, 1);
        
    //Add the grid to the viewport
    gtk_container_add ((GtkContainer *) viewport, (GtkWidget *) grid);  
    
    //free pango attributes
    //pango_attribute_destroy(text_scale); //seg faults if we do but told we should...
    pango_attr_list_unref(text_attributes);
    
    //show the dialog
    gtk_widget_show_all ((GtkWidget *)dialog);

    //Connect a response to the callback function
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL
    );
    
    return TRUE;
}

}
