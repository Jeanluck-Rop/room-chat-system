#include "controller.hpp"
#include "wrapper_controller.h"

/* */
void
controller_try_connection(int port,
			  const char* ip,
			  const char* user_name)
{
  Controller::instance().try_connection(ip, port, std::string(user_name));
}

/* */
void
controller_disconnect()
{
  Controller::instance().disconnect_user();
}
