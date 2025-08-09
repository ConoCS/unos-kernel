#ifndef _COMMAND_TERMINAL_
#define _COMMAND_TERMINAL_

#include <unostype.h>

/// Dependent Function

FUNCWITHSTATUS GraphicTerminalINIT();

/// Rest of function

void echo(const char *string);
void party(const char *wowo);
void help(const char *helpcommand);
void gettick(const char *arg);
void waitfortick(const char *arg);
void trybuffer(const char *argcount);
void uptime(const char *arg);
void parsefat32(const char *arg);
void cd(const char *arg);
VOID Shutdown(CONST IN CHARA8 *arg);
VOID Sleep(CONST IN CHARA8 *arg);
UNFUNCTION Graphic(CONST CHARA8 *arg);

#endif