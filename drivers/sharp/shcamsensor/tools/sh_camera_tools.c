/* drivers/sharp/shcamsensor/tools/sh_camera_tools.c  (Camera Driver)
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

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <mach/camera.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/clk.h>
#include <sharp/sh_smem.h>

#include "sh_camera_tools.h"

static int shcam_get_sensor_chipid(struct msm_sync *sync);
static int shcam_get_sensor_data(struct msm_sync *sync, void __user *argp);
static int shcam_set_sensor_data(struct msm_sync *sync, void __user *argp);
static int shcam_get_sensor_otp(struct msm_sync *sync, void __user *argp);
static int shcam_ctrl_power(struct msm_sync *sync, void __user *argp);
static int shcam_get_smem_otp(void __user *argp);

static void sensor_vreg_enable(struct platform_device *pdev);
static void sensor_vreg_disable(void);
static int sensor_power_ctrl(struct platform_device *pdev, int8_t power);


static int shcam_get_sensor_chipid(struct msm_sync *sync)
{
    int ret = 0;

    ret = sync->sctrl.s_chipid();

    return ret;
}

static int shcam_get_sensor_data(struct msm_sync *sync, void __user *argp)
{
    unsigned short temp = 0;
    unsigned short addr = 0;
    int ret = 1;

    struct sh_sensor_dspr {
        unsigned short  addr;
        unsigned short* data;
    } *pData = (struct sh_sensor_dspr*)argp;

    if(copy_from_user((void*)&addr, (void*)&pData->addr, sizeof(short))) {
        printk(KERN_ERR "[%d]:copy_from_user()\n", __LINE__);
        ret = 0;
    }
    else {
        ret = sync->sctrl.s_reg_read(addr, &temp);

        if(copy_to_user((void*)pData->data, (void*)&temp, sizeof(short))) {
            printk(KERN_ERR "[%d]:copy_to_user()\n", __LINE__);
            ret = 0;
        }
    }

    return ret;
}

static int shcam_set_sensor_data(struct msm_sync *sync, void __user *argp)
{
    int ret;

    struct sh_sensor_dspw {
        unsigned short  addr;
        unsigned short  data;
    } udata;

    if(copy_from_user((void*)&udata, argp, sizeof(struct sh_sensor_dspw))) {
        printk(KERN_ERR "[%d]:copy_from_user()\n", __LINE__);
        ret = 0;
    }
    else {
        ret = sync->sctrl.s_reg_write(udata.addr, udata.data);
    }

    return ret;
}

static int shcam_get_sensor_otp(struct msm_sync *sync, void __user *argp)
{
    int ret = 0;
    short len = 0;
    unsigned char* temp = NULL;
    struct sh_sensor_otp udata;
    struct sh_sensor_otp* pData = (struct sh_sensor_otp*)argp;

    if (copy_from_user((void*)&udata, argp, sizeof(struct sh_sensor_otp))) {
        printk(KERN_ERR "[%d]:copy_from_user()\n", __LINE__);
        ret = 0;
    }
    else {
        len = udata.len;
        temp = kmalloc(len, GFP_ATOMIC);
        if(NULL != temp) {
            ret = sync->sctrl.s_otp_read(udata.page, udata.offset, len, temp);

            if (copy_to_user((void*)pData->buf, (void*)&temp[0], len)) {
                printk(KERN_ERR "[%d]:copy_to_user()\n", __LINE__);
                ret = 0;
            }
            kfree(temp);
        }
    }

    return ret;
}

static int shcam_ctrl_power(struct msm_sync *sync, void __user *argp)
{
    struct platform_device *pdev = sync->pdev;
    int8_t power = 0;
    int ret = 1;

    if (copy_from_user((void*)&power, argp, sizeof(int8_t))) {
        printk(KERN_ERR "[%d]:copy_from_user()\n", __LINE__);
        ret = 0;
    }
    else {
        ret = sensor_power_ctrl(pdev, power);
    }

    return ret;
}


static struct vreg *vreg_gp2;
static struct vreg *vreg_gp9;
static struct vreg *vreg_gp15;
static struct vreg *vreg_lvsw1;

static void sensor_vreg_enable(struct platform_device *pdev)
{
    if(!strcmp(pdev->name, "msm_camera_t8ev4")) {
        vreg_lvsw1 = vreg_get(NULL, "lvsw1");
        if (IS_ERR(vreg_lvsw1)) {
            pr_err("%s: VREG LVSW1 get failed %ld\n", __func__,
            PTR_ERR(vreg_lvsw1));
            vreg_lvsw1 = NULL;
            return;
        }
        if (vreg_set_level(vreg_lvsw1, 1800)) {
            pr_err("%s: VREG LVSW1 set failed\n", __func__);
            goto lvsw1_put;
        }
        if (vreg_enable(vreg_lvsw1)) {
            pr_err("%s: VREG LVSW1 enable failed\n", __func__);
            goto lvsw1_put;
        }

        vreg_gp9 = vreg_get(NULL, "gp9");
        if (IS_ERR(vreg_gp9)) {
            pr_err("%s: VREG GP9 get failed %ld\n", __func__,
                PTR_ERR(vreg_gp9));
            vreg_gp9 = NULL;
            goto lvsw1_disable;
        }
        if (vreg_set_level(vreg_gp9, 2800)) {
            pr_err("%s: VREG GP9 set failed\n", __func__);
            goto gp9_put;
        }
        if (vreg_enable(vreg_gp9)) {
            pr_err("%s: VREG GP9 enable failed\n", __func__);
            goto gp9_put;
        }

        vreg_gp2 = vreg_get(NULL, "gp2");
        if (IS_ERR(vreg_gp2)) {
            pr_err("%s: VREG GP2 get failed %ld\n", __func__,
                PTR_ERR(vreg_gp2));
            vreg_gp2 = NULL;
            goto gp9_disable;
        }
        if (vreg_set_level(vreg_gp2, 3000)) {
            pr_err("%s: VREG GP2 set failed\n", __func__);
            goto gp2_put;
        }
        if (vreg_enable(vreg_gp2)) {
            pr_err("%s: VREG GP2 enable failed\n", __func__);
            goto gp2_put;
        }

        vreg_gp15 = vreg_get(NULL, "gp15");
        if (IS_ERR(vreg_gp15)) {
            pr_err("%s: VREG GP15 get failed %ld\n", __func__,
                PTR_ERR(vreg_gp15));
            vreg_gp15 = NULL;
            goto gp2_disable;
        }
        if (vreg_set_level(vreg_gp15, 1230)) {
            pr_err("%s: VREG GP15 set failed\n", __func__);
            goto gp15_put;
        }
        if (vreg_enable(vreg_gp15))
            pr_err("%s: VREG GP15 enable failed\n", __func__);

        return;
    }

gp15_put:
    vreg_put(vreg_gp15);
    vreg_gp15 = NULL;
gp2_disable:
    vreg_disable(vreg_gp2);
gp2_put:
    vreg_put(vreg_gp2);
    vreg_gp2 = NULL;
gp9_disable:
    vreg_disable(vreg_gp9);
gp9_put:
    vreg_put(vreg_gp9);
    vreg_gp9 = NULL;
lvsw1_disable:
    vreg_disable(vreg_lvsw1);
lvsw1_put:
    vreg_put(vreg_lvsw1);
    vreg_lvsw1 = NULL;

}

static void sensor_vreg_disable(void)
{
    if (vreg_gp15) {
        vreg_disable(vreg_gp15);
        vreg_put(vreg_gp15);
        vreg_gp15 = NULL;
    }
    if (vreg_gp2) {
        vreg_disable(vreg_gp2);
        vreg_put(vreg_gp2);
        vreg_gp2 = NULL;
    }
    if (vreg_gp9) {
        vreg_disable(vreg_gp9);
        vreg_put(vreg_gp9);
        vreg_gp9 = NULL;
    }
    if (vreg_lvsw1) {
        vreg_disable(vreg_lvsw1);
        vreg_put(vreg_lvsw1);
        vreg_lvsw1 = NULL;
    }
}

static int sensor_power_ctrl(struct platform_device *pdev, int8_t power)
{
    int rc  = 0;
    int ret = 1;
    struct msm_camera_sensor_info *sdata = pdev->dev.platform_data;
    struct msm_camera_device_platform_data *camdev = sdata->pdata;
    int gpioNo = sdata->sensor_reset;

    if(1 == power) {
        msm_camio_clk_rate_set(9600000);
        camdev->camera_gpio_on();
        sensor_vreg_enable(pdev);
        rc = gpio_request(gpioNo, sdata->sensor_name);
        if(!rc) {
            ndelay(1);
            rc = msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
            udelay(3);
            gpio_direction_output(gpioNo, 0);
            mdelay(20);
            gpio_set_value_cansleep(gpioNo, 1);
            udelay(750);
        } else {
            gpio_direction_input(gpioNo);
            gpio_free(gpioNo);
            ret = 0;
            printk(KERN_ERR "gpio reset fail\n");
        }
    }
    else {
        gpio_direction_output(gpioNo, 1);
        mdelay(20);
        gpio_set_value_cansleep(gpioNo, 0);
        mdelay(3);
        rc = msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);
        ndelay(1);
        gpio_direction_input(gpioNo);
        gpio_free(gpioNo);
        sensor_vreg_disable();
        camdev->camera_gpio_off();
    }

    return ret;
}

static int shcam_get_smem_otp(void __user *argp)
{
    int ret = 1;

    return ret;
}


int sh_camera_ioctl(struct msm_sync *sync, void __user *argp)
{
    int ret = -EINVAL;
    struct sh_sensor_clrt pctrl;

    if (copy_from_user((void*)&pctrl, argp, sizeof(struct sh_sensor_clrt))) {
        printk(KERN_ERR "[%d]:copy_from_user()\n", __LINE__);
        ret = 0;
    }

    switch(pctrl.cmd) {
    case SH_CAM_GET_SENSOR_CHIPID:
        ret = shcam_get_sensor_chipid(sync);
        break;
    case SH_CAM_GET_SENSOR_DATA:
        ret = shcam_get_sensor_data(sync, pctrl.ctrl);
        break;
    case SH_CAM_SET_SENSOR_DATA:
        ret = shcam_set_sensor_data(sync, pctrl.ctrl);
        break;
    case SH_CAM_GET_SENSOR_OTP:
        ret = shcam_get_sensor_otp(sync, pctrl.ctrl);
        break;
    case SH_CAM_CTRL_SENSOR_POWER:
        ret = shcam_ctrl_power(sync, pctrl.ctrl);
        break;
    case SH_CAM_GET_SMEM_OTP:
        ret = shcam_get_smem_otp(pctrl.ctrl);
        break;
    default:
        break;
    }

    return ret;
}

MODULE_DESCRIPTION("SHARP CAMERA DRIVER MODULE");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SHARP CORPORATION");
MODULE_VERSION("1.01");
