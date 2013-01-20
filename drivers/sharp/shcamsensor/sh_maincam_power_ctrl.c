/* drivers/sharp/shcamsensor/sh_maincam_power_ctrl.c  (Camera Driver)
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/err.h>
#include <mach/vreg.h>

#include "sh_maincam_power_ctrl.h"

#define POWER_ON_DELAY                1000
#define POWER_OFF_DELAY               6000
#define POWER_OFF_DELAY1              2000
#define POWER_OFF_DELAY100            100
#define POWER_CTRL_DEFAULT_DELAY      400
#define POWER_CTRL_NO_DELAY           0

static atomic_t sasuke = ATOMIC_INIT(0);
static atomic_t camera = ATOMIC_INIT(0);

struct mutex    maincam_power_mutex_lock;

static int gp3_ctrl(int ctrl);
static int lvsw1_ctrl(int ctrl);

int sh_maincam_drv_power_ctrl(int ctrl, int type)
{
    int result;
    int dev_sasuke;
    int dev_camera;
    int power_off_delay1;
    int power_off_delay2;

    if ((SH_MAINCAMDRV_USE_CAMERA_DRV != type)
    && (SH_MAINCAMDRV_USE_SASUKE_DRV != type)) {
        printk(KERN_ERR "[sh_maincam_drv_power_ctrl] out error type=%d\n", type);
        return -ENOEXEC;
    }

    mutex_lock(&maincam_power_mutex_lock);

    if (SH_MAINCAMDRV_USE_SASUKE_DRV == type) {
        power_off_delay1 = POWER_OFF_DELAY100;
        power_off_delay2 = POWER_OFF_DELAY;
    }
    else {
        power_off_delay1 = POWER_OFF_DELAY1;
        power_off_delay2 = POWER_CTRL_NO_DELAY;
    }

    result     = SH_MAINCAMDRV_NO_ERROR;
    dev_sasuke = atomic_read(&sasuke);
    dev_camera = atomic_read(&camera);

    switch (ctrl) {
    case SH_MAINCAMDRV_POWER_ON:
        if ((SH_MAINCAMDRV_POWER_OFF == dev_camera)
        && (SH_MAINCAMDRV_POWER_OFF == dev_sasuke)) {
            result = gp3_ctrl(SH_MAINCAMDRV_POWER_ON);
            udelay(POWER_ON_DELAY);

            result = lvsw1_ctrl(SH_MAINCAMDRV_POWER_ON);

            if (SH_MAINCAMDRV_NO_ERROR != result) {
                result = SH_MAINCAMDRV_POWER_ON_ERR;
                printk(KERN_ERR "error result=%d\n", result);
            }
        }

        if (SH_MAINCAMDRV_USE_SASUKE_DRV == type) {
            atomic_inc(&sasuke);
        }
        else {
            atomic_inc(&camera);
        }
        dev_sasuke = atomic_read(&sasuke);
        dev_camera = atomic_read(&camera);
        break;

    case SH_MAINCAMDRV_POWER_OFF:
        if (SH_MAINCAMDRV_USE_SASUKE_DRV == type) {
            if (SH_MAINCAMDRV_POWER_OFF < dev_sasuke) {
                atomic_dec(&sasuke);
            }
            else {
                result = SH_MAINCAMDRV_MULTIPLE;
                printk(KERN_ERR "error dev_sasuke=%d\n", dev_sasuke);
            }
        }
        else {
            if (SH_MAINCAMDRV_POWER_OFF < dev_camera) {
                atomic_dec(&camera);
            }
            else {
                result = SH_MAINCAMDRV_MULTIPLE;
                printk(KERN_ERR "error dev_camera=%d\n", dev_camera);
            }
        }

        if (SH_MAINCAMDRV_NO_ERROR == result) {
            dev_sasuke = atomic_read(&sasuke);
            dev_camera = atomic_read(&camera);
            if ((SH_MAINCAMDRV_POWER_OFF == dev_camera)
            && (SH_MAINCAMDRV_POWER_OFF == dev_sasuke)) {
                result = lvsw1_ctrl(SH_MAINCAMDRV_POWER_OFF);
                udelay(power_off_delay1);

                result = gp3_ctrl(SH_MAINCAMDRV_POWER_OFF);
                udelay(power_off_delay2);

                if (SH_MAINCAMDRV_NO_ERROR != result) {
                    result = SH_MAINCAMDRV_POWER_OFF_ERR;
                    printk(KERN_ERR "error result=%d\n", result);
                }
            }
            else {
                if (SH_MAINCAMDRV_POWER_OFF != dev_sasuke) {
                    result = SH_MAINCAMDRV_USE_SASUKE;
                }
                if (SH_MAINCAMDRV_POWER_OFF != dev_camera) {
                    result = SH_MAINCAMDRV_USE_CAMERA;
                }
            }
        }
        break;

    default:
        result = -ENOEXEC;
        break;
    }

    mutex_unlock(&maincam_power_mutex_lock);

    return result;
}

int sh_maincam_drv_sasuke_state(void)
{
    return atomic_read(&sasuke);
}


static int gp3_ctrl(int ctrl)
{
    int result = -EFAULT;
    struct vreg*    camdrv_vreg = NULL;

    camdrv_vreg = vreg_get(NULL, "gp3");
    if (!IS_ERR(camdrv_vreg)) {
        if (SH_MAINCAMDRV_POWER_ON == ctrl) {
            result = vreg_set_level(camdrv_vreg, 1200);
            result = vreg_enable(camdrv_vreg);
        }
        else {
            result = vreg_disable(camdrv_vreg);
        }
        udelay(POWER_CTRL_DEFAULT_DELAY);
    }

    return result;
}


static int lvsw1_ctrl(int ctrl)
{
    int result = -EFAULT;
    struct vreg*    camdrv_vreg = NULL;

    camdrv_vreg = vreg_get(NULL, "lvsw1");
    if (!IS_ERR(camdrv_vreg)) {
        if (SH_MAINCAMDRV_POWER_ON == ctrl) {
            result = vreg_set_level(camdrv_vreg, 1800);
            result = vreg_enable(camdrv_vreg);
        }
        else {
            result = vreg_disable(camdrv_vreg);
        }
        udelay(POWER_CTRL_DEFAULT_DELAY);
    }

    return result;
}

MODULE_DESCRIPTION("SHARP CAMERA DRIVER MODULE");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SHARP CORPORATION");
MODULE_VERSION("1.01");
