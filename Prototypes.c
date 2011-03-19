#import "Prototypes.h"

#define self Prototypes

def(void, Init) {
	this->path = String_New(0);
}

def(void, Destroy) {
	String_Destroy(&this->path);
}

def(bool, SetOption, RdString name, RdString value) {
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

	String s = String_New(Path_GetSize(this->path.rd));
	File_GetContents(this->path.rd, &s);

	RdString iter = $("");
	while (String_Split(s.rd, '\n', &iter)) {
		if (iter.len < 5) {
			continue;
		}

		if (iter.buf[0] == '\t' || iter.buf[0] == ' ') {
			continue;
		}

		RdString line = String_Trim(iter);

		if (String_BeginsWith(line, $("/*")) ||
			String_BeginsWith(line, $("//")) ||
			String_BeginsWith(line, $("#")))
		{
			continue;
		}

		if (String_EndsWith(line, $("}"))) {
			line = String_Slice(line, 0, -1);
			line = String_Trim(line);
		}

		if (String_EndsWith(line, $("{"))) {
			line = String_Slice(line, 0, -1);
			line = String_Trim(line);

			String_Print(line);
			String_Print($(";\n"));
		}
	}

	String_Destroy(&s);
}
