#ifndef __DRV_ADSP_EXT_H__
#define __DRV_ADSP_EXT_H__

#include "hi_drv_dev.h"
#include "hi_type.h"
#include "hi_audsp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct
{
    HI_U32  u32AoeRegBaseAddr;
    HI_BOOL bAoeSwFlag;
} ADSP_FIRMWARE_AOE_INFO_S;

typedef HI_S32 (*FN_ADSP_LoadFrm)(ADSP_CODEID_E);

typedef HI_S32 (*FN_ADSP_UnLoadFrm)(ADSP_CODEID_E);

typedef HI_S32 (*FN_ADSP_GetAoeFwmInfo)(ADSP_CODEID_E, ADSP_FIRMWARE_AOE_INFO_S*);

typedef struct
{
    FN_ADSP_LoadFrm pfnADSP_LoadFirmware;
    FN_ADSP_UnLoadFrm pfnADSP_UnLoadFirmware;
    FN_ADSP_GetAoeFwmInfo pfnADSP_GetAoeFwmInfo;
} ADSP_EXPORT_FUNC_S;

HI_S32  ADSP_DRV_ModInit(HI_VOID);
HI_VOID ADSP_DRV_ModExit(HI_VOID);

#if defined(HI_MCE_SUPPORT)
HI_S32  HI_DRV_ADSP_Init(HI_VOID);
HI_VOID HI_DRV_ADSP_DeInit(HI_VOID);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DRV_ADSP_EXT_H__ */
