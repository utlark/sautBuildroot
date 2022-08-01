/*$$$!!Warning: Huawei key information asset. No spread without permission.$$$*/
/*CODEMARK:HuE1v2Elv7J9C1aMkGmdiUQZoS/R2sDdIoxXqdBKL9eiPHH/FdSvQYZfBQZKkVdipTcRXX+G
kqk+/W4lTjU7wqFxjZf0LDwCjpr43YYwLpAVvVw4ERKDSiimgdao4nwOhxbHUZ3TWiJ5iG78
Gfd3ktUGWIbW4Wgmnfhcg735krJHS331ZFG+y7AafWPS7lH407LhKlE24NnqCJC8IGI5KWCl
v30b7TRNk1s061vApsllYGAWmWLtHkJthfIREWylILKyTEsfzXGo+yGsJJ/q3A==#*/
/*$$$!!Warning: Deleting or modifying the preceding information is prohibited.$$$*/
/******************************************************************************

  ��Ȩ���� (C), 2016-, ��Ϊ�������޹�˾

******************************************************************************
    �� �� ��   : vfmw_tee_client.c
    �� �� ��   : ����
    ��    ��   :
    ��������   :
    ����޸�   :
    ��������   : firmware�İ�ȫ�����


    �޸���ʷ   :
    1.��    �� : 2016-09-09
    ��    ��   :
    �޸�����   :

******************************************************************************/

#include "public.h"
#include "vfmw_proc.h"
#include "vfmw_osal_ext.h"
#include "vfmw_tee_client.h"

#include <teek_client_api.h>

#if (1 == DEBUG_SAVE_SUPPORT)
#include "sec_mmz.h"
#endif

#define PATH_LEN              (64)
#define PROC_CMD_LEN          (16)
#define SECURE_NOTIFY_IRQ_NUM (158+32)

#define VFMW_SEC_PROC_NAME    "secure"
#define PROC_CMD_HELP         "help"
#define PROC_CMD_SUSPEND      "suspend"
#define PROC_CMD_RESUME       "resume"
#define PROC_CMD_SAVERAW      "saveraw"
#define PROC_CMD_SAVEYUV      "saveyuv"
#define PROC_CMD_START        "start"
#define PROC_CMD_STOP         "stop"
#define PROC_CMD_PRINTTIME    "printtime"
#define PROC_CMD_SETPRINT     (0x000)
#define PROC_CMD_SELECT       (0x900)

#define INVALID_HANDLE        (-1)

#ifndef HI_ADVCA_FUNCTION_RELEASE
#define SecPrint(type, fmt, arg...)             \
    do{                                             \
        if (PRN_ALWS == type                        \
            || 0 != (g_SecPrintEnable & (1 << type)))  \
        {                                           \
            OSAL_FP_print(fmt, ##arg);       \
        }                                           \
    }while(0)
#else
// �߰����ܼ�⵽�ַ�����printk
#define SecPrint(type, fmt, arg...)             \
    do{                                             \
    }while(0)
#endif

#define  TVP_VDEC_ASSERT( cond, else_print )               \
do {                                                            \
    if( !(cond) )                                               \
    {                                                           \
        SecPrint(PRN_FATAL,"%s at L%d: %s\n", __func__, __LINE__, else_print ); \
        return VDEC_ERR;                                       \
    }                                                           \
}while(0)
typedef enum hiCHAN_STATE_E
{
    CHAN_INVALID = 0,
    CHAN_START,
    CHAN_STOP,
    CHAN_BUTT,
} CHAN_STATE_E;

typedef enum hiTHREAD_STATE_E
{
    THREAD_INVALID = 0,
    THREAD_SLEEP,
    THREAD_RUN,
    THREAD_EXIT,
    THREAD_BUTT,
} THREAD_STATE_E;

typedef enum
{
    VFMW_CMD_ID_INVALID = 0x0,
    VFMW_CMD_ID_INIT,
    VFMW_CMD_ID_EXIT,
    VFMW_CMD_ID_RESUME,
    VFMW_CMD_ID_SUSPEND,
    VFMW_CMD_ID_CONTROL,
    VFMW_CMD_ID_THREAD,
    VFMW_CMD_ID_GET_IMAGE,
    VFMW_CMD_ID_RELEASE_IMAGE,
    VFMW_CMD_ID_READ_PROC,
    VFMW_CMD_ID_WRITE_PROC,
} TEE_VFMW_CMD_ID;

typedef struct
{
    HI_BOOL            bIsSecMode;
    SINT32             OutputImgEn;
    SINT32             ChanID;
    CHAN_STATE_E       ChanState;
    OSAL_SEMA          ChanSema;
    OSAL_FILE         *pRawFile;
} CHAN_CONTEXT_S;

typedef struct hiVFMW_CALLBACK_S
{
    SINT32        (*event_report_vdec)(SINT32 ChanID, SINT32 type, VOID *p_args);
    SINT32        (*event_report_omxvdec)(SINT32 ChanID, SINT32 type, VOID *p_args);
}VFMW_CALLBACK_S;

/* Global pointer */
MEMORY_NEEDED_SECVFMW_S *g_pSecVfmwMem = NULL;
CALLBACK_ARRAY_NS       *g_pCallbackBuf = NULL;
RAW_ARRAY_NS            *g_pStreamBuf = NULL;
IMAGE_QUEUE_NS          *g_pImageQue    = NULL;
#if (1 == DEBUG_SAVE_SUPPORT)
UINT8                   *g_pSaveStreamBuf = NULL;
#endif
#ifndef  HI_ADVCA_FUNCTION_RELEASE
UINT8                   *g_pProcBuf     = NULL;
#endif

static UINT32 g_SecInvokeirqHandle = 0;

/*TEE ͨ����ر���*/
static TEEC_Context      g_TeeContext;
static TEEC_Session      g_TeeSession;

/*��̬ȫ�ֱ���*/
static MEM_DESC_S        g_SecVfmwMem;
static SINT32            g_SecPrintEnable       = 0x0;
static SINT32            g_SecProcSelect        = 0x0;
static SINT32            g_SecureInstNum        = 0;
static SINT32            g_TrustDecoderInitCnt  = 0;
static HI_BOOL           g_bSecEnvSetUp    = HI_FALSE;
static THREAD_STATE_E    g_THREAD_STATE         = THREAD_INVALID;
static OSAL_SEMA         g_stSem_s;
static VFMW_CALLBACK_S   g_CallBack;
OSAL_EVENT               g_SecThreadEvent;
#if (1 == DEBUG_SAVE_SUPPORT)
static HI_BOOL           g_RawSaveEnable        = HI_FALSE;
static HI_CHAR           g_SavePath[PATH_LEN]   = {'/', 'm', 'n', 't', '\0'};
#endif
static HI_BOOL           g_bPrintTime           = HI_FALSE;


/*ͨ�����ȫ�ֱ���*/
static STREAM_INTF_S     g_StreamIntf[MAX_CHAN_NUM];
static CHAN_CONTEXT_S    g_ChanContext[MAX_CHAN_NUM];

#if (1 == DEBUG_SAVE_SUPPORT)
VOID TVP_VDEC_SaveStream(SINT32 ChanID, HI_BOOL bSaveEnable, UADDR PhyAddr, SINT32 Length)
{
    HI_CHAR FilePath[PATH_LEN];
    OSAL_FILE **ppFile = &g_ChanContext[ChanID].pRawFile;

    if (HI_TRUE == bSaveEnable && *ppFile == NULL)
    {
        snprintf(FilePath, sizeof(FilePath), "%s/trusted_vfmw_chan%d.raw", g_SavePath, ChanID);
        *ppFile = OSAL_FP_fopen(FilePath, VFMW_O_RDWR | VFMW_O_CREATE | VFMW_O_TRUNC, 0);

        if (IS_ERR(*ppFile))
        {
            SecPrint(PRN_ERROR, "%s open raw file failed, ret=%ld\n", __func__, PTR_ERR(*ppFile));
            *ppFile = NULL;
        }
        else
        {
            SecPrint(PRN_ALWS, "Start to save stream of inst_%d in %s\n", ChanID, FilePath);
        }
    }
    else if (HI_FALSE == bSaveEnable && *ppFile != NULL)
    {
        OSAL_FP_fclose(*ppFile);
        *ppFile = NULL;
        SecPrint(PRN_ALWS, "Stop saving stream of inst_%d.\n", ChanID);
    }

    if (*ppFile != NULL)
    {
        HI_SEC_MMZ_TA2CA(PhyAddr, __pa(g_pSaveStreamBuf), Length);
        OSAL_FP_fwrite(g_pSaveStreamBuf, Length, *ppFile);
        SecPrint(PRN_ALWS, "Saving stream of inst_%d\n", ChanID);
    }

    return;
}
#endif


SINT32 TVP_VDEC_SetCallBack(VDEC_ADAPTER_TYPE_E eType, SINT32 (*event_report)(SINT32 ChanID, SINT32 type, VOID *p_args))
{
    TVP_VDEC_ASSERT(event_report != NULL, "event_report null!");

    switch (eType)
    {
        case ADAPTER_TYPE_VDEC:
            g_CallBack.event_report_vdec = event_report;
            break;

        case ADAPTER_TYPE_OMXVDEC:
            g_CallBack.event_report_omxvdec = event_report;
            break;

        default:
            SecPrint(PRN_FATAL, "%s Unkown Adapter Type: %d\n", __func__, eType);
            return VDEC_ERR;
    }

    return VDEC_OK;
}

SINT32 TVP_VDEC_ProcessCallBack(VOID)
{
    SINT32              ret = VDEC_OK;
    SINT32              ChanID;
    SINT32              Type;
    VDEC_ADAPTER_TYPE_E eAdapterType;
    UINT8               *pPara;

    while (g_pCallbackBuf->Tail != g_pCallbackBuf->Head)
    {
        if (g_pCallbackBuf->Message[g_pCallbackBuf->Tail].IsValid)
        {
            ChanID = g_pCallbackBuf->Message[g_pCallbackBuf->Tail].ChanID;
            Type   = g_pCallbackBuf->Message[g_pCallbackBuf->Tail].Type;
            pPara  = (HI_U8 *)g_pCallbackBuf->Message[g_pCallbackBuf->Tail].para;
            eAdapterType = g_pCallbackBuf->Message[g_pCallbackBuf->Tail].eAdapterType;

            switch (eAdapterType)
            {
                case ADAPTER_TYPE_VDEC:
                    if (NULL == g_CallBack.event_report_vdec)
                    {
                        SecPrint(PRN_FATAL, "s_VfmwCTrl.event_report_vdec = NULL\n");
                    }
                    else
                    {
                        ret = (g_CallBack.event_report_vdec)(ChanID, Type, pPara);
                    }
                    break;

                case ADAPTER_TYPE_OMXVDEC:
                    if (NULL == g_CallBack.event_report_omxvdec)
                    {
                        SecPrint(PRN_FATAL, "s_VfmwCTrl.event_report_omxvdec = NULL\n");
                    }
                    else
                    {
                        ret = (g_CallBack.event_report_omxvdec)(ChanID, Type, pPara);
                    }
                    break;

                default:
                    SecPrint(PRN_FATAL, "%s Unkown Adapter Type: %d\n", __func__, eAdapterType);
                    break;
            }
        }

        g_pCallbackBuf->Tail = (g_pCallbackBuf->Tail + 1) % MAX_EVENT_NUM;
    }

    return ret;
}

#ifdef VFMW_VPSS_BYPASS_EN
SINT32 TVP_VDEC_FindOccupiedFrame(SINT32 ChanID, VDEC_SPECIAL_FRM_S *pSpecialFrmInfo)
{
    SINT32 i = 0, j = 0;
    SINT32 ret = VDEC_ERR;
    OUTPUT_IMAGE_RECORD_S *pOutputRec;

    if (NULL != g_pImageQue)
    {
        pOutputRec = g_pImageQue[ChanID].StOutputRec;
        for (i = 0; i < MAX_QUEUE_IMAGE_NUM; i++)
        {
            if (1 == pOutputRec[i].IsUsed)
            {
                pSpecialFrmInfo->specialFrmRec[j++].PhyAddr = pOutputRec[i].TopLumaPhyAddr;

                pOutputRec[i].IsUsed = 0;

            }
        }

        pSpecialFrmInfo->specialFrameNum = j;
        OSAL_FP_memset(&g_pImageQue[ChanID], 0, sizeof(IMAGE_QUEUE_NS));
        ret = VDEC_OK;

    }

    return ret;
}
#endif

SINT32 TVP_VDEC_GetImage(SINT32 ChanID, IMAGE *pImage)
{
    SINT32 i;
    SINT32 ret = VDEC_ERR;
    UINT64 pOutFrame;
    IMAGE_LIST_S *pImgList;
    OUTPUT_IMAGE_RECORD_S *pOutputRec;

    //Down_Interruptible_with_Option(&g_stSem_s);
    if (!g_bSecEnvSetUp)
    {
        SecPrint(PRN_ERROR, "%s: TrustedDecoder not creat yet!\n", __func__);
        //Up_Interruptible_with_Option(&g_stSem_s);
        return ret;
    }

    if (1 == g_ChanContext[ChanID].OutputImgEn)
    {
        if ((NULL != g_pImageQue) && (ChanID >= TRUSTED_CHAN_BEGIN) && (ChanID < MAX_CHAN_NUM) && (NULL != pImage))
        {
            pImgList = &(g_pImageQue[ChanID].StOutputImg);
            if (pImgList->Head != pImgList->Tail)
            {
                pOutFrame = pImage->pOutFrame;
                OSAL_FP_memcpy(pImage, &(pImgList->StImage[pImgList->Head]), sizeof(IMAGE));
                pImage->pOutFrame = pOutFrame;
                pOutputRec = g_pImageQue[ChanID].StOutputRec;

                for (i=0; i<MAX_QUEUE_IMAGE_NUM; i++)
                {
                    if (pOutputRec[i].IsUsed == 0)
                    {
                        pOutputRec[i].TopLumaPhyAddr = pImage->top_luma_phy_addr;
                        pOutputRec[i].IsUsed        = 1;
                        break;
                    }
                }
                if (i < MAX_QUEUE_IMAGE_NUM)
                {
                    pImgList->Head = (pImgList->Head + 1) % MAX_QUEUE_IMAGE_NUM;
                    ret = VDEC_OK;
                }
                else
                {
                    SecPrint(PRN_ERROR, "%s no free output record slot\n", __func__);
                }
            }
        }
    }

    //Up_Interruptible_with_Option(&g_stSem_s);
    return ret;
}

SINT32 TVP_VDEC_ReleaseImage(SINT32 ChanID, IMAGE *pImage)
{
    SINT32 i;
    SINT32 ret = VDEC_ERR;
    IMAGE_LIST_S *pImgList;
    OUTPUT_IMAGE_RECORD_S *pOutputRec;

    //Down_Interruptible_with_Option(&g_stSem_s);

    if (!g_bSecEnvSetUp)
    {
        SecPrint(PRN_ERROR, "%s: TrustedDecoder not creat yet!\n", __func__);
        //Up_Interruptible_with_Option(&g_stSem_s);
        return ret;
    }

    //if (1 == g_ChanContext[ChanID].OutputImgEn)
    {
        if ((NULL!=g_pImageQue) && (ChanID>=TRUSTED_CHAN_BEGIN) && (ChanID<MAX_CHAN_NUM) && (NULL!=pImage))
        {
            pImgList = &g_pImageQue[ChanID].StReleaseImg;
            pImgList->StImage[pImgList->Tail].image_id = pImage->image_id;
            pImgList->StImage[pImgList->Tail].image_id_1 = pImage->image_id_1;
            pImgList->StImage[pImgList->Tail].top_luma_phy_addr = pImage->top_luma_phy_addr;
            pOutputRec = g_pImageQue[ChanID].StOutputRec;
            for (i=0; i<MAX_QUEUE_IMAGE_NUM; i++)
            {
                if ((pOutputRec[i].IsUsed == 1) && (pImage->top_luma_phy_addr == pOutputRec[i].TopLumaPhyAddr))
                {
                    pOutputRec[i].TopLumaPhyAddr = 0;
                    pOutputRec[i].IsUsed      = 0;
                    break;
                }
            }
            if (i < MAX_QUEUE_IMAGE_NUM)
            {
                pImgList->Tail = (pImgList->Tail + 1) % MAX_QUEUE_IMAGE_NUM;
                ret = VDEC_OK;
            }
            else
            {
                SecPrint(PRN_ERROR, "%s not found output record slot\n", __func__);
            }
        }
    }

    //Up_Interruptible_with_Option(&g_stSem_s);
    return ret;
}

/************************************************************************
   ��ȫ�ഥ�����жϴ������
 ************************************************************************/
SINT32 TVP_VDEC_SecNotify_Isr(SINT32 irq, VOID *dev_id)
{
    OSAL_GiveEvent_s(&g_SecThreadEvent);
    return 1;
}


VOID TVP_VDEC_GetChanStream(SINT32 ChanID)
{
    STREAM_DATA_S *pStrRawPacket = NULL;

    if (g_StreamIntf[ChanID].read_stream == NULL
        || g_ChanContext[ChanID].ChanState != CHAN_START)
    {
        //SecPrint(PRN_FATAL, "%s:%d chan:%d param is invalid state:%d\n", __func__, __LINE__, ChanID, g_ChanContext[ChanID].ChanState);

        return;
    }

    Down_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);

    while (1)
    {
        if (((g_pStreamBuf[ChanID].Head_NS + 1) % MAX_RAW_NUM) != g_pStreamBuf[ChanID].Tail_NS)
        {
            pStrRawPacket = &(g_pStreamBuf[ChanID].RawPacket[g_pStreamBuf[ChanID].Head_NS]);

            if (0 == g_StreamIntf[ChanID].read_stream(g_StreamIntf[ChanID].stream_provider_inst_id, pStrRawPacket))
            {
                g_pStreamBuf[ChanID].Head_NS = (g_pStreamBuf[ChanID].Head_NS + 1) % MAX_RAW_NUM;

            #if (1 == DEBUG_SAVE_SUPPORT)
                TVP_VDEC_SaveStream(ChanID, g_RawSaveEnable, pStrRawPacket->PhyAddr, pStrRawPacket->Length);
            #endif
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    Up_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);

    return;
}


VOID TVP_VDEC_ReleaseChanStream(SINT32 ChanID)
{
    STREAM_DATA_S *pStrRawPacket = NULL;

    if (g_StreamIntf[ChanID].release_stream == NULL)
    {
        //SecPrint(PRN_FATAL, "%s:%d ChanID:%d param is invalid\n", __func__, __LINE__, ChanID);

        return;
    }

    Down_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);

    while (1)
    {
        if (g_pStreamBuf[ChanID].Tail_NS != g_pStreamBuf[ChanID].Tail_S)
        {
            pStrRawPacket = &(g_pStreamBuf[ChanID].RawPacket[g_pStreamBuf[ChanID].Tail_NS]);

            if (0 == g_StreamIntf[ChanID].release_stream(g_StreamIntf[ChanID].stream_provider_inst_id, pStrRawPacket))
            {
                g_pStreamBuf[ChanID].Tail_NS = (g_pStreamBuf[ChanID].Tail_NS + 1) % MAX_RAW_NUM;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    Up_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);

    return;
}


/************************************************************************
   �ǰ�ȫ���̣߳�����������callback��Ϣ
 ************************************************************************/
SINT32 TVP_VDEC_MiddleWare(VOID *pArgs)
{
    SINT32 NextInstID;
    TEEC_Operation operation;
    TEEC_Result result;

    static UINT32 BeginTime = 0;
    static UINT32 StartTime = 0;
    static UINT32 EndTime = 0;
    static UINT32 TotalTime = 0;
    static UINT32 Count = 0;

    UINT32 LastStartTime = OSAL_GetTimeInUs();
    UINT32 CurrentTime = 0;

    while (1)
    {
        if (g_bPrintTime)
        {
            CurrentTime = OSAL_GetTimeInUs();

            if (CurrentTime - LastStartTime > 1000 * 15)
            {
                SecPrint(PRN_TEE, "NS: last time sleep exceed cost:   %d   us\n", CurrentTime - LastStartTime);
            }
        }

        switch (g_THREAD_STATE)
        {
            case THREAD_RUN:
                break;

            case THREAD_SLEEP:
                goto sleep;

            case THREAD_EXIT:
                goto exit;

            default:
                ;
        }

        /*������*/
        for ( NextInstID = TRUSTED_CHAN_BEGIN; NextInstID < MAX_CHAN_NUM; NextInstID++ )
        {
            TVP_VDEC_GetChanStream(NextInstID);
        }

        if (g_bPrintTime)
        {
            StartTime = OSAL_GetTimeInUs();
            if (Count == 0)
            {
                BeginTime = StartTime;
            }
        }

        /*���ð�ȫ������̺߳���*/
        operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
        operation.started = 1;

        result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_THREAD, &operation, NULL);

        if (TEEC_SUCCESS != result)
        {
            SecPrint(PRN_FATAL, "InvokeCommand VFMW_CMD_ID_VFMW_THREADPROC Failed!\n");
            return VDEC_ERR;
        }

#ifdef VDH_DISTRIBUTOR_ENABLE
        if (g_HalDisable != 1)
        {
            VDH_Secure_WakeUp_Distributor();
        }
#endif

        if (g_bPrintTime)
        {
            EndTime = OSAL_GetTimeInUs();
            TotalTime += EndTime - StartTime;
            Count++;

            if (EndTime - BeginTime >= 1000000)
            {
                SecPrint(PRN_TEE, "NS: Invoke Total:   %d   Count: %d  Avg:    %d     us\n", TotalTime, Count, TotalTime / Count);
                TotalTime = BeginTime = StartTime = EndTime = 0;
                Count = 0;
            }

            if (EndTime - StartTime >= 1000*10)
            {
                SecPrint(PRN_TEE, "NS: Invoke exceed cost:  %d  start:  %d   end:   %d   us\n", \
                                    EndTime - StartTime, StartTime, EndTime);
            }
        }

        /*�ϱ�callback��Ϣ*/
        TVP_VDEC_ProcessCallBack();

        /*�ͷ�����*/
        for ( NextInstID = TRUSTED_CHAN_BEGIN; NextInstID < MAX_CHAN_NUM; NextInstID++ )
        {
            TVP_VDEC_ReleaseChanStream(NextInstID);
        }

    sleep:
        LastStartTime = OSAL_GetTimeInUs();

        if (g_bPrintTime)
        {
            if (LastStartTime - CurrentTime >= 1000*10)
            {
                SecPrint(PRN_TEE, "NS: run NS thread exceed cost:  %d \n", LastStartTime - CurrentTime);
            }
        }

        OSAL_WaitEvent_s(&g_SecThreadEvent, 10);
    }

exit:
    g_THREAD_STATE = THREAD_INVALID;
    return VDEC_OK;
}

/************create thread*******************/
SINT32 TVP_VDEC_Thread_Init(VOID)
{
    OSAL_TASK pTask;
    OSAL_InitEvent_s(&g_SecThreadEvent, 0);

    OSAL_CreateTask((VOID *)&pTask, "SecVideoDec", TVP_VDEC_MiddleWare);

    if ( NULL == pTask)
    {
        SecPrint(PRN_FATAL, "Creat thread SecVideoDec Failed!\n");
        return VDEC_ERR;
    }

    g_THREAD_STATE = THREAD_SLEEP;

    return VDEC_OK;
}

/************reset chan release_stream*******************/
SINT32 TVP_VDEC_Release_Stream(SINT32 ChanID)
{
    STREAM_DATA_S *pStrRawPacket;

    /*�廹δ����ȫ���ȡ������*/
    if (NULL != g_StreamIntf[ChanID].release_stream)
    {
        while (1)
        {
            if (g_pStreamBuf[ChanID].Tail_NS != g_pStreamBuf[ChanID].Head_NS)
            {
                pStrRawPacket = &(g_pStreamBuf[ChanID].RawPacket[g_pStreamBuf[ChanID].Tail_NS]);

                if (0 == g_StreamIntf[ChanID].release_stream(g_StreamIntf[ChanID].stream_provider_inst_id, pStrRawPacket))
                {
                    g_pStreamBuf[ChanID].Tail_NS = (g_pStreamBuf[ChanID].Tail_NS + 1) % MAX_RAW_NUM;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    OSAL_FP_memset(&g_pStreamBuf[ChanID], 0, sizeof(RAW_ARRAY_NS));

    return VDEC_OK;
}

/********����Ӧͨ����callback��Ϣ******/
SINT32 TVP_VDEC_Release_Callback(SINT32 ChanID)
{
    SINT32 Tail;

    Tail = g_pCallbackBuf->Tail;

    while (Tail != g_pCallbackBuf->Head)
    {
        if (ChanID == g_pCallbackBuf->Message[Tail].ChanID)
        {
            g_pCallbackBuf->Message[Tail].IsValid = 0;
        }

        Tail = (Tail + 1) % MAX_EVENT_NUM;
    }

    return VDEC_OK;
}

SINT32 TVP_VDEC_Suspend(VOID)
{
    SINT32 ret = VDEC_OK;
    TEEC_Result result;
    TEEC_Operation operation;

    Down_Interruptible_with_Option(&g_stSem_s);

    if (HI_TRUE == g_bSecEnvSetUp)
    {
        operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

        operation.started = 1;
        operation.params[0].value.a = 0;
        operation.params[0].value.b = TEEC_VALUE_UNDEF;

        result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_SUSPEND, &operation, NULL);

        if (result != TEEC_SUCCESS)
        {
            SecPrint(PRN_FATAL, "InvokeCommand VFMW_CMD_ID_VDEC_SUSPEND Failed!\n");
            Up_Interruptible_with_Option(&g_stSem_s);
        }

        g_THREAD_STATE = THREAD_SLEEP;
        ret = operation.params[0].value.a;
    }

    Up_Interruptible_with_Option(&g_stSem_s);

    return ret;
}

SINT32 TVP_VDEC_Resume(VOID)
{
    SINT32 ret = VDEC_OK;
    TEEC_Result result;
    TEEC_Operation operation;

    Down_Interruptible_with_Option(&g_stSem_s);

    if (HI_TRUE == g_bSecEnvSetUp)
    {
        operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

        operation.started = 1;
        operation.params[0].value.a = 0;
        operation.params[0].value.b = TEEC_VALUE_UNDEF;

        result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_RESUME, &operation, NULL);

        if (result != TEEC_SUCCESS)
        {
            SecPrint(PRN_FATAL, "InvokeCommand VFMW_CMD_ID_VDEC_RESUME Failed!\n");
            Up_Interruptible_with_Option(&g_stSem_s);
        }

        g_THREAD_STATE = THREAD_RUN;
        ret = operation.params[0].value.a;
    }

    Up_Interruptible_with_Option(&g_stSem_s);

    return ret;
}

VBOOL TVP_VDEC_IsLocalComand(VDEC_CID_E eCmdID)
{
    if (eCmdID == VDEC_CID_GET_CHAN_STATE
     || eCmdID == VDEC_CID_SET_STREAM_INTF)
    {
        return VDEC_TRUE;
    }

#ifdef VFMW_VPSS_BYPASS_EN
    if (eCmdID == VDEC_CID_SET_IMG_OUTPUT_INFO
     || eCmdID == VDEC_CID_REPORT_OCCUQIED_FRM)
    {
        return VDEC_TRUE;
    }
#endif

    return VDEC_FALSE;
}

SINT32 TVP_VDEC_LocalControl(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs, UINT32 ParamLength)
{
    SINT32 ret = VDEC_ERR;

    switch (eCmdID)
    {
    #ifdef VFMW_VPSS_BYPASS_EN
        case VDEC_CID_SET_IMG_OUTPUT_INFO:
            if ((NULL != pArgs) && (ChanID >= TRUSTED_CHAN_BEGIN) && (ChanID < MAX_CHAN_NUM))
            {
                g_ChanContext[ChanID].OutputImgEn = *(SINT32*)pArgs;
                ret = VDEC_OK;
            }
            break;

        case VDEC_CID_REPORT_OCCUQIED_FRM:
            if ((NULL != pArgs) && (ChanID >= TRUSTED_CHAN_BEGIN) && (ChanID < MAX_CHAN_NUM))
            {
                ret = TVP_VDEC_FindOccupiedFrame(ChanID, pArgs);
            }
            break;
    #endif
        case VDEC_CID_GET_CHAN_STATE:
            if ((NULL != pArgs) && (NULL != g_pSecVfmwMem) &&
                (ChanID >=TRUSTED_CHAN_BEGIN) && (ChanID < MAX_CHAN_NUM))
            {
                OSAL_FP_memcpy(pArgs, &(g_pSecVfmwMem->ChanState[ChanID]), sizeof(VDEC_CHAN_STATE_S));
                ret = VDEC_OK;
            }
            else
            {
                SecPrint(PRN_FATAL, "VDEC_CID_GET_CHAN_STATE Invalid Param\n");
            }
            break;

        case VDEC_CID_SET_STREAM_INTF:
            if (NULL != pArgs)
            {
                OSAL_FP_memcpy(&g_StreamIntf[ChanID], (STREAM_INTF_S *)pArgs, sizeof(STREAM_INTF_S));
                ret = VDEC_OK;
            }
            else
            {
                SecPrint(PRN_FATAL, "VDEC_CID_SET_STREAM_INTF Invalid Param\n");
            }
            break;

        default:
            SecPrint(PRN_ERROR, "%s: unsupport command %d, param length %d.\n", __func__, eCmdID, ParamLength);
            break;
    }

    return ret;
}

VOID TVP_VDEC_PrepareRemoteParam(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs)
{
    SINT64 *p64Array = NULL;
    SINT8 *pArgParam = NULL;

    if (eCmdID == VDEC_CID_CREATE_CHAN
     || eCmdID == VDEC_CID_GET_CHAN_MEMSIZE)
    {
        p64Array = (SINT64 *)pArgs;
        pArgParam = UINT64_PTR(p64Array[1]);
        OSAL_FP_memcpy(&g_pSecVfmwMem->ChanOption, pArgParam, sizeof(VDEC_CHAN_OPTION_S));
    }

    return;
}

VOID TVP_VDEC_ProcessRemoteResult(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs, SINT32 RemoteResult)
{
    SINT64 *p64Array = (SINT64 *)pArgs;
    SINT8 *pArgParam = NULL;
    VDEC_CHAN_RESET_OPTION_S *pResetOption = NULL;

    switch (eCmdID)
    {
        case VDEC_CID_CREATE_CHAN:
            if (RemoteResult == VDEC_OK)
            {
                SINT32 *p32Array = (SINT32 *)pArgs;
                SINT32 RetChanID = p32Array[0];

                g_ChanContext[RetChanID].ChanID = RetChanID;
                g_ChanContext[RetChanID].bIsSecMode = HI_TRUE;

                Sema_Init_with_Option(&g_ChanContext[RetChanID].ChanSema);
                g_SecureInstNum++;
            }

            pArgParam = UINT64_PTR(p64Array[1]);
            if (pArgParam != NULL)
            {
                OSAL_FP_memcpy(pArgParam, &g_pSecVfmwMem->ChanOption, sizeof(VDEC_CHAN_OPTION_S));
            }
            break;

        case VDEC_CID_GET_CHAN_MEMSIZE:
            pArgParam = UINT64_PTR(p64Array[1]);
            if (pArgParam != NULL)
            {
                OSAL_FP_memcpy(pArgParam, &g_pSecVfmwMem->ChanOption, sizeof(VDEC_CHAN_OPTION_S));
            }
            break;

        case VDEC_CID_START_CHAN:
            g_ChanContext[ChanID].ChanState = CHAN_START;
            g_ChanContext[ChanID].OutputImgEn = 1;
            g_THREAD_STATE = THREAD_RUN;
            break;

        case VDEC_CID_STOP_CHAN:
            g_ChanContext[ChanID].ChanState = CHAN_STOP;
            break;

        case VDEC_CID_DESTROY_CHAN:
            Down_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);
            TVP_VDEC_Release_Callback(ChanID);
            TVP_VDEC_Release_Stream(ChanID);
            OSAL_FP_memset(&g_pImageQue[ChanID], 0, sizeof(IMAGE_QUEUE_NS));

            g_SecureInstNum--;

            if (g_SecureInstNum <= 0)
            {
                g_SecureInstNum = 0;
                g_THREAD_STATE = THREAD_SLEEP;
            }

            if (g_ChanContext[ChanID].pRawFile != NULL)
            {
                OSAL_FP_fclose(g_ChanContext[ChanID].pRawFile);
                g_ChanContext[ChanID].pRawFile = NULL;
            }

            g_ChanContext[ChanID].ChanState = CHAN_INVALID;

            Up_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);
            break;

        case VDEC_CID_RESET_CHAN:
            Down_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);
            TVP_VDEC_Release_Callback(ChanID);
            TVP_VDEC_Release_Stream(ChanID);

            OSAL_FP_memset(&(g_pImageQue[ChanID].StOutputImg), 0, sizeof(IMAGE_LIST_S));
            Up_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);
            break;

        case VDEC_CID_RESET_CHAN_WITH_OPTION:
            pResetOption = (VDEC_CHAN_RESET_OPTION_S *)pArgs;

            if (pResetOption->s32KeepBS == 0)
            {
                Down_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);

                TVP_VDEC_Release_Stream(ChanID);
                TVP_VDEC_Release_Callback(ChanID);
                OSAL_FP_memset(&(g_pImageQue[ChanID].StOutputImg), 0, sizeof(IMAGE_LIST_S));

                Up_Interruptible_with_Option(&g_ChanContext[ChanID].ChanSema);
            }
            break;

        default:
            break;
    }

    return;
}

SINT32 TVP_VDEC_RemoteControl(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs, UINT32 ParamLength)
{
    SINT32 ret = VDEC_ERR;
    UADDR  ArgPhyAddr;
    TEEC_Result TeeResult;
    TEEC_Operation TeeOperation;

    static UINT32 BeginTime = 0;
    static UINT32 StartTime = 0;
    static UINT32 EndTime = 0;
    static UINT32 TotalTime = 0;
    static UINT32 Count = 0;

    TVP_VDEC_PrepareRemoteParam(ChanID, eCmdID, pArgs);

    ArgPhyAddr = (NULL == pArgs) ? 0 : (__pa(pArgs));

    TeeOperation.started = 1;
    TeeOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
    TeeOperation.params[0].value.a = ChanID;
    TeeOperation.params[0].value.b = eCmdID;
    TeeOperation.params[1].value.a = ArgPhyAddr;
    TeeOperation.params[1].value.b = ParamLength;

    if (g_bPrintTime)
    {
        StartTime = OSAL_GetTimeInUs();
        BeginTime = (Count == 0) ? StartTime : BeginTime;
    }

    TeeResult = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_CONTROL, &TeeOperation, NULL);
    if (TeeResult != TEEC_SUCCESS)
    {
        SecPrint(PRN_FATAL, "TEEC_InvokeCommand VFMW_CMD_ID_VDEC_CONTROL Failed!\n");
        return VDEC_ERR;
    }

    if (g_bPrintTime)
    {
        EndTime = OSAL_GetTimeInUs();
        TotalTime += EndTime - StartTime;
        Count++;

        if (EndTime - BeginTime >= 1000000)
        {
            SecPrint(PRN_TEE, "Control Total: %d, Count: %d, Avg: %d us\n", TotalTime, Count, TotalTime / Count);
            TotalTime = BeginTime = StartTime = EndTime = 0;
            Count = 0;
        }
    }

    ret = TeeOperation.params[0].value.a;

    TVP_VDEC_ProcessRemoteResult(ChanID, eCmdID, pArgs, ret);

    return ret;
}

SINT32 TVP_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs, UINT32 ParamLength)
{
    SINT32 ret;

    Down_Interruptible_with_Option(&g_stSem_s);

    if (HI_FALSE == g_bSecEnvSetUp)
    {
        SecPrint(PRN_FATAL, "%s: secure decoder not inited yet!\n", __func__);
        Up_Interruptible_with_Option(&g_stSem_s);
        return VDEC_ERR;
    }

    if (TVP_VDEC_IsLocalComand(eCmdID) == VDEC_TRUE)
    {
        ret = TVP_VDEC_LocalControl(ChanID, eCmdID, pArgs, ParamLength);
    }
    else
    {
        ret = TVP_VDEC_RemoteControl(ChanID, eCmdID, pArgs, ParamLength);
    }

    Up_Interruptible_with_Option(&g_stSem_s);

    return ret;
}

#ifndef  HI_ADVCA_FUNCTION_RELEASE
static inline SINT32 TVP_VDEC_String2Value(HI_PCHAR str, UINT32 *data)
{
    UINT32 i, d, dat, weight;

    dat = 0;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        i = 2;
        weight = 16;
    }
    else
    {
        i = 0;
        weight = 10;
    }

    for (; i < 10; i++)
    {
        if (str[i] < 0x20)
        {
            break;
        }
        else if (weight == 16 && str[i] >= 'a' && str[i] <= 'f')
        {
            d = str[i] - 'a' + 10;
        }
        else if (weight == 16 && str[i] >= 'A' && str[i] <= 'F')
        {
            d = str[i] - 'A' + 10;
        }
        else if (str[i] >= '0' && str[i] <= '9')
        {
            d = str[i] - '0';
        }
        else
        {
            return -1;
        }

        dat = dat * weight + d;
    }

    *data = dat;

    return 0;
}

VOID TVP_VDEC_HelpProc(VOID)
{
    SecPrint(PRN_ALWS, "\n");
    SecPrint(PRN_ALWS, "================= SEC_VFMW HELP =================\n");
    SecPrint(PRN_ALWS, "USAGE:echo [cmd] [para] >/proc/sec_vfmw\n");
    SecPrint(PRN_ALWS, "  help,     [not_care]    :read help infomation\n");
    SecPrint(PRN_ALWS, "  saveraw,  [start/stop]  :enable/disable raw save,debug only\n");
    SecPrint(PRN_ALWS, "  saveyuv,  [start/stop]  :enable/disable yuv save,debug only\n");
    SecPrint(PRN_ALWS, "  printtime               :enable/disable print time\n");
    SecPrint(PRN_ALWS, "\n");
    SecPrint(PRN_ALWS, "Further command avalible in trusted decoder:\n");
    SecPrint(PRN_ALWS, "  0x0,      [print_word]  :set print enable word\n");
    SecPrint(PRN_ALWS, "  0x2,      [0~100]       :set err thr\n");
    SecPrint(PRN_ALWS, "  0x4,      [0/1]         :set output order(0/1=DISP/DEC)\n");
    SecPrint(PRN_ALWS, "  0x5,      [0/1/2]       :set dec mode(0/1/2=IPB/IP/I)\n");
    SecPrint(PRN_ALWS, "  0x7,      [0~100]       :set discard before dec thr\n");
    SecPrint(PRN_ALWS, "  0xb,      [0/1]         :set frame/adaptive(0/1) storage\n");
    SecPrint(PRN_ALWS, "  0xd,      [8~15]        :set to abserve specify channel\n");
    SecPrint(PRN_ALWS, "  0x400,    [ms]          :set dec task schedule delay\n");
    SecPrint(PRN_ALWS, "  0x402,    [0/1]         :start/stop syntax proccess\n");
    SecPrint(PRN_ALWS, "  0x501,    [ms]          :set scd state period\n");
    SecPrint(PRN_ALWS, "  0x502,    [ms]          :set vdh state period\n");
    SecPrint(PRN_ALWS, "  0x503,    [ms]          :set rcv/rls frame period\n");
    SecPrint(PRN_ALWS, "=================================================\n");
    SecPrint(PRN_ALWS, "\n");

    return;
}

SINT32 TVP_VDEC_ParamProccess(const SINT8 __user *buffer, size_t count, UINT32 *p_option, UINT32 *p_value)
{
    SINT32  i, j;
    SINT32  ret = HI_FAILURE;
    HI_CHAR buf[PROC_CMD_LEN * 2];
    HI_CHAR str1[PROC_CMD_LEN];
    HI_CHAR str2[PROC_CMD_LEN];

    if (count < 1 || count >= sizeof(buf))
    {
        SecPrint(PRN_ALWS, "Parameter Invalid!\n");

        return HI_FAILURE;
    }

    OSAL_FP_memset(buf, 0, sizeof(buf));

    if (copy_from_user(buf, buffer, count))
    {
        SecPrint(PRN_ALWS, "Copy from user Failed!\n");

        return HI_FAILURE;
    }

    buf[count] = 0;

    /* 1.��ȡ����1 */
    i = 0;
    j = 0;

    for (; i < count; i++)
    {
        if (j == 0 && buf[i] == ' ')
        {
            continue;
        }

        if (buf[i] > ' ')
        {
            str1[j++] = buf[i];
        }

        if (j > 0 && buf[i] == ' ')
        {
            break;
        }
    }

    str1[j] = 0;

    /* 2.��ȡ����2 */
    j = 0;

    for (; i < count; i++)
    {
        if (j == 0 && buf[i] == ' ')
        {
            continue;
        }

        if (buf[i] > ' ')
        {
            str2[j++] = buf[i];
        }

        if (j > 0 && buf[i] == ' ')
        {
            break;
        }
    }

    str2[j] = 0;

    /* 3.�����ж� */
    if (!OSAL_FP_strncmp(str1, PROC_CMD_HELP, PROC_CMD_LEN))
    {
        TVP_VDEC_HelpProc();
    }

#if (1 == DEBUG_SAVE_SUPPORT)
    else if (!OSAL_FP_strncmp(str1, PROC_CMD_SAVERAW, PROC_CMD_LEN))
    {
        if (!OSAL_FP_strncmp(str2, PROC_CMD_START, PROC_CMD_LEN))
        {
            SecPrint(PRN_ALWS, "Enable Raw Stream Save.\n");
            g_RawSaveEnable = HI_TRUE;
        }
        else if (!OSAL_FP_strncmp(str2, PROC_CMD_STOP, PROC_CMD_LEN))
        {
            SecPrint(PRN_ALWS, "Disable Raw Stream Save.\n");
            g_RawSaveEnable = HI_FALSE;
        }
    }

#endif
    else if (!OSAL_FP_strncmp(str1, PROC_CMD_SUSPEND, PROC_CMD_LEN))
    {
        TVP_VDEC_Suspend();
    }
    else if (!OSAL_FP_strncmp(str1, PROC_CMD_PRINTTIME, PROC_CMD_LEN))
    {
        g_bPrintTime = !g_bPrintTime;

        SecPrint(PRN_ALWS, "enable print time:%d\n", g_bPrintTime);
    }
    else if (!OSAL_FP_strncmp(str1, PROC_CMD_RESUME, PROC_CMD_LEN))
    {
        TVP_VDEC_Resume();
    }
    else
    {
        /*ת������1*/
        if (TVP_VDEC_String2Value(str1, p_option) != 0)
        {
            SecPrint(PRN_ALWS, "Unknown param %s\n", str1);
        }
        /*ת������2*/
        else if (TVP_VDEC_String2Value(str2, p_value) != 0)
        {
            SecPrint(PRN_ALWS, "Unknown param %s\n", str2);
        }
        /*������������Ч*/
        else
        {
            switch (*p_option)
            {
                case PROC_CMD_SETPRINT:
                    g_SecPrintEnable = *p_value;
                    SecPrint(PRN_ALWS, "Set SecPrintEnable = 0x%x\n", g_SecPrintEnable);
                    break;

                case PROC_CMD_SELECT:
                    g_SecProcSelect = *p_value;
                    SecPrint(PRN_ALWS, "Set g_SecProcSelect = 0x%x\n", g_SecProcSelect);
                    break;

                default: /*Ĭ�ϲ�������ͨ��TEE����*/
                    if (HI_FALSE == g_bSecEnvSetUp)
                    {
                        SecPrint(PRN_ALWS, "Trusted decoder NOT create yet!\n");
                    }

                    break;
            }

            /*��ȫ��������Чʱ�ŷ���HI_SUCCESS*/
            if (HI_TRUE == g_bSecEnvSetUp)
            {
                ret = HI_SUCCESS;
            }
        }
    }

    return ret;
}

SINT32 TVP_VDEC_SendProcCommand(UINT32 option, UINT32 value)
{
    TEEC_Result result;
    TEEC_Operation operation;

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].value.a = option;
    operation.params[0].value.b = value;

    result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_WRITE_PROC, &operation, NULL);

    if (result != TEEC_SUCCESS)
    {
        SecPrint(PRN_ALWS, "InvokeCommand VFMW_CMD_ID_VFMW_WRITEPROC Failed!\n");
        return VDEC_ERR;
    }

    return operation.params[0].value.a;
}

static SINT32 TVP_VDEC_ReadProc(struct seq_file *p, VOID *v)
{
    UINT32 i;
    TEEC_Result result;
    TEEC_Operation operation;
    SINT8 *proc_buf = (SINT8 *)(g_pProcBuf);

    Down_Interruptible_with_Option(&g_stSem_s);

    //��û����������֮ǰӦ�������ѯ������Ϣ
    PROC_PRINT(p, "\n");
    PROC_PRINT(p, "------------------------- SECURE INFO -------------------------\n");
    PROC_PRINT(p, "SecEnvSetUp      :%-7d  | SecPrintEnable      :0x%-5x\n", g_bSecEnvSetUp,  g_SecPrintEnable);
#ifdef HI_SMMU_SUPPORT
    PROC_PRINT(p, "ThreadState      :%-7d  | VfmwMemPhyAddr      :0x%-5x\n", g_THREAD_STATE,  g_SecVfmwMem.MmuAddr);
#else
    PROC_PRINT(p, "ThreadState      :%-7d  | VfmwMemPhyAddr      :0x%-5x\n", g_THREAD_STATE,  g_SecVfmwMem.PhyAddr);
#endif
    PROC_PRINT(p, "SecureInstNum    :%-7d  | VfmwMemLength       :%d\n",     g_SecureInstNum, g_SecVfmwMem.Length);
#if (1 == DEBUG_SAVE_SUPPORT)
    PROC_PRINT(p, "RawSaveEnable    :%-7d  | SavePath            :%s\n",     g_RawSaveEnable, g_SavePath);
#endif
    PROC_PRINT(p, "PrintTime        :%-7d\n", g_bPrintTime);
    PROC_PRINT(p, "\n");

    for (i = 0; i < MAX_CHAN_NUM; i++)
    {
        if (g_ChanContext[i].ChanState != CHAN_INVALID)
        {
            PROC_PRINT(p, "Chan %d\n", i);
            PROC_PRINT(p, "State            :%-7d  | SecMode             :%d\n", g_ChanContext[i].ChanState, g_ChanContext[i].bIsSecMode);

            if (HI_TRUE == g_ChanContext[i].bIsSecMode)
            {
            PROC_PRINT(p, "StreamList       : %d/%d (%d,%d,%d,%d)\n",
                           (g_pStreamBuf[i].Head_S - g_pStreamBuf[i].Tail_S + MAX_RAW_NUM) % MAX_RAW_NUM,
                           (g_pStreamBuf[i].Head_NS - g_pStreamBuf[i].Tail_NS + MAX_RAW_NUM) % MAX_RAW_NUM,
                           g_pStreamBuf[i].Head_S, g_pStreamBuf[i].Tail_S, g_pStreamBuf[i].Head_NS, g_pStreamBuf[i].Tail_NS);

            PROC_PRINT(p, "Output Image list: %d/%d/%d(History Head Tail)\n", g_pImageQue[i].StOutputImg.History, g_pImageQue[i].StOutputImg.Head, g_pImageQue[i].StOutputImg.Tail);
            PROC_PRINT(p, "Release Image list: %d/%d/%d(History Head Tail)\n", g_pImageQue[i].StReleaseImg.History, g_pImageQue[i].StReleaseImg.Head, g_pImageQue[i].StReleaseImg.Tail);
            }
        }
    }

    PROC_PRINT(p, "\n");
    if (g_bSecEnvSetUp && g_SecureInstNum > 0)
    {
        operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
        operation.started = 1;
        operation.params[0].value.a = 0;
        operation.params[0].value.b = MAX_PROC_SIZE;

        result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_READ_PROC, &operation, NULL);

        if (result != TEEC_SUCCESS)
        {
            SecPrint(PRN_ALWS, "InvokeCommand VFMW_CMD_ID_VFMW_READPROC Failed!\n");
            Up_Interruptible_with_Option(&g_stSem_s);
            return VDEC_ERR;
        }

        PROC_PRINT(p, proc_buf);
    }
    else
    {
        PROC_PRINT(p, " No secure channel exit.\n");
    }

    PROC_PRINT(p, "\n");

    Up_Interruptible_with_Option(&g_stSem_s);

    return 0;
}


SINT32 TVP_VDEC_WriteProc(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    UINT32 option, value;
    SINT32 ret;

    ret = TVP_VDEC_ParamProccess(buffer, count, &option, &value);

    Down_Interruptible_with_Option(&g_stSem_s);

    if (HI_SUCCESS != ret)
    {
        Up_Interruptible_with_Option(&g_stSem_s);
        return count;
    }

    if (HI_TRUE == g_bSecEnvSetUp)
    {
        ret = TVP_VDEC_SendProcCommand(option, value);

        if (HI_SUCCESS != ret)
        {
            SecPrint(PRN_ALWS, "Invalid CMD(%d %d), refer to help.\n", option, value);
            TVP_VDEC_HelpProc();
        }
    }

    Up_Interruptible_with_Option(&g_stSem_s);

    return count;
}
#endif

VOID TVP_VDEC_ContextInit(VOID)
{
    /*ȫ��ָ���ʼ��*/
    g_pSecVfmwMem         = NULL;
    g_pCallbackBuf        = NULL;
    g_pStreamBuf          = NULL;

    g_pImageQue           = NULL;
#if (1 == DEBUG_SAVE_SUPPORT)
    g_pSaveStreamBuf      = NULL;
#endif
#ifndef  HI_ADVCA_FUNCTION_RELEASE
    g_pProcBuf            = NULL;
#endif

    /*��̬ȫ�ֱ�����ʼ��*/
    g_SecureInstNum       = 0;
    g_TrustDecoderInitCnt = 0;
    g_bSecEnvSetUp        = HI_FALSE;
    g_THREAD_STATE        = THREAD_INVALID;

    /*ȫ�ֽṹ���ʼ��*/
    OSAL_FP_memset(&g_SecVfmwMem, 0, sizeof(MEM_DESC_S));
    OSAL_FP_memset(g_StreamIntf,  0, sizeof(STREAM_INTF_S)*MAX_CHAN_NUM);
    OSAL_FP_memset(&g_CallBack, 0, sizeof(VFMW_CALLBACK_S));

    //��Ϊ����ṹ��ͷǰ�ͨ����أ���Ӧ�÷��������ʼ��
    //OSAL_FP_memset(g_ChanContext, 0, sizeof(CHAN_CONTEXT_S)*TOTAL_MAX_CHAN_NUM);
}

SINT32 TVP_VDEC_TrustDecoderInit(VDEC_OPERATION_S *pArgs)
{
    TEEC_Result result;
    TEEC_Operation operation;
    TEEC_Operation session_operation;

    u8 package_name[] = "hisi_vfmw_sec";
    u32 vfmw_sec_id = 0;

    TEEC_UUID svc_id = {0x3c2bfc84, 0xc03c, 0x11e6, {0xa4, 0xa6, 0xce, 0xc0, 0xc9, 0x32, 0xce, 0x01}};
    VDEC_OPERATION_S stInitArgs;
    SINT32 ret;
    UADDR phy_addr = 0;

    SecPrint(PRN_ERROR, "%s Enter\n", __func__);

    Down_Interruptible_with_Option(&g_stSem_s);

    if ((NULL == pArgs)||(NULL == pArgs->VdecCallback))
    {
        SecPrint(PRN_FATAL, "%s: Param Invalid!\n", __func__);
        Up_Interruptible_with_Option(&g_stSem_s);
        return VDEC_ERR;
    }

    if (HI_TRUE == g_bSecEnvSetUp)
    {
        SecPrint(PRN_ERROR, "TrustedDecoder already init(%d), return OK.\n", g_TrustDecoderInitCnt);
        g_TrustDecoderInitCnt++;

        ret = TVP_VDEC_SetCallBack(pArgs->eAdapterType, pArgs->VdecCallback);
        if (VDEC_OK != ret)
        {
            SecPrint(PRN_ERROR, "Set CallBack Failed!\n");
        }

        Up_Interruptible_with_Option(&g_stSem_s);
        return ret;
    }

    /* 1. ����ͨ�Ż��� */
    result = TEEK_InitializeContext(NULL, &g_TeeContext);
    if (result != TEEC_SUCCESS)
    {
        SecPrint(PRN_FATAL, "TEEC_InitializeContext Failed!\n");
        Up_Interruptible_with_Option(&g_stSem_s);
        return VDEC_ERR;
    }

    OSAL_FP_memset(&session_operation, 0, sizeof(TEEC_Operation));

    session_operation.started = 1;
    session_operation.cancel_flag = 0;
    session_operation.paramTypes = TEEC_PARAM_TYPES(
            TEEC_NONE,
            TEEC_NONE,
            TEEC_MEMREF_TEMP_INPUT,
            TEEC_MEMREF_TEMP_INPUT);
    session_operation.params[2].tmpref.buffer = (void *)(&vfmw_sec_id);
    session_operation.params[2].tmpref.size   = sizeof(vfmw_sec_id);
    session_operation.params[3].tmpref.buffer = (void *)(package_name);
    session_operation.params[3].tmpref.size   = strlen(package_name) + 1;

    /* 2. �����Ự*/
    result = TEEK_OpenSession(&g_TeeContext, &g_TeeSession, &svc_id, TEEC_LOGIN_IDENTIFY, NULL, &session_operation, NULL);

    if (result != TEEC_SUCCESS)
    {
        SecPrint(PRN_FATAL, "TEEC_OpenSession Failed!\n");

        goto InitWithfree_0;
    }

    /* 3. ��ʼ��ȫ����Ϣ*/
    TVP_VDEC_ContextInit();

    /* 4. �����ڴ棬ȫ����Ϣ��ֵ*/
    if (HI_SUCCESS != KernelMemMalloc("VFMW_Share_Buf", sizeof(MEMORY_NEEDED_SECVFMW_S), 4, 0, &g_SecVfmwMem))
    {
        SecPrint(PRN_FATAL, "Alloc Sec Vfmw Buffer Failed!\n");
        goto InitWithfree_1;
    }
#ifdef HI_SMMU_SUPPORT
    g_SecVfmwMem.PhyAddr = g_SecVfmwMem.MmuAddr;
#endif
    g_pSecVfmwMem = (MEMORY_NEEDED_SECVFMW_S *)(ULONG)(g_SecVfmwMem.VirAddr);
    OSAL_FP_memset(g_pSecVfmwMem, 0, sizeof(MEMORY_NEEDED_SECVFMW_S));
    g_pCallbackBuf   = (CALLBACK_ARRAY_NS *)(&g_pSecVfmwMem->CallBackBuf);
    g_pStreamBuf     = (RAW_ARRAY_NS *)(g_pSecVfmwMem->StreamBuf);
    g_pImageQue      = (IMAGE_QUEUE_NS *)(g_pSecVfmwMem->ImageQue);
#ifndef  HI_ADVCA_FUNCTION_RELEASE
    g_pProcBuf       = (UINT8 *)(g_pSecVfmwMem->ProcBuf);
#endif
#if (1 == DEBUG_SAVE_SUPPORT)
    g_pSaveStreamBuf = (UINT8 *)(g_pSecVfmwMem->SaveStreamBuf);
#endif

    OSAL_FP_memcpy(&stInitArgs, pArgs, sizeof(VDEC_OPERATION_S));
    OSAL_FP_memset(&stInitArgs.stExtHalMem, 0, sizeof(MEM_DESC_S));
    stInitArgs.ShareData.share_mem_addr = g_SecVfmwMem.PhyAddr;

#ifdef VDH_DISTRIBUTOR_ENABLE
    if (g_HalDisable != 1)
    {
        ret = VDH_Get_ShareData(&stInitArgs.ShareData);
        if (ret != VDH_OK)
        {
            SecPrint(PRN_FATAL, "%s VDH_Get_ContextData failed\n", __func__);
        }
    }
    /* ͬ��Ӳ������������*/
    stInitArgs.ShareData.hal_disable = g_HalDisable;
#endif
    /* 4. ͬ����ȫ��ӡ������*/
    stInitArgs.ShareData.sec_print_word = g_SecPrintEnable;

    phy_addr = __pa((&stInitArgs));

    /* 5. ���ð�ȫ���ʼ���ӿ�*/
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].value.a = phy_addr;
    operation.params[0].value.b = TEEC_VALUE_UNDEF;

    result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_INIT, &operation, NULL);

    if (result != TEEC_SUCCESS || operation.params[0].value.a != VDEC_OK)
    {
        SecPrint(PRN_FATAL, "InvokeCommand VFMW_CMD_ID_VDEC_INIT failed, result_val=%d\n", operation.params[0].value.a);
        goto InitWithfree_2;
    }

    ret = TVP_VDEC_SetCallBack(pArgs->eAdapterType, pArgs->VdecCallback);
    if (VDEC_OK != ret)
    {
        SecPrint(PRN_ERROR, "Set CallBack Failed!\n");
        goto InitWithfree_2;
    }

    /* 6. ������ȫ�����߳�*/
    ret = TVP_VDEC_Thread_Init();

    if (ret != VDEC_OK)
    {
        SecPrint(PRN_FATAL, "Call VDEC_Thread_Init() Failed!\n");
        goto InitWithfree_3;
    }

    /* 7. ע�ᰲȫ�ഥ�����ж�*/
    ret = vfmw_Osal_Func_Ptr_S.pfun_Osal_request_irq(SECURE_NOTIFY_IRQ_NUM, TVP_VDEC_SecNotify_Isr, 0x00000080, "SecInvokeirq", &g_SecInvokeirqHandle);

    if (ret != VDEC_OK)
    {
        SecPrint(PRN_FATAL, "Call VDEC_Thread_Init() Failed!\n");
        goto InitWithfree_4;
    }
    g_TrustDecoderInitCnt = 1;
    g_bSecEnvSetUp = HI_TRUE;

    SecPrint(PRN_ERROR, "TrustedDecoder init Success.\n");

    Up_Interruptible_with_Option(&g_stSem_s);

    return VDEC_OK;
InitWithfree_4:
    vfmw_Osal_Func_Ptr_S.pfun_Osal_free_irq(SECURE_NOTIFY_IRQ_NUM, "SecInvokeirq", &g_SecInvokeirqHandle);

InitWithfree_3:
    //���ð�ȫ��ȥ��ʼ������
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].value.a = 0;
    operation.params[0].value.b = TEEC_VALUE_UNDEF;
    TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_EXIT, &operation, NULL);

InitWithfree_2:
    KernelMemFree(&g_SecVfmwMem);
    TVP_VDEC_ContextInit();

InitWithfree_1:
    TEEK_CloseSession(&g_TeeSession);

InitWithfree_0:
    //��Ϊclose����finalize�ᵼ�¹�����������δ������ȹ�ܣ�ֻ��close���ɣ����ڿ��Դ�������
    TEEK_FinalizeContext(&g_TeeContext);

    Up_Interruptible_with_Option(&g_stSem_s);
    return VDEC_ERR;

}

SINT32 TVP_VDEC_TrustDecoderExit(VOID)
{
    UINT32 i = 0;
    TEEC_Result result;
    TEEC_Operation operation;
    SINT32 ret;

    SecPrint(PRN_ERROR, "%s Enter\n", __func__);

    Down_Interruptible_with_Option(&g_stSem_s);

    if (HI_FALSE == g_bSecEnvSetUp)
    {
        SecPrint(PRN_ERROR, "TrustedDecoder not init, return OK.\n");
        Up_Interruptible_with_Option(&g_stSem_s);
        return VDEC_OK;
    }

    if (g_SecureInstNum > 0)
    {
        SecPrint(PRN_ERROR, "SecureInstNum(%d) > 0, return OK.\n", g_SecureInstNum);
        Up_Interruptible_with_Option(&g_stSem_s);
        return VDEC_OK;
    }

    g_THREAD_STATE = THREAD_EXIT;

    for (i = 0; i < 50; i++)
    {
        if (THREAD_INVALID == g_THREAD_STATE)
        {
            break;
        }
        else
        {
            OSAL_MSLEEP(10);
        }
    }

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].value.a = 0;
    operation.params[0].value.b = TEEC_VALUE_UNDEF;

    result = TEEK_InvokeCommand(&g_TeeSession, VFMW_CMD_ID_EXIT, &operation, NULL);

    if (result != TEEC_SUCCESS)
    {
        SecPrint(PRN_FATAL, "InvokeCommand VFMW_CMD_ID_VDEC_EXIT Failed!\n");
    }

    ret = operation.params[0].value.a;
    vfmw_Osal_Func_Ptr_S.pfun_Osal_free_irq(SECURE_NOTIFY_IRQ_NUM, "SecInvokeirq", &g_SecInvokeirqHandle);

    /*�ر�VFMW��TEECͨ��*/
    TEEK_CloseSession(&g_TeeSession);
    // ��Ϊclose����finalize�ᵼ�¹�����������δ������ȹ�ܣ�ֻ��close���ɣ����ڿ��Դ�������
    TEEK_FinalizeContext(&g_TeeContext);

    KernelMemFree(&g_SecVfmwMem);
    g_TrustDecoderInitCnt = 0;
    g_bSecEnvSetUp = HI_FALSE;

    SecPrint(PRN_ERROR, "TrustedDecoder deinit Success.\n");

    Up_Interruptible_with_Option(&g_stSem_s);
    return ret;
}

VOID TVP_VDEC_InitProc(VOID)
{
    SINT32 ret;

    ret = OSAL_FP_proc_create(VFMW_SEC_PROC_NAME, TVP_VDEC_ReadProc, TVP_VDEC_WriteProc);
    if (0 != ret)
    {
        SecPrint(PRN_ERROR, "%s create proc %s failed!\n", __func__, VFMW_SEC_PROC_NAME);

        return;
    }

    return;
}

VOID TVP_VDEC_ExitProc(VOID)
{
    OSAL_FP_proc_destroy(VFMW_SEC_PROC_NAME);

    return;
}

VOID TVP_VDEC_OpenModule(VOID)
{
#ifndef  HI_ADVCA_FUNCTION_RELEASE
    TVP_VDEC_InitProc();
#endif

    Sema_Init_with_Option(&g_stSem_s);

    OSAL_FP_memset(g_ChanContext, 0, sizeof(CHAN_CONTEXT_S)*MAX_CHAN_NUM);
    return;

}

VOID TVP_VDEC_ExitModule(VOID)
{
#ifndef  HI_ADVCA_FUNCTION_RELEASE
    TVP_VDEC_ExitProc();
#endif

    return;
}

