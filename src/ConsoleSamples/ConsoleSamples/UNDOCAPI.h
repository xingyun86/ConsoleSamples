
#include <windows.h>

#include <map>
#include <string>

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRING std::string
#else
#define TSTRING std::wstring
#endif

#define _tstring TSTRING
#define tstring TSTRING

/////////////////////////////////////////////////////////////////////////////////
namespace PPSHUAI{

		typedef LONG NTSTATUS;

#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)   
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)   
#define STATUS_NOT_IMPLEMENTED          ((NTSTATUS)0xC0000002L)   
#define STATUS_INVALID_INFO_CLASS       ((NTSTATUS)0xC0000003L)   
#define STATUS_INFO_LENGTH_MISMATCH     ((NTSTATUS)0xC0000004L)   

		typedef enum _SYSTEM_INFORMATION_CLASS
		{
			SystemBasicInformation,                 //  0 Y N   
			SystemProcessorInformation,             //  1 Y N   
			SystemPerformanceInformation,           //  2 Y N   
			SystemTimeOfDayInformation,             //  3 Y N   
			SystemNotImplemented1,                  //  4 Y N   
			SystemProcessesAndThreadsInformation,   //  5 Y N   
			SystemCallCounts,                       //  6 Y N   
			SystemConfigurationInformation,         //  7 Y N   
			SystemProcessorTimes,                   //  8 Y N   
			SystemGlobalFlag,                       //  9 Y Y   
			SystemNotImplemented2,                  // 10 Y N   
			SystemModuleInformation,                // 11 Y N   
			SystemLockInformation,                  // 12 Y N   
			SystemNotImplemented3,                  // 13 Y N   
			SystemNotImplemented4,                  // 14 Y N   
			SystemNotImplemented5,                  // 15 Y N   
			SystemHandleInformation,                // 16 Y N   
			SystemObjectInformation,                // 17 Y N   
			SystemPagefileInformation,              // 18 Y N   
			SystemInstructionEmulationCounts,       // 19 Y N   
			SystemInvalidInfoClass1,                // 20   
			SystemCacheInformation,                 // 21 Y Y   
			SystemPoolTagInformation,               // 22 Y N   
			SystemProcessorStatistics,              // 23 Y N   
			SystemDpcInformation,                   // 24 Y Y   
			SystemNotImplemented6,                  // 25 Y N   
			SystemLoadImage,                        // 26 N Y   
			SystemUnloadImage,                      // 27 N Y   
			SystemTimeAdjustment,                   // 28 Y Y   
			SystemNotImplemented7,                  // 29 Y N   
			SystemNotImplemented8,                  // 30 Y N   
			SystemNotImplemented9,                  // 31 Y N   
			SystemCrashDumpInformation,             // 32 Y N   
			SystemExceptionInformation,             // 33 Y N   
			SystemCrashDumpStateInformation,        // 34 Y Y/N   
			SystemKernelDebuggerInformation,        // 35 Y N   
			SystemContextSwitchInformation,         // 36 Y N   
			SystemRegistryQuotaInformation,         // 37 Y Y   
			SystemLoadAndCallImage,                 // 38 N Y   
			SystemPrioritySeparation,               // 39 N Y   
			SystemNotImplemented10,                 // 40 Y N   
			SystemNotImplemented11,                 // 41 Y N   
			SystemInvalidInfoClass2,                // 42   
			SystemInvalidInfoClass3,                // 43   
			SystemTimeZoneInformation,              // 44 Y N   
			SystemLookasideInformation,             // 45 Y N   
			SystemSetTimeSlipEvent,                 // 46 N Y   
			SystemCreateSession,                    // 47 N Y   
			SystemDeleteSession,                    // 48 N Y   
			SystemInvalidInfoClass4,                // 49   
			SystemRangeStartInformation,            // 50 Y N   
			SystemVerifierInformation,              // 51 Y Y   
			SystemAddVerifier,                      // 52 N Y   
			SystemSessionProcessesInformation       // 53 Y N   

		} SYSTEM_INFORMATION_CLASS;

		typedef struct _LSA_UNICODE_STRING
		{
			USHORT Length;
			USHORT MaximumLength;
			PWSTR Buffer;

		} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

		typedef struct _CLIENT_ID
		{
			HANDLE UniqueProcess;
			HANDLE UniqueThread;

		} CLIENT_ID;

		typedef enum _THREAD_STATE
		{
			StateInitialized,
			StateReady,
			StateRunning,
			StateStandby,
			StateTerminated,
			StateWait,
			StateTransition,
			StateUnknown

		} THREAD_STATE;

		typedef enum _KWAIT_REASON
		{
			Executive,
			FreePage,
			PageIn,
			PoolAllocation,
			DelayExecution,
			Suspended,
			UserRequest,
			WrExecutive,
			WrFreePage,
			WrPageIn,
			WrPoolAllocation,
			WrDelayExecution,
			WrSuspended,
			WrUserRequest,
			WrEventPair,
			WrQueue,
			WrLpcReceive,
			WrLpcReply,
			WrVirtualMemory,
			WrPageOut,
			WrRendezvous,
			Spare2,
			Spare3,
			Spare4,
			Spare5,
			Spare6,
			WrKernel

		} KWAIT_REASON;

		/*typedef struct _IO_COUNTERS
		{
		LARGE_INTEGER ReadOperationCount;   //I/O读操作数目
		LARGE_INTEGER WriteOperationCount;  //I/O写操作数目
		LARGE_INTEGER OtherOperationCount;  //I/O其他操作数目
		LARGE_INTEGER ReadTransferCount;    //I/O读数据数目
		LARGE_INTEGER WriteTransferCount;   //I/O写数据数目
		LARGE_INTEGER OtherTransferCount;   //I/O其他操作数据数目

		} IO_COUNTERS, *PIO_COUNTERS;
		*/
		typedef struct _VM_COUNTERS
		{
			ULONG PeakVirtualSize;              //虚拟存储峰值大小   
			ULONG VirtualSize;                  //虚拟存储大小   
			ULONG PageFaultCount;               //页故障数目   
			ULONG PeakWorkingSetSize;           //工作集峰值大小   
			ULONG WorkingSetSize;               //工作集大小   
			ULONG QuotaPeakPagedPoolUsage;      //分页池使用配额峰值   
			ULONG QuotaPagedPoolUsage;          //分页池使用配额   
			ULONG QuotaPeakNonPagedPoolUsage;   //非分页池使用配额峰值   
			ULONG QuotaNonPagedPoolUsage;       //非分页池使用配额   
			ULONG PagefileUsage;                //页文件使用情况   
			ULONG PeakPagefileUsage;            //页文件使用峰值   

		} VM_COUNTERS, *PVM_COUNTERS;

		typedef LONG KPRIORITY;

		typedef struct _SYSTEM_THREADS32
		{
			LARGE_INTEGER KernelTime;
			LARGE_INTEGER UserTime;
			LARGE_INTEGER CreateTime;
			ULONG WaitTime;
			PVOID StartAddress;
			CLIENT_ID ClientId;
			KPRIORITY Priority;
			KPRIORITY BasePriority;
			ULONG ContextSwitchCount;
			THREAD_STATE State;
			KWAIT_REASON WaitReason;
		} SYSTEM_THREADS32, *PSYSTEM_THREADS32;

		typedef struct _SYSTEM_THREADS64
		{
			LARGE_INTEGER KernelTime;
			LARGE_INTEGER UserTime;
			LARGE_INTEGER CreateTime;
			ULONG WaitTime;
			PVOID StartAddress;
			CLIENT_ID ClientId;
			KPRIORITY Priority;
			KPRIORITY BasePriority;
			ULONG ContextSwitchCount;
			THREAD_STATE State;
			KWAIT_REASON WaitReason;
			ULONG Reserved;
		} SYSTEM_THREADS64, *PSYSTEM_THREADS64;

		typedef struct _SYSTEM_PROCESSES32
		{
			ULONG NextEntryDelta;
			ULONG ThreadCount;
			ULONG Reserved1[6];
			LARGE_INTEGER CreateTime;
			LARGE_INTEGER UserTime;
			LARGE_INTEGER KernelTime;
			UNICODE_STRING ProcessName;
			KPRIORITY BasePriority;
			ULONG ProcessId;
			ULONG InheritedFromProcessId;
			ULONG HandleCount;
			ULONG Reserved2[2];
			VM_COUNTERS VmCounters;
			IO_COUNTERS IoCounters;
			SYSTEM_THREADS32 Threads[1];
		} SYSTEM_PROCESSES32, *PSYSTEM_PROCESSES32;

		typedef struct _SYSTEM_PROCESSES64
		{
			ULONG NextEntryDelta;
			ULONG ThreadCount;
			ULONG Reserved1[6];
			LARGE_INTEGER CreateTime;
			LARGE_INTEGER UserTime;
			LARGE_INTEGER KernelTime;
			UNICODE_STRING ProcessName;
			KPRIORITY BasePriority;
			HANDLE ProcessId;
			HANDLE InheritedFromProcessId;
			ULONG HandleCount;
			ULONG Reserved2[2];
			SIZE_T PageDirectoryBase;
			VM_COUNTERS VmCounters;
			SIZE_T PrivatePageCount;
			IO_COUNTERS IoCounters;
			SYSTEM_THREADS64 Threads[1];
		} SYSTEM_PROCESSES64, *PSYSTEM_PROCESSES64;

		typedef struct _SYSTEM_BASIC_INFORMATION
		{
			BYTE Reserved1[24];
			PVOID Reserved2[4];
			CCHAR NumberOfProcessors;

		} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

		typedef struct _SYSTEM_MODULE_INFORMATION32 {
			ULONG Reserved[2];
			PVOID Base;
			ULONG Size;
			ULONG Flags;
			USHORT Index;
			USHORT Unknown;
			USHORT LoadCount;
			USHORT ModuleNameOffset;
			CHAR ImageName[256];
		} SYSTEM_MODULE_INFORMATION32, *PSYSTEM_MODULE_INFORMATION32;

		typedef struct _SYSTEM_MODULE_INFORMATION64 {
			ULONG Reserved[3];
			PVOID Base;
			ULONG Size;
			ULONG Flags;
			USHORT Index;
			USHORT Unknown;
			USHORT LoadCount;
			USHORT ModuleNameOffset;
			CHAR ImageName[256];
		} SYSTEM_MODULE_INFORMATION64, *PSYSTEM_MODULE_INFORMATION64;

#if !defined(_WIN64) && !defined(WIN64)
		typedef _SYSTEM_MODULE_INFORMATION32 _SYSTEM_MODULE_INFORMATION;
		typedef SYSTEM_MODULE_INFORMATION32 SYSTEM_MODULE_INFORMATION;
		typedef PSYSTEM_MODULE_INFORMATION32 PSYSTEM_MODULE_INFORMATION;

		typedef _SYSTEM_THREADS32 _SYSTEM_THREADS;
		typedef SYSTEM_THREADS32 SYSTEM_THREADS;
		typedef PSYSTEM_THREADS32 PSYSTEM_THREADS;

		typedef _SYSTEM_PROCESSES32 _SYSTEM_PROCESSES;
		typedef SYSTEM_PROCESSES32 SYSTEM_PROCESSES;
		typedef PSYSTEM_PROCESSES32 PSYSTEM_PROCESSES;
#else
		typedef _SYSTEM_MODULE_INFORMATION64 _SYSTEM_MODULE_INFORMATION;
		typedef SYSTEM_MODULE_INFORMATION64 SYSTEM_MODULE_INFORMATION;
		typedef PSYSTEM_MODULE_INFORMATION64 PSYSTEM_MODULE_INFORMATION;

		typedef _SYSTEM_THREADS64 _SYSTEM_THREADS;
		typedef SYSTEM_THREADS64 SYSTEM_THREADS;
		typedef PSYSTEM_THREADS64 PSYSTEM_THREADS;

		typedef _SYSTEM_PROCESSES64 _SYSTEM_PROCESSES;
		typedef SYSTEM_PROCESSES64 SYSTEM_PROCESSES;
		typedef PSYSTEM_PROCESSES64 PSYSTEM_PROCESSES;
#endif // !defined(_WIN64) && !defined(WIN64)

		typedef enum _OBJECT_INFORMATION_CLASSEX {
			ObjectBasicInformation = 0,
			ObjectNameInformation,
			ObjectTypeInformation,
			ObjectAllInformation,
			ObjectDataInformation,
		} OBJECT_INFORMATION_CLASSEX;

		typedef enum _PROCESSINFOCLASSEX
		{
			ProcessHandleInformation = 20,
		}PROCESSINFOCLASSEX;

		typedef struct _SYSTEM_HANDLE
		{
			ULONG ProcessId;
			BYTE ObjectTypeNumber;
			BYTE Flags;
			USHORT Handle;
			PVOID Object;
			ACCESS_MASK GrantAccess;
		}SYSTEM_HANDLE;

		typedef struct _SYSTEM_HANDLE_INFORMATION
		{
			DWORD HandleCount;
			SYSTEM_HANDLE Handles[1];
		}SYSTEM_HANDLE_INFORMATION;

		typedef struct _OBJECT_BASIC_INFORMATION {
			ULONG                   Attributes;
			ACCESS_MASK             DesiredAccess;
			ULONG                   HandleCount;
			ULONG                   ReferenceCount;
			ULONG                   PagedPoolUsage;
			ULONG                   NonPagedPoolUsage;
			ULONG                   Reserved[3];
			ULONG                   NameInformationLength;
			ULONG                   TypeInformationLength;
			ULONG                   SecurityDescriptorLength;
			LARGE_INTEGER           CreationTime;
		} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

		typedef struct _OBJECT_NAME_INFORMATION {
			UNICODE_STRING Name;
			WCHAR NameBuffer[1];
		} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

		typedef enum _POOL_TYPE {
			NonPagedPool,
			NonPagedPoolExecute = NonPagedPool,
			PagedPool,
			NonPagedPoolMustSucceed = NonPagedPool + 2,
			DontUseThisType,
			NonPagedPoolCacheAligned = NonPagedPool + 4,
			PagedPoolCacheAligned,
			NonPagedPoolCacheAlignedMustS = NonPagedPool + 6,
			MaxPoolType,
			NonPagedPoolBase = 0,
			NonPagedPoolBaseMustSucceed = NonPagedPoolBase + 2,
			NonPagedPoolBaseCacheAligned = NonPagedPoolBase + 4,
			NonPagedPoolBaseCacheAlignedMustS = NonPagedPoolBase + 6,
			NonPagedPoolSession = 32,
			PagedPoolSession = NonPagedPoolSession + 1,
			NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
			DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
			NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
			PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
			NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,
			NonPagedPoolNx = 512,
			NonPagedPoolNxCacheAligned = NonPagedPoolNx + 4,
			NonPagedPoolSessionNx = NonPagedPoolNx + 32
		} POOL_TYPE;

		typedef struct _OBJECT_TYPE_INFORMATION {
			UNICODE_STRING          TypeName;
			ULONG                   TotalNumberOfHandles;
			ULONG                   TotalNumberOfObjects;
			WCHAR                   Unused1[8];
			ULONG                   HighWaterNumberOfHandles;
			ULONG                   HighWaterNumberOfObjects;
			WCHAR                   Unused2[8];
			ACCESS_MASK             InvalidAttributes;
			GENERIC_MAPPING         GenericMapping;
			ACCESS_MASK             ValidAttributes;
			BOOLEAN                 SecurityRequired;
			BOOLEAN                 MaintainHandleCount;
			USHORT                  MaintainTypeList;
			POOL_TYPE               PoolType;
			ULONG                   DefaultPagedPoolCharge;
			ULONG                   DefaultNonPagedPoolCharge;
			BYTE                    Unknown2[16];
		} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

		typedef enum _MEMORY_INFORMATION_CLASS {
			MemoryBasicInformation
		} MEMORY_INFORMATION_CLASS, *PMEMORY_INFORMATION_CLASS;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	动态模块方法使用定义区
#if !defined(_UNICODE) && !defined(UNICODE)
#define __MY_T(V)	#V
#else
#define __MY_T(V)	L###V
#endif // !defined(_UNICODE) && !defined(UNICODE)
#define __MY_A(V)	#V

#define STRING	std::string

#define FUNC_TYPE(NAME)			(P_##NAME)

		typedef DWORD(WINAPI *FUNC_TYPE(ZwSuspendProcess))(HANDLE);
		typedef DWORD(WINAPI *FUNC_TYPE(ZwResumeProcess))(HANDLE);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwQueryInformationProcess))(HANDLE, PROCESSINFOCLASSEX, LPVOID, DWORD, PDWORD);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwQuerySystemInformation))(DWORD, PVOID, DWORD, DWORD*);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwQueryObject))(HANDLE, OBJECT_INFORMATION_CLASSEX, PVOID, ULONG, PULONG);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(RtlAdjustPrivilege))(DWORD, BOOL, BOOL, PDWORD);

		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwReadVirtualMemory))(HANDLE, PVOID, PVOID, ULONG, PULONG);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwWriteVirtualMemory))(HANDLE, PVOID, PVOID, ULONG, PULONG);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwQueryVirtualMemory))(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, ULONG, PULONG);

		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwWow64ReadVirtualMemory64))(HANDLE, PVOID64, PVOID, ULONGLONG, PULONGLONG);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwWow64WriteVirtualMemory64))(HANDLE, PVOID64, PVOID, ULONGLONG, PULONGLONG);
		typedef NTSTATUS(WINAPI *FUNC_TYPE(ZwWow64QueryVirtualMemory64))(HANDLE, PVOID64, MEMORY_INFORMATION_CLASS, PVOID, ULONGLONG, PULONGLONG);

		typedef std::map<TSTRING, TSTRING> TSTRINGSTRINGMAP;
		typedef TSTRINGSTRINGMAP::iterator TSTRINGSTRINGMAPIT;
		typedef TSTRINGSTRINGMAP::value_type TSTRINGSTRINGMAPPAIR;
#define MAPKV_INIT_STR(NAME)  {__MY_T(#NAME), __MY_T(#NAME)}

		typedef std::map<TSTRING, VOID *> TSTRINGHMODULEMAP;
		typedef TSTRINGHMODULEMAP::iterator TSTRINGHMODULEMAPIT;
		typedef TSTRINGHMODULEMAP::value_type TSTRINGHMODULEMAPPAIR;

#define LIB_MAP_BEGIN(NAME)	static TSTRINGHMODULEMAP G_##NAME_LIBMAP = {
#define LIB_MAP_END()		};
#define MAPKV_INIT_LIB(NAME)  {__MY_T(NAME), 0}

		typedef std::map<STRING, VOID *> STRINGMETHODMAP;
		typedef STRINGMETHODMAP::iterator STRINGMETHODMAPIT;
		typedef STRINGMETHODMAP::value_type STRINGMETHODMAPPAIR;
#define FUN_MAP_BEGIN(NAME)	static STRINGMETHODMAP G_##NAME_FUNMAP = {
#define FUN_MAP_END()		};
#define MAPKV_INIT_FUN(NAME)  {__MY_A(NAME), 0}

		////////////////////////////////////////////////////////////////////////////////////
		//	LIBRARY MODULES
		//
		LIB_MAP_BEGIN()
			MAPKV_INIT_LIB(NTDLL),
			MAPKV_INIT_LIB(KERNEL32),
			LIB_MAP_END()

#define LIB_MAP(NAME)			(&G_##NAME_LIBMAP)

			////////////////////////////////////////////////////////////////////////////////////
			//	MODULES METHODS
			//
			FUN_MAP_BEGIN()
			MAPKV_INIT_FUN(ZwQueryInformationProcess),
			MAPKV_INIT_FUN(ZwQuerySystemInformation),
			MAPKV_INIT_FUN(ZwQueryObject),
			MAPKV_INIT_FUN(RtlAdjustPrivilege),
			MAPKV_INIT_FUN(ZwSuspendProcess),
			MAPKV_INIT_FUN(ZwResumeProcess),
			FUN_MAP_END()

#define FUNC_PROC(NAME)			(FUNC_TYPE(NAME)(G_##NAME_FUNMAP[#NAME]))

#define FUN_MAP(NAME)			(&G_##NAME_FUNMAP)
	
		namespace UNDOCAPI {
			__inline static int InitUnDocumentApis()
			{
				int nResult = 0;

				for (auto itX = LIB_MAP()->begin(); itX != LIB_MAP()->end(); itX++)
				{
					itX->second = (VOID *)GetModuleHandle(itX->first.c_str());
					if (!itX->second)
					{
						itX->second = (VOID *)LoadLibrary(itX->first.c_str());
					}
					if (itX->second)
					{
						for (auto itY = FUN_MAP()->begin(); itY != FUN_MAP()->end(); itY++)
						{
							if (!itY->second)
							{
								itY->second = GetProcAddress((HMODULE)itX->second, itY->first.c_str());
								if (itY->second)
								{
									nResult += 1;
								}
							}
						}
					}
				}

				return nResult;
			}
			//	动态模块方法使用定义区
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
}