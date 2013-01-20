/* drivers/sharp/shcamsensor/sh_maincam_power_ctrl.h  (Camera Driver)
 *
 * Copyright (C) 2009-2011 SHARP CORPORATION
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

#ifndef __SH_MAINCAM_POWER_CTL_H
#define __SH_MAINCAM_POWER_CTL_H

#define SH_MAINCAMDRV_POWER_ON      (1)
#define SH_MAINCAMDRV_POWER_OFF     (0)

#define SH_MAINCAMDRV_USE_CAMERA_DRV    (0)
#define SH_MAINCAMDRV_USE_SASUKE_DRV    (1)

#define SH_MAINCAMDRV_NO_ERROR      (0)
#define SH_MAINCAMDRV_MULTIPLE      (-200)
#define SH_MAINCAMDRV_USE_CAMERA    (-201)
#define SH_MAINCAMDRV_USE_SASUKE    (-202)
#define SH_MAINCAMDRV_POWER_ON_ERR  (-203)
#define SH_MAINCAMDRV_POWER_OFF_ERR (-204)


int sh_maincam_drv_power_ctrl(int ctrl, int type);
int sh_maincam_drv_sasuke_state(void);

MODULE_DESCRIPTION("SHARP CAMERA DRIVER MODULE");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SHARP CORPORATION");
MODULE_VERSION("1.01");

#endif
