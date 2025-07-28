#ifndef WRAPPER_CONTROLLER_H
#define WRAPPER_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* */
  void controller_try_connection(int port, const char *ip, const char *user_name);
  
  /* */
  void controller_disconnect();
  
#ifdef __cplusplus
}
#endif

#endif // CONTROLLER_WRAPPER_H
