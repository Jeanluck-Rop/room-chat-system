#ifndef NOTIFIES_H
#define NOTIFIES_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "models.h"
#include "auxiliar.h"

/**
 * Initialize and attach notification popover functionality to the chat interface.
 *
 * @param chatty Pointer to the main chat data structure.
 **/
void set_notifs(ChatData *chatty);
  
#endif // NOTIFIES_H
