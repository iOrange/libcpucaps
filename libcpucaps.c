#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#include "libcpucaps.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>    /* memcpy, memset, memcmp */

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__ICC) || defined(__INTEL_COMPILER)
#include <cpuid.h>
#endif

#ifdef __linux__

#include <sched.h>
#include <unistd.h>
#if __GLIBC_PREREQ(2,30)
/* we have gettid !!! */
#else
#include <sys/syscall.h>
static pid_t gettid() {
    return syscall(SYS_gettid);
}
#endif

#else

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

#endif

typedef struct _s_cpuid_result {
    uint32_t eax, ebx, ecx, edx;
} cpuid_result_t;

int cpuid_wrapper(uint32_t func, uint32_t subfunc, cpuid_result_t* result);
void query_Intel_caches(cpucaps_t* caps);
void query_Intel_topology(uint32_t highestFunc, cpucaps_t* caps);
void query_AMD_caches(uint32_t highestFuncEx, cpucaps_t* caps);
void query_AMD_topology(uint32_t highestFuncEx, cpucaps_t* caps);

int libcpucaps_GetCaps(cpucaps_t* caps) {
    uint32_t highestFunc, highestFuncEx;
    cpuid_result_t cpuidResult;

    if (!caps) {
        return LIBCPUCAPS_ERROR_INVALID_PARAM;
    }
    memset(caps, 0, sizeof(cpucaps_t));

    /* get the highest function id */
    memset(&cpuidResult, 0, sizeof(cpuidResult));
    cpuid_wrapper(0, 0, &cpuidResult);
    highestFunc = cpuidResult.eax;

    /* copy over the CPU vendor string */
    memcpy(&caps->vendor[0], &cpuidResult.ebx, 4);
    memcpy(&caps->vendor[4], &cpuidResult.edx, 4);
    memcpy(&caps->vendor[8], &cpuidResult.ecx, 4);

    if (!memcmp(caps->vendor, "GenuineIntel", 12)) {
        caps->isIntel = 1;
    } else if (!memcmp(caps->vendor, "AuthenticAMD", 12)) {
        caps->isAMD = 1;
    }

    if (highestFunc >= 1) {
        cpuid_wrapper(1, 0, &cpuidResult);
        caps->func1_ecx = cpuidResult.ecx;
        caps->func1_edx = cpuidResult.edx;

        caps->stepping = cpuidResult.eax & 0xF;
        caps->model = (cpuidResult.eax >> 4) & 0xF;
        caps->family = (cpuidResult.eax >> 8) & 0xF;
        caps->cpuType = (cpuidResult.eax >> 12) & 0x3;
        caps->modelEx = (cpuidResult.eax >> 16) & 0xF;
        caps->familyEx = (cpuidResult.eax >> 20) & 0xFF;
    }

    if (highestFunc >= 4 && caps->isIntel) {
        query_Intel_caches(caps);                   /* Intel's "Deterministic Cache Parameters Leaf" */
        query_Intel_topology(highestFunc, caps);    /* Intel's "Extended Topology Enumeration leaf" (both V1 & V2) */
    }

    if (highestFunc >= 7) {
        cpuid_wrapper(7, 0, &cpuidResult);
        caps->func7_ebx = cpuidResult.ebx;
        caps->func7_ecx = cpuidResult.ecx;
    }

    /* get the highest extended function id */
    cpuid_wrapper(0x80000000, 0, &cpuidResult);
    highestFuncEx = cpuidResult.eax;

    if (highestFuncEx >= 0x80000001) {
        cpuid_wrapper(0x80000001, 0, &cpuidResult);
        caps->func80000001_ecx = cpuidResult.ecx;
        caps->func80000001_edx = cpuidResult.edx;
    }

    /* copy over the CPU name string */
    if (highestFuncEx >= 0x80000002) {
        cpuid_wrapper(0x80000002, 0, &cpuidResult);
        memcpy(&caps->name[0], &cpuidResult, 16);

        if (highestFuncEx >= 0x80000003) {
            cpuid_wrapper(0x80000003, 0, &cpuidResult);
            memcpy(&caps->name[16], &cpuidResult, 16);

            if (highestFuncEx >= 0x80000004) {
                cpuid_wrapper(0x80000004, 0, &cpuidResult);
                memcpy(&caps->name[32], &cpuidResult, 16);
            }
        }
    }

    /* AMD caches info */
    if (highestFuncEx >= 0x80000005 && caps->isAMD) {
        query_AMD_caches(highestFuncEx, caps);
        query_AMD_topology(highestFuncEx, caps);
    }

    return LIBCPUCAPS_ERROR_OK;
}



#define GET_BIT(a, bit)  (((a) >> (bit)) & 1)

int libcpucaps_HasFPU(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 0);
}
int libcpucaps_HasPSE(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 3);
}
int libcpucaps_HasTSC(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 4);
}
int libcpucaps_HasCMPXCHG8(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 8);
}
int libcpucaps_HasCMPXCHG16B(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 13);
}
int libcpucaps_HasMMX(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 23);
}
int libcpucaps_HasMMXExt(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_edx, 23);
}
int libcpucaps_Has3DNow(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_edx, 31);
}
int libcpucaps_Has3DNowExt(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_edx, 30);
}
int libcpucaps_HasSSE(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 25);
}
int libcpucaps_HasSSE2(cpucaps_t* caps) {
    return GET_BIT(caps->func1_edx, 26);
}
int libcpucaps_HasSSE3(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 0);
}
int libcpucaps_HasSSSE3(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 9);
}
int libcpucaps_HasSSE41(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 19);
}
int libcpucaps_HasSSE42(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 20);
}
int libcpucaps_HasABM(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_ecx, 5);
}
int libcpucaps_HasSSE4a(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_ecx, 6);
}
int libcpucaps_HasMisalignSSE(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_ecx, 7);
}
int libcpucaps_HasAES(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 25);
}
int libcpucaps_HasAVX(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 28);
}
int libcpucaps_HasAVX2(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 5);
}
int libcpucaps_HasAVX512F(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 16);
}
int libcpucaps_HasAVX512PF(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 26);
}
int libcpucaps_HasAVX512ER(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 27);
}
int libcpucaps_HasAVX512CD(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 28);
}
int libcpucaps_HasF16C(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 29);
}
int libcpucaps_HasRDRAND(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 30);
}
int libcpucaps_HasRDSEED(cpucaps_t* caps) {
    return GET_BIT(caps->func7_ebx, 18);
}
int libcpucaps_HasFMA3(cpucaps_t* caps) {
    return GET_BIT(caps->func1_ecx, 12);
}
int libcpucaps_HasFMA4(cpucaps_t* caps) {
    return GET_BIT(caps->func80000001_ecx, 16);
}


int cpuid_wrapper(uint32_t func, uint32_t subfunc, cpuid_result_t* result) {
#ifdef _MSC_VER
    int cpuInfo[4];
    memset(cpuInfo, 0, sizeof(cpuInfo));

    __cpuidex(cpuInfo, (int)func, (int)subfunc);

    memcpy(result, cpuInfo, sizeof(cpuid_result_t));
    return 1;
#else
    return __get_cpuid_count(func, subfunc, &result->eax, &result->ebx, &result->ecx, &result->edx);
#endif
}

static size_t get_current_thread_wrapper() {
#ifdef _MSC_VER
    return (size_t)GetCurrentThread();
#else
    return (size_t)gettid();
#endif
}

static size_t set_thread_affinity_wrapper(size_t threadHandle, size_t affinityMask) {
#ifdef _MSC_VER
    return (size_t)SetThreadAffinityMask((HANDLE)threadHandle, (DWORD_PTR)affinityMask);
#else
    size_t oldAffinity = 0;
    cpu_set_t mask;

    CPU_ZERO(&mask);
    sched_getaffinity((pid_t)threadHandle, sizeof(mask), &mask);
    for (size_t i = 0; i < 64; ++i) {
        if (CPU_ISSET(i, &mask)) {
            oldAffinity |= (1 << i);
        }
    }

    CPU_ZERO(&mask);
    for (size_t i = 0; i < 64; ++i) {
        if (affinityMask & (1 << i)) {
            CPU_SET(i, &mask);
        }
    }

    sched_setaffinity((pid_t)threadHandle, sizeof(mask), &mask);

    return oldAffinity;
#endif
}


/* https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf */
/* Deterministic Cache Parameters Leaf */
/* In theory we should be able to just while (1) {} and break of cacheType == 0 */
/*   but it's better to cap our iterations for sanity */
#define MAX_INTEL_FN4_ITERATIONS    8
void query_Intel_caches(cpucaps_t* caps) {
    uint32_t subFunc, cacheType, cacheLevel, lineSize, linePartitions, assocWays, setsNum, cacheSizeKB;
    cpuid_result_t cpuidResult;

    for (subFunc = 0; subFunc < MAX_INTEL_FN4_ITERATIONS; ++subFunc) {
        cpuid_wrapper(4, subFunc, &cpuidResult);

        cacheType = cpuidResult.eax & 0x1F;
        if (!cacheType) { /* Null - No more caches */
            break;
        }

        cacheLevel = (cpuidResult.eax >> 5) & 0x7;

        lineSize = (cpuidResult.ebx & 0xFFF) + 1;
        linePartitions = ((cpuidResult.ebx >> 12) & 0x3FF) + 1;
        assocWays = ((cpuidResult.ebx >> 22) & 0x3FF) + 1;
        setsNum = cpuidResult.ecx + 1;
        /* from https://stackoverflow.com/a/64607673 */
        /* Cache size = Ways * Partitions * Line_Size * Sets */
        cacheSizeKB = (assocWays * linePartitions * lineSize * setsNum) / 1024;


        if (cacheLevel == 1) {
            if (cacheType == 1) {                        /* 1 = Data Cache */
                caps->L1d_lineSizeBytes = (int)lineSize;
                caps->L1d_sizeKibiBytes = (int)cacheSizeKB;
                caps->L1d_associativityType = (int)assocWays;
            } else if (cacheType == 2) {                 /* 2 = Instruction Cache */
                caps->L1i_lineSizeBytes = (int)lineSize;
                caps->L1i_sizeKibiBytes = (int)cacheSizeKB;
                caps->L1i_associativityType = (int)assocWays;
            }
        } else if (cacheLevel == 2 && cacheType == 3) {  /* 3 = Unified Cache */
            caps->L2_lineSizeBytes = (int)lineSize;
            caps->L2_sizeKibiBytes = (int)cacheSizeKB;
            caps->L2_associativityType = (int)assocWays;
        } else if (cacheLevel == 3 && cacheType == 3) {
            caps->L3_lineSizeBytes = (int)lineSize;
            caps->L3_sizeKibiBytes = (int)cacheSizeKB;
            caps->L3_associativityType = (int)assocWays;
        }
    }
}

/* https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf */
/* https://www.intel.com/content/dam/develop/external/us/en/documents/intel-64-architecture-processor-topology-enumeration.pdf */
/* Extended Topology Enumeration Leaf */
/* https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html */
/* V2 Extended Topology Enumeration Leaf */
/* In theory we should be able to just while (1) {} and break of level type == 0 */
/*   but it's better to cap our iterations for sanity */
#define MAX_INTEL_TOPOLOGY_ITERATIONS   7
void query_Intel_topology(uint32_t highestFunc, cpucaps_t* caps) {
    uint32_t topologyFunc, level, levelType, smtValue, coreValue, nextShift, smtShift, coreShift, smtMask, coreMask, core;
    size_t thread, affinityMask;
    cpuid_result_t cpuidResult;

    if (highestFunc >= 11) {
        topologyFunc = (highestFunc >= 31) ? 31 : 11;

        smtValue = 1;
        coreValue = 1;

        for (level = 0; level < MAX_INTEL_TOPOLOGY_ITERATIONS; ++level) {
            cpuid_wrapper(topologyFunc, level, &cpuidResult);

            /* Level types:      */
            /* 0: Invalid.       */
            /* 1: SMT.           */
            /* 2: Core.          */
            /* 3: Module.        */
            /* 4: Tile.          */
            /* 5: Die.           */
            /* 6 - 255: Reserved */
            levelType = (cpuidResult.ecx >> 8) & 0xFF;
            if (!levelType) {
                break;
            }

            /* Bits 04 - 00: Number of bits to shift right on x2APIC ID to get a unique topology ID of the next level type*. */
            /* All logical processors with the same next level ID share current level. */
            nextShift = cpuidResult.eax & 0x1F;

            if (levelType == 1) {
                smtValue = cpuidResult.ebx & 0xFFFF;
                smtShift = nextShift;
                smtMask = (~0u) >> (32 - nextShift);
            } else if (levelType == 2) {
                coreValue = cpuidResult.ebx & 0xFFFF;
                coreShift = nextShift;
                coreMask = (~0u) >> (32 - nextShift);
            }
        }

        caps->numLogicalCores = (int)coreValue;
        caps->numCores = caps->numLogicalCores / smtValue;

        if (caps->numLogicalCores > 1) {
            thread = get_current_thread_wrapper();
            affinityMask = set_thread_affinity_wrapper(thread, 1);
            level = 1; //#NOTE_SK: reusing it to count physical cores
            for (core = 0; core < (uint32_t)caps->numLogicalCores; ++core) {
                set_thread_affinity_wrapper(thread, (size_t)1 << core);
                cpuid_wrapper(topologyFunc, 0, &cpuidResult);

                coreValue = (cpuidResult.edx >> smtShift) & coreMask;
                caps->coreIDs[core] = (char)(coreValue & 0xFF);

                if (core && (caps->coreIDs[core] != caps->coreIDs[core - 1])) {
                    ++level;
                }
            }
            set_thread_affinity_wrapper(thread, affinityMask);

            caps->numCores = (int)level;
        }
    } else {
        /* TODO: implement older ways of topology query mechanisms ? */
        caps->numCores = 1;
        caps->numLogicalCores = 1;
    }
}

/* https://developer.amd.com/wp-content/resources/56255_3_03.PDF */
void query_AMD_caches(uint32_t highestFuncEx, cpucaps_t* caps) {
    cpuid_result_t cpuidResult;

    if (highestFuncEx >= 0x80000005) {
        cpuid_wrapper(0x80000005, 0, &cpuidResult);

        caps->L1d_lineSizeBytes = cpuidResult.ecx & 0xFF;
        caps->L1d_sizeKibiBytes = (cpuidResult.ecx >> 24) & 0xFF;
        caps->L1d_associativityType = (cpuidResult.ecx >> 16) & 0xFF;

        caps->L1i_lineSizeBytes = cpuidResult.edx & 0xFF;
        caps->L1i_sizeKibiBytes = (cpuidResult.edx >> 24) & 0xFF;
        caps->L1i_associativityType = (cpuidResult.edx >> 16) & 0xFF;
    }

    if (highestFuncEx >= 0x80000006) {
        cpuid_wrapper(0x80000006, 0, &cpuidResult);

        caps->L2_lineSizeBytes = cpuidResult.ecx & 0xFF;
        caps->L2_sizeKibiBytes = (cpuidResult.ecx >> 16) & 0xFFFF;
        caps->L2_associativityType = (cpuidResult.ecx >> 12) & 0xF;

        /* The L3 cache size in 512 KB units. */
        caps->L3_sizeKibiBytes = ((cpuidResult.edx >> 18) & 0x3FFFF) * 512;
        if (caps->L3_sizeKibiBytes) {  /* 0 means L3 is disabled */
            caps->L3_lineSizeBytes = cpuidResult.edx & 0xFF;
            caps->L3_associativityType = (cpuidResult.edx >> 12) & 0xF;

            /* 9 is reserved, and on Zen 2 L3 assoc. will always be 9 */
            /* indicating that we have to use the new way of caches query - function 0x8000001D */
            /* this func on AMD works almost the same way as func 4 on Intel */
            if (caps->L3_associativityType == 9 && highestFuncEx >= 0x8000001D) {
                cpuid_wrapper(0x8000001D, 3, &cpuidResult);

                caps->L3_lineSizeBytes = (cpuidResult.ebx & 0xFFF) + 1;
                caps->L3_associativityType = ((cpuidResult.ebx >> 22) & 0x3FF) + 1;
            }
        }
    }
}

// https://www.amd.com/system/files/TechDocs/25481.pdf
// page 34: CPUID Fn8000_001E
void query_AMD_topology(uint32_t highestFuncEx, cpucaps_t* caps) {
    uint32_t numLogicalCores, numCores, core;
    size_t thread, affinityMask;
    cpuid_result_t cpuidResult;

    caps->numCores = 1;
    caps->numLogicalCores = 1;

    // If CPUID Fn8000_0001_ECX[TopologyExtensions]==0 then CPUID Fn8000_001E_E[D,C,B,A]X is reserved
    if (GET_BIT(caps->func80000001_ecx, 22) && highestFuncEx >= 0x8000001E) {
        cpuid_wrapper(1, 0, &cpuidResult);
        numLogicalCores = (cpuidResult.ebx >> 16) & 0xFF;

        if (numLogicalCores > 1) {
            thread = get_current_thread_wrapper();
            affinityMask = set_thread_affinity_wrapper(thread, 1);
            numCores = 1;
            for (core = 0; core < numLogicalCores; ++core) {
                set_thread_affinity_wrapper(thread, (size_t)1 << core);
                cpuid_wrapper(0x8000001E, 0, &cpuidResult);

                caps->coreIDs[core] = (char)(cpuidResult.ebx & 0xFF);

                if (core && (caps->coreIDs[core] != caps->coreIDs[core - 1])) {
                    ++numCores;
                }
            }
            set_thread_affinity_wrapper(thread, affinityMask);

            caps->numCores = (int)numCores;
            caps->numLogicalCores = (int)numLogicalCores;
        }
    }
}
