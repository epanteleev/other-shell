#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include <stdlib.h>
#include <unistd.h>

void signal_handler_child(int p);
void signal_handler_int(int p);
void signal_handler_hup(int p);
void signal_handler_tstp(int p );
void signal_handler_quit(int p);
#endif //SIGNAL_HANDLER_H
