/******************************************************************************

  Copyright (C), 2012-2014, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name	: drv_hdmirx_ioctl.h
  Version	: Initial Draft
  Author	: Hisilicon multimedia software group
  Created	: 2014/02/26
  Description	:
  History	:
  1.Date	: 2014/02/26
    Author	: t00202585
    Modification: Created file
******************************************************************************/

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __DRV_HDMIRX_EXT_H__
#define __DRV_HDMIRX_EXT_H__

#include "hi_type.h"
typedef enum hiDRV_HDMIRX_DATA_ROUTE_E
{
    HI_DRV_HDMIRX_DATA_ROUTE_CTRL0 = 0,	   // HDMIRX data route to HDMI Control Core 0.
    HI_DRV_HDMIRX_DATA_ROUTE_CTRL1,	   // HDMIRX data route to HDMI Control Core 1.
    HI_DRV_HDMIRX_DATA_ROUTE_CTRL0_CTRL1,  // HDMIRX data route to HDMI Control Core 0 and 1.

    HI_DRV_HDMIRX_DATA_ROUTE_BUTT,
} HI_DRV_HDMIRX_DATA_ROUTE_E;

typedef struct
{
    HI_U32 (*pfnHDMIRXGetDataRoute)(HI_VOID);
} HDMIRX_EXPORT_FUNC_S;

#endif
