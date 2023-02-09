#include <Arduino.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "famikeysconfig.h"
#include "famikeyslib.h"
#include "famikeyswrite.h"
#include "usbkeysconfig.h"
#include "usbkeyslib.h"

#ifdef USBK_USE_HOST_SHIELD
#include <hidboot.h>
#include <usbhub.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#endif

void setup() {
  pinMode(FK_SW_HOST_CLIENT_MODE, INPUT_PULLUP);
  pinMode(FK_SW_SERIAL_USB_MODE, INPUT_PULLUP);
  if (digitalRead(FK_SW_HOST_CLIENT_MODE)) {
    if (digitalRead(FK_SW_SERIAL_USB_MODE)) {
      pinMode(FK_SW_SETTING_1, INPUT_PULLUP);
      pinMode(FK_SW_SETTING_2, INPUT_PULLUP);
      if (digitalRead(FK_SW_SETTING_1)) {
        if (digitalRead(FK_SW_SETTING_2)) {
          serial_input_debug();
        } else {
          serial_input_ascii();
        }
      } else {
        if (digitalRead(FK_SW_SETTING_2)) {
          serial_input_hex();
        } else {
          serial_input_raw();
        }
      }
    } else {
      usb_input_mode();
    }
  } else {
    if (digitalRead(FK_SW_SERIAL_USB_MODE)) {
      pinMode(FK_SW_SETTING_1, INPUT_PULLUP);
      pinMode(FK_SW_SETTING_2, INPUT_PULLUP);
      if (digitalRead(FK_SW_SETTING_1)) {
        if (digitalRead(FK_SW_SETTING_2)) {
          serial_output_debug();
        } else {
          serial_output_ascii();
        }
      } else {
        if (digitalRead(FK_SW_SETTING_2)) {
          serial_output_hex();
        } else {
          serial_output_raw();
        }
      }
    } else {
      usb_output_mode();
    }
  }
}

void loop() {}

void serial_input_debug() {
  unsigned char packet;
  unsigned char pressed;
  fk_start_input();
  Serial.begin(9600);
  while (!fk_connected());
  Serial.println("READY");
  while (true) {
    packet = fk_read_packet();
    pressed = packet & 0x80;
    if ((packet &= 0x7F) < 0x7F) {
      Serial.print(pressed ? "PRESSED " : "RELEASED ");
      Serial.print(packet, HEX);
      Serial.print(" ");
      Serial.println(fk_key_name(packet));
    }
  }
}

void serial_input_ascii() {
  unsigned char packet;
  unsigned char pressed;
  unsigned char mods = 0;
  fk_start_input();
  Serial.begin(9600);
  while (!fk_connected());
  while (true) {
    packet = fk_read_packet();
    pressed = packet & 0x80;
    if ((packet &= 0x7F) < 0x7F) {
      if (packet == FK_GRPH) {
        if (pressed) mods |=  0x01;
        else         mods &=~ 0x01;
      } else if (packet == FK_CTR) {
        if (pressed) mods |=  0x02;
        else         mods &=~ 0x02;
      } else if (packet == FK_LT_SHIFT) {
        if (pressed) mods |=  0x04;
        else         mods &=~ 0x04;
      } else if (packet == FK_RT_SHIFT) {
        if (pressed) mods |=  0x08;
        else         mods &=~ 0x08;
      } else if (pressed) {
        if ((packet = fk_key_to_ascii(packet, mods & 0x0C))) {
          if (mods & 0x02) {
            if (packet >= 0x40) packet &= 0x1F;
            if (packet == 0x3F) packet  = 0x7F;
          }
          if (mods & 0x01) {
            packet |= 0x80;
          }
          Serial.write(packet);
        }
      }
    }
  }
}

void serial_input_hex() {
  unsigned char packet;
  fk_start_input();
  Serial.begin(9600);
  while (!fk_connected());
  while (true) {
    packet = fk_read_packet();
    if ((packet & 0x7F) < 0x7F) {
      Serial.print(packet, HEX);
    }
  }
}

void serial_input_raw() {
  unsigned char packet;
  fk_start_input();
  Serial.begin(9600);
  while (!fk_connected());
  while (true) {
    packet = fk_read_packet();
    if ((packet & 0x7F) < 0x7F) {
      Serial.write(packet);
    }
  }
}

void usb_input_mode() {
  unsigned char packet;
  unsigned char pressed;
  unsigned char grphKey;
  unsigned char kanaKey;
  unsigned char escKey;
  unsigned char stopKey;
  unsigned char insKey;
  unsigned char delKey;
  unsigned char fnKey = FK_CODA;
  boolean fnDown = false;
  pinMode(FK_LED, OUTPUT);
  pinMode(FK_SW_SETTING_1, INPUT_PULLUP);
  pinMode(FK_SW_SETTING_2, INPUT_PULLUP);
  digitalWrite(FK_LED, LOW);
  if (digitalRead(FK_SW_SETTING_1)) {
    grphKey = USBK_LT_ALT;
    kanaKey = USBK_RT_META;
  } else {
    grphKey = USBK_LT_META;
    kanaKey = USBK_RT_ALT;
  }
  if (digitalRead(FK_SW_SETTING_2)) {
    // Match semantics
    escKey = USBK_ESC;
    stopKey = USBK_PAUSE;
    insKey = USBK_TAB;
    delKey = USBK_BACKSPACE;
  } else {
    // Match position
    escKey = USBK_TAB;
    stopKey = USBK_BACKSPACE;
    insKey = USBK_INS;
    delKey = USBK_DEL;
  }
  fk_start_input();
  usbk_start_output();
  while (!fk_connected());
  while (true) {
    packet = fk_read_packet();
    pressed = packet & 0x80;
    if ((packet &= 0x7F) < 0x7F) {
      if (packet == fnKey) {
        fnDown = pressed;
        digitalWrite(FK_LED, pressed);
        continue;
      } else if (packet == FK_GRPH) {
        packet = grphKey;
      } else if (packet == FK_KANA) {
        packet = kanaKey;
      } else if (fnDown) {
        packet = fk_key_to_usb(packet, 1);
        if (!packet) continue;
      } else if (packet == FK_ESC) {
        packet = escKey;
      } else if (packet == FK_STOP) {
        packet = stopKey;
      } else if (packet == FK_INS) {
        packet = insKey;
      } else if (packet == FK_DEL) {
        packet = delKey;
      } else {
        packet = fk_key_to_usb(packet, 0);
        if (!packet) continue;
      }
      if (pressed) usbk_key_pressed(packet);
      else usbk_key_released(packet);
    }
  }
}

#define SERIAL_BUFFER_SIZE 80

void serial_output_debug() {
  unsigned char buf[SERIAL_BUFFER_SIZE];
  int ptr = 0;
  int ch;
  fk_start_output();
  Serial.begin(9600);
  Serial.println("READY");
  while (true) {
    if ((ch = Serial.read()) > 0) {
      if (ch == '\n' || ch == '\r') {
        buf[ptr] = 0;
        if      (buf[0] == '!') Serial.println(free_ram());
        else if (buf[0] == '#') /* "comment" / no-op */;
        else if (buf[0] == '$') fk_write_hex  (&buf[1]);
        else if (buf[0] == '&') fk_write_coded(&buf[1]);
        else if (buf[0] == '@') fk_write_ascii(&buf[1]);
        else                    fk_write_ascii(&buf[0]);
        ptr = 0;
      } else if (ptr < (SERIAL_BUFFER_SIZE - 1)) {
        buf[ptr] = ch;
        ptr++;
      }
    }
    fk_flush();
  }
}

void serial_output_ascii() {
  unsigned char buf[2];
  int ch;
  fk_start_output();
  Serial.begin(9600);
  while (true) {
    if ((ch = Serial.read()) > 0) {
      buf[0] = ch;
      buf[1] = 0;
      fk_write_ascii(buf);
    }
    fk_flush();
  }
}

static unsigned char digit_value(unsigned char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
  return 0xFF;
}

void serial_output_hex() {
  boolean inValue = false;
  unsigned char value;
  unsigned char ch;
  fk_start_output();
  Serial.begin(9600);
  while (true) {
    if ((ch = digit_value(Serial.read())) < 16) {
      if (inValue) {
        inValue = false;
        fk_write_packet(value | ch);
        fk_delay(FK_WRITE_DELAY);
        value = 0;
      } else {
        inValue = true;
        value = ch << 4;
      }
    }
    fk_flush();
  }
}

void serial_output_raw() {
  int ch;
  fk_start_output();
  Serial.begin(9600);
  while (true) {
    if ((ch = Serial.read()) >= 0) {
      fk_write_packet(ch);
      fk_delay(FK_WRITE_DELAY);
    }
    fk_flush();
  }
}

void usb_output_mode() {
  unsigned char packet;
  unsigned char shifted;
  unsigned char modState = 0;
  unsigned char altKey, metaKey, tabKey, bkspKey;
  unsigned int e;
  pinMode(FK_SW_SETTING_1, INPUT_PULLUP);
  pinMode(FK_SW_SETTING_2, INPUT_PULLUP);
  if (digitalRead(FK_SW_SETTING_1)) {
    altKey = FK_GRPH;
    metaKey = FK_KANA;
  } else {
    altKey = FK_KANA;
    metaKey = FK_GRPH;
  }
  if (digitalRead(FK_SW_SETTING_2)) {
    // Match semantics
    tabKey = FK_INS;
    bkspKey = FK_DEL;
  } else {
    // Match position
    tabKey = FK_ESC;
    bkspKey = FK_STOP;
  }
  fk_start_output();
  usbk_start_input();
  while (true) {
    if ((e = usbk_read_keys())) {
      if (e & USBK_EVENT_MOD_STATE) {
        modState = e;
      } else if (e & USBK_EVENT_MOD_DOWN) {
        if (e & USBK_MOD_LT_SHIFT) fk_write_packet(FK_LT_SHIFT | FK_PRESSED);
        if (e & USBK_MOD_LT_CTRL ) fk_write_packet(FK_CTR      | FK_PRESSED);
        if (e & USBK_MOD_LT_ALT  ) fk_write_packet(altKey      | FK_PRESSED);
        if (e & USBK_MOD_LT_META ) fk_write_packet(metaKey     | FK_PRESSED);
        if (e & USBK_MOD_RT_SHIFT) fk_write_packet(FK_RT_SHIFT | FK_PRESSED);
        if (e & USBK_MOD_RT_CTRL ) fk_write_packet(FK_CTR      | FK_PRESSED);
        if (e & USBK_MOD_RT_ALT  ) fk_write_packet(altKey      | FK_PRESSED);
        if (e & USBK_MOD_RT_META ) fk_write_packet(metaKey     | FK_PRESSED);
      } else if (e & USBK_EVENT_MOD_UP) {
        if (e & USBK_MOD_LT_SHIFT) fk_write_packet(FK_LT_SHIFT | FK_RELEASED);
        if (e & USBK_MOD_LT_CTRL ) fk_write_packet(FK_CTR      | FK_RELEASED);
        if (e & USBK_MOD_LT_ALT  ) fk_write_packet(altKey      | FK_RELEASED);
        if (e & USBK_MOD_LT_META ) fk_write_packet(metaKey     | FK_RELEASED);
        if (e & USBK_MOD_RT_SHIFT) fk_write_packet(FK_RT_SHIFT | FK_RELEASED);
        if (e & USBK_MOD_RT_CTRL ) fk_write_packet(FK_CTR      | FK_RELEASED);
        if (e & USBK_MOD_RT_ALT  ) fk_write_packet(altKey      | FK_RELEASED);
        if (e & USBK_MOD_RT_META ) fk_write_packet(metaKey     | FK_RELEASED);
      } else if (e & USBK_EVENT_KEY_DOWN) {
        packet = e;
        if (packet == USBK_TAB) {
          fk_write_packet(tabKey | FK_PRESSED);
        } else if (packet == USBK_BACKSPACE) {
          fk_write_packet(bkspKey | FK_PRESSED);
        } else {
          packet = fk_key_from_usb(packet);
          shifted = packet & 0x80;
          if ((packet &= 0x7F) < 0x7F) {
            if (shifted) {
              fk_write_packet(FK_LT_SHIFT | FK_PRESSED);
              fk_delay(FK_WRITE_DELAY);
            }
            fk_write_packet(packet | FK_PRESSED);
          }
        }
      } else if (e & USBK_EVENT_KEY_UP) {
        packet = e;
        if (packet == USBK_TAB) {
          fk_write_packet(tabKey | FK_RELEASED);
        } else if (packet == USBK_BACKSPACE) {
          fk_write_packet(bkspKey | FK_RELEASED);
        } else {
          packet = fk_key_from_usb(packet);
          shifted = packet & 0x80;
          if ((packet &= 0x7F) < 0x7F) {
            fk_write_packet(packet | FK_RELEASED);
            if (shifted) {
              fk_delay(FK_WRITE_DELAY);
              fk_write_packet(FK_LT_SHIFT | FK_RELEASED);
            }
          }
        }
      }
    }
    fk_flush();
  }
}

int free_ram() {
  extern int __heap_start, * __brkval; int v;
  return (int)&v - ((__brkval == 0) ? (int)&__heap_start : (int)__brkval);
}
