#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <adwaita.h>

#ifdef __cplusplus
extern "C"
{
#endif
  
  typedef struct
  {
    char* server_ip;
    int port;
  }
    StartData;

  void launch_gui(char* server_ip, int port);
  
#ifdef __cplusplus
}
#endif

#endif // VIEW_H
