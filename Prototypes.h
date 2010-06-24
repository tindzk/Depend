#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <String.h>

#include "Utils.h"

typedef struct {
	String path;
} Prototypes;

void Prototypes_Init(Prototypes *this);
void Prototypes_Destroy(Prototypes *this);
bool Prototypes_SetOption(Prototypes *this, String name, String value);
void Prototypes_Generate(Prototypes *this);

#endif
