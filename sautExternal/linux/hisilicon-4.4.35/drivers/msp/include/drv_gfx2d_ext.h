/******************************************************************************
*
* Copyright (C) 2016 Hisilicon Technologies Co., Ltd.  All rights reserved.
*
* This program is confidential and proprietary to Hisilicon  Technologies Co., Ltd. (Hisilicon),
* and may not be copied, reproduced, modified, disclosed to others, published or used, in
* whole or in part, without the express prior written permission of Hisilicon.
*
******************************************************************************
File Name	    : drv_gfx2d_ext.h
Version		    : Initial Draft
Author		    :
Created		    : 2016/02/25
Description	    :
Function List	    :
History		    :
Date			    Author		    Modification
2016/02/25		    y00181162		    Created file
******************************************************************************/

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef	 __DRV_GFX2D_EXT_H__
#define	 __DRV_GFX2D_EXT_H__

/*********************************add include here******************************/
#include "hi_type.h"

/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/


/***************************  The enum of Jpeg image format  ******************/

/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/
#include "hi_drv_dev.h"

typedef HI_S32	(*FN_GFX2D_ModInit)(HI_VOID);
typedef HI_VOID (*FN_GFX2D_ModeExit)(HI_VOID);
typedef HI_S32	(*FN_GFX2D_Suspend)(PM_BASEDEV_S *, pm_message_t);
typedef HI_S32	(*FN_GFX2D_Resume)(PM_BASEDEV_S *);

typedef struct
{
    FN_GFX2D_ModInit	   pfnGfx2dModInit;
    FN_GFX2D_ModeExit	   pfnGfx2dModExit;
	FN_GFX2D_Suspend	   pfnGfx2dSuspend;
	FN_GFX2D_Resume		   pfnGfx2dResume;
} GFX2D_EXPORT_FUNC_S;


HI_S32 GFX2D_OS_ModInit(HI_VOID);
HI_VOID GFX2D_OS_ModExit(HI_VOID);

#endif /*__DRV_GFX2D_EXT_H__ */
