/*
 * Copyright (C) 2010 SHARP CORPORATION All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * SHARP COMPASS DRIVER(ALPS_HSCD)
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <sharp/compass.h>
#include <linux/earlysuspend.h>


#define MAKE_UINT16(a, b)		((unsigned short)( ((((unsigned short)a) <<   8)&     0xFF00) | \
												   (( (unsigned short)b)        &     0x00FF) ) )


#define COMPASS_DEBUG_MSG	0
#define COMPASS_DEBUG_FUNC	0


#define MAX_FAILURE_COUNT	3
#define COMPASS_DEFAULT_DELAY	200


#define CONVERT_A_X			(1)
#define CONVERT_A_Y			(1)
#define CONVERT_A_Z			(1)

#define CONVERT_M_X			(1)
#define CONVERT_M_Y			(1)
#define CONVERT_M_Z			(1)

#define CONVERT_O_X			(1)
#define CONVERT_O_Y			(1)
#define CONVERT_O_Z			(1)



#if COMPASS_DEBUG_MSG
#define COMPSDBG(format, ...)	printk(KERN_INFO "[COMPASS] " format "\n", ## __VA_ARGS__)
#else
#define COMPSDBG(format, ...)
#endif

#if COMPASS_DEBUG_FUNC
#define COMPSFUNC(func) printk(KERN_INFO "[COMPASS] " func " is called\n")
#else
#define COMPSFUNC(func)
#endif

static struct i2c_client *this_client;

struct compass_data {
	struct input_dev *input_dev;
	struct work_struct work;
};

/* Addresses to scan -- protected by sense_data_mutex */
static short sense_data[3];
static struct mutex sense_data_mutex;
static DECLARE_WAIT_QUEUE_HEAD(data_ready_wq);
static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static atomic_t data_ready = ATOMIC_INIT(0);
static atomic_t open_count = ATOMIC_INIT(0);
static atomic_t open_flag = ATOMIC_INIT(0);
static atomic_t reserve_open_flag = ATOMIC_INIT(0);

static atomic_t m_flag = ATOMIC_INIT(0);
static atomic_t a_flag = ATOMIC_INIT(0);
static atomic_t mv_flag = ATOMIC_INIT(0);


static int failure_count = 0;
static short compass_delay = COMPASS_DEFAULT_DELAY;
static struct sh_i2c_compass_platform_data *pdata;
static int16_t offset_data[3] = {0,0,0};


static int I2C_RxData(char *rxData, int length)
{
	int nResult;
	unsigned char buf[10];
	uint8_t loop_i;

    buf[0] = rxData[0];

	for (loop_i = 0; loop_i < MAX_FAILURE_COUNT; loop_i++)
	{
		nResult = i2c_master_send(this_client, buf, 1);
		if(nResult == 1)
		{
			nResult = i2c_master_recv(this_client, rxData, length);
			if(nResult != length)
			{
			    printk(KERN_ERR "I2cRead: receive error\n");
			    continue;
			}
			return 0;
		}
		else
		{
			printk(KERN_ERR "I2cRead: send error\n");
		}
		mdelay(10);
	}

	return -EIO;
}


static int I2C_TxData(char *txData, int length)
{
	uint8_t loop_i;
	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr,
			.flags = 0,
			.len = length,
			.buf = txData,
		},
	};

	for (loop_i = 0; loop_i < MAX_FAILURE_COUNT; loop_i++) {
		if (i2c_transfer(this_client->adapter, msg, 1) > 0) {
			break;
		}
		mdelay(10);
	}
	
	if (loop_i >= MAX_FAILURE_COUNT) {
		printk(KERN_ERR "%s retry over %d\n", __func__, MAX_FAILURE_COUNT);
		return -EIO;
	}

	return 0;
}


static int COMPS_CheckDevice(void)
{
	char buffer[2];
	int ret;

	printk(KERN_ERR "COMPS_CheckDevice\n");

	/* Set measure mode */
	buffer[0] = ALPS_REG_WIA;

	/* Read data */
	ret = I2C_RxData(buffer, 1);
	if (ret < 0) {
		return ret;
	}

	printk(KERN_ERR "COMPS_CheckDevice: 0x%X\n",buffer[0]);

	/* Check read data */
	if (buffer[0] != 0x49) {
		return -ENXIO;
	}

	return 0;
}


static int COMPS_SetMode_ForceState(void)
{
	char buffer[2];

	atomic_set(&data_ready, 0);

	enable_irq(this_client->irq);

	/* Set DRDY INT */
	buffer[0] = ALPS_REG_CNTL2;
	buffer[1] = 0x08;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	/* Set ForceState */
	buffer[0] = ALPS_REG_CNTL1;
	buffer[1] = 0x82;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	/* Set measure mode */
	buffer[0] = ALPS_REG_CNTL3;
	buffer[1] = 0x40;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	return 0;
}


static int COMPS_SetMode_NormalState(void)
{
	char buffer[2];

	atomic_set(&data_ready, 0);

	enable_irq(this_client->irq);

	/* Set DRDY INT */
	buffer[0] = ALPS_REG_CNTL2;
	buffer[1] = 0x08;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	/* Set NormalState */
	buffer[0] = ALPS_REG_CNTL1;
	buffer[1] = 0x80;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}


	return 0;
}


static int COMPS_SetMode_SelfTest(void)
{
	char buffer[2];
	int ret;

	/* Set measure mode */
	buffer[0] = ALPS_REG_STB;

	/* Read data */
	ret = I2C_RxData(buffer, 1);
	if (ret < 0) {
		return ret;
	}

	printk(KERN_ERR "SelfTest Step1:0x%X\n",buffer[0]);

	if(buffer[0] != 0x55)
	{
		return -EIO;
	}

	/* Set DRDY INT */
	buffer[0] = ALPS_REG_CNTL3;
	buffer[1] = 0x10;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	msleep(5);

	/* Set measure mode */
	buffer[0] = ALPS_REG_STB;

	/* Read data */
	ret = I2C_RxData(buffer, 1);
	if (ret < 0) {
		return ret;
	}

	printk(KERN_ERR "SelfTest Step2:0x%X\n",buffer[0]);

	if(buffer[0] != 0xAA)
	{
		return -EIO;
	}

	return 0;
}


static int COMPS_SetMode_PowerDown(void)
{
	char buffer[2];

	/* Set ForceState */
	buffer[0] = ALPS_REG_CNTL1;
	buffer[1] = 0x02;

	/* Set data */
	if(I2C_TxData(buffer, 2) < 0)
	{
		return -EIO;
	}

	return 0;
}


static int COMPS_SetMode(char mode)
{
	int ret;
	
	switch (mode) {
		case COMPS_MODE_FORCE:
			ret = COMPS_SetMode_ForceState();
			break;
		case COMPS_MODE_NORMAL:
			ret = COMPS_SetMode_NormalState();
			break;
		case COMPS_MODE_SELFTEST:
			ret = COMPS_SetMode_SelfTest();
			break;
		case COMPS_MODE_POWERDOWN:
			ret = COMPS_SetMode_PowerDown();
			/* wait at least 100us after changing mode */
			udelay(100);
			break;
		default:
			COMPSDBG("%s: Unknown mode(%d)", __func__, mode);
			return -EINVAL;
	}

	return ret;
}


static int COMPS_GetData(short *rbuf)
{
	wait_event_interruptible_timeout(data_ready_wq,
									 atomic_read(&data_ready), msecs_to_jiffies(1000));
	if (!atomic_read(&data_ready)) {
		COMPSDBG("%s: data_ready is not set.", __func__);
		failure_count++;
		if (failure_count >= MAX_FAILURE_COUNT) {
			printk(KERN_ERR
			       "COMPS_GetData: successive %d failure.\n",
			       failure_count);
			atomic_set(&open_flag, -1);
			wake_up(&open_wq);
			failure_count = 0;
		}
		return -1;
	}
	
	mutex_lock(&sense_data_mutex);
	memcpy(rbuf, sense_data, sizeof(sense_data));
	atomic_set(&data_ready, 0);
	mutex_unlock(&sense_data_mutex);
	
	failure_count = 0;
	return 0;
}


static int COMPS_FactoryShipmentTest(void)
{
	int ret = 0;

	ret = COMPS_SetMode_SelfTest();

    return ret;
}

static int COMPS_GetOpenStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) != 0));
	return atomic_read(&open_flag);
}

static int COMPS_GetCloseStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) <= 0));
	return atomic_read(&open_flag);
}

static void COMPS_SetYPR(short *rbuf)
{
	struct compass_data *data = i2c_get_clientdata(this_client);
	short tmpX, tmpY, tmpZ;

	/* Report magnetic sensor information */
	if (atomic_read(&m_flag)) {
		tmpX = (rbuf[0] * CONVERT_O_X);
		tmpY = (rbuf[1] * CONVERT_O_Y);
		tmpZ = (rbuf[2] * CONVERT_O_Z);

		if( (data->input_dev->abs[ABS_RX] != tmpX) ||
			(data->input_dev->abs[ABS_RY] != tmpY) ||
			(data->input_dev->abs[ABS_RZ] != tmpZ) )
		{
			/*  Magnetic Accuracy Init */
			data->input_dev->abs[ABS_RUDDER] = 99;
		}
		input_report_abs(data->input_dev, ABS_RX, tmpX );
		input_report_abs(data->input_dev, ABS_RY, tmpY );
		input_report_abs(data->input_dev, ABS_RZ, tmpZ );
		input_report_abs(data->input_dev, ABS_RUDDER, rbuf[3]);
	}
	
	/* Report acceleration sensor information */
	if (atomic_read(&a_flag)) {
		tmpX = (rbuf[5] * CONVERT_A_X);
		tmpY = (rbuf[6] * CONVERT_A_Y);
		tmpZ = (rbuf[7] * CONVERT_A_Z);

		if( (data->input_dev->abs[ABS_X] != tmpX) ||
			(data->input_dev->abs[ABS_Y] != tmpY) ||
			(data->input_dev->abs[ABS_Z] != tmpZ) )
		{
			/* Accel Accuracy Init */
			data->input_dev->abs[ABS_WHEEL] = 99;
		}
		input_report_abs(data->input_dev, ABS_X, tmpX );
		input_report_abs(data->input_dev, ABS_Y, tmpY );
		input_report_abs(data->input_dev, ABS_Z, tmpZ );
		input_report_abs(data->input_dev, ABS_WHEEL, rbuf[4]);
	}
	
	/* Report magnetic vector information */
	if (atomic_read(&mv_flag)) {
		input_report_abs(data->input_dev, ABS_HAT0X, (rbuf[8]  * CONVERT_M_X) );
		input_report_abs(data->input_dev, ABS_HAT0Y, (rbuf[9] * CONVERT_M_Y) );
		input_report_abs(data->input_dev, ABS_BRAKE, (rbuf[10] * CONVERT_M_Z) );
	}
	
	input_sync(data->input_dev);
}

/***** compass_aot functions ***************************************/
static int compass_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;

	COMPSFUNC("compass_aot_open");
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int compass_aot_release(struct inode *inode, struct file *file)
{
	COMPSFUNC("compass_aot_release");

	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
compass_aot_ioctl(struct inode *inode, struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;
	
	switch (cmd) {
		case ECS_IOCTL_APP_SET_MFLAG:
		case ECS_IOCTL_APP_SET_AFLAG:
		case ECS_IOCTL_APP_SET_MVFLAG:
			if (copy_from_user(&flag, argp, sizeof(flag))) {
				return -EFAULT;
			}
			if (flag < 0 || flag > 1) {
				return -EINVAL;
			}
			break;
		case ECS_IOCTL_APP_SET_DELAY:
			if (copy_from_user(&flag, argp, sizeof(flag))) {
				return -EFAULT;
			}
			break;
		default:
			break;
	}

	switch (cmd) {
		case ECS_IOCTL_APP_SET_MFLAG:
			atomic_set(&m_flag, flag);
			COMPSDBG("MFLAG is set to %d", flag);
			break;
		case ECS_IOCTL_APP_GET_MFLAG:
			flag = atomic_read(&m_flag);
			break;
		case ECS_IOCTL_APP_SET_AFLAG:
			atomic_set(&a_flag, flag);
			COMPSDBG("AFLAG is set to %d", flag);
			break;
		case ECS_IOCTL_APP_GET_AFLAG:
			flag = atomic_read(&a_flag);
			break;
		case ECS_IOCTL_APP_SET_MVFLAG:
			atomic_set(&mv_flag, flag);
			COMPSDBG("MVFLAG is set to %d", flag);
			break;
		case ECS_IOCTL_APP_GET_MVFLAG:
			flag = atomic_read(&mv_flag);
			break;
		case ECS_IOCTL_APP_SET_DELAY:
			compass_delay = flag;
			COMPSDBG("Delay is set to %d", flag);
			break;
		case ECS_IOCTL_APP_GET_DELAY:
			flag = compass_delay;
			break;
		default:
			return -ENOTTY;
	}
	
	switch (cmd) {
		case ECS_IOCTL_APP_GET_MFLAG:
		case ECS_IOCTL_APP_GET_AFLAG:
		case ECS_IOCTL_APP_GET_MVFLAG:
		case ECS_IOCTL_APP_GET_DELAY:
			if (copy_to_user(argp, &flag, sizeof(flag))) {
				return -EFAULT;
			}
			break;
		default:
			break;
	}
	
	return 0;
}

/***** functions ********************************************/
static int compass_open(struct inode *inode, struct file *file)
{
	COMPSFUNC("compass_open");
	return nonseekable_open(inode, file);
}

static int compass_release(struct inode *inode, struct file *file)
{
	COMPSFUNC("compass_release");
	return 0;
}

static int
compass_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		   unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	
	/* NOTE: In this function the size of "char" should be 1-byte. */
	short sData[3];/* for GETDATA */
	char rwbuf[RWBUF_SIZE];		/* for READ/WRITE */
	char mode;					/* for SET_MODE*/
	short value[12];			/* for SET_YPR */
	int ret = -1;				/* Return value. */
	int status;					/* for OPEN/CLOSE_STATUS */
	char AppFlag[3];
	short delay;				/* for GET_DELAY */

	/*COMPSDBG("%s (0x%08X).", __func__, cmd);*/
	
	switch (cmd) {
		case ECS_IOCTL_WRITE:
		case ECS_IOCTL_READ:
			if (argp == NULL) {
				COMPSDBG("invalid argument.");
				return -EINVAL;
			}
			if (copy_from_user(&rwbuf, argp, sizeof(rwbuf))) {
				COMPSDBG("copy_from_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_SET_MODE:
			if (argp == NULL) {
				COMPSDBG("invalid argument.");
				return -EINVAL;
			}
			if (copy_from_user(&mode, argp, sizeof(mode))) {
				COMPSDBG("copy_from_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_SET_YPR:
			if (argp == NULL) {
				COMPSDBG("invalid argument.");
				return -EINVAL;
			}
			if (copy_from_user(&value, argp, sizeof(value))) {
				COMPSDBG("copy_from_user failed.");
				return -EFAULT;
			}
			break;
		default:
			break;
	}
	
	switch (cmd) {
		case ECS_IOCTL_WRITE:
			COMPSFUNC("IOCTL_WRITE");
			if ((rwbuf[0] < 2) || (rwbuf[0] > (RWBUF_SIZE-1))) {
				COMPSDBG("invalid argument.");
				return -EINVAL;
			}
			ret = I2C_TxData(&rwbuf[1], rwbuf[0]);
			if (ret < 0) {
				return ret;
			}
			break;
		case ECS_IOCTL_READ:
			COMPSFUNC("IOCTL_READ");
			if ((rwbuf[0] < 1) || (rwbuf[0] > (RWBUF_SIZE-1))) {
				COMPSDBG("invalid argument.");
				return -EINVAL;
			}
			ret = I2C_RxData(&rwbuf[1], rwbuf[0]);
			if (ret < 0) {
				return ret;
			}
			break;
		case ECS_IOCTL_SET_MODE:
			COMPSFUNC("IOCTL_SET_MODE");
			ret = COMPS_SetMode(mode);
			if (ret < 0) {
				return ret;
			}
			break;
		case ECS_IOCTL_GETDATA:
			COMPSFUNC("IOCTL_GET_DATA");
			ret = COMPS_GetData(sData);
			if (ret < 0) {
				return ret;
			}
			break;
		case ECS_IOCTL_SET_YPR:
			COMPS_SetYPR(value);
			break;
		case ECS_IOCTL_GET_OPEN_STATUS:
			COMPSFUNC("IOCTL_GET_OPEN_STATUS");
			status = COMPS_GetOpenStatus();
			COMPSDBG("COMPS_GetOpenStatus returned (%d)", status);
			break;
		case ECS_IOCTL_GET_CLOSE_STATUS:
			COMPSFUNC("IOCTL_GET_CLOSE_STATUS");
			status = COMPS_GetCloseStatus();
			COMPSDBG("COMPS_GetCloseStatus returned (%d)", status);
			break;
		case ECS_IOCTL_GET_DELAY:
			COMPSFUNC("IOCTL_GET_DELAY");
			delay = compass_delay;
			break;
		case ECS_IOCTL_FACTORY_TEST:
			COMPSFUNC("ECS_IOCTL_FACTORY_TEST");
			ret = COMPS_FactoryShipmentTest();
			if (ret < 0) {
				return ret;
			}
			break;
		case ECS_IOCTL_GET_APPFLAG:
			AppFlag[0] = atomic_read(&m_flag);
			AppFlag[1] = atomic_read(&a_flag);
			AppFlag[2] = atomic_read(&mv_flag);
			break;
		default:
			return -ENOTTY;
	}
	
	switch (cmd) {
		case ECS_IOCTL_READ:
			if (copy_to_user(argp, &rwbuf, rwbuf[0]+1)) {
				COMPSDBG("copy_to_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_GETDATA:
			if (copy_to_user(argp, &sData, sizeof(sData))) {
				COMPSDBG("copy_to_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_GET_OPEN_STATUS:
		case ECS_IOCTL_GET_CLOSE_STATUS:
			if (copy_to_user(argp, &status, sizeof(status))) {
				COMPSDBG("copy_to_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_GET_DELAY:
			if (copy_to_user(argp, &delay, sizeof(delay))) {
				COMPSDBG("copy_to_user failed.");
				return -EFAULT;
			}
			break;
		case ECS_IOCTL_GET_APPFLAG:
			if (copy_to_user(argp, &AppFlag, sizeof(AppFlag))) {
				COMPSDBG("copy_to_user failed.");
				return -EFAULT;
			}
			break;
		default:
			break;
	}

	return 0;
}

static void COMPS_Work_Func(struct work_struct *work)
{
	uint8_t buffer[6];
	int16_t XYZ[3];
	int16_t offset_local[3];
	char status;
	uint8_t i;
	int ret;

	COMPSFUNC("COMPS_Work_Func");

	memset(buffer, 0, sizeof(buffer));

	status = ALPS_REG_STAT;
	ret = I2C_RxData(&status, 1);
	if (ret < 0) {
		printk(KERN_ERR "COMPS_Work_Func: I2C failed\n");
		return;
	}

	buffer[0] = ALPS_REG_DATAX;
	ret = I2C_RxData(buffer, SENSOR_DATA_SIZE);
	if (ret < 0) {
		printk(KERN_ERR "COMPS_Work_Func: I2C failed\n");
		return;
	}

	/* Check ST bit */
	if ((status & 0x40) != 0x40) {
		printk(KERN_ERR "COMPS_Work_Func: DRDY is not set\n");
		return;
	}

	/* convert */
	XYZ[0] = MAKE_UINT16(buffer[1],buffer[0]);
	XYZ[1] = MAKE_UINT16(buffer[3],buffer[2]);
	XYZ[2] = MAKE_UINT16(buffer[5],buffer[4]);

	for (i=0; i<3; i++) {
		XYZ[i] += offset_data[i];

		offset_local[i] = offset_data[i];

		if ((XYZ[i] - offset_data[i]) > 1500) {
			switch (offset_data[i]) {
				case 0:
					offset_local[i] = 2047;
					break;
				case -2048:
					offset_local[i] = 0;
					break;
			}
		}
		else if ((XYZ[i] - offset_data[i]) < -1500) {
			switch (offset_data[i]) {
				case 0:
					offset_local[i] = -2048;
					break;
				case 2047:
					offset_local[i] = 0;
					break;
			}
		}
	}

	if((offset_data[0] != offset_local[0]) || (offset_data[1] != offset_local[1]) || (offset_data[2] != offset_local[2]))
	{
		uint8_t send_buf[7];

		offset_data[0] = offset_local[0];
		offset_data[1] = offset_local[1];
		offset_data[2] = offset_local[2];

		/* Set offset mode */
		send_buf[0] = ALPS_REG_OFFX;
		send_buf[1] = (uint8_t)(offset_data[0] & 0xFF);
		send_buf[2] = (uint8_t)((offset_data[0] >> 8) &0x0F);
		send_buf[3] = (uint8_t)(offset_data[1] & 0xFF);
		send_buf[4] = (uint8_t)((offset_data[1] >> 8) &0x0F);
		send_buf[5] = (uint8_t)(offset_data[2] & 0xFF);
		send_buf[6] = (uint8_t)((offset_data[2] >> 8) &0x0F);

		/* Set data */
		if(I2C_TxData(send_buf, 7) < 0)
		{
		}
	}

	mutex_lock(&sense_data_mutex);
	memcpy(sense_data, XYZ, sizeof(XYZ));
	atomic_set(&data_ready, 1);
	wake_up(&data_ready_wq);
	mutex_unlock(&sense_data_mutex);
}


static irqreturn_t COMPS_Interrupt(int irq, void *dev_id)
{
	struct compass_data *data = dev_id;

	COMPSFUNC("COMPS_Interrupt");
	disable_irq_nosync(this_client->irq);
	schedule_work(&data->work);
	return IRQ_HANDLED;
}


/*********************************************/
static struct file_operations comps_fops = {
	.owner = THIS_MODULE,
	.open = compass_open,
	.release = compass_release,
	.ioctl = compass_ioctl,
};

static struct file_operations comps_aot_fops = {
	.owner = THIS_MODULE,
	.open = compass_aot_open,
	.release = compass_aot_release,
	.ioctl = compass_aot_ioctl,
};

static struct miscdevice comps_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "compass_dev",
	.fops = &comps_fops,
};

static struct miscdevice comps_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "compass_aot",
	.fops = &comps_aot_fops,
};

static int compass_ConfigGPIO(struct sh_i2c_compass_platform_data *poSetupData)
{
	if(poSetupData == NULL)
		return -EINVAL;
	return poSetupData->gpio_setup();
}

static int compass_ReleaseGPIO(struct sh_i2c_compass_platform_data *poSetupData)
{
	if(poSetupData == NULL)
		return -EINVAL;
	/* GPIO */
	poSetupData->gpio_shutdown();
	return 0;
}


int compass_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct compass_data *data;
	int err = 0;
	
	COMPSFUNC("compass_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "compass_probe: check_functionality failed.\n");
		err = -ENODEV;
		goto exit0;
	}
	
	/* Allocate memory for driver data */
	data = kzalloc(sizeof(struct compass_data), GFP_KERNEL);
	if (!data) {
		printk(KERN_ERR "compass_probe: memory allocation failed.\n");
		err = -ENOMEM;
		goto exit1;
	}
	
	INIT_WORK(&data->work, COMPS_Work_Func);
	i2c_set_clientdata(client, data);
	
	/* Check platform data*/
	if (client->dev.platform_data == NULL) {
		printk(KERN_ERR "compass_probe: platform data is NULL\n");
		err = -ENOMEM;
		goto exit2;
	}	
	/* Copy to global variable */
	pdata = client->dev.platform_data;
	this_client = client;
	
	if(compass_ConfigGPIO(pdata))
		goto exit2;

	/* Check connection */
	err = COMPS_CheckDevice();
	if (err < 0) {
		printk(KERN_ERR "compass_probe: set power down mode error\n");
		goto exit3;
	}
	
	/* IRQ */
	err = request_irq(client->irq, COMPS_Interrupt, IRQF_TRIGGER_HIGH | IRQF_DISABLED,
					  "compass_DRDY", data);
	if (err < 0) {
		printk(KERN_ERR "compass_probe: request irq failed\n");
		goto exit4;
	}

	disable_irq(client->irq);	

	/* Declare input device */
	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		err = -ENOMEM;
		printk(KERN_ERR
		       "compass_probe: Failed to allocate input device\n");
		goto exit5;
	}
	/* Setup input device */
	set_bit(EV_ABS, data->input_dev->evbit);
	/* yaw (0, 360) */
	input_set_abs_params(data->input_dev, ABS_RX, 0, 360, 0, 0);
	/* pitch (-180, 180) */
	input_set_abs_params(data->input_dev, ABS_RY, -180, 180, 0, 0);
	/* roll (-90, 90) */
	input_set_abs_params(data->input_dev, ABS_RZ, -90, 90, 0, 0);
	/* x-axis acceleration (512 x 8G) */
	input_set_abs_params(data->input_dev, ABS_X, -4096, 4095, 0, 0);
	/* y-axis acceleration (512 x 8G) */
	input_set_abs_params(data->input_dev, ABS_Y, -4096, 4095, 0, 0);
	/* z-axis acceleration (512 x 8G) */
	input_set_abs_params(data->input_dev, ABS_Z, -4096, 4095, 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(data->input_dev, ABS_RUDDER, 0, 3, 0, 0);
	/* status of acceleration sensor */
	input_set_abs_params(data->input_dev, ABS_WHEEL,  0, 3, 0, 0);
	/* x-axis of raw magnetic vector (-4096, 4095) */
	input_set_abs_params(data->input_dev, ABS_HAT0X, -4096, 4095, 0, 0);
	/* y-axis of raw magnetic vector (-4096, 4095) */
	input_set_abs_params(data->input_dev, ABS_HAT0Y, -4096, 4095, 0, 0);
	/* z-axis of raw magnetic vector (-4096, 4095) */
	input_set_abs_params(data->input_dev, ABS_BRAKE, -4096, 4095, 0, 0);
	/* Set name */
	data->input_dev->name = "compass";
	
	/* Register */
	err = input_register_device(data->input_dev);
	if (err) {
		printk(KERN_ERR
		       "compass_probe: Unable to register input device\n");
		goto exit6;
	}
	
	err = misc_register(&comps_device);
	if (err) {
		printk(KERN_ERR
			   "compass_probe: comps_device register failed\n");
		goto exit7;
	}
	
	err = misc_register(&comps_aot_device);
	if (err) {
		printk(KERN_ERR
			   "compass_probe: comps_aot_device register failed\n");
		goto exit8;
	}
	
	mutex_init(&sense_data_mutex);
	
	init_waitqueue_head(&data_ready_wq);
	init_waitqueue_head(&open_wq);
	
	/* As default, report all information */
	atomic_set(&m_flag, 0);
	atomic_set(&a_flag, 0);
	atomic_set(&mv_flag, 0);
	
	COMPSDBG("successfully probed.");
	return 0;
	
exit8:
	misc_deregister(&comps_device);
exit7:
	input_unregister_device(data->input_dev);
exit6:
	input_free_device(data->input_dev);
exit5:
	free_irq(client->irq, data);
exit4:
exit3:
	compass_ReleaseGPIO(pdata);
exit2:
	kfree(data);
exit1:
exit0:
	return err;
	
}

static int compass_remove(struct i2c_client *client)
{
	struct compass_data *data = i2c_get_clientdata(client);
	COMPSFUNC("compass_remove");
	misc_deregister(&comps_aot_device);
	misc_deregister(&comps_device);
	input_unregister_device(data->input_dev);
	free_irq(client->irq, data);
	kfree(data);
	COMPSDBG("successfully removed.");
	return 0;
}

static const struct i2c_device_id compass_id[] = {
	{SH_COMPS_I2C_DEVNAME, 0 },
	{ }
};

static struct i2c_driver compass_driver = {
	.probe		= compass_probe,
	.remove 	= compass_remove,
	.id_table	= compass_id,
	.driver = {
		.name = SH_COMPS_I2C_DEVNAME,
	},
};


static int __init compass_init(void)
{
	COMPSFUNC("compass_init");
	return i2c_add_driver(&compass_driver);
}

static void __exit compass_exit(void)
{
	COMPSFUNC("compass_exit");
	i2c_del_driver(&compass_driver);
}

module_init(compass_init);
module_exit(compass_exit);

MODULE_DESCRIPTION("Compass driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SHARP CORPORATION");
MODULE_VERSION("1.0");
