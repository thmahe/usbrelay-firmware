// SPDX-License-Identifier: GPL-2.0-only
/*
 * v-usb & usbrelay-firmware configuration header
 *
 * Copyright (c) 2022 Thomas Mahe <contact@tmahe.dev>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#ifndef _USBCONFIG_H

#define _USBCONFIG_H

/* ---------------------------- Relay Config ---------------------------- */

#define RELAY_COUNT     4
#define CMD_ON          0xff
#define CMD_OFF         0xfd
#define CMD_SET_SERIAL  0xfa

#define SERIAL_NUMBER_LEN   6

/* ---------------------------- Hardware Config ---------------------------- */


#define USB_CFG_IOPORTNAME      B
#define USB_CFG_DMINUS_BIT      0
#define USB_CFG_DPLUS_BIT       1
#define USB_CFG_CLOCK_KHZ       F_CPU / 1000
#define USB_CFG_CHECK_CRC       0

/* --------------------------- Functional Range ---------------------------- */

#define USB_CFG_HAVE_INTRIN_ENDPOINT    1
#define USB_CFG_HAVE_INTRIN_ENDPOINT3   0
#define USB_CFG_EP3_NUMBER              3
#define USB_CFG_IMPLEMENT_HALT          0

#define USB_CFG_SUPPRESS_INTR_CODE          0
#define USB_CFG_INTR_POLL_INTERVAL          5
#define USB_CFG_IS_SELF_POWERED             0
#define USB_CFG_MAX_BUS_POWER               500
#define USB_CFG_IMPLEMENT_FN_WRITE          1
#define USB_CFG_IMPLEMENT_FN_READ           1
#define USB_CFG_IMPLEMENT_FN_WRITEOUT       0
#define USB_CFG_HAVE_FLOWCONTROL            0
#define USB_CFG_DRIVER_FLASH_PAGE           0
#define USB_CFG_LONG_TRANSFERS              0
#define USB_COUNT_SOF                       0
#define USB_CFG_CHECK_DATA_TOGGLING         0
#define USB_CFG_HAVE_MEASURE_FRAME_LENGTH   0
#define USB_USE_FAST_CRC                    0

/* -------------------------- Device Description --------------------------- */

#define USB_CFG_VENDOR_ID           0xc0, 0x16
#define USB_CFG_DEVICE_ID           0xdf, 0x05
#define USB_CFG_DEVICE_VERSION      0x00, 0x01
#define USB_CFG_VENDOR_NAME         't', 'h', 'm', 'a', 'h', 'e', '.', 'd', 'e', 'v'
#define USB_CFG_VENDOR_NAME_LEN     10
#define USB_CFG_DEVICE_NAME         'U','S', 'B', 'R', 'e', 'l', 'a', 'y', 0x30 + RELAY_COUNT
#define USB_CFG_DEVICE_NAME_LEN     9

#define USB_CFG_DEVICE_CLASS        0
#define USB_CFG_DEVICE_SUBCLASS     0
#define USB_CFG_INTERFACE_CLASS     3
#define USB_CFG_INTERFACE_SUBCLASS  0
#define USB_CFG_INTERFACE_PROTOCOL  0
#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    22

#define USB_CFG_DESCR_PROPS_DEVICE                  0
#define USB_CFG_DESCR_PROPS_CONFIGURATION           0
#define USB_CFG_DESCR_PROPS_STRINGS                 0
#define USB_CFG_DESCR_PROPS_STRING_0                0
#define USB_CFG_DESCR_PROPS_STRING_VENDOR           0
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT          0
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    (USB_PROP_IS_DYNAMIC | USB_PROP_IS_RAM | USB_PROP_LENGTH(10))
#define USB_CFG_DESCR_PROPS_HID                     0
#define USB_CFG_DESCR_PROPS_HID_REPORT              0
#define USB_CFG_DESCR_PROPS_UNKNOWN                 0

#endif