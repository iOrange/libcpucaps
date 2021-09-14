#ifndef LIBCPUCAPS_H_HEADER
#define LIBCPUCAPS_H_HEADER

#define LIBCPUCAPS_MAX_CPU_NAME_LEN     48
#define LIBCPUCAPS_MAX_CPU_VENDOR_LEN   (12 + 1)

#define LIBCPUCAPS_ERROR_OK              0
#define LIBCPUCAPS_ERROR_FAILED         -1
#define LIBCPUCAPS_ERROR_INVALID_PARAM  -2

typedef struct _s_cpucaps {
    char  vendor[LIBCPUCAPS_MAX_CPU_VENDOR_LEN];
    char  name[LIBCPUCAPS_MAX_CPU_NAME_LEN];

    /* cpu vendor flags */
    char  isIntel;
    char  isAMD;

    /* cpu info */
    char  stepping;
    char  model;
    char  family;
    char  cpuType;
    char  modelEx;
    char  familyEx;

    /* cache info */
    int   L1d_lineSizeBytes;
    int   L1d_sizeKibiBytes;
    int   L1d_associativityType;
    int   L1i_lineSizeBytes;
    int   L1i_sizeKibiBytes;
    int   L1i_associativityType;
    int   L2_lineSizeBytes;
    int   L2_sizeKibiBytes;
    int   L2_associativityType;
    int   L3_lineSizeBytes;
    int   L3_sizeKibiBytes;
    int   L3_associativityType;

    /* cpu features */
    int   func1_ecx;
    int   func1_edx;
    int   func7_ebx;
    int   func7_ecx;

    /* cpu extended features */
    int   func80000001_ecx;
    int   func80000001_edx;
} cpucaps_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* returns LIBCPUCAPS_ERROR_xxx */
int libcpucaps_GetCaps(cpucaps_t* caps);

/* functions to query specific features support (return 1 or 0) */
int libcpucaps_HasFPU(cpucaps_t* caps);
int libcpucaps_HasPSE(cpucaps_t* caps);
int libcpucaps_HasTSC(cpucaps_t* caps);
int libcpucaps_HasCMPXCHG8(cpucaps_t* caps);
int libcpucaps_HasCMPXCHG16B(cpucaps_t* caps);
int libcpucaps_HasMMX(cpucaps_t* caps);
int libcpucaps_HasMMXExt(cpucaps_t* caps);
int libcpucaps_Has3DNow(cpucaps_t* caps);
int libcpucaps_Has3DNowExt(cpucaps_t* caps);
int libcpucaps_HasSSE(cpucaps_t* caps);
int libcpucaps_HasSSE2(cpucaps_t* caps);
int libcpucaps_HasSSE3(cpucaps_t* caps);
int libcpucaps_HasSSSE3(cpucaps_t* caps);
int libcpucaps_HasSSE41(cpucaps_t* caps);
int libcpucaps_HasSSE42(cpucaps_t* caps);
int libcpucaps_HasABM(cpucaps_t* caps);		/* POPCNT & LZCNT */
int libcpucaps_HasSSE4a(cpucaps_t* caps);
int libcpucaps_HasMisalignSSE(cpucaps_t* caps);
int libcpucaps_HasAVX(cpucaps_t* caps);
int libcpucaps_HasAVX2(cpucaps_t* caps);
int libcpucaps_HasAVX512F(cpucaps_t* caps);
int libcpucaps_HasAVX512PF(cpucaps_t* caps);
int libcpucaps_HasAVX512ER(cpucaps_t* caps);
int libcpucaps_HasAVX512CD(cpucaps_t* caps);
int libcpucaps_HasF16C(cpucaps_t* caps);
int libcpucaps_HasRDRAND(cpucaps_t* caps);
int libcpucaps_HasRDSEED(cpucaps_t* caps);
int libcpucaps_HasFMA3(cpucaps_t* caps);
int libcpucaps_HasFMA4(cpucaps_t* caps);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBCPUCAPS_H_HEADER */
