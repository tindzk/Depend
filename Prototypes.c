#include "Prototypes.h"

void Prototypes_Init(Prototypes *this) {
	this->path = HeapString(0);
}

void Prototypes_Destroy(Prototypes *this) {
	String_Destroy(&this->path);
}

bool Prototypes_SetOption(Prototypes *this, String name, String value) {
	if (String_Equals(&name, $("file"))) {
		String_Copy(&this->path, value);
	}

	return true;
}

void Prototypes_Generate(Prototypes *this) {
	if (this->path.len == 0) {
		String_Print($("Please specify a file.\n"));
		return;
	}

	String s = File_GetContents(this->path);

	StringArray arr = String_Split(&s, '\n');

	String_Destroy(&s);

	for (size_t i = 0; i < arr.len; i++) {
		if (arr.buf[i].len < 5) {
			continue;
		}

		if (arr.buf[i].buf[0] == '\t'
		 || arr.buf[i].buf[0] == ' ') {
			continue;
		}

		String_Trim(&arr.buf[i]);

		if (String_BeginsWith(&arr.buf[i], $("/*"))
		 || String_BeginsWith(&arr.buf[i], $("//"))
		 || String_BeginsWith(&arr.buf[i], $("#"))) {
			continue;
		}

		if (String_EndsWith(&arr.buf[i], $("}"))) {
			String_Crop(&arr.buf[i], 0, -1);
			String_Trim(&arr.buf[i]);
		}

		if (String_EndsWith(&arr.buf[i], $("{"))) {
			String_Crop(&arr.buf[i], 0, -1);
			String_Trim(&arr.buf[i]);

			String_Print(arr.buf[i]);
			String_Print($(";\n"));
		}
	}

	StringArray_Destroy(&arr);
}
