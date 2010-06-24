#ifndef INTERFACE_H
#define INTERFACE_H

#include <String.h>

#include "Deps.h"
#include "Utils.h"
#include "Builder.h"
#include "Prototypes.h"

typedef enum {
	Interface_Action_Build,
	Interface_Action_ListDeps,
	Interface_Action_DepTree,
	Interface_Action_PrintQueue,
	Interface_Action_Prototypes,
	Interface_Action_Help,
	Interface_Action_Unsupported
} Interface_Action;

typedef struct {
	Interface_Action action;

	Deps deps;
	Builder builder;
	Prototypes proto;
} Interface;

void Interface_Init(Interface *this);
void Interface_Destroy(Interface *this);
void Interface_SetAction(Interface *this, String action);
bool Interface_SetOption(Interface *this, String name, String value);
bool Interface_Run(Interface *this);

#endif
