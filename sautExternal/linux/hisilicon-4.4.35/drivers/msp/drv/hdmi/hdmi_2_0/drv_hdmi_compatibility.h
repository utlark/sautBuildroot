#ifndef __DRV_HDMI_COMPATIBILITY_H__
#define __DRV_HDMI_COMPATIBILITY_H__

#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"
typedef struct
{
    HI_U8           u8IDManufactureName[4];     /**<Manufacture name*//**<CNcomment:�豸���̱�ʶ */
    HI_U32          u32IDProductCode;           /**<Product code*//**<CNcomment:�豸ID */
    HI_U32          u32DelayTimes;
    HI_U32          u32MuteDelay;
    HI_U8           u8ProductType[32];          /**<Product Type*//**<CNcomment:��Ʒ�ͺ� */
}HDMI_DELAY_TIME_S;

typedef struct
{
    HI_U32  u32CrgDivMs;
    HI_U32  u32PhySrstMs;
}COMPAT_CRG_SRST_S;

HI_S32 FormatDelayGet(HDMI_DEVICE_S* pstHdmiDev, HI_U32 *pu32DelayTime);
HI_S32 AvMuteDelayGet(HDMI_DEVICE_S* pstHdmiDev, HI_U32 *pu32DelayTime);
HI_BOOL DRV_HDMI_CompatCecValidCheck(HDMI_DEVICE_S *pstHdmiDev);
HI_S32  DRV_HDMI_CompatHdcpMsGet(HDMI_DEVICE_ID_E enHdmiId,HI_U64 *pu64WaitMs);
HI_S32  DRV_HDMI_CompatAvmuteGet(HDMI_DEVICE_ID_E enHdmiId,HI_BOOL bAvmute,HDMI_AVMUTE_CFG_S *pstAvmuteCfg);
HI_S32  DRV_HDMI_CompatCrgMsGet(HDMI_DEVICE_ID_E enHdmiId,HI_U32 *pu32CrgDivMs,HI_U32 *pu32PhySrstMs);
HI_BOOL DRV_HDMI_CompatScdcInStartGet(HDMI_DEVICE_ID_E enHdmiId);
HI_S32  DRV_HDMI_CompatAudNCtsGet(HDMI_DEVICE_ID_E enHdmiId, HI_U32 *pu32NValue, HI_U32 *pu32CtsValue);
#ifdef HDMI_HDR_SUPPORT
HI_S32  DRV_HDMI_CompatHdrDebugModeGet(HDMI_DEVICE_ID_E enHdmiId, HDMI_HDR_DEBUG_MOD_E *penHdrDebugMode);
#endif

#endif //__DRV_HDMI_COMPATIBILITY_H__