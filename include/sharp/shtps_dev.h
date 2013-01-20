/* include/sharp/shtps_dev.h
 *
 * Copyright (C) 2011 SHARP CORPORATION
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
#ifndef __SHTPS_DEV_H__
#define __SHTPS_DEV_H__

#define SH_TOUCH_DEVNAME		"shtps_rmi"
#define SH_TOUCH_IF_DEVNAME 	"shtpsif"
#define SH_TOUCH_IF_DEVPATH 	"/dev/shtpsif"
#define SHTPS_TM_TXNUM_MAX		19
#define SHTPS_TM_RXNUM_MAX		9
#define SHTPS_TM_RXNUM			18

#define TPS_IOC_MAGIC					0xE0

#define TPSDEV_ENABLE					_IO  ( TPS_IOC_MAGIC,  1)
#define TPSDEV_DISABLE					_IO  ( TPS_IOC_MAGIC,  2)
#define TPSDEV_RESET					_IO  ( TPS_IOC_MAGIC,  3)
#define TPSDEV_SOFT_RESET				_IO  ( TPS_IOC_MAGIC,  4)
#define TPSDEV_GET_FW_VERSION			_IOR ( TPS_IOC_MAGIC,  5, unsigned short)
#define TPSDEV_ENTER_BOOTLOADER			_IOR ( TPS_IOC_MAGIC,  6, struct shtps_bootloader_info)
#define TPSDEV_LOCKDOWN_BOOTLOADER		_IOW ( TPS_IOC_MAGIC,  7, struct shtps_ioctl_param)
#define TPSDEV_ERASE_FLASE				_IO  ( TPS_IOC_MAGIC,  8)
#define TPSDEV_WRITE_IMAGE				_IOW ( TPS_IOC_MAGIC,  9, struct shtps_ioctl_param)
#define TPSDEV_WRITE_CONFIG				_IOW ( TPS_IOC_MAGIC, 10, struct shtps_ioctl_param)
#define TPSDEV_GET_TOUCHINFO			_IOR ( TPS_IOC_MAGIC, 11, struct shtps_touch_info)
#define TPSDEV_GET_TOUCHINFO_UNTRANS	_IOR ( TPS_IOC_MAGIC, 12, struct shtps_touch_info)
#define TPSDEV_SET_TOUCHMONITOR_MODE	_IOW ( TPS_IOC_MAGIC, 13, unsigned char)
#define TPSDEV_READ_REG					_IOWR( TPS_IOC_MAGIC, 14, struct shtps_ioctl_param)
#define TPSDEV_READ_ALL_REG				_IOR ( TPS_IOC_MAGIC, 15, struct shtps_ioctl_param)
#define TPSDEV_WRITE_REG				_IOW ( TPS_IOC_MAGIC, 16, struct shtps_ioctl_param)
#define TPSDEV_START_TM					_IOW ( TPS_IOC_MAGIC, 17, struct shtps_ioctl_param)
#define TPSDEV_STOP_TM					_IO  ( TPS_IOC_MAGIC, 18)
#define TPSDEV_GET_BASELINE				_IOR ( TPS_IOC_MAGIC, 19, unsigned short*)
#define TPSDEV_GET_FRAMELINE			_IOR ( TPS_IOC_MAGIC, 20, unsigned char*)
#define TPSDEV_START_FACETOUCHMODE		_IO  ( TPS_IOC_MAGIC, 21)
#define TPSDEV_STOP_FACETOUCHMODE		_IO  ( TPS_IOC_MAGIC, 22)
#define TPSDEV_POLL_FACETOUCHOFF		_IO  ( TPS_IOC_MAGIC, 23)
#define TPSDEV_GET_FWSTATUS				_IOR ( TPS_IOC_MAGIC, 24, unsigned char)
#define TPSDEV_GET_FWDATE				_IOR ( TPS_IOC_MAGIC, 25, unsigned short)
#define TPSDEV_CALIBRATION_PARAM		_IOW ( TPS_IOC_MAGIC, 26, struct shtps_ioctl_param)
#define TPSDEV_DEBUG_REQEVENT			_IOW ( TPS_IOC_MAGIC, 27, int)
#define TPSDEV_SET_DRAGSTEP				_IOW ( TPS_IOC_MAGIC, 28, int)
#define TPSDEV_SET_POLLINGINTERVAL		_IOW ( TPS_IOC_MAGIC, 29, int)
#define TPSDEV_SET_FINGERFIXTIME		_IOW ( TPS_IOC_MAGIC, 30, int)
#define TPSDEV_GET_HW_REVISION			_IOW ( TPS_IOC_MAGIC, 31, int)
#define TPSDEV_REZERO					_IO  ( TPS_IOC_MAGIC, 32)
#define TPSDEV_GET_BL_VERSION			_IOR ( TPS_IOC_MAGIC, 33, unsigned char*)
#define TPSDEV_WRITE_BL_CONFIG			_IOW ( TPS_IOC_MAGIC, 34, struct shtps_ioctl_param)

#define TPSDEV_FACETOUCHOFF_NOCHG	0x00
#define TPSDEV_FACETOUCHOFF_DETECT	0x01

#define TPSDEV_TOUCHINFO_MODE_LCDSIZE	0
#define TPSDEV_TOUCHINFO_MODE_DEVSIZE	1

struct shtps_ioctl_param {
	int				size;
	unsigned char*	data;
};

struct shtps_bootloader_info {
	unsigned long	block_size;
	unsigned long	program_block_num;
	unsigned long	config_block_num;
};

struct shtps_touch_info {
	struct fingers{
		unsigned char	state;
		unsigned short	x;
		unsigned short	y;
		unsigned char	wx;
		unsigned char	wy;
		unsigned char	z;
	} fingers[5];
	
	unsigned char		gs1;
	unsigned char		gs2;
	unsigned char		flick_x;
	unsigned char		flick_y;
	unsigned char		flick_time;
};

/* -----------------------------------------------------------------------------------
 */
void msm_tps_setsleep(int on);

#endif /* __SHTPS_DEV_H__ */
