#define DEBUG 0

#ifdef __PPU__
/*   SCE CONFIDENTIAL                                       */
/*   Cell Evaluation System 022.009                       */
/*   Copyright (C) 2004 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

/* File: launcher.cc
 * Description
 *   A PPU program to handle SPU printf of a Raw SPU. 
 *
 *   SPU's printf does not actually output a string to a console.  It only
 *   put the argument list in a stack in its local storage, and pass its 
 *   address to PPU as an event, which is actually sent via PPU interrupt mb
 *   expecting that PPU handles it properly.  Therefore, the PPU-side must 
 *   has a system to constantly check an event from SPUs.  This program 
 *   realizes it by creating a PPU interrupt thread which wakes up when
 *   PPU mb interrupt is caught. 
 */

#include "spulauncher.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/spu_initialize.h>
#include <sys/raw_spu.h>
#include <sys/spu_utility.h>
#include <sys/ppu_thread.h>
#include <sys/interrupt.h>
#include <sys/timer.h>
#include <sys/paths.h>
#include <spu_printf.h>
#include <libsn.h>
#include <sdk_version.h>

#include "EABase/eabase.h"
#include "EAStdC/EAStopwatch.h"
#include "raw_spu_mmio.h"

#define EIEIO                 asm ("eieio");

#define INT_STAT_MAILBOX      0x01UL

#define PPU_STACK_SIZE 4096

#define MAX_PHYSICAL_SPU       6 
#define MAX_RAW_SPU            1

#include "../harness/unittest_output.h"
#include "../harness/unittest_spuproxy_enums.h"

static volatile int exitNow = 0;
static unsigned int gTestsTotal = 0;
static unsigned int gTestsPassed = 0;

bool HaveAllTestsPassed()
{
    return gTestsPassed == gTestsTotal;
}

// Converts a spu base address and offset to a pointer (raw spu ls is memory mapped) that the PPU
// can read/write to.
void * SPUToPPUAddress( uint64_t ls_base, uint64_t offset )
{
    return (void*) (uintptr_t)(ls_base + offset );
}

const int SPU_STACK_VARIABLE_SIZE = 16;

double GetSPUDouble( uint64_t ls_base, uint64_t arg_base_addr, uint64_t argumentNum )
{
    return *(double*)SPUToPPUAddress(ls_base, (uint32_t)(arg_base_addr + SPU_STACK_VARIABLE_SIZE * argumentNum));
}

uint32_t GetSPUUInt( uint64_t ls_base, uint64_t arg_base_addr, uint64_t argumentNum )
{
    return *(uint32_t*)SPUToPPUAddress(ls_base, (uint32_t)(arg_base_addr + SPU_STACK_VARIABLE_SIZE * argumentNum));
}

char *GetSPUCharPtr( uint64_t ls_base, uint64_t arg_base_addr, uint64_t argumentNum )
{
    uint32_t lsAddress = *(uint32_t*)SPUToPPUAddress(ls_base, (uint32_t)(arg_base_addr + SPU_STACK_VARIABLE_SIZE * argumentNum));
    return (char*)SPUToPPUAddress(ls_base, lsAddress);
}

int syscall_spuproxy(uint64_t ls_base, uint32_t arg_addr)
{
    UnitTestOutputManager& outputManager = UnitTestOutputManager::OpenInstance();

    UnitTestOutputMethod methodEnum = *(UnitTestOutputMethod*)SPUToPPUAddress(ls_base, arg_addr);

    switch(methodEnum)
    {
    case UTOM_OutputInitial:
        {
            outputManager.OutputInitial();
            break;
        }
    case UTOM_OutputClassName:
        {
            char *className = GetSPUCharPtr(ls_base, arg_addr, 1);
            outputManager.OutputClassName(className);
            break;
        }
    case UTOM_OutputTestResult:
        {
            bool result = (bool)GetSPUUInt(ls_base, arg_addr, 1);
            char *functionName = GetSPUCharPtr(ls_base, arg_addr, 2);
            char *fileName = GetSPUCharPtr(ls_base, arg_addr, 3);
            uint32_t lineNumber = GetSPUUInt(ls_base, arg_addr, 4);
            outputManager.OutputTestResult(result, functionName, fileName, lineNumber);
            break;
        }
    case UTOM_OutputBenchmarkResult:
        {
            char *testIdentifier = GetSPUCharPtr(ls_base, arg_addr, 1);
            char *functionName = GetSPUCharPtr(ls_base, arg_addr, 2);
            double timeValue = GetSPUDouble(ls_base, arg_addr, 3);
            outputManager.OutputBenchmarkResult(testIdentifier, functionName, timeValue);
            break;
        }
    case UTOM_OutputTotals:
        {
            uint32_t totalTests = GetSPUUInt(ls_base, arg_addr, 1);
            uint32_t totalPassed = GetSPUUInt(ls_base, arg_addr, 2);
            gTestsTotal += totalTests;
            gTestsPassed += totalPassed;
            outputManager.OutputTotals(totalTests, totalPassed);
            break;
        }
    case UTOM_OutputClassTotals:
        {
            char *className = GetSPUCharPtr(ls_base, arg_addr, 1);
            uint32_t totalTests = GetSPUUInt(ls_base, arg_addr, 2);
            uint32_t totalPassed = GetSPUUInt(ls_base, arg_addr, 3);
            outputManager.OutputClassTotals(className, totalTests, totalPassed);
            break;
        }
    case UTOM_OutputMessage:
        {
            char *message = GetSPUCharPtr(ls_base, arg_addr, 1);
            outputManager.OutputMessage(message);
            break;
        }
    case UTOM_OutputFinal:
        {
            outputManager.OutputFinal();
            break;
        }
    default:
        break;
    }
    return 1;
}

int syscall_sprintf(uint64_t ls_base, uint32_t arg_addr)
{
    const int STACK_VARIABLE_SIZE = 16;
    uint32_t adjusted_arg_address = arg_addr + STACK_VARIABLE_SIZE;
    // The first value on the stack is the buffer that sprintf should be writing too.
    uint32_t lsBufferAddress = *(uint32_t*)SPUToPPUAddress(ls_base, arg_addr);

    // Note:
    //   For raw SPU's, we can get a pointer into SPU memory and write directly to
    //   memory.  If we switch to a system that does not use raw SPU's, this will
    //   probably need to change.
    //   We should be able to use sys_spu_thread_write_ls to write to local store
    //   memory.  Refer to the "Lv2 System Call and Library - Reference" for more info.

    char *lsBuffer = (char*)SPUToPPUAddress(ls_base, lsBufferAddress);
    // The size of the write is the return value.
    int retval = spu_raw_sprintf(lsBuffer, ls_base, adjusted_arg_address);
    return retval;
}

/*
 * This is the function that actually output a string to the console.
 * The argument list is created by the SPU in its local storage. Its 
 * address is passed to PPU via PPU interrupt mb, and eventually passed to 
 * this function. 
 * The LS base address can be calculated by statically defined offsets
 * and Raw SPU ID.  raw_spu_mmio.h provides LS_BASE_ADDR(id) macro for this 
 * task.
 * 
 * ls_base: Base LS address of the Raw SPU that is requesting printf.
 * arg_addr: Address of argument list of printf.
 *
 * Returns size of the printed string.  
 */
int syscall_printf (uint64_t ls_base, uint32_t arg_addr)
{
    char buffer[1024];

    spu_raw_sprintf(buffer, ls_base, arg_addr);

    int retval = printf("%s", buffer);

    return retval;
}


/*
 * Loaded on an interrupt PPU thread, handles SPU's printf request.
 */
void handle_syscall(uint64_t arg)
{
    sys_raw_spu_t id = arg;
    uint64_t stat;
    uint32_t mail;
    int ret;
    int sys_ret = -1;
    bool exitRequested = false;

    /*
     * Create a tag to handle class 2 interrupt, because PPU Interrupt MB is 
     * handled by class 2. 
     */
    ret = sys_raw_spu_get_int_stat(id, 2, &stat);
    if (ret) {
        printf("sys_raw_spu_get_int_stat is failed %d\n", ret);
        sys_interrupt_thread_eoi();
    }

    /*
     * If the caught class 2 interrupt includes mailbox interrupt, handle it.
     */
    if ((stat & INT_STAT_MAILBOX) == INT_STAT_MAILBOX) {
        ret = sys_raw_spu_read_puint_mb(id, &mail);
        if (ret) {
            printf("sys_raw_spu_read_puint_mb is failed %d\n", ret);
            sys_interrupt_thread_eoi();
        }
        uint32_t lsaddr;
        switch (mail >> 24) {
        case 0x1:
            lsaddr = mmio_read_prob_reg(id, PU_MB);
            sys_ret = syscall_printf(LS_BASE_ADDR(id), lsaddr);
            break;
        case 0x2:
            lsaddr = mmio_read_prob_reg(id, PU_MB);
            sys_ret = syscall_sprintf(LS_BASE_ADDR(id), lsaddr);
            break;
        case 0x3:
            lsaddr = mmio_read_prob_reg(id, PU_MB);
            sys_ret = syscall_spuproxy(LS_BASE_ADDR(id), lsaddr);
            break;
        case 0x4:
            exitRequested = true;
            sys_ret = 1;
            break;
        default:
            printf("undefined syscall.\n");
            break;
        }

        /*
         * Reset the PPU_INTR_MB interrupt status bit.
         */
        ret = sys_raw_spu_set_int_stat(id, 2, stat & INT_STAT_MAILBOX);
        if (ret) {
            printf("sys_raw_spu_set_int_stat is failed %d\n", ret);
            char *reasonStr;
            switch( ret )
            {
                case ESRCH:
                    reasonStr = "ESRCH: Specified id is not found";
                    break;
                case EINVAL:
                    reasonStr = "EINVAL: Invalid argument";
                    break;
                default:
                    reasonStr = "Unknown error";
                    break;
            };
            printf("    %s\n", reasonStr);
            sys_interrupt_thread_eoi();
        }

        /* 
         * SPU's printf is expecting a return value is passed to SPU MB. 
         * Return it via SPU MB.  This is going to be the return value of SPU's
         *  printf.
         */

        mmio_write_prob_reg(id, SPU_MB, sys_ret);
        EIEIO;

    } else {
        /*
         * Must reset interrupt status bit of those not handled.  
         *
         */
        ret = sys_raw_spu_set_int_stat(id, 2, stat);
        if (ret) {
            printf("sys_raw_spu_set_int_stat is failed %d\n", ret);
            sys_interrupt_thread_eoi();
        }
    }

    if ( exitRequested )
    {
        exitNow = 1;
    }

    /*
     * End of interrupt
     */
    sys_interrupt_thread_eoi();
}



int executeSPUProgram(char *SPU_PROG)
{
    int ret;
    sys_raw_spu_t id;
    uint32_t entry;
    static bool spusInit = false;
    uint64_t spuTimer;

    if( !spusInit )
    {
        /*
         * Initialize SPUs
         */
#if DEBUG
        printf("Initializing SPUs\n");
#endif
        ret = sys_spu_initialize(MAX_PHYSICAL_SPU, MAX_RAW_SPU);
        switch (ret) {
            case CELL_OK:
                break;
#if CELL_SDK_VERSION < 0x080000
            case EBUSY:
#if DEBUG
                printf("SPUs have already been initialized.\n");
#endif
                break;
#endif
            default:
                fprintf(stderr, "sys_spu_initialize failed: %d\n", ret);
                exit(ret);
        }

        snInit();
        spusInit = true;
    }

    /*
     * Execute a series of system calls to load a program to a Raw SPU.
     */
#if DEBUG
    printf("syscall sys_raw_spu_create...");
#endif
    ret = sys_raw_spu_create(&id, NULL);
    if (ret) {
        printf("sys_raw_spu_create is failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. raw_spu number is %d\n", id);
#endif
    
    snRawSPUNotifyCreation(id);

    /* 
     * Create an interrupt handler and establish it on an interrupt PPU 
     * thread.  This PPU interrupt thread is going to handle SPU's printf
     * request.
     */
    sys_interrupt_tag_t intrtag;
    sys_ppu_thread_t handler;
    sys_interrupt_thread_handle_t ih;

#if DEBUG
    printf("syscall pu_thread_create...");
#endif
    ret = sys_ppu_thread_create(&handler, handle_syscall, 0, 100,
            PPU_STACK_SIZE, SYS_PPU_THREAD_CREATE_INTERRUPT,
            "Interrupt Thread");
    if (ret) {
        printf("ppu_thread_create is failed %d\n", ret);
    }
#if DEBUG
    printf("succeeded.\n");
#endif

#if DEBUG
    printf("syscall sys_ppu_thread_set_priority...");
#endif
    ret = sys_ppu_thread_set_priority(handler, 111);
    if (ret) {
        printf("sys_ppu_thread_set_priority failed %d\n", ret);
    }
#if DEBUG
    printf("succeeded.\n");
#endif

#if DEBUG
    printf("syscall sys_raw_spu_create_interrupt_tag...");
#endif
    ret = sys_raw_spu_create_interrupt_tag(id, 2, 0, &intrtag);
    if (ret) {
        printf("sys_raw_spu_create_interrupt_tag failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. \n");
#endif

#if DEBUG
    printf("syscall intr_thread_establish...");
#endif
    ret = sys_interrupt_thread_establish(&ih, intrtag, handler, id);
    if (ret) {
        printf("intr_thread_establish is failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. \n");
#endif

#if DEBUG
    printf("syscall sys_raw_spu_load...");
#endif
    ret = sys_raw_spu_load(id, SPU_PROG, &entry);
    if (ret) {
        printf("raw_spu_load is failed %d on %s\n", ret,SPU_PROG);
        char *reasonStr;
        switch( ret )
        {
            case CELL_OK:
                reasonStr = "CELL_OK: Success";
                break;
            case ENOMEM:
                reasonStr = "ENOMEM: Insufficient memory";
                break;
            case EINVAL:
                reasonStr = "EINVAL: Invalid argument";
                break;
            case ENOENT:
                reasonStr = "ENOENT: No such file or directory";
                break;
            case ENOEXEC:
                reasonStr = "ENOEXEC: File is not ELF executable";
                break;
            default:
                reasonStr = "Unknown error";
                break;
        };
        printf("    %s\n", reasonStr);
        goto errorexit;
    }
#if DEBUG
    printf("succeeded. entry %x\n", entry);
#endif

    /*
     * Set interrupt mask.
     * The third argument = 7 enables Halt, Stop-and-Signal and PPU Mailbox 
     * interrupts.
     */
#if DEBUG
    printf("syscall sys_raw_spu_set_int_mask...");
#endif
    ret = sys_raw_spu_set_int_mask(id, 2, 7);
    if (ret) {
        printf("sys_raw_spu_set_int_mask failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. \n");
#endif

    snRawSPUNotifyElfLoad (id, entry, SPU_PROG);
  
    /*
     * Run the Raw SPU.
     */
    // Set exitNow to false before executing the SPU program.
    exitNow = 0;
    snRawSPUNotifySPUStarted(id);
    mmio_write_prob_reg(id, SPU_NPC, entry);
    mmio_write_prob_reg(id, SPU_RunCntl, 0x1);

    // Exit this routine when the flag is set.
    spuTimer = EA::StdC::Stopwatch::GetCPUCycle();
    while(!exitNow)
    {
        if( (EA::StdC::Stopwatch::GetCPUCycle()-spuTimer) > (5*EA::StdC::Stopwatch::GetCPUFrequency()) )
        {
            printf("SPU program '%s' stalled.\n", SPU_PROG);
            spuTimer = EA::StdC::Stopwatch::GetCPUCycle();
        }
    }

#if DEBUG
    printf("syscall sys_interrupt_thread_disestablish...");
#endif
    if ((ret = sys_interrupt_thread_disestablish(ih)) != CELL_OK)
    {
        printf("sys_interrupt_thread_disestablish failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. \n");
#endif
    
#if DEBUG
    printf("syscall sys_interrupt_tag_destroy...");
#endif
    if ((ret = sys_interrupt_tag_destroy(intrtag)) != CELL_OK)
    {
        printf("sys_interrupt_tag_destroy failed %d\n", ret);
        return -1;
    }
#if DEBUG
    printf("succeeded. \n");
#endif
    
errorexit:
#if DEBUG
    printf("Destroying Raw SPU %d\n", id);
#endif
    if ((ret = sys_raw_spu_destroy(id)) != CELL_OK) {
        fprintf(stderr, "sys_raw_spu_destroy returned %d\n", ret);
        fprintf(stderr, "Exiting...\n");
        exit(1);
    }
    return 1;
}
#endif

int SPULauncher(int argc, char **argv)
{
    if (argc<2)
    {
        printf("SPULauncher [--enable-csv-file-output] <spu_prog1.elf [spu_prog2.elf ...] OR \n"
               "                         --elf-sequence spu_prog#.elf start_index end_index>\n");
        printf("%c",'Z'&31); // tell SN's ps3run.exe to exit
        return 1;
    }

    UnitTestOutputManager& outputManager = UnitTestOutputManager::OpenInstance();
    outputManager.AddOutput("stdout");

    for(int argIndex = 1; argIndex < argc; argIndex++)
    {
        if( strcmp(argv[argIndex], "csvfile") == 0 )
        {
            //#define RW_MATH_UNITTEST_CSV_FILE_PATH SYS_APP_HOME "/rwmath_test_results_spu.csv"

            if(!outputManager.AddOutput("csvfile"))
            {
                printf("Error opening csv file for writing\n");
                printf("%c",'Z'&31); // tell SN's ps3run.exe to exit
                return 1;
            }
        }
    }

    outputManager.OutputInitial();

    for(int argIndex = 1; argIndex < argc; argIndex++)
    {
        if( strcmp(argv[argIndex], "--elf-sequence") == 0 )
        {
            int startIndex = atoi(argv[argIndex + 2]);
            int endIndex = atoi(argv[argIndex + 3]);
            for( int progIndex = startIndex; progIndex <= endIndex; ++progIndex )
            {
                char buf1[512];
                char buf2[512];
                char *readPtr = buf1;
                char *writePtr = buf2;
                sprintf(buf1,SYS_APP_HOME "/%s", argv[argIndex + 1]);

                // Replace '#' characters with the program index.
                do
                {
                    if( *readPtr == '#' )
                    {
                        writePtr += sprintf( writePtr, "%d", progIndex );
                    }
                    else
                    {
                        *(writePtr++) = *(readPtr);
                    }
                } while(*readPtr++);
                executeSPUProgram(buf2);
            }

            argIndex += 3;
        }
        else if( strcmp(argv[argIndex], "--enable-csv-file-output") != 0 )
        {
            char buf[512];
            sprintf(buf, SYS_APP_HOME "/%s", argv[argIndex]);
            executeSPUProgram(buf);
        }
    }

    outputManager.OutputTotals(gTestsTotal, gTestsPassed);
    outputManager.OutputFinal();

    printf("%c",'Z'&31); // tell SN's ps3run.exe to exit

    return 0;
}

