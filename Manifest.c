#import "Manifest.h"

static const char* codes[] = {
	[Memory_OutOfMemory] = "OutOfMemory",
	[Memory_Overlapping] = "Overlapping",
	[String_DoubleFree] = "DoubleFree",
	[String_BufferOverflow] = "BufferOverflow",
	[String_ElementMismatch] = "ElementMismatch",
	[Channel_IsFolder] = "IsFolder",
	[Channel_ReadingFailed] = "ReadingFailed",
	[Channel_ReadingInterrupted] = "ReadingInterrupted",
	[Channel_UnknownError] = "UnknownError",
	[Channel_WritingFailed] = "WritingFailed",
	[Channel_WritingInterrupted] = "WritingInterrupted",
	[MemoryMappedFile_InvalidFile] = "InvalidFile",
	[MemoryMappedFile_UnknownError] = "UnknownError",
	[ELF_InvalidFile] = "InvalidFile",
	[ELF_UnknownError] = "UnknownError",
	[Integer_Overflow] = "Overflow",
	[Integer_Underflow] = "Underflow",
	[Time_GetTimeOfDayFailed] = "GetTimeOfDayFailed",
	[Path_AccessDenied] = "AccessDenied",
	[Path_AlreadyExists] = "AlreadyExists",
	[Path_AttributeNonExistent] = "AttributeNonExistent",
	[Path_BufferTooSmall] = "BufferTooSmall",
	[Path_FolderNotEmpty] = "FolderNotEmpty",
	[Path_InsufficientSpace] = "InsufficientSpace",
	[Path_IsFolder] = "IsFolder",
	[Path_NameTooLong] = "NameTooLong",
	[Path_NonExistentFile] = "NonExistentFile",
	[Path_NonExistentPath] = "NonExistentPath",
	[Path_NotFolder] = "NotFolder",
	[Path_PermissionDenied] = "PermissionDenied",
	[Path_UnknownError] = "UnknownError",
	[Exception_AssertionFailed] = "AssertionFailed",
	[File_AccessDenied] = "AccessDenied",
	[File_AlreadyExists] = "AlreadyExists",
	[File_AttributeNonExistent] = "AttributeNonExistent",
	[File_BufferTooSmall] = "BufferTooSmall",
	[File_CannotOpenFile] = "CannotOpenFile",
	[File_GettingAttributeFailed] = "GettingAttributeFailed",
	[File_InvalidFileDescriptor] = "InvalidFileDescriptor",
	[File_InvalidParameter] = "InvalidParameter",
	[File_IsFolder] = "IsFolder",
	[File_NotFound] = "NotFound",
	[File_NotWritable] = "NotWritable",
	[File_SeekingFailed] = "SeekingFailed",
	[File_SettingAttributeFailed] = "SettingAttributeFailed",
	[File_StatFailed] = "StatFailed",
	[File_TruncatingFailed] = "TruncatingFailed",
	[NetworkAddress_GetAddrInfoFailed] = "GetAddrInfoFailed",
	[SocketConnection_ConnectionRefused] = "ConnectionRefused",
	[SocketConnection_ConnectionReset] = "ConnectionReset",
	[SocketConnection_FileDescriptorUnusable] = "FileDescriptorUnusable",
	[SocketConnection_InvalidFileDescriptor] = "InvalidFileDescriptor",
	[SocketConnection_NotConnected] = "NotConnected",
	[SocketConnection_UnknownError] = "UnknownError",
	[Socket_SetSocketOption] = "SetSocketOption",
	[Socket_SocketFailed] = "SocketFailed",
	[SocketServer_AddressInUse] = "AddressInUse",
	[SocketServer_AcceptFailed] = "AcceptFailed",
	[SocketServer_BindFailed] = "BindFailed",
	[SocketServer_ListenFailed] = "ListenFailed",
	[SocketServer_SetSocketOption] = "SetSocketOption",
	[ChannelWatcher_ChannelAlreadyAdded] = "ChannelAlreadyAdded",
	[ChannelWatcher_ChannelNotSupported] = "ChannelNotSupported",
	[ChannelWatcher_InvalidChannel] = "InvalidChannel",
	[ChannelWatcher_SettingCloexecFailed] = "SettingCloexecFailed",
	[ChannelWatcher_UnknownChannel] = "UnknownChannel",
	[ChannelWatcher_UnknownError] = "UnknownError",
	[Signal_Alarm] = "Alarm",
	[Signal_ArithmeticError] = "ArithmeticError",
	[Signal_BusError] = "BusError",
	[Signal_IllegalInstruction] = "IllegalInstruction",
	[Signal_InvalidMemoryAccess] = "InvalidMemoryAccess",
	[Signal_Pipe] = "Pipe",
	[Signal_UnknownError] = "UnknownError",
	[Folder_CannotOpenFolder] = "CannotOpenFolder",
	[Folder_ReadingFailed] = "ReadingFailed",
	[HashTable_TableIsFull] = "TableIsFull",
	[Locale_CorruptFile] = "CorruptFile",
	[Locale_Duplicate] = "Duplicate",
	[Terminal_IoctlFailed] = "IoctlFailed",
	[Terminal_ElementMismatch] = "ElementMismatch",
	[Process_ForkFailed] = "ForkFailed",
	[Process_SpawningProcessFailed] = "SpawningProcessFailed",
};

const char* Manifest_ResolveCode(unsigned int code) {
	if (code > sizeof(codes) / sizeof(codes[0])) {
		return "";
	}

	return codes[code];
}

const char* Manifest_ResolveName(int module) {
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
		case Modules_MemoryMappedFile ... Modules_MemoryMappedFile_Length:
			return "MemoryMappedFile";
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
		case Modules_EventQueue ... Modules_EventQueue_Length:
			return "EventQueue";
		case Modules_File ... Modules_File_Length:
			return "File";
		case Modules_NetworkAddress ... Modules_NetworkAddress_Length:
			return "NetworkAddress";
		case Modules_SocketConnection ... Modules_SocketConnection_Length:
			return "SocketConnection";
		case Modules_Socket ... Modules_Socket_Length:
			return "Socket";
		case Modules_SocketServer ... Modules_SocketServer_Length:
			return "SocketServer";
		case Modules_ChannelWatcher ... Modules_ChannelWatcher_Length:
			return "ChannelWatcher";
		case Modules_EventLoop ... Modules_EventLoop_Length:
			return "EventLoop";
		case Modules_Signal ... Modules_Signal_Length:
			return "Signal";
		case Modules_FPU ... Modules_FPU_Length:
			return "FPU";
		case Modules_Folder ... Modules_Folder_Length:
			return "Folder";
		case Modules_HashTable ... Modules_HashTable_Length:
			return "HashTable";
		case Modules_Locale ... Modules_Locale_Length:
			return "Locale";
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
		case Modules_Deps ... Modules_Deps_Length:
			return "Deps";
		case Modules_Process ... Modules_Process_Length:
			return "Process";
		case Modules_Block ... Modules_Block_Length:
			return "Block";
		case Modules_Unicode ... Modules_Unicode_Length:
			return "Unicode";
		case Modules_Terminal_Buffer ... Modules_Terminal_Buffer_Length:
			return "Terminal.Buffer";
		case Modules_Terminal_Prompt ... Modules_Terminal_Prompt_Length:
			return "Terminal.Prompt";
		case Modules_CPU ... Modules_CPU_Length:
			return "CPU";
		case Modules_Queue ... Modules_Queue_Length:
			return "Queue";
		case Modules_ManifestWriter ... Modules_ManifestWriter_Length:
			return "ManifestWriter";
		case Modules_Builder ... Modules_Builder_Length:
			return "Builder";
		case Modules_Interface ... Modules_Interface_Length:
			return "Interface";
	}

	return "Unknown module";
}
