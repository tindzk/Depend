#import "ManifestWriter.h"

#define self ManifestWriter

rsdef(self, new, Logger *logger, Deps *deps) {
	return (self) {
		.deps   = deps,
		.logger = logger
	};
}

def(void, destroy) { }

static def(void, writeHeader, Deps_Modules *modules) {
	File file = File_New($("Manifest.h"),
		FileStatus_Create    |
		FileStatus_WriteOnly |
		FileStatus_Truncate);

	File_Write(&file, $("enum {\n"));

	each(module, modules) {
		String fmt = String_Format($("\tModules_%,\n"), module->name.rd);
		File_Write(&file, fmt.rd);
		String_Destroy(&fmt);

		each(exc, module->exc) {
			String fmt2 = String_Format($("\t%_%,\n"), module->name.rd, exc->rd);
			File_Write(&file, fmt2.rd);
			String_Destroy(&fmt2);
		}

		String fmt3 = String_Format($("\tModules_%_Length,\n"), module->name.rd);
		File_Write(&file, fmt3.rd);
		String_Destroy(&fmt3);
	}

	File_Write(&file, $("};\n\n"));
	File_Write(&file, $("const char* Manifest_ResolveCode(unsigned int code);\n"));
	File_Write(&file, $("const char* Manifest_ResolveName(int module);\n"));

	File_Destroy(&file);
}

static def(void, writeSource, Deps_Modules *modules) {
	File file = File_New($("Manifest.c"),
		FileStatus_Create    |
		FileStatus_WriteOnly |
		FileStatus_Truncate);

	File_Write(&file, $("#import \"Manifest.h\"\n\n"));
	File_Write(&file, $("static const char* codes[] = {\n"));

	each(module, modules) {
		each(exc, module->exc) {
			String fmt = String_Format($("\t[%_%] = \"%\",\n"),
				module->name.rd, exc->rd, exc->rd);
			File_Write(&file, fmt.rd);
			String_Destroy(&fmt);
		}
	}

	File_Write(&file, $(
		"};\n"
		"\n"
		"const char* Manifest_ResolveCode(unsigned int code) {\n"
		"\tif (code > sizeof(codes) / sizeof(codes[0])) {\n"
		"\t\treturn \"\";\n"
		"\t}\n"
		"\n"
		"\treturn codes[code];\n"
		"}\n"
		"\n"
		"const char* Manifest_ResolveName(int module) {\n"
		"\tswitch (module) {\n"));

	each(module, modules) {
		CarrierString readable = String_ReplaceAll(module->name.rd,
			$("_"),
			$("."));

		String fmt = String_Format($(
			"\t\tcase Modules_% ... Modules_%_Length:\n"
			"\t\t\treturn \"%\";\n"),
			module->name.rd,
			module->name.rd,
			readable.rd);

		CarrierString_Destroy(&readable);

		File_Write(&file, fmt.rd);
		String_Destroy(&fmt);
	}

	File_Write(&file, $(
		"\t}\n"
		"\n"
		"\treturn \"Unknown module\";\n"
		"}\n"));

	File_Destroy(&file);
}

def(void, create) {
	Deps_Modules *modules = Deps_getModules(this->deps);

	call(writeHeader, modules);
	call(writeSource, modules);

	Logger_Debug(this->logger,
		$("Manifest written to 'Manifest.h' and 'Manifest.c'."));
}

