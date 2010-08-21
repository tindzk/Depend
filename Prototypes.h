#import <String.h>

#import "Utils.h"

typedef struct {
	String path;
} Prototypes;

void Prototypes_Init(Prototypes *this);
void Prototypes_Destroy(Prototypes *this);
bool Prototypes_SetOption(Prototypes *this, String name, String value);
void Prototypes_Generate(Prototypes *this);
