// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */



/*
 * PD9302A voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "MOON_PD9302AVAF_DRV"
#define AF_I2C_SLAVE_ADDR 0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...)                                               \
	pr_info(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4CurrPosition = 0;
static unsigned long g_u4TargetPosition = 0;

static int s4MOON_PD9302AVAF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

    i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
}

static int s4MOON_PD9302AVAF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[3] = {0x03 , (char)(a_u2Data >> 8) , (char)(a_u2Data)};

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);

    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}


static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
	struct stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo,
			 sizeof(struct stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

/* initAF include driver initialization and standby mode */
static int initAF(void)
{
	char puSendCmd1[2]={(char)(0x02),(char)(0x01)};
	char puSendCmd2[2]={(char)(0x02),(char)(0x00)};
	char puSendCmd3[2]={(char)(0x02),(char)(0x02)};
	char puSendCmd4[2]={(char)(0x06),(char)(0x80)};
	char puSendCmd5[2]={(char)(0x07),(char)(0x03)};


	int i4RetValue = 0;
	int ret = 0;

	LOG_INF("+\n");

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd1, 2);
	if (i4RetValue < 0)
	{
		pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
		return -1;
	}
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2);
    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
        return -1;
    }

    mdelay(1);

    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd3, 2);
    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
        return -1;
    }

    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd4, 2);
    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
        return -1;
    }

    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd5, 2);
    if (i4RetValue < 0)
    {
        pr_info("[MOON_PD9302AVAF] I2C send failed!! \n");
        return -1;
    }

    if (*g_pAF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4MOON_PD9302AVAF_ReadReg(&InitPos);

        spin_lock(g_pAF_SpinLock);
        if(ret == 0)
        {
            pr_info("[MOON_PD9302AVAF] Init Pos %6d \n", InitPos);
            g_u4CurrPosition = (unsigned long)InitPos;
        }
        else
        {
            g_u4CurrPosition = 0;
        }
        *g_pAF_Opened = 2;
        spin_unlock(g_pAF_SpinLock);
    }


	LOG_INF("-\n");

	return 0;
}

/* moveAF only use to control moving the motor */
static inline int moveAF(unsigned long a_u4Position)
{
    if((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF))
    {
        pr_info("[MOON_PD9302AVAF] out of range \n");
        return -EINVAL;
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(g_pAF_SpinLock);
    //    g_i4Dir = 1;
        spin_unlock(g_pAF_SpinLock);
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(g_pAF_SpinLock);
    //    g_i4Dir = -1;
        spin_unlock(g_pAF_SpinLock);
    }
    else
    {
		return 0;
    }

    spin_lock(g_pAF_SpinLock);
    g_u4TargetPosition = a_u4Position;
    spin_unlock(g_pAF_SpinLock);

    pr_info("[MOON_PD9302AVAF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

    spin_lock(g_pAF_SpinLock);
    spin_unlock(g_pAF_SpinLock);

    if(s4MOON_PD9302AVAF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
    {
        spin_lock(g_pAF_SpinLock);
        g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
        spin_unlock(g_pAF_SpinLock);
    }
    else
    {
        pr_info("[MOON_PD9302AVAF] set I2C failed when moving the motor \n");
    }

    return 0;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

/* ////////////////////////////////////////////////////////////// */
long MOON_PD9302AVAF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
		    unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue =
			getAFInfo((__user struct stAF_MotorInfo *)(a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		LOG_INF("moveAF %d\n", a_u4Param);
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int MOON_PD9302AVAF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");
		pr_info("[MOON_PD9302AVAF] feee \n");

		s4MOON_PD9302AVAF_WriteReg(0x0138);
		mdelay(15);
		s4MOON_PD9302AVAF_WriteReg(0x019c);
		mdelay(15);
		s4MOON_PD9302AVAF_WriteReg(0x0200);
		mdelay(15);
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

int MOON_PD9302AVAF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			  spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	initAF();

	return 1;
}

int MOON_PD9302AVAF_GetFileName(unsigned char *pFileName)
{
	#if SUPPORT_GETTING_LENS_FOLDER_NAME
	char FilePath[256];
	char *FileString;

	sprintf(FilePath, "%s", __FILE__);
	FileString = strrchr(FilePath, '/');
	*FileString = '\0';
	FileString = (strrchr(FilePath, '/') + 1);
	strncpy(pFileName, FileString, AF_MOTOR_NAME);
	LOG_INF("FileName : %s\n", pFileName);
	#else
	pFileName[0] = '\0';
	#endif
	return 1;
}
