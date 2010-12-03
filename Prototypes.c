#import "Prototypes.h"

#define self Prototypes

def(void, Init) {
	this->path = HeapString(0);
}

def(void, Destroy) {
	String_Destroy(&this->path);
}

def(bool, SetOption, String name, String value) {
	if (String_Equals(name, $("file"))) {
		String_Copy(&this->path, value);
	}

	return true;
}

def(void, Generate) {
	if (this->path.len == 0) {
		String_Print($("Please specify a file.\n"));
		return;
	}

	String s = HeapString(Path_GetSize(this->path));
	File_GetContents(this->path, &s);

	StringArray *arr = String_Split(s, '\n');

	for (size_t i = 0; i < arr->len; i++) {
		if (arr->buf[i].len < 5) {
			continue;
		}

		if (arr->buf[i].buf[0] == '\t'
		 || arr->buf[i].buf[0] == ' ') {
			continue;
		}

		String_Trim(&arr->buf[i]);

		if (String_BeginsWith(arr->buf[i], $("/*"))
		 || String_BeginsWith(arr->buf[i], $("//"))
		 || String_BeginsWith(arr->buf[i], $("#"))) {
			continue;
		}

		if (String_EndsWith(arr->buf[i], $("}"))) {
			String_Crop(&arr->buf[i], 0, -1);
			String_Trim(&arr->buf[i]);
		}

		if (String_EndsWith(arr->buf[i], $("{"))) {
			String_Crop(&arr->buf[i], 0, -1);
			String_Trim(&arr->buf[i]);

			String_Print(arr->buf[i]);
			String_Print($(";\n"));
		}
	}

	StringArray_Destroy(arr);
	String_Destroy(&s);
}
