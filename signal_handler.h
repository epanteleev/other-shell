#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include <stdlib.h>
#include <unistd.h>

void signal_handler_child(int p);

///
/// \brief lock_sigchld маскирует сигнал SIGCHLD
int lock_sigchld(sigset_t* set);

int unlock_sigchld(sigset_t* set);

#endif //SIGNAL_HANDLER_H
