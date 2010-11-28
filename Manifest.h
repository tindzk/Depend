#define Manifest_GapSize 256
#define Modules_Path 256
#define Modules_Time 512
#define Modules_Kernel 768
#define Modules_String 1024
#define Modules_Memory 1280
#define Modules_Exception 1536
#define Modules_Hex 1792
#define Modules_Integer 2048
#define Modules_Logger 2304
#define Modules_Terminal 2560
#define Modules_File 2816
#define Modules_Terminal_Controller 3072
#define Modules_Typography 3328
#define Modules_Tree 3584
#define Modules_StringStream 3840
#define Modules_Interface 4096
#define Modules_Deps 4352
#define Modules_Directory 4608
#define Modules_Builder 4864
#define Modules_Process 5120
#define Modules_Prototypes 5376

static inline char* Manifest_ResolveName(unsigned int module) {
	switch (module) {
		case Modules_Path ... Modules_Path + Manifest_GapSize - 1:
			return "Path";
		case Modules_Time ... Modules_Time + Manifest_GapSize - 1:
			return "Time";
		case Modules_Kernel ... Modules_Kernel + Manifest_GapSize - 1:
			return "Kernel";
		case Modules_String ... Modules_String + Manifest_GapSize - 1:
			return "String";
		case Modules_Memory ... Modules_Memory + Manifest_GapSize - 1:
			return "Memory";
		case Modules_Exception ... Modules_Exception + Manifest_GapSize - 1:
			return "Exception";
		case Modules_Hex ... Modules_Hex + Manifest_GapSize - 1:
			return "Hex";
		case Modules_Integer ... Modules_Integer + Manifest_GapSize - 1:
			return "Integer";
		case Modules_Logger ... Modules_Logger + Manifest_GapSize - 1:
			return "Logger";
		case Modules_Terminal ... Modules_Terminal + Manifest_GapSize - 1:
			return "Terminal";
		case Modules_File ... Modules_File + Manifest_GapSize - 1:
			return "File";
		case Modules_Terminal_Controller ... Modules_Terminal_Controller + Manifest_GapSize - 1:
			return "Terminal.Controller";
		case Modules_Typography ... Modules_Typography + Manifest_GapSize - 1:
			return "Typography";
		case Modules_Tree ... Modules_Tree + Manifest_GapSize - 1:
			return "Tree";
		case Modules_StringStream ... Modules_StringStream + Manifest_GapSize - 1:
			return "StringStream";
		case Modules_Interface ... Modules_Interface + Manifest_GapSize - 1:
			return "Interface";
		case Modules_Deps ... Modules_Deps + Manifest_GapSize - 1:
			return "Deps";
		case Modules_Directory ... Modules_Directory + Manifest_GapSize - 1:
			return "Directory";
		case Modules_Builder ... Modules_Builder + Manifest_GapSize - 1:
			return "Builder";
		case Modules_Process ... Modules_Process + Manifest_GapSize - 1:
			return "Process";
		case Modules_Prototypes ... Modules_Prototypes + Manifest_GapSize - 1:
			return "Prototypes";
	}

	return "Unknown module";
}
