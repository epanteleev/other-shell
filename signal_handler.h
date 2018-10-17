#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include <stdlib.h>
#include <unistd.h>

void signal_handler_child(int p);
void signal_handler_int(int p);

void shell_prompt();
#endif //SIGNAL_HANDLER_H
