/*******************************************************************************
*	      Copyright 2016 - 2016, Hisilicon Tech. Co., Ltd.
*			    ALL RIGHTS RESERVED
* FileName: drv_tsio_ext.h
* Description: export symbols to other kernel modules.
*
* History:
* Version   Date	      Author	  DefectNum	 Description
* main\1    2016-08-15	      l00326021	    NULL	    init.
*******************************************************************************/

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __DRV_TSIO_EXT_H__
#define __DRV_TSIO_EXT_H__

/*
 * export to demux modules.
 */
typedef HI_S32 (*FN_TSIO_DmxFqBpTsioPort)(HI_U32 DmxId, HI_U32 FqId, HI_U32 DmxTsioPortId, HI_S32 (*Op)(HI_U32 DmxId, HI_U32 DmxTsioPortId, HI_U32 FqId, HI_U32 TsioRamPortId));

typedef struct
{
    FN_TSIO_DmxFqBpTsioPort pfnDmxFqBpTsioPort;
} TSIO_EXPORT_FUNC_S;

#endif	// __DRV_TSIO_EXT_H__
