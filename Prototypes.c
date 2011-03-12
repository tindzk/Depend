#import "Prototypes.h"

#define self Prototypes

def(void, Init) {
	this->path = String_New(0);
}

def(void, Destroy) {
	String_Destroy(&this->path);
}

def(bool, SetOption, ProtString name, ProtString value) {
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

	String s = String_New(Path_GetSize(this->path.prot));
	File_GetContents(this->path.prot, &s);

	ProtString iter = $("");
	while (String_Split(s.prot, '\n', &iter)) {
		if (iter.len < 5) {
			continue;
		}

		if (iter.buf[0] == '\t' || iter.buf[0] == ' ') {
			continue;
		}

		ProtString line = String_Trim(iter);

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
