/* kernel/include/sharp/sh_android_usb_desc_str_bec.h
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

#ifndef __SH_ANDROID_USB_DESC_STR_BEC_H
#define __SH_ANDROID_USB_DESC_STR_BEC_H

#define USB_OBEX_STRING_DESC_WORD	"SBM005SH OBEX Port"
#define USB_MDLM_STRING_DESC_WORD	"SBM005SH High Speed Serial Port"
#define USB_GS_STRING_DESC_WORD		"SBM005SH"
#define USB_ADB_STRING_DESC_WORD	"Android ADB Interface"
#define USB_QXDM_STRING_DESC_WORD	"Qualcomm Port"
#define USB_MSC_STRING_DESC_WORD	"SBM005SH SD Storage"
#define USB_MTP_STRING_DESC_WORD	"SBM005SH MTP Port"

#define USB_PID_MODE_CDC			0x93C8
#define USB_PID_MODE_MTP			0x93CA
#define USB_PID_MODE_MSC			0x93C9
#define USB_PID_MODE_CDC_2			0x933A
#define USB_PID_MODE_RNDIS			0x94B9

#define USB_PID_MODE_MTP_C			0x94BB
#define USB_PID_MODE_MSC_C			0x94BA
#define USB_PID_MODE_RNDIS_C		0x94BC


#define USB_CDC_GUID				0xDA, 0x08, 0x0C, 0x9F, 0x0B, 0x5C, 0x48, 0xd7,\
									0x98, 0x44, 0xF9, 0x2F, 0x19, 0x62, 0x62, 0x1F

#define USB_MSC_VENDOR_ID_LEN	(8)
#define USB_MSC_VENDOR_ID		"SHARP   "
#define USB_MSC_PRODUCT_ID_LEN	(16)
#define USB_MSC_PRODUCT_ID		"SBM005SH microSD"

#endif /* __SH_ANDROID_USB_DESC_STR_BEC_H */
