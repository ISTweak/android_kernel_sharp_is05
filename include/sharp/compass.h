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

/*
 * Definitions for compass chip.
 */
#ifndef COMPASS_H
#define COMPASS_H

#include <linux/ioctl.h>


#define	SH_COMPS_I2C_DEVNAME	"ALPS_HSCD"
#define SH_COMPS_I2C_SLAVE		0x0c
#define	SH_COMPS_IRQ			181
#define	SH_COMPS_DRDY			117


#define SENSOR_DATA_SIZE		6	/* XYZ Sensor Data Size */
#define RWBUF_SIZE				16	/* Read/Write buffer size.*/

enum {
	COMPS_MODE_FORCE = 0,
	COMPS_MODE_NORMAL,
	COMPS_MODE_SELFTEST,
	COMPS_MODE_POWERDOWN
};



/* Register Address */
#define ALPS_REG_STB			0x0C
#define ALPS_REG_INFO			0x0D
#define ALPS_REG_WIA			0x0F
#define ALPS_REG_DATAX			0x10
#define ALPS_REG_DATAY			0x12
#define ALPS_REG_DATAZ			0x14
#define ALPS_REG_INS			0x16
#define ALPS_REG_STAT			0x18
#define ALPS_REG_INL			0x1A
#define ALPS_REG_CNTL1			0x1B
#define ALPS_REG_CNTL2			0x1C
#define ALPS_REG_CNTL3			0x1D
#define ALPS_REG_INC			0x1E
#define ALPS_REG_AMP			0x1F
#define ALPS_REG_OFFX			0x20
#define ALPS_REG_OFFY			0x22
#define ALPS_REG_OFFZ			0x24
#define ALPS_REG_ITHR			0x26
#define ALPS_REG_CNTL4			0x28


/* IOCTLs */
#define IO_ID					0xA1

/* IOCTLs for Driver */
#define ECS_IOCTL_WRITE					_IOW(IO_ID,  0x01, char*)
#define ECS_IOCTL_READ					_IOWR(IO_ID, 0x02, char*)
#define ECS_IOCTL_SET_MODE				_IOW(IO_ID,  0x04, short)
#define ECS_IOCTL_GETDATA				_IOR(IO_ID,  0x05, short[3])
#define ECS_IOCTL_SET_YPR				_IOW(IO_ID,  0x06, short[11])
#define ECS_IOCTL_GET_OPEN_STATUS		_IOR(IO_ID,  0x07, int)
#define ECS_IOCTL_GET_CLOSE_STATUS		_IOR(IO_ID,  0x08, int)
#define ECS_IOCTL_GET_DELAY				_IOR(IO_ID,  0x30, short)
#define ECS_IOCTL_FACTORY_TEST			_IO(IO_ID,   0x31)
#define ECS_IOCTL_GET_ADJ_DATA			_IOR(IO_ID,  0x32, char[3])
#define ECS_IOCTL_INIT_ACCURACY			_IO(IO_ID,   0x33)
#define ECS_IOCTL_GET_APPFLAG           _IOR(IO_ID,  0x34, char[3])


/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MODE			_IOW(IO_ID, 0x10, short)
#define ECS_IOCTL_APP_SET_MFLAG			_IOW(IO_ID, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG			_IOW(IO_ID, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG			_IOW(IO_ID, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG			_IOR(IO_ID, 0x14, short)



#define ECS_IOCTL_APP_SET_DELAY			_IOW(IO_ID, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY			ECS_IOCTL_GET_DELAY
#define ECS_IOCTL_APP_SET_MVFLAG		_IOW(IO_ID, 0x19, short)
#define ECS_IOCTL_APP_GET_MVFLAG		_IOR(IO_ID, 0x1A, short)


struct sh_i2c_compass_platform_data {
    int     gpio_IRQ;
    int     gpio_DRDY;
    int     (*gpio_setup) (void);
    void    (*gpio_shutdown)(void);
};

#endif /* COMPASS_H */

