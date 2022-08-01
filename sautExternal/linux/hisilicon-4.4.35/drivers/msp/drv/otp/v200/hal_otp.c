/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hal_otp.c
  Version       : Initial Draft
  Author        : Hisilicon hisecurity team
  Created       :
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include <linux/delay.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include "hi_drv_reg.h"
#include "hi_type.h"
#include "hal_otp.h"
#include "hi_drv_otp.h"

/******* proc function begin ********/
#if (1 == HI_PROC_SUPPORT)
extern HI_BOOL DRV_OTP_IsFakeOTPAccessErr_Flag(HI_VOID);
extern HI_BOOL DRV_OTP_IsFakeOTPBuffer_Flag(HI_VOID);
extern HI_S32 DRV_OTP_FakeOTP_ReadByte(HI_U32 addr);
extern HI_S32 DRV_OTP_FakeOTP_WriteByte(HI_U32 addr, HI_U8 value);

#define DRV_FAKE_OTP_ACCESSERR_CHECK() do{                          \
        if(DRV_OTP_IsFakeOTPAccessErr_Flag() == HI_TRUE)         \
        {                                                           \
            HI_FATAL_OTP("OTP Access Error Mode\n");                \
            return HI_FAILURE;                                      \
        }                                                           \
    }while(0)

#endif
/******* proc function end   ********/

static HI_VOID otp_write_reg(HI_U32 addr, HI_U32 val)
{
    HI_U8 *pu8VirAddr = HI_NULL;
    pu8VirAddr = (HI_U8 *)ioremap_nocache(addr, 0x10);
    if (HI_NULL == pu8VirAddr)
    {
        HI_ERR_OTP("ioremap_nocache map error\n");
        return ;
    }
    HI_REG_WRITE32(pu8VirAddr, val);
    HI_DBG_OTP("addr=0x%x, val=0x%x\n",addr, val);
    iounmap(pu8VirAddr);
    return;
}

static HI_U32 otp_read_reg(HI_U32 addr)
{
    HI_U32 val = 0;
    HI_U8 *pu8VirAddr = HI_NULL;

    pu8VirAddr = (HI_U8 *)ioremap_nocache(addr, 0x10);
    if (HI_NULL == pu8VirAddr)
    {
        HI_ERR_OTP("ioremap_nocache map error\n");
        return HI_FAILURE;
    }
    HI_REG_READ32(pu8VirAddr, val);
    HI_DBG_OTP("addr=0x%x, val=0x%x\n",addr, val);
    iounmap(pu8VirAddr);
    return val;
}

static HI_S32 s_check_error_status(HI_VOID)
{
    OTP_CTR_ST0_U st0;

    st0.u32 = 0;
    st0.u32 = otp_read_reg(OTP_CTR_ST0);

    if (0 == st0.bits.otp_init_rdy)
    {
        HI_ERR_OTP("OTP initialization not finished\n");
        return HI_FAILURE;
    }
    else if (1 == st0.bits.err)
    {
        HI_WARN_OTP("hostCPU read or the program permission check fail\n");
        return HI_SUCCESS;
    }
    else if (1 == st0.bits.prm_rd_fail)
    {
        HI_ERR_OTP("data read back check fail\n");
        return HI_FAILURE;
    }
    else if (1 == st0.bits.rd_fail)
    {
        HI_ERR_OTP("read operation abnormal\n");
        return HI_FAILURE;
    }
    else if (1 == st0.bits.prog_disable)
    {
        HI_ERR_OTP("OTP fuse otp_global_lock_en is 1\n");
        return HI_FAILURE;
    }
    else
    {
        return HI_SUCCESS;
    }
}

static void s_wait_ctrl_idle(HI_VOID)
{
    HI_U32 reg_data[1] = {0};

    reg_data[0] = otp_read_reg(OTP_RW_CTRL);
    while ((*reg_data & 0x1) == 0x1)
    {
        reg_data[0] = otp_read_reg(OTP_RW_CTRL);
    }

    return;
}

HI_S32 HAL_OTP_Reset(HI_VOID)
{
    HI_U32 reg_data[1] = {0};
    HI_U32 cnt = 0;

    otp_write_reg(OTP_SH_UPDATE, 0x1);

    reg_data[0] = otp_read_reg(OTP_CTR_ST0);
    while ((*reg_data & 0x2) == 0x0)        //2b'10
    {
        reg_data[0] = otp_read_reg(OTP_CTR_ST0);
        cnt ++;
        udelay(10*100);
        if(cnt >= 100)
        {
            HI_ERR_OTP("OTP init failed, time out.\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HAL_OTP_Init(HI_VOID)
{
    HI_U32 reg_data[1] = {0};
    HI_U32 cnt = 0;

    reg_data[0] = otp_read_reg(OTP_CTR_ST0);
    while ((*reg_data & 0x1) == 0x0)
    {
        reg_data[0] = otp_read_reg(OTP_CTR_ST0);
        cnt ++;
        udelay(10*100);
        if(cnt >= 100)
        {
            HI_ERR_OTP("OTP init failed, time out.\n");
            return HI_FAILURE;
        }
    }

    return HAL_OTP_Reset();
}

/******* proc function begin ********/
#if (1 == HI_PROC_SUPPORT)
HI_S32 HAL_OTP_ProcRead(HI_U32 addr, HI_U32 *value)
{
    HI_U32 read1, read2, ramdom;

    CHECK_NULL_PTR(value);
    otp_write_reg(OTP_RADDR, addr);
    otp_write_reg(OTP_RW_CTRL, 0x3);
    s_wait_ctrl_idle();

    ramdom = otp_read_reg(RNG_BASE + RNG_FIFO_DATA);
    usleep_range(((ramdom & 0xFF) / 2), (ramdom & 0xFF));
    read1 = otp_read_reg(OTP_RDATA);

    ramdom = otp_read_reg(RNG_BASE + RNG_FIFO_DATA);
    usleep_range(((ramdom & 0xFF) / 2), (ramdom & 0xFF));
    read2 = otp_read_reg(OTP_RDATA);

    if (read1 != read2)
    {
        *value = 0;
        HI_ERR_OTP("Read otp error\n");
        return HI_FAILURE;
    }

    *value = read1;
    HI_DBG_OTP("addr:%x, *value:%x\n", *value);

    return s_check_error_status();
}

HI_S32 HAL_OTP_ProcReadByte(HI_U32 addr, HI_U8 *value)
{
    HI_S32 ret = 0;
    HI_U32 v = 0;
    HI_U32 addr_e = 0;

    CHECK_NULL_PTR(value);
    addr_e = addr & (~0x3);
    ret = HAL_OTP_ProcRead(addr_e, &v);
    if (HI_SUCCESS != ret)
    {
        return HI_FAILURE;
    }
    *value = (v >> ((addr & 0x3) * 8)) & 0xff ;
    return HI_SUCCESS;
}
#endif
/******* proc function end   ********/

HI_S32 HAL_OTP_Read(HI_U32 addr, HI_U32 *value)
{
    HI_U32 read1, read2, ramdom;

    /******* proc function begin ********/
#if (1 == HI_PROC_SUPPORT)
    DRV_FAKE_OTP_ACCESSERR_CHECK();
    if (DRV_OTP_IsFakeOTPBuffer_Flag() == HI_TRUE)
    {
        HI_U32 val = 0;

        val = DRV_OTP_FakeOTP_ReadByte(addr + 0x00)
              + DRV_OTP_FakeOTP_ReadByte(addr + 0x01) * 0x100
              + DRV_OTP_FakeOTP_ReadByte(addr + 0x02) * 0x10000
              + DRV_OTP_FakeOTP_ReadByte(addr + 0x03) * 0x1000000;
        * value = val;
        HI_DBG_OTP("fake read word, addr:%x, u8Value:%x\n", addr, *value);
        return HI_SUCCESS;
    }
#endif
    /******* proc function end   ********/
    CHECK_NULL_PTR(value);
    otp_write_reg(OTP_RADDR, addr);
    otp_write_reg(OTP_RW_CTRL, 0x3);
    s_wait_ctrl_idle();

    ramdom = otp_read_reg(RNG_BASE + RNG_FIFO_DATA);
    usleep_range(((ramdom & 0xFF) / 2), (ramdom & 0xFF));
    read1 = otp_read_reg(OTP_RDATA);

    ramdom = otp_read_reg(RNG_BASE + RNG_FIFO_DATA);
    usleep_range(((ramdom & 0xFF) / 2), (ramdom & 0xFF));
    read2 = otp_read_reg(OTP_RDATA);

    if (read1 != read2)
    {
        *value = 0;
        HI_ERR_OTP("Read otp error\n");
        return HI_FAILURE;
    }

    *value = read1;
    HI_DBG_OTP("read word, addr:%x, *value:%x\n",addr, *value);

    return s_check_error_status();
}

HI_S32 HAL_OTP_Write(HI_U32 addr, HI_U32 value)
{
    HI_DBG_OTP("write word, u32Addr:%x, u32Value:%x\n", addr, value);
    if ((addr & 0x03) != 0)
    {
        HI_ERR_OTP("OTP address:%x is not 4 bytes aligned\n");
        return HI_FAILURE;
    }

    (HI_VOID)HAL_OTP_WriteByte(addr + 0x00, value & 0xFF);
    (HI_VOID)HAL_OTP_WriteByte(addr + 0x01, (value >> 8) & 0xFF);
    (HI_VOID)HAL_OTP_WriteByte(addr + 0x02, (value >> 16) & 0xFF);
    (HI_VOID)HAL_OTP_WriteByte(addr + 0x03, (value >> 24) & 0xFF);

    return HI_SUCCESS;
}

HI_S32 HAL_OTP_ReadByte(HI_U32 addr, HI_U8 *value)
{
    HI_S32 ret = 0;
    HI_U32 v = 0;
    HI_U32 addr_e = 0;

    CHECK_NULL_PTR(value);
    addr_e = addr & (~0x3);
    ret = HAL_OTP_Read(addr_e, &v);
    if (HI_SUCCESS != ret)
    {
        return HI_FAILURE;
    }
    *value = (v >> ((addr & 0x3) * 8)) & 0xff ;

    HI_DBG_OTP("read byte, addr:%x, *pu8Value:%x\n", addr, *value);

    return HI_SUCCESS;
}

HI_S32 HAL_OTP_WriteByte(HI_U32 addr, HI_U8 value)
{
    /******* proc function begin ********/
#if (1 == HI_PROC_SUPPORT)
    DRV_FAKE_OTP_ACCESSERR_CHECK();
    if (DRV_OTP_IsFakeOTPBuffer_Flag() == HI_TRUE)
    {
        HI_U8 val = 0;
        HI_DBG_OTP("fake write byte, addr:%x, u8Value:%x\n", addr, value);
        val = value & 0xFF;
        return DRV_OTP_FakeOTP_WriteByte(addr, val);
    }
#endif
    /******* proc function end   ********/
    HI_DBG_OTP("write byte, addr:%x, u8Value:%x\n", addr, value);
    otp_write_reg(OTP_WDATA, value);
    otp_write_reg(OTP_WADDR, addr);
    otp_write_reg(OTP_RW_CTRL, 0x5);
    s_wait_ctrl_idle();

    return s_check_error_status();
}


HI_S32 HAL_OTP_WriteBit(HI_U32 addr, HI_U32 bit_pos, HI_U32 bit_value)
{
    HI_U8 data = 0;
    HI_S32 ret = 0;

    HI_DBG_OTP("write bit, addr:%x, bit_pos:%x, bit_value:%x\n", addr, bit_pos, bit_value);
    if (bit_value == 1)
    {
        ret = HAL_OTP_ReadByte(addr, &data);
        if (HI_SUCCESS != ret)
        {
            return ret;
        }
        data |= (1 << bit_pos);

        return HAL_OTP_WriteByte(addr, data);
    }
    else
    {
        //Do nothing when bit_value is 0.
    }
    return HI_SUCCESS;
}


HI_S32 HAL_OTP_ReadBitsOnebyte(HI_U32 addr, HI_U32 start_bit, HI_U32 bit_width, HI_U8 *value)
{
    HI_S32 ret = 0;
    HI_U8 data = 0;

    if (start_bit + bit_width > 8)
    {
        return HI_FAILURE;
    }

    ret = HAL_OTP_ReadByte(addr, &data);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }
    data &= GENMASK_U(start_bit + bit_width - 1, start_bit);
    *value = data >> start_bit;
    HI_DBG_OTP("read bits, addr:%x, start_bit:%x, bit_width:%x, value:%x\n", addr, start_bit, bit_width, *value);

    return HI_SUCCESS;
}

HI_S32 HAL_OTP_WriteBitsOnebyte(HI_U32 addr, HI_U32 start_bit, HI_U32 bit_width, HI_U8 value)
{
    HI_S32 ret = 0;
    HI_U8 data = 0;

    HI_DBG_OTP("write bits, addr:%x, start_bit:%x, bit_width:%x, value:%x\n", addr, start_bit, bit_width, value);
    if (start_bit + bit_width > 8)
    {
        return HI_FAILURE;
    }
    ret = HAL_OTP_ReadByte(addr, &data);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }
    data = (data & GENMASKREV_U(start_bit + bit_width - 1, start_bit)) |
           ((value << start_bit) & GENMASK_U(start_bit + bit_width - 1, start_bit));

    return HAL_OTP_WriteByte(addr, data);
}

EXPORT_SYMBOL(HAL_OTP_Read);
EXPORT_SYMBOL(HAL_OTP_ReadByte);
EXPORT_SYMBOL(HAL_OTP_Write);
EXPORT_SYMBOL(HAL_OTP_WriteByte);
EXPORT_SYMBOL(HAL_OTP_WriteBit);
