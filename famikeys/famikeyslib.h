#ifndef FAMIKEYSLIB_H
#define FAMIKEYSLIB_H

#define FK_PRESSED      0x80
#define FK_RELEASED     0x00

#define FK_F8           0x00
#define FK_RETURN       0x01
#define FK_LT_BRACKET   0x02
#define FK_RT_BRACKET   0x03
#define FK_KANA         0x04
#define FK_RT_SHIFT     0x05
#define FK_YEN          0x06
#define FK_STOP         0x07
#define FK_F7           0x08
#define FK_AT           0x09
#define FK_COLON        0x0A
#define FK_SEMICOLON    0x0B
#define FK_CODA         0x0C
#define FK_SLASH        0x0D
#define FK_HYPHEN       0x0E
#define FK_CARET        0x0F
#define FK_F6           0x10
#define FK_O            0x11
#define FK_L            0x12
#define FK_K            0x13
#define FK_PERIOD       0x14
#define FK_COMMA        0x15
#define FK_P            0x16
#define FK_0            0x17
#define FK_F5           0x18
#define FK_I            0x19
#define FK_U            0x1A
#define FK_J            0x1B
#define FK_M            0x1C
#define FK_N            0x1D
#define FK_9            0x1E
#define FK_8            0x1F
#define FK_F4           0x20
#define FK_Y            0x21
#define FK_G            0x22
#define FK_H            0x23
#define FK_B            0x24
#define FK_V            0x25
#define FK_7            0x26
#define FK_6            0x27
#define FK_F3           0x28
#define FK_T            0x29
#define FK_R            0x2A
#define FK_D            0x2B
#define FK_F            0x2C
#define FK_C            0x2D
#define FK_5            0x2E
#define FK_4            0x2F
#define FK_F2           0x30
#define FK_W            0x31
#define FK_S            0x32
#define FK_A            0x33
#define FK_X            0x34
#define FK_Z            0x35
#define FK_E            0x36
#define FK_3            0x37
#define FK_F1           0x38
#define FK_ESC          0x39
#define FK_Q            0x3A
#define FK_CTR          0x3B
#define FK_LT_SHIFT     0x3C
#define FK_GRPH         0x3D
#define FK_1            0x3E
#define FK_2            0x3F
#define FK_CLR_HOME     0x40
#define FK_UP_ARROW     0x41
#define FK_RT_ARROW     0x42
#define FK_LT_ARROW     0x43
#define FK_DN_ARROW     0x44
#define FK_SPACE        0x45
#define FK_DEL          0x46
#define FK_INS          0x47

#define FK_STATE_SIZE   9
#define FK_BUFFER_SIZE  16
#define FK_WRITE_DELAY  10

void fk_start_input();
boolean fk_connected();
void fk_read_state(unsigned char * state);
unsigned char fk_read_packet();

void fk_start_output();
void fk_write_state(unsigned char * state);
void fk_write_packet(unsigned char packet);
void fk_delay(unsigned long ms);
void fk_flush();

const char * fk_key_name(unsigned char key);
char fk_key_to_ascii(unsigned char key, boolean shift);
unsigned char fk_key_to_usb(unsigned char key, boolean alt);
unsigned char fk_key_from_usb(unsigned char usb);

#endif
