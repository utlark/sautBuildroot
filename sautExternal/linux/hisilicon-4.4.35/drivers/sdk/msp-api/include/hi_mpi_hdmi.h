/******************************************************************************

  Copyright (C), 2016-2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name	: mpi_hdmi.h
  Version	: Initial Draft
  Author	: Hisilicon multimedia software group
  Created	: 2010/11/3
  Description	:
  History	:
  1.Date	: 2010/11/3
    Author	: l00168554
    Modification: Created file

*******************************************************************************/

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __MPI_HDMI_H__
#define __MPI_HDMI_H__

#if defined  (CONFIG_HDMI_BVT_SDK)
#include "mkp_hdmi.h"
#include "hi_comm_hdmi.h"
#else
#include "hi_unf_disp.h"
#include "hi_drv_hdmi.h"
#include "drv_hdmi_ioctl.h"
#include "hi_drv_disp.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#define HDMI_UDEEPCOLOR_2_KDEEPCOLOR(udeepcolor, kdeepcolor) \
do{\
    if (HI_UNF_HDMI_DEEP_COLOR_24BIT == udeepcolor)\
    {\
	kdeepcolor = HDMI_DEEP_COLOR_24BIT;\
    }\
    else if (HI_UNF_HDMI_DEEP_COLOR_30BIT == udeepcolor)\
    {\
	kdeepcolor = HDMI_DEEP_COLOR_30BIT;\
    }\
    else if (HI_UNF_HDMI_DEEP_COLOR_36BIT == udeepcolor)\
    {\
	kdeepcolor = HDMI_DEEP_COLOR_36BIT;\
    }\
    else if (HI_UNF_HDMI_DEEP_COLOR_OFF == udeepcolor)\
    {\
	kdeepcolor = HDMI_DEEP_COLOR_OFF;\
    }\
    else\
    {\
	kdeepcolor = HDMI_DEEP_COLOR_BUTT;\
    }\
}while(0)

#define HDMI_KDEEPCOLOR_2_UDEEPCOLOR(kdeepcolor, udeepcolor) \
do{\
    if (HDMI_DEEP_COLOR_24BIT == kdeepcolor)\
    {\
	udeepcolor = HI_UNF_HDMI_DEEP_COLOR_24BIT;\
    }\
    else if (HDMI_DEEP_COLOR_30BIT == kdeepcolor)\
    {\
	udeepcolor = HI_UNF_HDMI_DEEP_COLOR_30BIT;\
    }\
    else if (HDMI_DEEP_COLOR_36BIT == kdeepcolor)\
    {\
	udeepcolor = HI_UNF_HDMI_DEEP_COLOR_36BIT;\
    }\
    else if (HDMI_DEEP_COLOR_OFF == kdeepcolor)\
    {\
	udeepcolor = HI_UNF_HDMI_DEEP_COLOR_OFF;\
    }\
    else\
    {\
	udeepcolor = HI_UNF_HDMI_DEEP_COLOR_BUTT;\
    }\
}while(0)

#define HDMI_UEVENTTYPE_2_KEVENTTYPE(ueventtype, keventtype) \
do{\
    if (HI_UNF_HDMI_EVENT_HOTPLUG == ueventtype)\
    {\
	keventtype = HDMI_EVENT_HOTPLUG;\
    }\
    else if (HI_UNF_HDMI_EVENT_NO_PLUG == ueventtype)\
    {\
	keventtype = HDMI_EVENT_HOTUNPLUG;\
    }\
    else if (HI_UNF_HDMI_EVENT_EDID_FAIL == ueventtype)\
    {\
	keventtype = HDMI_EVENT_EDID_FAIL;\
    }\
    else if (HI_UNF_HDMI_EVENT_HDCP_FAIL == ueventtype)\
    {\
	keventtype = HDMI_EVENT_HDCP_FAIL;\
    }\
    else if (HI_UNF_HDMI_EVENT_HDCP_SUCCESS == ueventtype)\
    {\
	keventtype = HDMI_EVENT_HDCP_SUCCESS;\
    }\
    else if( HI_UNF_HDMI_EVENT_RSEN_CONNECT == ueventtype)\
    {\
	keventtype = HDMI_EVENT_RSEN_CONNECT;\
    }\
    else if( HI_UNF_HDMI_EVENT_RSEN_DISCONNECT == ueventtype)\
    {\
	keventtype = HDMI_EVENT_RSEN_DISCONNECT;\
    }\
    else if (HI_UNF_HDMI_EVENT_HDCP_USERSETTING == ueventtype)\
    {\
	keventtype = HDMI_EVENT_HDCP_USERSETTING;\
    }\
    else\
    {\
	keventtype = HDMI_EVENT_BUTT;\
    }\
}while(0)

#define HDMI_KEVENTTYPE_2_UEVENTTYPE(keventtype, ueventtype) \
do{\
    if (HDMI_EVENT_HOTPLUG == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_HOTPLUG;\
    }\
    else if (HDMI_EVENT_HOTUNPLUG == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_NO_PLUG;\
    }\
    else if (HDMI_EVENT_EDID_FAIL == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_EDID_FAIL;\
    }\
    else if (HDMI_EVENT_HDCP_FAIL == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_HDCP_FAIL;\
    }\
    else if (HDMI_EVENT_HDCP_SUCCESS == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_HDCP_SUCCESS;\
    }\
    else if( HDMI_EVENT_RSEN_CONNECT == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_RSEN_CONNECT;\
    }\
    else if( HDMI_EVENT_RSEN_DISCONNECT == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_RSEN_DISCONNECT;\
    }\
    else if (HDMI_EVENT_HDCP_USERSETTING == keventtype)\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_HDCP_USERSETTING;\
    }\
    else\
    {\
	ueventtype = HI_UNF_HDMI_EVENT_BUTT;\
    }\
}while(0)

#define HDMI_KHDCPMODE_2_UHDCPMODE(khdcpmode, uhdcpmode) \
do{\
    if (HDMI_HDCP_MODE_AUTO == khdcpmode)\
    {\
	uhdcpmode = HI_UNF_HDMI_HDCP_MODE_AUTO;\
    }\
    else if (HDMI_HDCP_MODE_1_4 == khdcpmode)\
    {\
	uhdcpmode = HI_UNF_HDMI_HDCP_MODE_1_4;\
    }\
    else if (HDMI_HDCP_MODE_2_2 == khdcpmode)\
    {\
	uhdcpmode = HI_UNF_HDMI_HDCP_MODE_2_2;\
    }\
    else\
    {\
	uhdcpmode = HI_UNF_HDMI_HDCP_MODE_AUTO;\
    }\
}while(0)

#define HDMI_UHDCPMODE_2_KHDCPMODE(uhdcpmode, khdcpmode) \
do{\
    if (HI_UNF_HDMI_HDCP_MODE_AUTO == uhdcpmode)\
    {\
	khdcpmode = HDMI_HDCP_MODE_AUTO;\
    }\
    else if (HI_UNF_HDMI_HDCP_MODE_1_4 == uhdcpmode)\
    {\
	khdcpmode = HDMI_HDCP_MODE_1_4;\
    }\
    else if (HI_UNF_HDMI_HDCP_MODE_2_2 == uhdcpmode)\
    {\
	khdcpmode = HDMI_HDCP_MODE_2_2;\
    }\
    else\
    {\
	khdcpmode = HDMI_HDCP_MODE_AUTO;\
    }\
}while(0)

HI_S32 HI_MPI_HDMI_ComInit(void);
HI_S32 HI_MPI_HDMI_ComDeInit(void);
HI_S32 HI_MPI_HDMI_ComOpen(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_OPEN_PARA_S *pstOpenPara);
HI_S32 HI_MPI_HDMI_ComClose(HI_UNF_HDMI_ID_E enHdmi);
HI_S32 HI_MPI_HDMI_ComStart(HI_UNF_HDMI_ID_E enHdmi);
HI_S32 HI_MPI_HDMI_ComStop(HI_UNF_HDMI_ID_E enHdmi);
HI_S32 HI_MPI_HDMI_ComSetAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_APP_ATTR_S *pstAppAttr);
HI_S32 HI_MPI_HDMI_ComGetAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_APP_ATTR_S *pstAppAttr);
HI_S32 HI_MPI_HDMI_ComSetAOAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_AO_ATTR_S *pstDrvAOAttr);
HI_S32 HI_MPI_HDMI_ComGetAOAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_AO_ATTR_S *pstDrvAOAttr);
HI_S32 HI_MPI_HDMI_ComSetVOAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_VO_ATTR_S *pstDrvVOAttr);
HI_S32 HI_MPI_HDMI_ComGetVOAttr(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_VO_ATTR_S *pstDrvVOAttr);

HI_S32 HI_MPI_HDMI_ComSetInfoFrame(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_INFOFRAME_S *pstInfoFrame);
HI_S32 HI_MPI_HDMI_ComGetInfoFrame(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_INFOFRAME_S *pstInfoFrame);
HI_S32 HI_MPI_HDMI_ComSetDeepColor(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_DEEP_COLOR_E enDeepColor);
HI_S32 HI_MPI_HDMI_ComGetDeepColor(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_DEEP_COLOR_E *penDeepColor);
HI_S32 HI_MPI_HDMI_ComSetxvYCCMode(HI_UNF_HDMI_ID_E enHdmi, HI_BOOL bEnalbe);
HI_S32 HI_MPI_HDMI_ComSetAVMute(HI_UNF_HDMI_ID_E enHdmi, HI_BOOL bAvMute);

HI_S32 HI_MPI_HDMI_ComGetStatus(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_STATUS_S *pHdmiStatus);

HI_S32 HI_MPI_HDMI_ComGetSinkCapability(HI_UNF_HDMI_ID_E enHdmi, DRV_HDMI_SINK_CAPABILITY_S *pstDrvCap);
HI_S32 HI_MPI_HDMI_ComForceGetEDID(HI_UNF_HDMI_ID_E enHdmi, HI_U8 *u8Edid, HI_U32 *u32EdidLength);
HI_S32 HI_MPI_HDMI_ComReadEDID(HI_U8 *u8Edid, HI_U32 *u32EdidLength);

HI_S32 HI_MPI_HDMI_ComRegCallbackFunc(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);
HI_S32 HI_MPI_HDMI_ComUnRegCallbackFunc(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);

HI_S32 HI_MPI_HDMI_ComRegCECCallBackFunc(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CECCALLBACK pCECCallback);
HI_S32 HI_MPI_HDMI_ComUnRegCECCallBackFunc(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CECCALLBACK pCECCallback);
HI_S32 HI_MPI_HDMI_ComSetCECCommand(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CEC_CMD_S  *pCECCmd);
HI_S32 HI_MPI_HDMI_ComGetCECCommand(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CEC_CMD_S  *pCECCmd);
HI_S32 HI_MPI_HDMI_ComCECStatus(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_CEC_STATUS_S  *pStatus);
HI_S32 HI_MPI_HDMI_ComCECEnable(HI_UNF_HDMI_ID_E enHdmi);
HI_S32 HI_MPI_HDMI_ComCECDisable(HI_UNF_HDMI_ID_E enHdmi);

HI_S32 HI_MPI_HDMI_ComGetDelay(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_DELAY_S *pstDelay);
HI_S32 HI_MPI_HDMI_ComSetDelay(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_HDMI_DELAY_S *pstDelay);
HI_BOOL HI_MPI_HDMI_GetResumeState(HI_UNF_HDMI_ID_E enHdmi);
HI_S32 HI_MPI_HDMI_ClearResumeState(HI_UNF_HDMI_ID_E enHdmi);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MPI_HDMI_H__ */
