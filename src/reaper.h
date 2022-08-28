#ifndef _REAPER_H_
#define _REAPER_H_

#include <memory>
#include "connection.h"


void reaper_add_work(std::shared_ptr<Connection> c);
std::shared_ptr<Connection> reaper_wait_for_work();
void reaper(int* master_socket_fd);

#endif