#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"
#include "wrapper_controller.h"

/**
 * Displays the list of available rooms in the UI.
 *
 * @param list_box The GtkListBox to populate with room entries.
 * @param row_id The resource ID of the row template.
 * @param callback The function to call when the "select" button is clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
void display_rooms_list(GtkListBox *list_box, const char* row_id, GCallback callback, gpointer user_data);

/**
 * Displays a list of available users in the UI.
 * Populates a GtkListBox with rows containing usernames, statuses, 
 * and an "Add" button for inviting them to a room.
 * Disables the "Add" button for the current user.
 *
 * @param list_box The GtkListBox to populate with user entries.
 * @param usernames Null-terminated array of usernames.
 * @param statuses Null-terminated array of statuses corresponding to the usernames.
 * @param callback The function to call when the "Add" button is clicked.
 * @param user_data Pointer to the ChatActions structure.
 **/
void display_users_list(GtkListBox *list_box, char** usernames, char **statuses, GCallback callback, gpointer user_data);

/**
 * Handles acceptance of an invite for selected users.
 * Sends the list of selected users and room name to the controller.
 *
 * @param button The GTK button clicked to accept.
 * @param user_data Pointer to the ChatActions structure.
 **/
void invite_users_accept(GtkButton *button, gpointer user_data);

/**
 * Handles the selection and deselection of guests in the invite list.
 *
 * @param button The GTK button clicked to toggle guest selection.
 * @param user_data Pointer to the ChatActions structure.
 **/
void guests_selected(GtkButton *button, gpointer user_data);

/**
 * Handles selection of a room in the invitations UI.
 *
 * @param button The GTK button clicked to select the room.
 * @param user_data Pointer to the ChatActions structure.
 **/
void invitations_room_selected(GtkButton *button, gpointer user_data);

/**
 * Initializes and connects all chat-related UI actions.
 * This function creates a new ChatActions structure, loads the UI elements
 * from resources, and connects all relevant buttons to their corresponding callbacks.
 *
 * @param builder The GtkBuilder object containing the main UI.
 * @param user_data User-defined data (unused here).
 **/
void set_actions(GtkBuilder *builder, gpointer user_data);

#endif // ACTIONS_H
