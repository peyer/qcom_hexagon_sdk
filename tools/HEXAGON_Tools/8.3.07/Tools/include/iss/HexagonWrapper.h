/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
 *****************************************************************/

#ifndef _HEXWRAPPER_H_
#define _HEXWRAPPER_H_

#include "HexagonTypes.h"

struct HEX_uarch_callback_info_struct;
typedef void (*HEX_uarch_callback_t) (void *user_data, struct HEX_uarch_callback_info_struct *callback_info_struct);

typedef struct HEX_opcode_info_struct HEX_opcode_info_t;

#include <stdio.h>

#ifndef DLL_A_EXPORT
#ifdef _WIN32
#ifndef IN_DLL
#define DLL_A_EXPORT __declspec(dllexport)
#else
#define DLL_A_EXPORT __declspec(dllimport)
#endif
class DLL_A_EXPORT HexagonWrapper;
#undef INTERFACE
#define INTERFACE __declspec (dllexport)
#else
#define DLL_A_EXPORT
#define INTERFACE
class HexagonWrapper;
#endif
#endif

#define MAJ_API_VERSION     "3"
#define MIN_API_VERSION     "35"

const char HEXAGON_WRAPPER_VERSION[] =   MAJ_API_VERSION  "."  MIN_API_VERSION;

#define HEXAGON_MAX_RECORD_BUFFER_SZ 512
#define RESERVE_SIZE (64)

class DLL_A_EXPORT HexagonWrapper
{
    public:
        HexagonWrapper(HEXAPI_Cpu cpu, HEX_4u_t flags = 0, HEX_4u_t recordflag = 0);
        HexagonWrapper(char *cpu_name, HEX_4u_t flags = 0, HEX_4u_t recordflag = 0);
        HexagonWrapper(HEXAPI_Cpu cpu, const char *libPath, HEX_4u_t flags = 0, HEX_4u_t recordflag = 0);
        ~HexagonWrapper();

        // development/debug info
        void PrintBuildTag(); // output to stdio only

        // Configuration Methods
        HEXAPI_Status  ConfigureAppCommandLine(int argc, char **argv);
        HEXAPI_Status  ConfigureRemoteDebug(HEX_4u_t portNum, bool reversed = false);
        HEXAPI_Status  ConfigureAHB(HEX_PA_t AHBstartAddr, HEX_PA_t AHBendAddr);
        HEXAPI_Status  ConfigureAXI2(HEX_PA_t lowStartAddr, HEX_PA_t highEndAddr);
        HEXAPI_Status  ConfigureTCM(HEX_PA_t TCMstartAddr);
        HEXAPI_Status  ConfigureInitialEVB(HEX_PA_t init_evb);

        // for the next 4 functions, only used to write values into memory
        // they don't trigger any other function or behavior.
        // only most significant five nibbles are in the input arg
        // lower 16b of PA are assumed to be zero
        HEXAPI_Status  ConfigureSubsystemBase(HEX_4u_t subStart);
        HEXAPI_Status  ConfigureL2tcmBase(HEX_4u_t l2tcmStart);
        HEXAPI_Status  ConfigureEtmcfgBase(HEX_4u_t etmStart);
        HEXAPI_Status  ConfigureL2cfgBase(HEX_4u_t l2cfgStart);
        HEXAPI_Status  ConfigureAxim2Base(HEX_4u_t l2cfgStart);
        HEXAPI_Status  ConfigureStreamerBase(HEX_4u_t l2cfgStart);
        HEXAPI_Status  ConfigureCladeBase(HEX_4u_t cladeStart);
        HEXAPI_Status  ConfigureFastL2vicBase(HEX_4u_t fastL2vicStart);

        HEXAPI_Status  ConfigureCosim(const char *CosimConfigFile, HEXAPI_OpenMode mode = HEX_MODE_READ);
        HEXAPI_Status  ConfigureOSAwareness(const char *OsamConfigFile, HEXAPI_OpenMode mode = HEX_MODE_READ);
        HEXAPI_Status  ConfigureGProf(const char *gprofFilePath = NULL);
        HEXAPI_Status  ConfigureCoreFrequency(HEX_8u_t DSPclkHz);
        HEXAPI_Status  ConfigureTimingMode(HEXAPI_TimingMode mode);
        HEXAPI_Status  ConfigureSimStdin(const char *fIn, HEXAPI_OpenMode mode = HEX_MODE_READ);
        HEXAPI_Status  ConfigureSimStdout(const char *fOut, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status  ConfigureSimStderr(const char *fErr, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status  ConfigureExecutableBinary(const char *fElf);
        HEXAPI_Status  ConfigureMemFill(HEX_1u_t value);
        HEXAPI_Status  ConfigureMemFillRandom(HEX_4u_t seed);
        HEXAPI_Status  ConfigureStatisticsFile(const char *fStats, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status  ConfigurePmuStatisticsFile(const char *fStats, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status  ConfigurePCRangeFilter(HEX_VA_t PCstart, HEX_VA_t PCend);
        HEXAPI_Status  ConfigureTimeRangeFilter(HEX_8u_t TimeStart, HEXAPI_Interval StartUnits, HEX_8u_t TimeEnd, HEXAPI_Interval EndUnits);
        HEXAPI_Status  ConfigureNULLPointerBehavior(HEXAPI_Nullptr behavior);
        HEXAPI_Status  ConfigureConnectionTimeout(HEX_4s_t seconds);
        HEXAPI_Status  ConfigureHVXLength(HEX_4s_t HVXlengthBytes);
#ifdef _WIN32
        __declspec(deprecated) HEXAPI_Status  ConfigureHeapInfo(HEX_PA_t HeapBase, HEX_4u_t HeapSize);
        HEXAPI_Status  ConfigureStackInfo(HEX_PA_t StackBase, HEX_4u_t StackSize);
#else
        HEXAPI_Status  ConfigureHeapInfo(HEX_PA_t HeapBase, HEX_4u_t HeapSize) __attribute__ ((deprecated));
        HEXAPI_Status  ConfigureStackInfo(HEX_PA_t StackBase, HEX_4u_t StackSize);
#endif
        HEXAPI_Status ConfigureCladeTraceFile(const char * filename,
          HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status SetCladeTraceLevel(int level);
        HEXAPI_Status GetCladeTraceLevel(int *level);
        HEXAPI_Status SetCladeTraceString(const char *str);

        // busratio and buspenalty
        HEXAPI_Status  ConfigureBusRatio(HEX_4f_t BusRatio);
        HEXAPI_Status  ConfigureAXI2BusRatio(HEX_4f_t BusRatio);
        HEXAPI_Status  ConfigureAHBBusRatio(HEX_4f_t BusRatio);
        HEXAPI_Status  ConfigureBusPenalty(HEX_8u_t BusPenaltyInterval, HEXAPI_Interval units);
        HEXAPI_Status  ConfigureAXI2BusPenalty(HEX_8u_t BusPenaltyInterval, HEXAPI_Interval units);
        HEXAPI_Status  ConfigureAHBBusPenalty(HEX_8u_t BusPenaltyInterval, HEXAPI_Interval units);

        HEXAPI_Status  ConfigureProfileMode(bool bTimeZero);
        HEXAPI_Status  ConfigureArchString(const char *archstring);

        HEXAPI_Status  EndOfConfiguration();

        // External Device Callback Register/UnRegister
        HEXAPI_Status  AddBusAccessCallback(void *deviceHandle, HEX_PA_t phyStartAddr, HEX_PA_t phyEndAddr,
                bus_transaction_request_callback btrc);
        HEXAPI_Status  RemoveBusAccessCallback(void *deviceHandle);
        HEXAPI_Status  AddFrequencyChangeCallback(void *deviceHandle, frequency_change_callback fcc);
        HEXAPI_Status  RemoveFrequencyChangeCallback(void *deviceHandle);
        HEXAPI_Status  AddTimedCallbackFP(void *deviceHandle, HEX_8f_t interval , HEXAPI_Interval iTyp,
                cosim_callback stc);
        HEXAPI_Status  AddTimedCallback(void *deviceHandle, HEX_8u_t interval, HEXAPI_Interval iTyp,
                cosim_callback stc);
        HEXAPI_Status  AddOneShotTimedCallback(void *deviceHandle, HEX_8u_t interval, HEXAPI_Interval iTyp,
                cosim_callback stc);
        HEXAPI_Status  RemoveTimedCallback(void *deviceHandle);
        HEXAPI_Status  AddMemWasWrittenCallback(void *deviceHandle, HEX_PA_t phyStartAddr, HEX_PA_t phyEndAddr,
                memory_written_callback mwc);
        HEXAPI_Status  RemoveMemWasWrittenCallback(void *deviceHandle);
        HEXAPI_Status  AddMemWasReadCallback(void *deviceHandle, HEX_PA_t phyStartAddr, HEX_PA_t phyEndAddr,
                memory_read_callback mrc);
        HEXAPI_Status  RemoveMemWasReadCallback(void *deviceHandle);
        HEXAPI_Status  AddPCCallback(void *deviceHandle, HEX_VA_t virtAddr, cosim_callback pcc);
        HEXAPI_Status  RemovePCCallback(void *deviceHandle, HEX_VA_t virtAddr);
        HEXAPI_Status  AddBeforeSimulationStartsCallback( void *deviceHandle, cosim_callback sc );
        HEXAPI_Status  AddEndOfSimulationCallback( void *deviceHandle, cosim_callback sc );
        HEXAPI_Status  AddCoreReadyCallback( void *deviceHandle, core_ready_callback cc );
        HEXAPI_Status  AddUserDefinedInstCallback(void *deviceHandle, user_defined_instruction_callback udic );
        HEXAPI_Status  AddPrivilegeChangeCallback (void *deviceHandle, privilege_change_callback cc);
        HEXAPI_Status  AddQtimerCallback(void *deviceHandle, qtimer_callback qtc);
        HEXAPI_Status  AddUarchCallback(HEX_uarch_callback_t uc, void * ud);
        void GetAPIVersion(HEX_4u_t *major, HEX_4u_t *minor, HEX_4u_t *build,
            HEX_8u_t *internal = NULL);

        // Runtime simulator calls
        void EVB(HEX_PA_t EVBstartAddr);
        HEX_8u_t  CoreFrequency(HEX_8u_t DSPclkHz);
        void VerboseMode(HEXAPI_VerboseMode mode);
        void DebugMode(int mode);
        HEXAPI_Status ConfigureBypassIdle(bool mode);
        HEXAPI_Status ConfigureCoreDump(const char * filename);
        HEXAPI_Status  AddSymbolFile(const char *fSymFile);
        HEXAPI_Status  SetTracing(HEXAPI_TracingType type, const char *fTraceFile, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status  PostMessageToSimulator(HEXAPI_RxMsgType, void* param_addr);
        HEX_4s_t GetOpcodeInfo(HEX_2u_t opcode, HEX_opcode_info_t *info);
        HEX_4u_t GetMaxOpcodes(void);

        // Simulator Control
        HEXAPI_Status       LoadExecutableBinary(const char *fElf = NULL);
        HEXAPI_CoreState    Run(HEX_4u_t *result = NULL);
        HEXAPI_CoreState    RunInteractive(HEX_4u_t *result = NULL);
        HEXAPI_CoreState    Step(HEX_4u_t cycles, HEX_4u_t *cyclesExecuted, HEX_4u_t *result = NULL);
        HEXAPI_CoreState    StepTime(HEX_8u_t time, HEXAPI_Interval units, HEX_4u_t *cyclesExecuted, HEX_4u_t *result = NULL);
        HEXAPI_Status  SetInterrupt(HEX_4u_t  interruptNum, HEXAPI_InterruptPinState state);
        HEXAPI_Status  AssertNMI();
        HEXAPI_Status  DeassertNMI();
        HEXAPI_Status  ClearInterrupt(HEX_4u_t  interruptNum);
        void ClearAllInterrupts();
        HEXAPI_Status  SetBreakpoint(HEX_VA_t virtAddr);
        HEXAPI_Status  ClearBreakpoint(HEX_VA_t virtAddr);
        HEXAPI_Status  ClearAllBreakpoints();
        HEXAPI_Status  AssertReset();
        HEXAPI_Status  DeassertReset();
        void  BusTransactionFinished(HEX_1u_t *data, HEX_4u_t size, HEX_4u_t requestID);
        HEXAPI_Status  WriteThreadRegister(HEX_4u_t tnum, HEXAPI_TH_REG index, HEX_4u_t value);
        HEXAPI_Status  ReadThreadRegister(HEX_4u_t tnum, HEXAPI_TH_REG index, HEX_4u_t *value);
        HEXAPI_Status  WriteGlobalRegister(HEXAPI_G_REG index, HEX_4u_t value);
        HEXAPI_Status  ReadGlobalRegister(HEXAPI_G_REG index, HEX_4u_t *value);
        HEXAPI_Status  WriteCoprocRegister(HEX_4u_t coproc_id, char * regtype, HEX_4u_t reg_id, HEX_4u_t index, HEX_4u_t value);
        HEXAPI_Status  ReadCoprocRegister(HEX_4u_t coproc_id, char * regtype, HEX_4u_t reg_id, HEX_4u_t index, HEX_4u_t *value);
        HEXAPI_Status  WriteTLBRegister(int index, HEX_8u_t value);
        HEXAPI_Status  ReadTLBRegister(int index, HEX_8u_t *value);

        // the next two meant for v60 vector register
        HEXAPI_Status  WriteVecRegister(char * regtype, HEX_4u_t, int extno, HEX_4u_t reg_id, HEX_4u_t index, HEX_4u_t value);
        HEXAPI_Status  ReadVecRegister(char * regtype, HEX_4u_t, int extno, HEX_4u_t reg_id, HEX_4u_t index, HEX_4u_t *value);
        HEXAPI_Status  WriteMemory(HEX_PA_t phyAddr, HEX_4u_t size, HEX_8u_t value);
        HEXAPI_Status  ReadMemory(HEX_PA_t phyAddr, HEX_4u_t size, void *value);
        HEXAPI_Status  ReadConfigInfo(HEX_PA_t phyAddr, HEX_4u_t size, void *value);
        HEXAPI_Status  WriteCache(HEX_PA_t phyAddr, HEX_4u_t size, HEX_8u_t value);
        HEXAPI_Status  ReadCache(HEX_PA_t phyAddr, HEX_4u_t size, void *value);
        HEXAPI_Status  WriteVirtual(HEX_VA_t virtAddr, HEX_4u_t asid, HEX_4u_t size, HEX_8u_t value);
        HEXAPI_Status  ReadVirtual(HEX_VA_t virtAddr, HEX_4u_t asid, HEX_4u_t size, void *value);
        HEXAPI_Status  ReadSymbolValue(const char *symbol, HEX_4u_t *value);
        HEXAPI_Status  GetCurrentHWThreadNum(HEX_4u_t *currentThread);
        HEXAPI_Status  TranslateVirtualToPhysical(HEX_VA_t vAddr, HEX_4u_t asid, HEX_PA_t *pAddr);
        void CycleToTime(HEX_8u_t cycleCount, HEX_8u_t *time, HEXAPI_Interval *units);
        void TimeToCycles(HEX_8u_t time, HEX_8u_t *cycleCount, HEXAPI_Interval units);
        HEXAPI_Status  GetPowerStatistics(bool reset, float *all_wait, float *avg_active, float *avg_pkt);
        HEXAPI_Status  GetSimulationStatistics(HEX_StatisticsType type, void *value, HEX_4u_t thread);
        HEXAPI_Status  EmitStatistics();
        HEXAPI_Status  EmitPerfStatistics(HEX_8u_t startTimeSec, HEX_8u_t startTimeUsec,
                HEX_8u_t endTimeSec, HEX_8u_t endTimeUsec, char *buffer, HEX_4u_t bufferSize);
        HEXAPI_Status  GetSimulatedCycleCount(HEX_8u_t *cycles);
        HEXAPI_Status  GetElapsedSimulationTime( HEX_8u_t *time, HEXAPI_Interval units );
        HEXAPI_Status  ConfigureARFilesystem(char *filename);
        HEXAPI_Status  AxiSlaveAccess(HEX_4u_t tnum, HEX_PA_t pAddr, HEX_4u_t width, HEX_4u_t type, axi_bus_finished_callback cb, HEX_4u_t id, unsigned char *dataptr, HEX_4u_t *rval);
        HEXAPI_Status  GetInfo(HEXAPI_Info qtype, char **bufptr, int *size);
        HEXAPI_Status  EnableVirtualToPhysicalTranslation(bool value);
        HEXAPI_Status  DisassemblePacket(HEX_4u_t tnum, HEX_VA_t pc, char *outbuf1, int size1, HEX_VA_t *pc_prev, HEX_VA_t *pc_next);

        // PMU API's
        HEXAPI_Status  GetPmuIndexedStats(HEX_4u_t tnum, HEX_4u_t pe_num, HEX_8u_t *value);
        HEXAPI_Status  PmuIsStatModeled(HEX_4u_t tnum, HEX_4u_t pe_num, HEX_4u_t *value);
        HEXAPI_Status  PmuGetName(HEX_4u_t tnum, HEX_4u_t pe_num, const char **name);
        HEXAPI_Status  PmuIsMaskable(HEX_4u_t tnum, HEX_4u_t pe_num, HEX_4u_t *value);

        HEXAPI_Status  EnablePmu(HEX_4u_t tnum = 0xffffffff);
        HEXAPI_Status  DisablePmu(HEX_4u_t tnum = 0xffffffff);
        HEXAPI_Status  ResetPmu(HEX_4u_t tnum = 0xffffffff);
        HEXAPI_Status  DumpPmu(FILE *fp, int including_iss_only_stats, HEX_4u_t tnum = 0xffffffff);

        // Record mode API. Needed basically to pass the record flag all the way to sim_globals struct.
        void RecordMode(int mode);
        void ReconnectMode(int mode);
        HEXAPI_Status ConfigurePacketAnalysis(const char * filename, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status EnablePacketAnalysis(bool enable_disable);
        HEXAPI_Status ConfigureInstHistogram(const char * filename, HEXAPI_OpenMode mode = HEX_MODE_WRITE);
        HEXAPI_Status EnableInstHistogram(bool enable_disable);
        HEXAPI_Status ResetInstHistogram();
        HEXAPI_Status DumpInstHistogram(const char * msg);

        // misc. utility
        HEXAPI_VerboseMode GetVerboseMode()
        {
            return verbose;
        }

        void StreamerPush(int streamerId, HEX_8u_t rawPins);
        HEX_8u_t StreamerPull(int streamerId);

    private:
        HEXAPI_Status  AddBusAccessCallbackEasy(void *deviceHandle, HEX_PA_t phyStartAddr, HEX_PA_t phyEndAddr,
                bus_transaction_request_callback btrc);
        HEXAPI_Status ConfigureOutputFile(const char *in_fname, HEXAPI_OpenMode mode, const char **omode, char *out_fname);
        void GetCosimLibNames(FILE *fcosim);
        void Init();
        void PluginCosims();
        void ParseCosimVersion();
        void UnPlugCosims();
        void GetLibPath(char *path, const char *suffix
#ifdef _WIN32
                , int fUpdatePath
#endif
                );
        void *LoadMyDLL(const char * arch, char *libPath);

#define HEX_LIBPATH_MAX 4096

        void *pIss;
        void *simFunc;
        void *IssDlib;
        char fsPath[HEX_LIBPATH_MAX];
        char *errString;
        FILE *fpcosim;
        char szRecBuf[HEXAGON_MAX_RECORD_BUFFER_SZ];
        int recBufBytes;
        unsigned int DebugPortNum;
        struct cosim_lib_args
        {
            char *name;
            char *args;
            void *cdlib;
            void *device;
        } CosimLibsArgs[100];
        int numCosims;
        bool endofConfiguration;
        HEXAPI_Cpu m_cpu;
        HEXAPI_CoreState runState;
        HEXAPI_TimingMode timeMode;
        HEXAPI_VerboseMode verbose;
        HEX_4u_t debug;
        HEX_4u_t recordFlag;
        bool Reserved_0;      // Unused. Available for future use
        FILE *m_fpRecord;     // used by the record feature
        HEX_4u_t Reserved_1;  // Unused. Available for future use
        HEX_4u_t Reserved_2;  // Unused. Available for future use
        HEX_8u_t Reserved_3;  // Unused. Available for future use
        bool bDeassertOccurred;
        bool bDebuggerConnected;
        const char *pLibPath;
        unsigned char Reserved_x[RESERVE_SIZE]; // unused, available for future

        HEXAPI_Status  AddBusAccessCommon(void *deviceHandle,
          HEX_PA_t phyStartAddr, HEX_PA_t phyEndAddr,
          bus_transaction_request_callback btrc, unsigned int flags);

        friend HEXAPI_TransactionStatus RecordBusAccessCallback(void *deviceHandle,
                HEX_PA_t address,
                HEX_4u_t lengthInBytes,
                HEX_1u_t *data,
                HEX_4u_t requestID,
                HEXAPI_BusAccessType type,
                HEX_4u_t tnum,
                HEXAPI_BusBurstType burst);

        friend void RecordTimedCallback(void *deviceHandle);

        friend void RecordOneShotTimedCallback(void *deviceHandle);

        friend void RecordMemWasWrittenCallback(void *handle,
                HEX_PA_t addr,
                HEX_8u_t value,
                HEX_4u_t sizeInBytes);

        friend void RecordMemWasReadCallback(void *handle,
                HEX_PA_t addr,
                HEX_8u_t value,
                HEX_4u_t sizeInBytes);

        friend void RecordCoreReadyCallback(void *callbackPtrs,
                HEXAPI_CoreReadyState polarity);

        inline HEX_8u_t CoreFrequency(void);
        inline HEX_8u_t GetCycles(void);
        inline HEX_4u_t GetPC(HEX_4u_t tnum);
        inline HEX_4u_t CurrThreadNum(void);
        inline HEX_8u_t TimeToCycles(HEX_8u_t time, HEXAPI_Interval units);
};

/* Prototypes for API callback recording functions */

HEXAPI_TransactionStatus  RecordBusAccessCallback( void *deviceHandle,
        HEX_PA_t address,
        HEX_4u_t lengthInBytes,
        HEX_1u_t *data,
        HEX_4u_t requestID,
        HEXAPI_BusAccessType type,
        HEX_4u_t tnum,
        HEXAPI_BusBurstType burst);

void  RecordTimedCallback(void *deviceHandle);

void RecordOneShotTimedCallback(void *deviceHandle);

void RecordMemWasWrittenCallback(void *handle,
        HEX_PA_t address,
        HEX_8u_t value,
        HEX_4u_t lengthInBytes);

void RecordMemWasReadCallback(void *handle,
        HEX_PA_t address,
        HEX_8u_t value,
        HEX_4u_t lengthInBytes);

void RecordCoreReadyCallback(void *callbackPtrs,
        HEXAPI_CoreReadyState polarity);

#endif
