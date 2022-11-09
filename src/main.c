// SPDX-License-Identifier: GPL-2.0-only
/*
 * Atmel AVR Firmware for controlling relay board over USB
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

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "usbconfig.h"
#include "usbdrv.h"

/* ---------------------- CUSTOMIZABLE RELAY MAPPING ----------------------- */

/* Change RELAY_PORT and RELAY_BIT according to your targeted hardware */
/* Given example below, first relay is mapped on pin PC0 (atmega8)     */
uint8_t *OUPTUT_REG[RELAY_COUNT] = {&DDRC, &DDRC, &DDRC, &DDRC};
uint8_t *RELAY_PORT[RELAY_COUNT] = {&PORTC, &PORTC, &PORTC, &PORTC};
int RELAY_BIT[RELAY_COUNT] = {0, 1, 2, 3};

/* Initial Serial Number for your board */
uchar INITIAL_SERIAL_NUMBER[SERIAL_NUMBER_LEN] = "UNSET\0";

/* ----------------------------- USB HID SETUP ----------------------------- */

int RELAY_STATUS_BIT_MASK = 0;
uchar SERIAL_NUMBER_READ_FROM_EEPROM = 0;
uchar SERIAL_NUMBER[SERIAL_NUMBER_LEN];
uint8_t SN_DESCRIPTOR_STRING[16];

PROGMEM const char usbHidReportDescriptor[22] = {
    /* USB HID report descriptor */
    0x06, 0x00, 0xff, //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,       //   USAGE (Vendor Usage 1)
    0xa1, 0x01,       //   COLLECTION (Application)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,       //   REPORT_SIZE (8)
    0x95, 0x08,       //   REPORT_COUNT (1)
    0x09, 0x00,       //   USAGE (Undefined)
    0xb2, 0x02, 0x01, //   FEATURE (Data,Var,Abs,Buf)
    0xc0              //   END_COLLECTION
};

/* This function is called when the driver receives a SETUP transaction from
 * the host which is not answered by the driver itself (in practice: class and
 * vendor requests). All control transfers start with a SETUP transaction where
 * the host communicates the parameters of the following (optional) data
 * transfer. The SETUP data is available in the 'data' parameter which can
 * (and should) be casted to 'usbRequest_t *' for a more user-friendly access
 * to parameters.
 */
usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (void *)data;

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    if ((rq->bRequest == USBRQ_HID_GET_REPORT) ||
        (rq->bRequest == USBRQ_HID_SET_REPORT)) {
      return 0xFF;
    }
  }

  return 0;
}

/* ----------------- RELATED TO CONFIGURABLE SERIAL NUMBER ----------------- */

/* Called when the host requests serial number change.
 * Serial number is stored in eeprom and variable serial_number updated
 */
void update_serial_number(uchar *serial_number, uchar *new_serial_number,
                          uchar len) {
  uchar i;
  eeprom_write_block(new_serial_number, (void *)0x00, len);

  for (i = 0; i < SERIAL_NUMBER_LEN; i++) {
    serial_number[i] = new_serial_number[i];
  }
}

/* Called to retrieve serial number from eeprom
 * If serial number is not stored, default value INITIAL_SERIAL_NUMBER is
 * written in eeprom and stored in serial_number variable
 */
void fetch_serial_number(uchar *serial_number, uchar serial_number_read) {
  if (!SERIAL_NUMBER_READ_FROM_EEPROM) {
    eeprom_read_block(SERIAL_NUMBER, 0x00, SERIAL_NUMBER_LEN);
    if (serial_number[0] == 0xff) {
      update_serial_number(serial_number, INITIAL_SERIAL_NUMBER,
                           SERIAL_NUMBER_LEN);
    }
    serial_number_read = 1;
  }
}

/* Provide USB descriptors at runtime. Similar to usbFunctionSetup() above,
 * but it is called only to request USB descriptor data
 */
usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq) {
  fetch_serial_number(SERIAL_NUMBER, SERIAL_NUMBER_READ_FROM_EEPROM);

  SN_DESCRIPTOR_STRING[0] = (2 * SERIAL_NUMBER_LEN) + 2;
  SN_DESCRIPTOR_STRING[1] = (3);
  uint8_t xx;
  for (xx = 0; xx < SERIAL_NUMBER_LEN; xx++) {
    SN_DESCRIPTOR_STRING[(xx * 2) + 2] = SERIAL_NUMBER[xx];
    SN_DESCRIPTOR_STRING[(xx * 2) + 3] = 0;
  }

  usbMsgPtr = (usbMsgPtr_t)SN_DESCRIPTOR_STRING;
  return SN_DESCRIPTOR_STRING[0];
}

/* Return relay state bit mask (1 == NC) */
int getRelayStatus(void) {
  RELAY_STATUS_BIT_MASK = 0;

  for (int i = 0; i < RELAY_COUNT; i++) {
    if (*RELAY_PORT[i] & _BV(RELAY_BIT[i])) {
      RELAY_STATUS_BIT_MASK += (1 << i);
    }
  }
  return RELAY_STATUS_BIT_MASK;
}

/* Called when the host requests a chunk of data from the device. */
uchar usbFunctionRead(uchar *data, uchar len) {
  uchar i;

  if (len != 0) {
    fetch_serial_number(SERIAL_NUMBER, SERIAL_NUMBER_READ_FROM_EEPROM);
    for (i = 0; i < SERIAL_NUMBER_LEN; i++) {
      data[i] = SERIAL_NUMBER[i];
    }
    data[7] = getRelayStatus();
    return len;
  }

  return 1;
}

/* ---------------------------- COMMAND HANDLER ---------------------------- */

/* Called when the host sends a chunk of data to the device.*/
uchar usbFunctionWrite(uchar *data, uchar len) {
  /* data[0] : command code received from host
   * data[1] : relay number to operate or new serial number
   * When data[1] > relay number, command is applied to all relays
   */
  if (data[0] == CMD_ON) {
    if (data[1] > RELAY_COUNT) {
      for (int j = 0; j < RELAY_COUNT; j++) {
        *RELAY_PORT[j] |= _BV(RELAY_BIT[j]);
      }
      return len;
    }
    *RELAY_PORT[data[1] - 1] |= _BV(RELAY_BIT[data[1] - 1]);
  } else if (data[0] == CMD_OFF) {
    if (data[1] > RELAY_COUNT) {
      for (int j = 0; j < RELAY_COUNT; j++) {
        *RELAY_PORT[j] &= ~_BV(RELAY_BIT[j]);
      }
      return len;
    }
    *RELAY_PORT[data[1] - 1] &= ~_BV(RELAY_BIT[data[1] - 1]);
  } else if (data[0] == CMD_SET_SERIAL) {
    update_serial_number(SERIAL_NUMBER, &data[1], SERIAL_NUMBER_LEN);
  }
  return len;
}

/* ------------------------------- MAIN LOOP ------------------------------- */

int main(void) {
  // enable watchdog timeout to 1 sec
  wdt_enable(WDTO_1S);

  usbInit();

  // enforce_device_enumeration
  // reset during 200 ms
  usbDeviceDisconnect();
  uint8_t i = 0;
  while (i++ < 200) {
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();

  // set relay bit as output
  for (uint8_t j = 0; j < RELAY_COUNT; j++) {
    *OUPTUT_REG[j] |= _BV(RELAY_BIT[j]);
  }

  // enable interrupts
  sei();

  while (1) {
    wdt_reset();
    usbPoll();
  }
}