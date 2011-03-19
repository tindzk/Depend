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
	Modules_Kernel,
	Modules_Kernel_Length,
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
	Modules_Signal,
	Signal_SigAlrm,
	Signal_SigBus,
	Signal_SigFpe,
	Signal_SigIll,
	Signal_SigInt,
	Signal_SigPipe,
	Signal_SigQuit,
	Signal_SigSegv,
	Signal_SigTerm,
	Signal_SignalHandlerNotSet,
	Signal_Unknown,
	Modules_Signal_Length,
	Modules_Logger,
	Modules_Logger_Length,
	Modules_Terminal,
	Terminal_IoctlFailed,
	Terminal_ElementMismatch,
	Modules_Terminal_Length,
	Modules_Application,
	Modules_Application_Length,
	Modules_Tree,
	Modules_Tree_Length,
	Modules_Typography,
	Typography_IllegalNesting,
	Modules_Typography_Length,
	Modules_StringStream,
	Modules_StringStream_Length,
	Modules_Terminal_Controller,
	Modules_Terminal_Controller_Length,
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

static char* codes[] = {
	[String_DoubleFree] = "DoubleFree",
	[String_BufferOverflow] = "BufferOverflow",
	[String_ElementMismatch] = "ElementMismatch",
	[Pool_HasParent] = "HasParent",
	[Pool_NotBundling] = "NotBundling",
	[Pool_AlreadyBundling] = "AlreadyBundling",
	[Memory_NullPointer] = "NullPointer",
	[Memory_OutOfBounds] = "OutOfBounds",
	[Memory_OutOfMemory] = "OutOfMemory",
	[Memory_Overlapping] = "Overlapping",
	[File_AccessDenied] = "AccessDenied",
	[File_AlreadyExists] = "AlreadyExists",
	[File_AttributeNonExistent] = "AttributeNonExistent",
	[File_BufferTooSmall] = "BufferTooSmall",
	[File_CannotOpenFile] = "CannotOpenFile",
	[File_GettingAttributeFailed] = "GettingAttributeFailed",
	[File_InvalidFileDescriptor] = "InvalidFileDescriptor",
	[File_InvalidParameter] = "InvalidParameter",
	[File_IsDirectory] = "IsDirectory",
	[File_NotFound] = "NotFound",
	[File_NotReadable] = "NotReadable",
	[File_NotWritable] = "NotWritable",
	[File_ReadingFailed] = "ReadingFailed",
	[File_ReadingInterrupted] = "ReadingInterrupted",
	[File_SeekingFailed] = "SeekingFailed",
	[File_SettingAttributeFailed] = "SettingAttributeFailed",
	[File_StatFailed] = "StatFailed",
	[File_TruncatingFailed] = "TruncatingFailed",
	[File_WritingFailed] = "WritingFailed",
	[File_WritingInterrupted] = "WritingInterrupted",
	[Integer_Overflow] = "Overflow",
	[Integer_Underflow] = "Underflow",
	[Signal_SigAlrm] = "SigAlrm",
	[Signal_SigBus] = "SigBus",
	[Signal_SigFpe] = "SigFpe",
	[Signal_SigIll] = "SigIll",
	[Signal_SigInt] = "SigInt",
	[Signal_SigPipe] = "SigPipe",
	[Signal_SigQuit] = "SigQuit",
	[Signal_SigSegv] = "SigSegv",
	[Signal_SigTerm] = "SigTerm",
	[Signal_SignalHandlerNotSet] = "SignalHandlerNotSet",
	[Signal_Unknown] = "Unknown",
	[Terminal_IoctlFailed] = "IoctlFailed",
	[Terminal_ElementMismatch] = "ElementMismatch",
	[Typography_IllegalNesting] = "IllegalNesting",
	[Time_GetTimeOfDayFailed] = "GetTimeOfDayFailed",
	[Path_AccessDenied] = "AccessDenied",
	[Path_AlreadyExists] = "AlreadyExists",
	[Path_AttributeNonExistent] = "AttributeNonExistent",
	[Path_BufferTooSmall] = "BufferTooSmall",
	[Path_CreationFailed] = "CreationFailed",
	[Path_DeletingFailed] = "DeletingFailed",
	[Path_DirectoryNotEmpty] = "DirectoryNotEmpty",
	[Path_EmptyPath] = "EmptyPath",
	[Path_GettingAttributeFailed] = "GettingAttributeFailed",
	[Path_InsufficientSpace] = "InsufficientSpace",
	[Path_IsDir] = "IsDir",
	[Path_NameTooLong] = "NameTooLong",
	[Path_NonExistentFile] = "NonExistentFile",
	[Path_NonExistentPath] = "NonExistentPath",
	[Path_NotDirectory] = "NotDirectory",
	[Path_PermissionDenied] = "PermissionDenied",
	[Path_ReadingLinkFailed] = "ReadingLinkFailed",
	[Path_ResolvingFailed] = "ResolvingFailed",
	[Path_SettingAttributeFailed] = "SettingAttributeFailed",
	[Path_SettingTimeFailed] = "SettingTimeFailed",
	[Path_StatFailed] = "StatFailed",
	[Path_TruncatingFailed] = "TruncatingFailed",
	[Directory_CannotOpenDirectory] = "CannotOpenDirectory",
	[Directory_ReadingFailed] = "ReadingFailed",
	[Process_ForkFailed] = "ForkFailed",
	[Process_SpawningProcessFailed] = "SpawningProcessFailed",
	[Builder_RuntimeError] = "RuntimeError",
};

static inline char* Manifest_ResolveCode(unsigned int code) {
	if (code > sizeof(codes) / sizeof(codes[0])) {
		return "";
	}

	return codes[code];
}

static inline char* Manifest_ResolveName(int module) {
	switch (module) {
		case Modules_String ... Modules_String_Length:
			return "String";
		case Modules_Pool ... Modules_Pool_Length:
			return "Pool";
		case Modules_Memory ... Modules_Memory_Length:
			return "Memory";
		case Modules_Kernel ... Modules_Kernel_Length:
			return "Kernel";
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
		case Modules_Signal ... Modules_Signal_Length:
			return "Signal";
		case Modules_Logger ... Modules_Logger_Length:
			return "Logger";
		case Modules_Terminal ... Modules_Terminal_Length:
			return "Terminal";
		case Modules_Application ... Modules_Application_Length:
			return "Application";
		case Modules_Tree ... Modules_Tree_Length:
			return "Tree";
		case Modules_Typography ... Modules_Typography_Length:
			return "Typography";
		case Modules_StringStream ... Modules_StringStream_Length:
			return "StringStream";
		case Modules_Terminal_Controller ... Modules_Terminal_Controller_Length:
			return "Terminal.Controller";
		case Modules_Time ... Modules_Time_Length:
			return "Time";
		case Modules_Path ... Modules_Path_Length:
			return "Path";
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
