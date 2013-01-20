/* kernel/include/sharp/sh_android_usb_products_jir.h
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

#ifndef __SH_ANDROID_USB_PRODUCTS_JIR_H
#define __SH_ANDROID_USB_PRODUCTS_JIR_H

#define	ANDROID_USB_SH_DEFAULT_PID		(USB_PID_MODE_MSC)
#define	ANDROID_USB_SH_PRODUCT_NAME		("SBM003SH")
#define	ANDROID_USB_SH_MANUFACTER_NAME	("SHARP Corporation")

static char *usb_functions_cdc[] = {
	"obex",
	"mdlm",
	"modem",
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};

static char *usb_functions_cdc2[] = {
	"obex",
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};


static char *usb_functions_default[] = {
	"usb_mass_storage",
};

static char *usb_functions_default_adb[] = {
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};

static char *usb_functions_mtp[] = {
	"mtp",
};

static char *usb_functions_mtp_adb[] = {
	"mtp",
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};


static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif /* CONFIG_USB_ANDROID_RNDIS */
	"obex",
	"mdlm",
	"modem",
	"usb_mass_storage",
#ifdef CONFIG_USB_ANDROID_SH_MTP
	"mtp",
#endif /* CONFIG_USB_ANDROID_SH_MTP */
	"adb",
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif /* CONFIG_USB_ANDROID_DIAG */
};

static struct android_usb_product usb_products[] = {
	
	/* mode:CDC */
	{
		.product_id	= USB_PID_MODE_CDC,
		.num_functions	= ARRAY_SIZE(usb_functions_cdc),
		.functions	= usb_functions_cdc,
	},

	/* mode:CDC2 */
	{
		.product_id	= USB_PID_MODE_CDC_2,
		.num_functions	= ARRAY_SIZE(usb_functions_cdc2),
		.functions	= usb_functions_cdc2,
	},

	/* mode:MSC */
	{
		.product_id	= USB_PID_MODE_MSC,
		.num_functions	= ARRAY_SIZE(usb_functions_default),
		.functions	= usb_functions_default,
	},

	/* mode:MSC c */
	{
		.product_id	= USB_PID_MODE_MSC_C,
		.num_functions	= ARRAY_SIZE(usb_functions_default_adb),
		.functions	= usb_functions_default_adb,
	},

	/* mode:MTP */
	{
		.product_id	= USB_PID_MODE_MTP,
		.num_functions	= ARRAY_SIZE(usb_functions_mtp),
		.functions	= usb_functions_mtp,
	},

	/* mode:MTP c */
	{
		.product_id	= USB_PID_MODE_MTP_C,
		.num_functions	= ARRAY_SIZE(usb_functions_mtp_adb),
		.functions	= usb_functions_mtp_adb,
	},


	/* mode:RNDIS */
	{
		.product_id	= USB_PID_MODE_RNDIS,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},

	/* mode:RNDIS c */
	{
		.product_id	= USB_PID_MODE_RNDIS_C,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},

};
#endif /* __SH_ANDROID_USB_PRODUCTS_JIR_H */
