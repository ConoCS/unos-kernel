#ifndef _USERLAND_DEFINE_
#define _USERLAND_DEFINE_

#include <unostype.h>

UNFUNCTION
UnStartUserland(IN VPTR EntryPoint, IN VPTR UserStack);

UNFUNCTION
UnGoToUserland(IN CONST CHARA8 *path);

#endif