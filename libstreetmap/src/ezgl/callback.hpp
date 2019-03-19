#ifndef EZGL_CALLBACK_HPP
#define EZGL_CALLBACK_HPP

#include <ezgl/application.hpp>
#include <ezgl/camera.hpp>
#include <ezgl/canvas.hpp>
#include <ezgl/control.hpp>
#include <ezgl/graphics.hpp>

#include <iostream>

namespace ezgl {

/**** Callback functions for keyboard and mouse input, and for all the ezgl predefined buttons. *****/

/**
 * React to a <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-key-press-event">keyboard
 * press event</a>.
 *
 * @param widget The GUI widget where this event came from.
 * @param event The keyboard event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_key(GtkWidget *widget, GdkEventKey *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-press-event">mouse click
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-release-event">mouse release
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean release_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-release-event">mouse release
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean move_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-scroll-event"> scroll_event
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean scroll_mouse(GtkWidget *widget, GdkEvent *event, gpointer data);

/**
 * React to the clicked zoom_fit button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_fit(GtkWidget *widget, gpointer data);

/**
 * React to the clicked zoom_in button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_in(GtkWidget *widget, gpointer data);

/**
 * React to the clicked zoom_out button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_out(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_up(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_down(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_left(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_right(GtkWidget *widget, gpointer data);

/**
 * React to the clicked proceed button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_proceed(GtkWidget *widget, gpointer data);

//Creates a help dialog when the help button is pressed
gboolean press_help(GtkWidget *widget, gpointer data);

//destroys dialog on response
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);

// React to clicked find button
gboolean press_find(GtkWidget *widget, gpointer data);

// React to when the transit checkbox is toggled
gboolean transit_toggled(GtkToggleButton *toggle_button, gpointer data);

// React to when the bikes checkbox is toggled
gboolean bikes_toggled(GtkToggleButton *toggle_button, gpointer data);

// React to when the poi checkbox is toggled
gboolean poi_toggled(GtkToggleButton *toggle_button, gpointer data);

// Callback function for search bar
gboolean search_entry_handle_event (GtkSearchEntry *entry, GdkEvent *event);

//Callback function for each search suggestion
gboolean handle_search_suggestion (GtkMenuItem *menu_item, gpointer data);

//Callback function for search swap 
gboolean handle_search_swap (GtkEntry *to_entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer data);

//Callback function for directions to/from on right click popup
gboolean handle_to_from (GtkMenuItem *menu_item, gpointer data);

// React to clicked directions button
gboolean press_directions(GtkWidget *widget, gpointer data);
}

//adds a new row with and image and a label with font size * scale_factor
void gtk_grid_new_row_primary(GtkGrid* grid, int row, double scale_factor, GtkWidget* image, std::string text);

//adds a new row with two labels
void gtk_grid_new_row_secondary(GtkGrid* grid, int row, std::string text_1, std::string text_2);

#endif //EZGL_CALLBACK_HPP
