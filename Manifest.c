#import "Manifest.h"

static char* codes[] = {
	[Memory_OutOfMemory] = "OutOfMemory",
	[Memory_Overlapping] = "Overlapping",
	[String_DoubleFree] = "DoubleFree",
	[String_BufferOverflow] = "BufferOverflow",
	[String_ElementMismatch] = "ElementMismatch",
	[Channel_IsDirectory] = "IsDirectory",
	[Channel_ReadingFailed] = "ReadingFailed",
	[Channel_ReadingInterrupted] = "ReadingInterrupted",
	[Channel_UnknownError] = "UnknownError",
	[Channel_WritingFailed] = "WritingFailed",
	[Channel_WritingInterrupted] = "WritingInterrupted",
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
	[File_NotWritable] = "NotWritable",
	[File_SeekingFailed] = "SeekingFailed",
	[File_SettingAttributeFailed] = "SettingAttributeFailed",
	[File_StatFailed] = "StatFailed",
	[File_TruncatingFailed] = "TruncatingFailed",
	[ELF_InvalidFile] = "InvalidFile",
	[ELF_UnknownError] = "UnknownError",
	[Integer_Overflow] = "Overflow",
	[Integer_Underflow] = "Underflow",
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
	[Exception_AssertFailed] = "AssertFailed",
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
	[Directory_CannotOpenDirectory] = "CannotOpenDirectory",
	[Directory_ReadingFailed] = "ReadingFailed",
	[Process_ForkFailed] = "ForkFailed",
	[Process_SpawningProcessFailed] = "SpawningProcessFailed",
	[Builder_RuntimeError] = "RuntimeError",
};

char* Manifest_ResolveCode(unsigned int code) {
	if (code > sizeof(codes) / sizeof(codes[0])) {
		return "";
	}

	return codes[code];
}

char* Manifest_ResolveName(int module) {
	switch (module) {
		case Modules_DynObject ... Modules_DynObject_Length:
			return "DynObject";
		case Modules_Memory ... Modules_Memory_Length:
			return "Memory";
		case Modules_Buffer ... Modules_Buffer_Length:
			return "Buffer";
		case Modules_String ... Modules_String_Length:
			return "String";
		case Modules_Kernel ... Modules_Kernel_Length:
			return "Kernel";
		case Modules_Channel ... Modules_Channel_Length:
			return "Channel";
		case Modules_System ... Modules_System_Length:
			return "System";
		case Modules_File ... Modules_File_Length:
			return "File";
		case Modules_ELF ... Modules_ELF_Length:
			return "ELF";
		case Modules_Hex ... Modules_Hex_Length:
			return "Hex";
		case Modules_Integer ... Modules_Integer_Length:
			return "Integer";
		case Modules_Time ... Modules_Time_Length:
			return "Time";
		case Modules_Path ... Modules_Path_Length:
			return "Path";
		case Modules_LEB128 ... Modules_LEB128_Length:
			return "LEB128";
		case Modules_DWARF ... Modules_DWARF_Length:
			return "DWARF";
		case Modules_Backtrace ... Modules_Backtrace_Length:
			return "Backtrace";
		case Modules_Exception ... Modules_Exception_Length:
			return "Exception";
		case Modules_Signal ... Modules_Signal_Length:
			return "Signal";
		case Modules_Logger ... Modules_Logger_Length:
			return "Logger";
		case Modules_Terminal ... Modules_Terminal_Length:
			return "Terminal";
		case Modules_Memory_Map ... Modules_Memory_Map_Length:
			return "Memory.Map";
		case Modules_Memory_Libc ... Modules_Memory_Libc_Length:
			return "Memory.Libc";
		case Modules_Memory_Logger ... Modules_Memory_Logger_Length:
			return "Memory.Logger";
		case Modules_Application ... Modules_Application_Length:
			return "Application";
		case Modules_Ecriture ... Modules_Ecriture_Length:
			return "Ecriture";
		case Modules_StringReader ... Modules_StringReader_Length:
			return "StringReader";
		case Modules_Ecriture_Parser ... Modules_Ecriture_Parser_Length:
			return "Ecriture.Parser";
		case Modules_Terminal_Controller ... Modules_Terminal_Controller_Length:
			return "Terminal.Controller";
		case Modules_Tree ... Modules_Tree_Length:
			return "Tree";
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
