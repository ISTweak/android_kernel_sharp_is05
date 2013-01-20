/* drivers/sharp/shkeyled/shkeyled_kerl.c  (Key LED Driver)
 *
 * Copyright (C) 2010 SHARP CORPORATION
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
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#include <mach/pmic.h>

#define SHKEYLED_SHTERM_ENABLE

#ifdef SHKEYLED_SHTERM_ENABLE
#include <sharp/shterm_k.h>
#endif /* SHKEYLED_SHTERM_ENABLE */

#define SHKEYLED_LOG_TAG "SHKEYLEDkerl"
#define  SHKEYLED_DEBUG_LOG_ENABLE_1
//#define  SHKEYLED_DEBUG_LOG_ENABLE_2

#ifdef SHKEYLED_DEBUG_LOG_ENABLE_1
#define SHKEYLED_DEBUG_LOG_1(fmt, args...)	printk(KERN_INFO "[%s][%s(%d)] " fmt"\n", SHKEYLED_LOG_TAG, __func__, __LINE__, ## args)
#else
#define SHKEYLED_DEBUG_LOG_1(fmt, args...)
#endif

#ifdef SHKEYLED_DEBUG_LOG_ENABLE_2
#define SHKEYLED_DEBUG_LOG_2(fmt, args...)	printk(KERN_INFO "[%s][%s(%d)] " fmt"\n", SHKEYLED_LOG_TAG, __func__, __LINE__, ## args)
#else
#define SHKEYLED_DEBUG_LOG_2(fmt, args...)
#endif

#define KEYLED_FULL 4

static void shkeyled_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	int ret;
	uint ma;
	
	if(value == LED_OFF)
	{
		ma = 0;
	}
	else if(value == LED_HALF)
	{
		ma = KEYLED_FULL / 2;
	}
	else if(value == LED_FULL)
	{
		ma = KEYLED_FULL;
	}
	else
	{
		ma = 0;
	}
	
	SHKEYLED_DEBUG_LOG_2("value = %d, mA = %d", value, ma);
	
	ret = pmic_low_current_led_set_current(LOW_CURRENT_LED_DRV2, ma);
	if(ret)
	{
		SHKEYLED_DEBUG_LOG_1("pmic_low_current_led_set_current Error");
	}
	
	if(ret == 0)
	{
#ifdef SHKEYLED_SHTERM_ENABLE
		if(ma == 0)
		{
			if(shterm_k_set_info(SHTERM_INFO_KEYBACKLIGHT, 0) != SHTERM_SUCCESS)
			{
				SHKEYLED_DEBUG_LOG_1("shterm_k_set_info 0 Error");
			}
			else
			{
				SHKEYLED_DEBUG_LOG_2("shterm_k_set_info 0");
			}
		}
		else
		{
			if(shterm_k_set_info(SHTERM_INFO_KEYBACKLIGHT, 1) != SHTERM_SUCCESS)
			{
				SHKEYLED_DEBUG_LOG_1("shterm_k_set_info 1 Error");
			}
			else
			{
				SHKEYLED_DEBUG_LOG_2("shterm_k_set_info 1");
			}
		}
#endif /* SHKEYLED_SHTERM_ENABLE */
	}
}

static struct led_classdev shkeyled_dev =
{
	.name			= "button-backlight",
	.brightness_set	= shkeyled_set,
	.brightness		= LED_OFF,
};

static int shkeyled_probe(struct platform_device *pdev)
{
	int rc;

	rc = led_classdev_register(&pdev->dev, &shkeyled_dev);
	if (rc)
	{
		SHKEYLED_DEBUG_LOG_1("led_classdev_register Error");
		return rc;
	}
	shkeyled_set(&shkeyled_dev, LED_OFF);
	return rc;
}

static int __devexit shkeyled_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&shkeyled_dev);

	return 0;
}

#ifdef CONFIG_PM
static int shkeyled_suspend(struct platform_device *dev, pm_message_t state)
{
	led_classdev_suspend(&shkeyled_dev);

	return 0;
}

static int shkeyled_resume(struct platform_device *dev)
{
	led_classdev_resume(&shkeyled_dev);

	return 0;
}
#else
#define shkeyled_suspend NULL
#define shkeyled_resume NULL
#endif

static struct platform_driver shkeyled_driver = {
	.probe		= shkeyled_probe,
	.remove		= __devexit_p(shkeyled_remove),
	.suspend	= shkeyled_suspend,
	.resume		= shkeyled_resume,
	.driver		= {
		.name	= "shkeyled",
		.owner	= THIS_MODULE,
	},
};

static int __init shkeyled_init(void)
{
	return platform_driver_register(&shkeyled_driver);
}

static void __exit shkeyled_exit(void)
{
	platform_driver_unregister(&shkeyled_driver);
}

module_exit(shkeyled_exit);
module_init(shkeyled_init);

MODULE_DESCRIPTION("SHARP KEYLED DRIVER MODULE");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SHARP CORPORATION");
MODULE_VERSION("1.0");
