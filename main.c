#include "libcpucaps.h"

#include <stdio.h>

int main() {
    int i, j, k;
    cpucaps_t caps;
    if (LIBCPUCAPS_ERROR_OK == libcpucaps_GetCaps(&caps)) {
        if (caps.isIntel) {
            printf("Intel cpu detected.\n\n");
        } else if (caps.isAMD) {
            printf("AMD cpu detected.\n\n");
        } else {
            printf("Unknown cpu detected.\n\n");
        }

        printf("CPU info:\n");
        printf("  CPU vendor : %s\n", caps.vendor);
        printf("    CPU name : %s\n", caps.name);
        printf("    stepping : %d\n", caps.stepping);
        printf("       model : %d\n", caps.model);
        printf("      family : %d\n", caps.family);
        printf("    cpu type : %d\n", caps.cpuType);
        printf("    model ex : %d\n", caps.modelEx);
        printf("   family ex : %d\n", caps.familyEx);
        printf(" phys. cores : %d\n", caps.numCores);
        printf(" logi. cores : %d\n", caps.numLogicalCores);
        printf("    L1d line : %d B\n", caps.L1d_lineSizeBytes);
        printf("    L1d size : %d KB\n", caps.L1d_sizeKibiBytes);
        printf("   L1d assoc : %d\n", caps.L1d_associativityType);
        printf("    L1i line : %d B\n", caps.L1i_lineSizeBytes);
        printf("    L1i size : %d KB\n", caps.L1i_sizeKibiBytes);
        printf("   L1i assoc : %d\n", caps.L1i_associativityType);
        printf("     L2 line : %d B\n", caps.L2_lineSizeBytes);
        printf("     L2 size : %d KB\n", caps.L2_sizeKibiBytes);
        printf("    L2 assoc : %d\n", caps.L2_associativityType);
        printf("     L3 line : %d B\n", caps.L3_lineSizeBytes);
        printf("     L3 size : %d KB\n", caps.L3_sizeKibiBytes);
        printf("    L3 assoc : %d\n", caps.L3_associativityType);

        printf("\n");
        printf("Extended topology:\n");
        for (i = 0, j = 0, k = 0; i < caps.numLogicalCores; ++i, ++j) {
            if (i && (caps.coreIDs[i - 1] != caps.coreIDs[i])) {
                printf("  Physical core #%d has %d logical cores\n", k, j);
                j = 0;
                k++;
            }
        }
        printf("  Physical core #%d has %d logical cores\n", k, j);

        printf("\n");
        printf("CPU caps:\n");

#define PRINT_CAP(cap)  printf("%*s\n", 18, libcpucaps_Has##cap(&caps) ? (#cap " : YES") : (#cap " :  NO"))

        PRINT_CAP(FPU);
        PRINT_CAP(PSE);
        PRINT_CAP(TSC);
        PRINT_CAP(CMPXCHG8);
        PRINT_CAP(CMPXCHG16B);
        PRINT_CAP(MMX);
        PRINT_CAP(MMXExt);
        PRINT_CAP(3DNow);
        PRINT_CAP(3DNowExt);
        PRINT_CAP(SSE);
        PRINT_CAP(SSE2);
        PRINT_CAP(SSE3);
        PRINT_CAP(SSSE3);
        PRINT_CAP(SSE41);
        PRINT_CAP(SSE42);
        PRINT_CAP(ABM);
        PRINT_CAP(SSE4a);
        PRINT_CAP(MisalignSSE);
        PRINT_CAP(AES);
        PRINT_CAP(AVX);
        PRINT_CAP(AVX2);
        PRINT_CAP(AVX512F);
        PRINT_CAP(AVX512PF);
        PRINT_CAP(AVX512ER);
        PRINT_CAP(AVX512CD);
        PRINT_CAP(F16C);
        PRINT_CAP(RDRAND);
        PRINT_CAP(RDSEED);
        PRINT_CAP(FMA3);
        PRINT_CAP(FMA4);

    } else {
        printf("Failed to get CPU caps\n");
    }

    printf("\nPress Enter to exit...\n");
    getchar();

    return 0;
}
