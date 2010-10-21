#define Modules_Path 0
#define Modules_Time 256
#define Modules_String 512
#define Modules_Memory 768
#define Modules_Integer 1024
#define Modules_Terminal 1280
#define Modules_File 1536
#define Modules_Typography 1792
#define Modules_Interface 2048
#define Modules_Deps 2304
#define Modules_Directory 2560
#define Modules_Builder 2816
#define Modules_Process 3072
#define Modules_Prototypes 3328

static inline char* Manifest_ResolveName(unsigned int module) {
	switch (module) {
		case Modules_Path:
			return "Path";
		case Modules_Time:
			return "Time";
		case Modules_String:
			return "String";
		case Modules_Memory:
			return "Memory";
		case Modules_Integer:
			return "Integer";
		case Modules_Terminal:
			return "Terminal";
		case Modules_File:
			return "File";
		case Modules_Typography:
			return "Typography";
		case Modules_Interface:
			return "Interface";
		case Modules_Deps:
			return "Deps";
		case Modules_Directory:
			return "Directory";
		case Modules_Builder:
			return "Builder";
		case Modules_Process:
			return "Process";
		case Modules_Prototypes:
			return "Prototypes";
	}

	return "Unknown module";
}
