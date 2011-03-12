enum {
	Modules_String,
	String_DoubleFree,
	String_BufferOverflow,
	String_ElementMismatch,
	Modules_String_Length,
	Modules_Pool,
	Pool_HasParent,
	Pool_NotBundling,
	Pool_AlreadyBundling,
	Modules_Pool_Length,
	Modules_Memory,
	Memory_NullPointer,
	Memory_OutOfBounds,
	Memory_OutOfMemory,
	Memory_Overlapping,
	Modules_Memory_Length,
	Modules_Hex,
	Modules_Hex_Length,
	Modules_File,
	File_AccessDenied,
	File_AlreadyExists,
	File_AttributeNonExistent,
	File_BufferTooSmall,
	File_CannotOpenFile,
	File_GettingAttributeFailed,
	File_InvalidFileDescriptor,
	File_InvalidParameter,
	File_IsDirectory,
	File_NotFound,
	File_NotReadable,
	File_NotWritable,
	File_ReadingFailed,
	File_ReadingInterrupted,
	File_SeekingFailed,
	File_SettingAttributeFailed,
	File_StatFailed,
	File_TruncatingFailed,
	File_WritingFailed,
	File_WritingInterrupted,
	Modules_File_Length,
	Modules_Integer,
	Integer_Overflow,
	Integer_Underflow,
	Modules_Integer_Length,
	Modules_BFD,
	Modules_BFD_Length,
	Modules_Exception,
	Modules_Exception_Length,
	Modules_Kernel,
	Modules_Kernel_Length,
	Modules_Time,
	Time_GetTimeOfDayFailed,
	Modules_Time_Length,
	Modules_Path,
	Path_AccessDenied,
	Path_AlreadyExists,
	Path_AttributeNonExistent,
	Path_BufferTooSmall,
	Path_CreationFailed,
	Path_DeletingFailed,
	Path_DirectoryNotEmpty,
	Path_EmptyPath,
	Path_GettingAttributeFailed,
	Path_InsufficientSpace,
	Path_IsDir,
	Path_NameTooLong,
	Path_NonExistentFile,
	Path_NonExistentPath,
	Path_NotDirectory,
	Path_PermissionDenied,
	Path_ReadingLinkFailed,
	Path_ResolvingFailed,
	Path_SettingAttributeFailed,
	Path_SettingTimeFailed,
	Path_StatFailed,
	Path_TruncatingFailed,
	Modules_Path_Length,
	Modules_Logger,
	Modules_Logger_Length,
	Modules_Terminal,
	Terminal_IoctlFailed,
	Terminal_ElementMismatch,
	Modules_Terminal_Length,
	Modules_Tree,
	Modules_Tree_Length,
	Modules_Typography,
	Typography_IllegalNesting,
	Modules_Typography_Length,
	Modules_StringStream,
	Modules_StringStream_Length,
	Modules_Terminal_Controller,
	Modules_Terminal_Controller_Length,
	Modules_Directory,
	Directory_CannotOpenDirectory,
	Directory_ReadingFailed,
	Modules_Directory_Length,
	Modules_Deps,
	Modules_Deps_Length,
	Modules_Process,
	Process_ForkFailed,
	Process_SpawningProcessFailed,
	Modules_Process_Length,
	Modules_Builder,
	Builder_RuntimeError,
	Modules_Builder_Length,
	Modules_Prototypes,
	Modules_Prototypes_Length,
	Modules_Interface,
	Modules_Interface_Length,
};

static inline char* Manifest_ResolveName(int module) {
	switch (module) {
		case Modules_String ... Modules_String_Length:
			return "String";
		case Modules_Pool ... Modules_Pool_Length:
			return "Pool";
		case Modules_Memory ... Modules_Memory_Length:
			return "Memory";
		case Modules_Hex ... Modules_Hex_Length:
			return "Hex";
		case Modules_File ... Modules_File_Length:
			return "File";
		case Modules_Integer ... Modules_Integer_Length:
			return "Integer";
		case Modules_BFD ... Modules_BFD_Length:
			return "BFD";
		case Modules_Exception ... Modules_Exception_Length:
			return "Exception";
		case Modules_Kernel ... Modules_Kernel_Length:
			return "Kernel";
		case Modules_Time ... Modules_Time_Length:
			return "Time";
		case Modules_Path ... Modules_Path_Length:
			return "Path";
		case Modules_Logger ... Modules_Logger_Length:
			return "Logger";
		case Modules_Terminal ... Modules_Terminal_Length:
			return "Terminal";
		case Modules_Tree ... Modules_Tree_Length:
			return "Tree";
		case Modules_Typography ... Modules_Typography_Length:
			return "Typography";
		case Modules_StringStream ... Modules_StringStream_Length:
			return "StringStream";
		case Modules_Terminal_Controller ... Modules_Terminal_Controller_Length:
			return "Terminal.Controller";
		case Modules_Directory ... Modules_Directory_Length:
			return "Directory";
		case Modules_Deps ... Modules_Deps_Length:
			return "Deps";
		case Modules_Process ... Modules_Process_Length:
			return "Process";
		case Modules_Builder ... Modules_Builder_Length:
			return "Builder";
		case Modules_Prototypes ... Modules_Prototypes_Length:
			return "Prototypes";
		case Modules_Interface ... Modules_Interface_Length:
			return "Interface";
	}

	return "Unknown module";
}
