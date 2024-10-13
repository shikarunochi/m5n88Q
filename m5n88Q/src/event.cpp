/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/
#ifdef _CORES3
#include <SD.h>
#include <SPIFFS.h>
#include <M5Unified.h>
#include <Wire.h>
//USBキーボード
//https://github.com/touchgadget/esp32-usb-host-demos
#include <elapsedMillis.h>
#include <usb/usb_host.h>
#include "usbhhelp.hpp"
bool isKeyboard = false;
bool isKeyboardReady = false;
uint8_t KeyboardInterval;
bool isKeyboardPolling = false;
elapsedMillis KeyboardTimer;

const size_t KEYBOARD_IN_BUFFER_SIZE = 8;
usb_transfer_t *KeyboardIn = NULL;
void check_interface_desc_boot_keyboard(const void *p);
void prepare_endpoint(const void *p);
void show_config_desc_full(const usb_config_desc_t *config_desc)
{
   // Full decode of config desc.
  const uint8_t *p = &config_desc->val[0];
  static uint8_t USB_Class = 0;
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i+=bLength, p+=bLength) {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength) {
      const uint8_t bDescriptorType = *(p + 1);
      switch (bDescriptorType) {
        case USB_B_DESCRIPTOR_TYPE_DEVICE:
          ESP_LOGI("", "USB Device Descriptor should not appear in config");
          break;
        case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
          //show_config_desc(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_STRING:
          ESP_LOGI("", "USB string desc TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_INTERFACE:
          //USB_Class = show_interface_desc(p);
          check_interface_desc_boot_keyboard(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
          //show_endpoint_desc(p);
          if (isKeyboard && KeyboardIn == NULL) prepare_endpoint(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
          // Should not be config config?
          ESP_LOGI("", "USB device qual desc TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
          // Should not be config config?
          ESP_LOGI("", "USB Other Speed TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_INTERFACE_POWER:
          // Should not be config config?
          ESP_LOGI("", "USB Interface Power TBD");
          break;
        case 0x21:
          if (USB_Class == USB_CLASS_HID) {
            //show_hid_desc(p);
          }
          break;
        default:
          ESP_LOGI("", "Unknown USB Descriptor Type: 0x%x", bDescriptorType);
          break;
      }
    }
    else {
      ESP_LOGI("", "USB Descriptor invalid");
      return;
    }
  }
}
#elif defined _ATOMS3R
#include <M5GFX.h>
#include <SPIFFS.h>
#include <Wire.h>
#include "Button.h"
#include "lgfx.h"
//USBキーボード
//https://github.com/touchgadget/esp32-usb-host-demos
#include <elapsedMillis.h>
#include <usb/usb_host.h>
#include "usbhhelp.hpp"
bool isKeyboard = false;
bool isKeyboardReady = false;
uint8_t KeyboardInterval;
bool isKeyboardPolling = false;
elapsedMillis KeyboardTimer;

extern Button extBtn;

const size_t KEYBOARD_IN_BUFFER_SIZE = 8;
usb_transfer_t *KeyboardIn = NULL;

void check_interface_desc_boot_keyboard(const void *p);
void prepare_endpoint(const void *p);
void show_config_desc_full(const usb_config_desc_t *config_desc)
{
   // Full decode of config desc.
  const uint8_t *p = &config_desc->val[0];
  static uint8_t USB_Class = 0;
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i+=bLength, p+=bLength) {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength) {
      const uint8_t bDescriptorType = *(p + 1);
      switch (bDescriptorType) {
        case USB_B_DESCRIPTOR_TYPE_DEVICE:
          ESP_LOGI("", "USB Device Descriptor should not appear in config");
          break;
        case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
          //show_config_desc(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_STRING:
          ESP_LOGI("", "USB string desc TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_INTERFACE:
          //USB_Class = show_interface_desc(p);
          check_interface_desc_boot_keyboard(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
          //show_endpoint_desc(p);
          if (isKeyboard && KeyboardIn == NULL) prepare_endpoint(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
          // Should not be config config?
          ESP_LOGI("", "USB device qual desc TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
          // Should not be config config?
          ESP_LOGI("", "USB Other Speed TBD");
          break;
        case USB_B_DESCRIPTOR_TYPE_INTERFACE_POWER:
          // Should not be config config?
          ESP_LOGI("", "USB Interface Power TBD");
          break;
        case 0x21:
          if (USB_Class == USB_CLASS_HID) {
            //show_hid_desc(p);
          }
          break;
        default:
          ESP_LOGI("", "Unknown USB Descriptor Type: 0x%x", bDescriptorType);
          break;
      }
    }
    else {
      ESP_LOGI("", "USB Descriptor invalid");
      return;
    }
  }
}
#else
#include <M5Stack.h>
#endif
#include "quasi88.h"
#include "device.h"
#include "initval.h"

#include "event.h"
#include "file-op.h"

extern "C"
{
#include "keyboard.h"
#include "drive.h"
#include "graph.h"
}

/*----------------------------------------------------------------------
 * xxx → KEY88_xxx 変換テーブル (デフォルト)
 *----------------------------------------------------------------------*/

static const int keysym2key88[] =
    {
        0, /*	UNKNOWN		= 0,	*/
        0, 0, 0, 0, 0, 0, 0,
        KEY88_INS_DEL, /*	BACKSPACE		= 8,	*/
        KEY88_TAB,     /*	TAB		= 9,	*/
        0, 0,
        KEY88_HOME,    /*	CLEAR		= 12,	*/
        KEY88_RETURNL, /*	RETURN		= 13,	*/
        0, 0, 0, 0, 0,
        KEY88_STOP, /*	PAUSE		= 19,	*/
        0, 0, 0, 0, 0, 0, 0,
        KEY88_ESC, /*	ESCAPE		= 27,	*/
        0, 0, 0, 0,

        KEY88_SPACE,        /*	SPACE		= 32,	*/
        KEY88_EXCLAM,       /*	EXCLAIM		= 33,	*/
        KEY88_QUOTEDBL,     /*	QUOTEDBL		= 34,	*/
        KEY88_NUMBERSIGN,   /*	HASH		= 35,	*/
        KEY88_DOLLAR,       /*	DOLLAR		= 36,	*/
        KEY88_PERCENT,      /*					*/
        KEY88_AMPERSAND,    /*	AMPERSAND		= 38,	*/
        KEY88_APOSTROPHE,   /*	QUOTE		= 39,	*/
        KEY88_PARENLEFT,    /*	LEFTPAREN		= 40,	*/
        KEY88_PARENRIGHT,   /*	RIGHTPAREN		= 41,	*/
        KEY88_ASTERISK,     /*	ASTERISK		= 42,	*/
        KEY88_PLUS,         /*	PLUS		= 43,	*/
        KEY88_COMMA,        /*	COMMA		= 44,	*/
        KEY88_MINUS,        /*	MINUS		= 45,	*/
        KEY88_PERIOD,       /*	PERIOD		= 46,	*/
        KEY88_SLASH,        /*	SLASH		= 47,	*/
        KEY88_0,            /*	0			= 48,	*/
        KEY88_1,            /*	1			= 49,	*/
        KEY88_2,            /*	2			= 50,	*/
        KEY88_3,            /*	3			= 51,	*/
        KEY88_4,            /*	4			= 52,	*/
        KEY88_5,            /*	5			= 53,	*/
        KEY88_6,            /*	6			= 54,	*/
        KEY88_7,            /*	7			= 55,	*/
        KEY88_8,            /*	8			= 56,	*/
        KEY88_9,            /*	9			= 57,	*/
        KEY88_COLON,        /*	COLON		= 58,	*/
        KEY88_SEMICOLON,    /*	SEMICOLON		= 59,	*/
        KEY88_LESS,         /*	LESS		= 60,	*/
        KEY88_EQUAL,        /*	EQUALS		= 61,	*/
        KEY88_GREATER,      /*	GREATER		= 62,	*/
        KEY88_QUESTION,     /*	QUESTION		= 63,	*/
        KEY88_AT,           /*	AT			= 64,	*/
        KEY88_A,            /*					*/
        KEY88_B,            /*					*/
        KEY88_C,            /*					*/
        KEY88_D,            /*					*/
        KEY88_E,            /*					*/
        KEY88_F,            /*					*/
        KEY88_G,            /*					*/
        KEY88_H,            /*					*/
        KEY88_I,            /*					*/
        KEY88_J,            /*					*/
        KEY88_K,            /*					*/
        KEY88_L,            /*					*/
        KEY88_M,            /*					*/
        KEY88_N,            /*					*/
        KEY88_O,            /*					*/
        KEY88_P,            /*					*/
        KEY88_Q,            /*					*/
        KEY88_R,            /*					*/
        KEY88_S,            /*					*/
        KEY88_T,            /*					*/
        KEY88_U,            /*					*/
        KEY88_V,            /*					*/
        KEY88_W,            /*					*/
        KEY88_X,            /*					*/
        KEY88_Y,            /*					*/
        KEY88_Z,            /*					*/
        KEY88_BRACKETLEFT,  /*	LEFTBRACKET	= 91,	*/
        KEY88_YEN,          /*	BACKSLASH		= 92,	*/
        KEY88_BRACKETRIGHT, /*	RIGHTBRACKET	= 93,	*/
        KEY88_CARET,        /*	CARET		= 94,	*/
        KEY88_UNDERSCORE,   /*	UNDERSCORE		= 95,	*/
        KEY88_BACKQUOTE,    /*	BACKQUOTE		= 96,	*/
        KEY88_a,            /*	a			= 97,	*/
        KEY88_b,            /*	b			= 98,	*/
        KEY88_c,            /*	c			= 99,	*/
        KEY88_d,            /*	d			= 100,	*/
        KEY88_e,            /*	e			= 101,	*/
        KEY88_f,            /*	f			= 102,	*/
        KEY88_g,            /*	g			= 103,	*/
        KEY88_h,            /*	h			= 104,	*/
        KEY88_i,            /*	i			= 105,	*/
        KEY88_j,            /*	j			= 106,	*/
        KEY88_k,            /*	k			= 107,	*/
        KEY88_l,            /*	l			= 108,	*/
        KEY88_m,            /*	m			= 109,	*/
        KEY88_n,            /*	n			= 110,	*/
        KEY88_o,            /*	o			= 111,	*/
        KEY88_p,            /*	p			= 112,	*/
        KEY88_q,            /*	q			= 113,	*/
        KEY88_r,            /*	r			= 114,	*/
        KEY88_s,            /*	s			= 115,	*/
        KEY88_t,            /*	t			= 116,	*/
        KEY88_u,            /*	u			= 117,	*/
        KEY88_v,            /*	v			= 118,	*/
        KEY88_w,            /*	w			= 119,	*/
        KEY88_x,            /*	x			= 120,	*/
        KEY88_y,            /*	y			= 121,	*/
        KEY88_z,            /*	z			= 122,	*/
        KEY88_BRACELEFT,    /*					*/
        KEY88_BAR,          /*					*/
        KEY88_BRACERIGHT,   /*					*/
        KEY88_TILDE,        /*					*/
        KEY88_DEL,          /*	DELETE		= 127,	*/

        0, 0, 0, 0, 0, 0, 0, 0, /*	WORLD_0		= 160,	*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
                                /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*		:			*/
        0, 0, 0, 0, 0, 0, 0, 0, /*	WORLD_95		= 255,	*/

        KEY88_KP_0,        /*	KP0		= 256,	*/
        KEY88_KP_1,        /*	KP1		= 257,	*/
        KEY88_KP_2,        /*	KP2		= 258,	*/
        KEY88_KP_3,        /*	KP3		= 259,	*/
        KEY88_KP_4,        /*	KP4		= 260,	*/
        KEY88_KP_5,        /*	KP5		= 261,	*/
        KEY88_KP_6,        /*	KP6		= 262,	*/
        KEY88_KP_7,        /*	KP7		= 263,	*/
        KEY88_KP_8,        /*	KP8		= 264,	*/
        KEY88_KP_9,        /*	KP9		= 265,	*/
        KEY88_KP_PERIOD,   /*	KP_PERIOD		= 266,	*/
        KEY88_KP_DIVIDE,   /*	KP_DIVIDE		= 267,	*/
        KEY88_KP_MULTIPLY, /*	KP_MULTIPLY	= 268,	*/
        KEY88_KP_SUB,      /*	KP_MINUS		= 269,	*/
        KEY88_KP_ADD,      /*	KP_PLUS		= 270,	*/
        KEY88_RETURNR,     /*	KP_ENTER		= 271,	*/
        KEY88_KP_EQUAL,    /*	KP_EQUALS		= 272,	*/
        KEY88_UP,          /*	UP			= 273,	*/
        KEY88_DOWN,        /*	DOWN		= 274,	*/
        KEY88_RIGHT,       /*	RIGHT		= 275,	*/
        KEY88_LEFT,        /*	LEFT		= 276,	*/
        KEY88_INS,         /*	INSERT		= 277,	*/
        KEY88_HOME,        /*	HOME		= 278,	*/
        KEY88_HELP,        /*	END		= 279,	*/
        KEY88_ROLLDOWN,    /*	PAGEUP		= 280,	*/
        KEY88_ROLLUP,      /*	PAGEDOWN		= 281,	*/
        KEY88_F1,          /*	F1			= 282,	*/
        KEY88_F2,          /*	F2			= 283,	*/
        KEY88_F3,          /*	F3			= 284,	*/
        KEY88_F4,          /*	F4			= 285,	*/
        KEY88_F5,          /*	F5			= 286,	*/
        KEY88_F6,          /*	F6			= 287,	*/
        KEY88_F7,          /*	F7			= 288,	*/
        KEY88_F8,          /*	F8			= 289,	*/
        KEY88_F9,          /*	F9			= 290,	*/
        KEY88_F10,         /*	F10		= 291,	*/
        KEY88_F11,         /*	F11		= 292,	*/
        KEY88_F12,         /*	F12		= 293,	*/
        KEY88_F13,         /*	F13		= 294,	*/
        KEY88_F14,         /*	F14		= 295,	*/
        KEY88_F15,         /*	F15		= 296,	*/
        0, 0, 0,
        0,            /*	NUMLOCK		= 300,	*/
        KEY88_CAPS,   /*	CAPSLOCK		= 301,	*/
        KEY88_KANA,   /*	SCROLLOCK		= 302,	*/
        KEY88_SHIFTR, /*	RSHIFT		= 303,	*/
        KEY88_SHIFTL, /*	LSHIFT		= 304,	*/
        KEY88_CTRL,   /*	RCTRL		= 305,	*/
        KEY88_CTRL,   /*	LCTRL		= 306,	*/
        KEY88_GRAPH,  /*	RALT		= 307,	*/
        KEY88_GRAPH,  /*	LALT		= 308,	*/
        KEY88_GRAPH,  /*	RMETA		= 309,	*/
        KEY88_GRAPH,  /*	LMETA		= 310,	*/
        0,            /*	LSUPER		= 311,	*/
        0,            /*	RSUPER		= 312,	*/
        0,            /*	MODE		= 313,	*/
        0,            /*	COMPOSE		= 314,	*/
        KEY88_HELP,   /*	HELP		= 315,	*/
        KEY88_COPY,   /*	PRINT		= 316,	*/
        0,            /*	SYSREQ		= 317,	*/
        KEY88_STOP,   /*	BREAK		= 318,	*/
        0,            /*	MENU		= 319,	*/
        0,            /*	POWER		= 320,	*/
        0,            /*	EURO		= 321,	*/
        0,            /*	UNDO		= 322,	*/
};

static const int withShiftKey[] = {
    0,    //0	0x00	NUL（null文字)
    0,    //1	0x01	SOH（ヘッダ開始）
    0,    //2	0x02	STX（テキスト開始）
    0,    //3	0x03	ETX（テキスト終了）
    0,    //4	0x04	EOT（転送終了）
    0,    //5	0x05	ENQ（照会）
    0,    //6	0x06	ACK（受信確認）
    0,    //7	0x07	BEL（警告）
    0,    //8	0x08	BS（後退）
    0,    //9	0x09	HT（水平タブ）
    0,    //10	0x0a	LF（改行）
    0,    //11	0x0b	VT（垂直タブ）
    0,    //12	0x0c	FF（改頁）
    0,    //13	0x0d	CR（復帰）
    0,    //14	0x0e	SO（シフトアウト）
    0,    //15	0x0f	SI（シフトイン）
    0,    //16	0x10	DLE（データリンクエスケー プ）
    0,    //17	0x11	DC1（装置制御１）
    0,    //18	0x12	DC2（装置制御２）
    0,    //19	0x13	DC3（装置制御３）
    0,    //20	0x14	DC4（装置制御４）
    0,    //21	0x15	NAK（受信失敗）
    0,    //22	0x16	SYN（同期）
    0,    //23	0x17	ETB（転送ブロック終了）
    0,    //24	0x18	CAN（キャンセル）
    0,    //25	0x19	EM（メディア終了）
    0,    //26	0x1a	SUB（置換）
    0,    //27	0x1b	ESC（エスケープ）
    0,    //28	0x1c	FS（フォーム区切り）
    0,    //29	0x1d	GS（グループ区切り）
    0,    //30	0x1e	RS（レコード区切り）
    0,    //31	0x1f	US（ユニット区切り）
    0,    //32	0x20	SPC（空白文字）
    '1',  //33	0x21	!
    '2',  //34	0x22	"
    '3',  //35	0x23	#
    '4',  //36	0x24	$
    '5',  //37	0x25	%
    '6',  //38	0x26	&
    '7',  //39	0x27	'
    '8',  //40	0x28	(
    '9',  //41	0x29	)
    ':',  //42	0x2a	*
    ';',  //43	0x2b	+
    0,    //44	0x2c	,
    0,    //45	0x2d	-
    0,    //46	0x2e	.
    0,    //47	0x2f	/
    0,    //48	0x30	0
    0,    //49	0x31	1
    0,    //50	0x32	2
    0,    //51	0x33	3
    0,    //52	0x34	4
    0,    //53	0x35	5
    0,    //54	0x36	6
    0,    //55	0x37	7
    0,    //56	0x38	8
    0,    //57	0x39	9
    0,    //58	0x3a	:
    0,    //59	0x3b	;
    ',',  //60	0x3c	<
    '-',  //61	0x3d	=
    '.',  //62	0x3e	>
    '/',  //63	0x3f	?
    0,    //64	0x40	@
    'a',  //65	0x41	A
    'b',  //66	0x42	B
    'c',  //67	0x43	C
    'd',  //68	0x44	D
    'e',  //69	0x45	E
    'f',  //70	0x46	F
    'g',  //71	0x47	G
    'h',  //72	0x48	H
    'i',  //73	0x49	I
    'j',  //74	0x4a	J
    'k',  //75	0x4b	K
    'l',  //76	0x4c	L
    'm',  //77	0x4d	M
    'n',  //78	0x4e	N
    'o',  //79	0x4f	O
    'p',  //80	0x50	P
    'q',  //81	0x51	Q
    'r',  //82	0x52	R
    's',  //83	0x53	S
    't',  //84	0x54	T
    'u',  //85	0x55	U
    'v',  //86	0x56	V
    'w',  //87	0x57	W
    'x',  //88	0x58	X
    'y',  //89	0x59	Y
    'x',  //90	0x5a	Z
    0,    //91	0x5b	[
    0,    //92	0x5c	\/
    0,    //93	0x5d	]
    0,    //94	0x5e	^
    0,    //95	0x5f	_
    '@',  //96	0x60	`
    0,    //97	0x61	a
    0,    //98	0x62	b
    0,    //99	0x63	c
    0,    //100	0x64	d
    0,    //101	0x65	e
    0,    //102	0x66	f
    0,    //103	0x67	g
    0,    //104	0x68	h
    0,    //105	0x69	i
    0,    //106	0x6a	j
    0,    //107	0x6b	k
    0,    //108	0x6c	l
    0,    //109	0x6d	m
    0,    //110	0x6e	n
    0,    //111	0x6f	o
    0,    //112	0x70	p
    0,    //113	0x71	q
    0,    //114	0x72	r
    0,    //115	0x73	s
    0,    //116	0x74	t
    0,    //117	0x75	u
    0,    //118	0x76	v
    0,    //119	0x77	w
    0,    //120	0x78	x
    0,    //121	0x79	y
    0,    //122	0x7a	z
    '[',  //123	0x7b	{
    '\\', //124	0x7c	|
    ']',  //125	0x7d	}
    '^',  //126	0x7e	~
    0x08  //127	0x7f	DEL（削除）
};

static int pressedKey88 = 0;
static int keyCheckFrameCount = 0;
static bool kanaMode = false;

void checkKeyboard();
void checkI2cKeyboard();
void checkSerialKeyboard();
void checkUSBKeyboard();
void checkAutoKeyboard();
void setAutoKey(String fileName);
void keyPress(int keyCode);
#ifdef _ATOMS3R
void selectDiskAtomS3R();
void selectDiskImageAtomS3R(int driveIndex);
void systemMenuAtomS3R();
#else
void selectDisk();
void selectDiskImage(int driveIndex);
void systemMenu();
void sortList(String fileList[], int fileListCount); 
#endif
#ifdef _CORES3
void coreS3ButtonUpdate();
#endif


/******************************************************************************
 * イベントハンドリング
 *
 *	1/60毎に呼び出される。
 *****************************************************************************/

/*
 * これは 起動時に1回だけ呼ばれる
 */
void event_init(void)
{
#if defined(_CORES3)||defined(_ATOMS3R)
  usbh_setup(show_config_desc_full);
#endif
}

/*
 * 約 1/60 毎に呼ばれる
 */
void event_update(void)
{
#ifdef _ATOMS3R
    extBtn.read();
    if (extBtn.wasReleasefor(500)){
        systemMenuAtomS3R();
    }else if (extBtn.wasReleased())
    {
        selectDiskAtomS3R();
    }
#else
    M5.update();
#ifdef _CORES3
    coreS3ButtonUpdate();
#endif
    if (M5.BtnB.wasReleased())
    {
        selectDisk();
    }
    if (M5.BtnC.wasReleased())
    {
        systemMenu();
    }
#endif
    checkKeyboard();
}

/*
 * これは 終了時に1回だけ呼ばれる
 */
void event_exit(void)
{
}

/***********************************************************************
 * 現在のマウス座標取得関数
 *
 ************************************************************************/

void event_get_mouse_pos(int *x, int *y)
{
    *x = 0;
    *y = 0;
}

/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *****************************************************************************/

int event_numlock_on(void)
{
    return FALSE;
}
void event_numlock_off(void)
{
}

/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void event_switch(void)
{
}

/******************************************************************************
 * ジョイスティック
 *
 *****************************************************************************/

int event_get_joystick_num(void)
{
    return 0;
}

void checkKeyboard()
{
    if (pressedKey88 != 0)
    {
        keyCheckFrameCount++;
        if (keyCheckFrameCount < 5)
        {
            return;
        }
        keyCheckFrameCount = 0;
        quasi88_key(pressedKey88, false);
        quasi88_key(KEY88_SHIFTR, false);
        pressedKey88 = 0;
    }

    checkI2cKeyboard();
    checkSerialKeyboard();
#if defined( _CORES3) || defined(_ATOMS3R)
    checkUSBKeyboard();
#endif
    checkAutoKeyboard();
}

//--------------------------------------------------------------
// I2C Keyboard Logic
//--------------------------------------------------------------
#define CARDKB_ADDR 0x5F
#define FACES_KEYBOARD_I2C_ADDR 0x08
void checkI2cKeyboard()
{
    //test
    return;
    int i2cKeyCode = 0;
    if (Wire.requestFrom(CARDKB_ADDR, 1))
    { // request 1 byte from keyboard
        while (Wire.available())
        {
            i2cKeyCode = Wire.read(); // receive a byte as
            break;
        }
    }

    if (Wire.requestFrom(FACES_KEYBOARD_I2C_ADDR, 1))
    { // request 1 byte from keyboard
        while (Wire.available())
        {
            i2cKeyCode = Wire.read(); // receive a byte as
            switch(i2cKeyCode){
            //カーソルキーコードを CardKBに合わせる。
            case 183: i2cKeyCode = 181;break;//Fn+K -> UP
            case 191: i2cKeyCode = 180;break;//Fn+N -> LEFT
            case 192: i2cKeyCode = 182;break;//Fn+M -> DOWN
            case 193: i2cKeyCode = 183 ;break;//Fn+$ -> RIGHT
            //ALT+ K/N/M/$ をカーソル扱いにする。
            case 161: i2cKeyCode = 181;break;//ALT+K -> UP
            case 170: i2cKeyCode = 180;break;//ALT+N -> LEFT
            case 171: i2cKeyCode = 182;break;//ALT+M -> DOWN
            case 172: i2cKeyCode = 183;break;//ALT+$ -> RIGHT
            
            case 144: i2cKeyCode = 128;break;//ALT+Q -> Fn+ESC -> SHIFT+STOP
            case 145: i2cKeyCode = 0x81;break; //ALT+W -> Fn+1 -> F1
            case 146: i2cKeyCode = 0x82;break; //ALT+E -> Fn+2 -> F2
            case 147: i2cKeyCode = 0x83;break; //ALT+R -> Fn+3 -> F3
            case 148: i2cKeyCode = 0x84;break; //ALT+T -> Fn+4 -> F4
            case 149: i2cKeyCode = 0x85;break; //ALT+Y -> Fn+5 -> F5 
            case 163: i2cKeyCode = 0x8B;break; //ALT+BS -> Fn+BS -> カナ
            }
            break;
        }
    }

    //Serial.println(i2cKeyCode,HEX);

    //特殊キー
    switch (i2cKeyCode)
    {
    case 0xB5:
        i2cKeyCode = 273; //UP
        break;
    case 0xB6:
        i2cKeyCode = 274; //DOWN
        break;
    case 0xB7:
        i2cKeyCode = 275; //RIGHT
        break;
    case 0xB4:
        i2cKeyCode = 276; //LEFT
        break;
    case 0x99:            //Fn + UP
        i2cKeyCode = 278; //HOME
        break;
    case 0xA4:           //Fn + Down
        i2cKeyCode = 12; //END -> CLR
        break;
    case 0x7F:            //Shift + BS
        i2cKeyCode = 277; //INST
        break;
    case 0x08: //BS
        i2cKeyCode = 8;
        break;
    case 0x1B: //ESC
        i2cKeyCode = 27;
        break;
    case 0x80: //Fn + ESC ->STOP
        i2cKeyCode = 19;
        break;
    case 0x8B:            //Fn + BS
        i2cKeyCode = 302; //かなモード 0x0B にかなボタンを割り当てている。
        break;
    case 0x81: //Fn+1
        i2cKeyCode = 282;
        break;
    case 0x82: //Fn+2
        i2cKeyCode = 283;
        break;
    case 0x83: //Fn+3
        i2cKeyCode = 284;
        break;
    case 0x84: //Fn+4
        i2cKeyCode = 285;
        break;
    case 0x85: //Fn+5
        i2cKeyCode = 286;
        break;
    case 0x86: //Fn+6
        i2cKeyCode = 287;
        break;
    case 0x87: //Fn+7
        i2cKeyCode = 288;
        break;
    case 0x88: //Fn+8
        i2cKeyCode = 289;
        break;
    case 0x89: //Fn+9
        i2cKeyCode = 290;
        break;
    case 0x8A: //Fn+10
        i2cKeyCode = 291;
        break;
    }
    keyPress(i2cKeyCode);
}

//--------------------------------------------------------------
// Serial Keyboard Logic
//--------------------------------------------------------------
void checkSerialKeyboard()
{
    int serialKeyCode = 0;
    if (Serial.available() > 0)
    {
        serialKeyCode = Serial.read();
        //Serial.println(serialKeyCode);
        //Special Key
        if (serialKeyCode == 27)
        { //ESC
            serialKeyCode = Serial.read();
            if (serialKeyCode == 91)
            {
                serialKeyCode = Serial.read();
                switch (serialKeyCode)
                {
                case 65:
                    serialKeyCode = 273; //UP
                    break;
                case 66:
                    serialKeyCode = 274; //DOWN
                    break;
                case 67:
                    serialKeyCode = 275; //RIGHT
                    break;
                case 68:
                    serialKeyCode = 276; //LEFT
                    break;
                case 49:
                    serialKeyCode = 278; //HOME
                    break;
                case 52:
                    serialKeyCode = 12; //END -> CLR
                    break;
                case 50:
                    serialKeyCode = 277; //INST
                    break;
                default:
                    serialKeyCode = 0;
                }
            }
            else if (serialKeyCode == 255) //ESC -> STOP
            {
                serialKeyCode = 0x1a;
            }
        }
        if (serialKeyCode == 127)
        { //BackSpace
            serialKeyCode = 8;
        }
        while (Serial.available() > 0 && Serial.read() != -1)
            ;
        if (serialKeyCode != 0)
        {
            keyPress(serialKeyCode);
        }
    }
}
#if defined(_ATOMS3R) || defined(_CORES3)
void checkUSBKeyboard(){
  usbh_task();

  if (isKeyboardReady && !isKeyboardPolling && (KeyboardTimer > KeyboardInterval)) {
    KeyboardIn->num_bytes = 8;
    esp_err_t err = usb_host_transfer_submit(KeyboardIn);
    if (err != ESP_OK) {
      ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
    }
    isKeyboardPolling = true;
    KeyboardTimer = 0;
  }
}
#endif
//--------------------------------------------------------------
// PC-8801 keyPress
//--------------------------------------------------------------
void keyPress(int keyCode)
{
    int key88 = 0;
    bool shiftFlag = false;
    if (keyCode != 0)
    {
        if (keyCode < 128)
        {
            if (withShiftKey[keyCode] != 0)
            {
                key88 = keysym2key88[withShiftKey[keyCode]];
                shiftFlag = true;
            }
        }

        if (shiftFlag == false)
        {
            key88 = keysym2key88[keyCode];
        }

        //かなキーは特別扱い。押すたびにON/OFFトグル
        if (key88 == KEY88_KANA)
        {
            if (kanaMode == false)
            {
                quasi88_key(KEY88_KANA, true);
                kanaMode = true;
            }
            else
            {
                quasi88_key(KEY88_KANA, false);
                kanaMode = false;
            }
            return;
        }

        if (key88 != 0)
        {
            if (shiftFlag == true)
            {
                quasi88_key(KEY88_SHIFTR, true);
            }
            quasi88_key(key88, true);
            pressedKey88 = key88;
        }
    }
}

#define MAX_DISK_FILES 255
#ifndef _ATOMS3R
void selectDisk()
{
    waitDrawing();
    graph_stopDrawing();
    delay(100);
    File d88FileRoot;
    String fileList[MAX_DISK_FILES];
    
    M5.Lcd.startWrite();
    M5.Lcd.fillScreen(0);
    M5.Lcd.endWrite();
    String diskDir = String(osd_dir_disk());
    if (diskDir.endsWith("/") == true)
    {
        diskDir = diskDir.substring(0, diskDir.length() - 1);
    }

    d88FileRoot = SD.open(diskDir);
    int fileListCount = 0;

    while (1)
    {
        File entry = d88FileRoot.openNextFile();
        if (!entry)
        { // no more files
            break;
        }
        //ファイルのみ取得
        if (!entry.isDirectory())
        {
            String fullFileName = entry.name();
            String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
            fileList[fileListCount] = fileName;
            fileListCount++;
            //Serial.println(fileName);
        }
        entry.close();
    }
    d88FileRoot.close();

    M5.Lcd.startWrite();
    delay(10);

    M5.Lcd.setTextSize(2);

    int startIndex = 0;
    int endIndex = startIndex + 10;
    if (endIndex > fileListCount)
    {
        endIndex = fileListCount;
    }

    sortList(fileList, fileListCount);

    boolean needRedraw = true;
    boolean longPressB = false;
    int selectIndex = 0;
    String curDiskFile = String(file_disk[0]);
    curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
    curDiskFile.trim();

    if (curDiskFile.length() > 0)
    {
        for (int index = 0; index < fileListCount; index++)
        {
            if (fileList[index].compareTo(curDiskFile) == 0)
            {
                selectIndex = index + 2;
                break;
            }
        }
    }

    while (true)
    {

        if (needRedraw == true)
        {
            M5.Lcd.fillScreen(0);
            M5.Lcd.setCursor(0, 0);
            startIndex = selectIndex - 5;
            if (startIndex < 0)
            {
                startIndex = 0;
            }
            endIndex = startIndex + 12;
            if (endIndex + 1 > fileListCount)
            {
                endIndex = fileListCount + 1;
                startIndex = endIndex - 12;
                if(startIndex < 0){
                    startIndex = 0;
                }
            }

            for (int index = startIndex; index < endIndex + 1; index++)
            {
                if (index == selectIndex)
                {
                    M5.Lcd.setTextColor(TFT_GREEN);
                }
                else
                {
                    M5.Lcd.setTextColor(TFT_WHITE);
                }
                if (index == 0)
                {
                    M5.Lcd.println("[BACK]");
                }
                else if (index == 1)
                {
                    M5.Lcd.println("[EJECT]");
                }
                else
                {
                    M5.Lcd.println(fileList[index - 2]);
                }
            }
            M5.Lcd.setTextColor(TFT_WHITE);

            M5.Lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("U P", 53, 240 - 17, 1);
            //M5.Lcd.setCursor(35, 240 - 17);
            //M5.Lcd.print("U P");
            M5.Lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
            if(longPressB == false){
                M5.Lcd.drawCentreString("Drive:1", 159, 240 - 17, 1);
            }else{
                M5.Lcd.setTextColor(TFT_RED);
                M5.Lcd.drawCentreString("Drive:2", 159, 240 - 17, 1);
                M5.Lcd.setTextColor(TFT_WHITE);
            }
            //M5.Lcd.setCursor(125, 240 - 17);
            //M5.Lcd.print("SELECT");
            M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("DOWN", 266, 240 - 17, 1);
            //M5.Lcd.setCursor(245, 240 - 17);
            //M5.Lcd.print("DOWN");
            needRedraw = false;
        }
        M5.update();
#ifdef _CORES3
        coreS3ButtonUpdate();
#endif

        if (M5.BtnA.wasReleased())
        {
            selectIndex--;
            if (selectIndex < 0)
            {
                selectIndex = fileListCount + 1;
            }
            needRedraw = true;
        }

        if (M5.BtnC.wasReleased())
        {
            selectIndex++;
            if (selectIndex > fileListCount + 1)
            {
                selectIndex = 0;
            }
            needRedraw = true;
        }

        if (M5.BtnB.wasReleased())
        {
            if (selectIndex == 0)
            {
                //何もせず戻る
                M5.Lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }
            else if (selectIndex == 1)
            {
                quasi88_disk_eject_all();
                M5.Lcd.fillScreen(TFT_BLACK);
                M5.Lcd.setCursor(0, 0);
                M5.Lcd.println("Disk Eject");
            }
            else
            {
                delay(10);
                //Set Disk
                M5.Lcd.fillScreen(TFT_BLACK);
                M5.Lcd.setCursor(0, 0);
                int driveIndex = (longPressB==false)?DRIVE_1:DRIVE_2;
                //if (curDiskFile.compareTo(fileList[selectIndex - 2]) != 0)
                //{ //変更されている場合のみセット
                    delay(10);
                    //quasi88_disk_eject_all();
                    //delay(10);
                    String fileName = diskDir + "/" + fileList[selectIndex - 2];
                    //もし txt ファイルだった場合は、autoKey設定を行う。
                    if(fileName.endsWith(".txt")){
                        //autoKey設定
                        setAutoKey(fileName);
                        M5.Lcd.fillScreen(TFT_BLACK);
                        delay(10);
                        graph_updateDrawFlag();
                        graph_restartDrawing();
                        return;
                    }

                    const char *cFileName = fileName.c_str();
                    M5.Lcd.endWrite();
                    if(quasi88_disk_insert(driveIndex, cFileName, 0, false) == true)
                    //if (quasi88_disk_insert_all(cFileName, false) == true) //DRIVE1とDRIVER2
                    {
                        M5.Lcd.startWrite();
                        M5.Lcd.print("Set Disk Image\nDrive:");
                        M5.Lcd.print((longPressB==false)?"1":"2");
                        M5.Lcd.println("");
                    }
                    else
                    {
                        M5.Lcd.startWrite();
                        M5.Lcd.print("Set Disk FAIL!:");
                    }
                //}
                //else
               // {
               //    M5.Lcd.print("Not Change:");
               //}
                M5.Lcd.println(fileList[selectIndex -2]);
                //イメージが複数ある場合、イメージのセレクト
                if (disk_image_num(driveIndex) > 1)
                {
                    selectDiskImage(driveIndex);
                }
            }
            delay(2000);
            M5.Lcd.fillScreen(TFT_BLACK);
            delay(10);
            graph_updateDrawFlag();
            graph_restartDrawing();
            return;
        }
        if(M5.BtnB.pressedFor(500) && longPressB ==false){
            longPressB = true;
            needRedraw = true;
        }
        delay(100);
    }
}

void selectDiskImage(int driveIndex)
{
    //if (disk_image_exist(driveIndex) == false)
    if (disk_image_exist(driveIndex) == null)
    {
        return;
    }
    int imageCount = disk_image_num(driveIndex);

    boolean needUpdate = true;
    boolean longPressB = false;
    int selectIndex = 0;
    while (true)
    {        
        if(needUpdate){
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextColor(TFT_WHITE);
            for (int imageIndex = 0; imageIndex < imageCount; imageIndex++)
            {
                M5.Lcd.setTextColor(TFT_WHITE);
                if(selectIndex == imageIndex){
                    M5.Lcd.setTextColor(TFT_GREEN);
                }
                M5.Lcd.printf("%d :", imageIndex + 1);
                M5.Lcd.println(drive[driveIndex].image[imageIndex].name);
            }
            M5.Lcd.setTextColor(TFT_WHITE);
            if(driveIndex == 0){
                M5.Lcd.drawString("LONG PRESS:SET Drive 1&2",0,200);
            }else{
                M5.Lcd.drawString("LONG PRESS:SET Drive 2&1",0,200);
            }
            M5.Lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("UP", 53, 240 - 17, 1);
            M5.Lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
            if(driveIndex == 0){
                if(longPressB == false){
                    M5.Lcd.drawCentreString("Drive:1", 159, 240 - 17, 1);
                }else{
                    M5.Lcd.setTextColor(TFT_RED);
                    M5.Lcd.drawCentreString("1 & 2", 159, 240 - 17, 1);
                }
            }else{
                if(longPressB == false){
                    M5.Lcd.drawCentreString("Drive:2", 159, 240 - 17, 1);
                }else{
                    M5.Lcd.setTextColor(TFT_RED);
                    M5.Lcd.drawCentreString("2 & 1", 159, 240 - 17, 1);
                }
            }
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("DOWN", 266, 240 - 17, 1);
            needUpdate = false;
        }


        M5.update();
#ifdef _CORES3
       coreS3ButtonUpdate();
#endif

        if (M5.BtnA.wasReleased())
        {
            selectIndex = selectIndex - 1;
            if(selectIndex < 0){
                selectIndex = imageCount - 1;
            }
            needUpdate = true;
        }
        if (M5.BtnB.wasReleased())
        {
            quasi88_disk_image_select(driveIndex, selectIndex);    
            if(longPressB == true){
                int anotherIndex = selectIndex + 1;
                if(anotherIndex  >= imageCount ){
                    anotherIndex = 0;
                }
                int src = 0;
                int dst = 1;
                if(driveIndex == 1){
                    src = 1;
                    dst = 0;
                }
                quasi88_disk_insert_A_to_B(src,dst,anotherIndex);
            }
            break;
        }
        if (M5.BtnB.pressedFor(500)&&longPressB==false){
            longPressB = true;
            needUpdate = true;
        }
        if (M5.BtnC.wasReleased())
        {
            selectIndex = selectIndex + 1;
            if(selectIndex >= imageCount ){
                selectIndex = 0;
            }
            needUpdate = true;
        }
        delay(100);
    }
    
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Set Disk Image");
    
    for (int driveIndex = 0; driveIndex < 2; driveIndex++)
    {
        M5.Lcd.printf("DRIVE %d:", driveIndex + 1);
        String curDiskFile = String(file_disk[driveIndex]);
        curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
        M5.Lcd.println(curDiskFile);
        M5.Lcd.print(" ");
        M5.Lcd.println(drive[driveIndex].image[drive[driveIndex].selected_image].name);
    }
}

#define MENU_ITEM_COUNT 5
void systemMenu()
{

    static String menuItem[] =
        {
            "[BACK]",
            "RESET:N88-BASIC V2",
            "RESET:N88-BASIC V1H",
            "RESET:N88-BASIC V1S",
            "RESET:N-BASIC",
            ""};

    waitDrawing();
    graph_stopDrawing();
    delay(100);
    M5.Lcd.fillScreen(TFT_BLACK);
    delay(10);
    M5.Lcd.setTextSize(2);
    bool needRedraw = true;

    int menuItemCount = 0;
    while(menuItem[menuItemCount] != ""){
        menuItemCount++;
    }

    int selectIndex = 0;
    while (true)
    {
        if (needRedraw == true)
        {
            M5.Lcd.fillScreen(0);
            M5.Lcd.setCursor(0, 0);
            for (int index = 0; index < menuItemCount; index++)
            {
                if (index == selectIndex)
                {
                    M5.Lcd.setTextColor(TFT_GREEN);
                }
                else
                {
                    M5.Lcd.setTextColor(TFT_WHITE);
                }
                M5.Lcd.println(menuItem[index]);
            }
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.println("");

            for (int driveIndex = 0; driveIndex < 2; driveIndex++)
            {
                M5.Lcd.printf("DRIVE %d:", driveIndex + 1);
                if (drive[driveIndex].empty == true)
                {
                    M5.Lcd.println("[EMPTY]");
                    M5.Lcd.println("");
                }
                else
                {
                    String curDiskFile = String(file_disk[driveIndex]);
                    curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
                    M5.Lcd.println(curDiskFile);
                    M5.Lcd.print(" ");
                    M5.Lcd.println(drive[driveIndex].image[drive[driveIndex].selected_image].name);
                }
            }
#ifdef _CORES3  
            //チェックするタイミングないのでここで…。
            if(isKeyboard == false){ //USBキーボードが無い場合、USBへの電源出力をOFF
                M5.Power.setUsbOutput(false);
            }
            M5.Lcd.print("USB 5V OUTPUT:");
            M5.Lcd.println(M5.Power.getUsbOutput()?"ON":"OFF");
            M5.Lcd.print("BATTERY LEVEL:");
            M5.Lcd.println(M5.Power.getBatteryLevel());

#endif

            M5.Lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.setCursor(35, 240 - 17);
            M5.Lcd.print("U P");
            M5.Lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.setCursor(125, 240 - 17);
            M5.Lcd.print("SELECT");
            M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.setCursor(245, 240 - 17);
            M5.Lcd.print("DOWN");

            needRedraw = false;
        }
        M5.update();
#ifdef _CORES3
        coreS3ButtonUpdate();
#endif

        if (M5.BtnA.wasReleased())
        {
            selectIndex--;
            if (selectIndex < 0)
            {
                selectIndex = menuItemCount -1;
            }
            needRedraw = true;
        }

        if (M5.BtnC.wasReleased())
        {
            selectIndex++;
            if (selectIndex >= menuItemCount)
            {
                selectIndex = 0;
            }
            needRedraw = true;
        }

        if (M5.BtnB.wasReleased())
        {
            if (selectIndex == 0)
            {
                M5.Lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }

            T_RESET_CFG cfg;
            quasi88_get_reset_cfg(&cfg);
            String basicMode = "";
            switch (selectIndex)
            {
            case 1:
                cfg.boot_basic = BASIC_V2;
                basicMode = "N88-BASIC V2";
                break;
            case 2:
                cfg.boot_basic = BASIC_V1H;
                basicMode = "N88-BASIC V1H";
                break;
            case 3:
                cfg.boot_basic = BASIC_V1S;
                basicMode = "N88-BASIC V1S";
                break;
            case 4:
                cfg.boot_basic = BASIC_N;
                basicMode = "N-BASIC";
                break;
            default:
                M5.Lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Reset QUASI88...");
            M5.Lcd.print("MODE : ");
            M5.Lcd.print(basicMode);
            delay(1000);
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.endWrite();
            quasi88_reset(&cfg);
            //M5.Lcd.startWrite();
            graph_updateDrawFlag();
            graph_restartDrawing();
            
            return;
        }
        delay(100);
    }
}
#endif
/* bubble sort filenames */
//https://github.com/tobozo/M5Stack-SD-Updater/blob/master/examples/M5Stack-SD-Menu/M5Stack-SD-Menu.ino
void sortList(String fileList[], int fileListCount) { 
  bool swapped;
  String temp;
  String name1, name2;
  do {
    swapped = false;
    for(int i = 0; i < fileListCount-1; i++ ) {
      name1 = fileList[i];
      name1.toUpperCase();
      name2 = fileList[i+1];
      name2.toUpperCase();
      if (name1.compareTo(name2) > 0) {
        temp = fileList[i];
        fileList[i] = fileList[i+1];
        fileList[i+1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}

#ifdef _CORES3
void coreS3ButtonUpdate(){
        auto ms = m5gfx::millis();
        uint_fast8_t btn_bits = 0;
        int i = M5.Touch.getCount();
        while (--i >= 0)
        {
          auto raw = M5.Touch.getTouchPointRaw(i);
          if (raw.y > 200)
          {
            auto det = M5.Touch.getDetail(i);
            if (det.state & m5::touch_state_t::touch)
            {
              if (M5.BtnA.isPressed()) { btn_bits |= 1 << 0; }
              if (M5.BtnB.isPressed()) { btn_bits |= 1 << 1; }
              if (M5.BtnC.isPressed()) { btn_bits |= 1 << 2; }
              if (btn_bits || !(det.state & m5::touch_state_t::mask_moving))
              {
                btn_bits |= 1 << ((raw.x - 2) / 107);
              }
            }
          }
       
        }
        M5.BtnA.setRawState(ms, btn_bits & 1);
        M5.BtnB.setRawState(ms, btn_bits & 2);
        M5.BtnC.setRawState(ms, btn_bits & 4);
}
#endif

#if defined(_CORES3)||defined(_ATOMS3R)
//USB Keyboard
unsigned char hid_to_emu[] ={
0xff,//0	0x00	Reserved (no event indicated)
0xff,//1	0x01	Keyboard ErrorRollOver?
0xff,//2	0x02	Keyboard POSTFail
0xff,//3	0x03	Keyboard ErrorUndefined?
KEY88_A,//4	0x04	Keyboard a and A
KEY88_B,//5	0x05	Keyboard b and B
KEY88_C,//6	0x06	Keyboard c and C
KEY88_D,//7	0x07	Keyboard d and D
KEY88_E,//8	0x08	Keyboard e and E
KEY88_F,//9	0x09	Keyboard f and F
KEY88_G,//10	0x0A	Keyboard g and G
KEY88_H,//11	0x0B	Keyboard h and H
KEY88_I,//12	0x0C	Keyboard i and I
KEY88_J,//13	0x0D	Keyboard j and J
KEY88_K,//14	0x0E	Keyboard k and K
KEY88_L,//15	0x0F	Keyboard l and L
KEY88_M,//16	0x10	Keyboard m and M
KEY88_N,//17	0x11	Keyboard n and N
KEY88_O,//18	0x12	Keyboard o and O
KEY88_P,//19	0x13	Keyboard p and P
KEY88_Q,//20	0x14	Keyboard q and Q
KEY88_R,//21	0x15	Keyboard r and R
KEY88_S,//22	0x16	Keyboard s and S
KEY88_T,//23	0x17	Keyboard t and T
KEY88_U,//24	0x18	Keyboard u and U
KEY88_V,//25	0x19	Keyboard v and V
KEY88_W,//26	0x1A	Keyboard w and W
KEY88_X,//27	0x1B	Keyboard x and X
KEY88_Y,//28	0x1C	Keyboard y and Y
KEY88_Z,//29	0x1D	Keyboard z and Z
KEY88_1,//30	0x1E	Keyboard 1 and !
KEY88_2,//31	0x1F	Keyboard 2 and @
KEY88_3,//32	0x20	Keyboard 3 and #
KEY88_4,//33	0x21	Keyboard 4 and $
KEY88_5,//34	0x22	Keyboard 5 and %
KEY88_6,//35	0x23	Keyboard 6 and ^
KEY88_7,//36	0x24	Keyboard 7 and &
KEY88_8,//37	0x25	Keyboard 8 and *
KEY88_9,//38	0x26	Keyboard 9 and (
KEY88_0,//39	0x27	Keyboard 0 and )
KEY88_RETURNL,//40	0x28	Keyboard Return (ENTER)
KEY88_ESC,//41	0x29	Keyboard ESCAPE
KEY88_INS_DEL,//42	0x2A	Keyboard DELETE (Backspace)
KEY88_TAB,//43	0x2B	Keyboard Tab
KEY88_SPACE,//44	0x2C	Keyboard Spacebar
KEY88_MINUS,//45	0x2D	Keyboard - and (underscore)
KEY88_CARET,//46	0x2E	Keyboard = and +
KEY88_AT,//47	0x2F	Keyboard [ and {
KEY88_BRACKETLEFT,//48	0x30	Keyboard ] and }
KEY88_YEN,//49	0x31	Keyboard \ and ｜
KEY88_BRACKETRIGHT,//50	0x32	Keyboard Non-US # and ~
KEY88_SEMICOLON,//51	0x33	Keyboard ; and :
KEY88_COLON,//52	0x34	Keyboard ' and "
KEY88_KANA,//53	0x35	Keyboard Grave Accent and Tilde
KEY88_COMMA,//54	0x36	Keyboard, and <
KEY88_PERIOD,//55	0x37	Keyboard . and >
KEY88_SLASH,//56	0x38	Keyboard / and ?
KEY88_CAPS,//57	0x39	Keyboard Caps Lock
KEY88_F1,//58	0x3A	Keyboard F1
KEY88_F2,//59	0x3B	Keyboard F2
KEY88_F3,//60	0x3C	Keyboard F3
KEY88_F4,//61	0x3D	Keyboard F4
KEY88_F5,//62	0x3E	Keyboard F5
0xff,//63	0x3F	Keyboard F6
0xff,//64	0x40	Keyboard F7
0xff,//65	0x41	Keyboard F8
0xff,//66	0x42	Keyboard F9
0xff,//67	0x43	Keyboard F10
0xff,//68	0x44	Keyboard F11
0xff,//69	0x45	Keyboard F12
0xff,//70	0x46	Keyboard PrintScreen
0xff,//71	0x47	Keyboard Scroll Lock
KEY88_STOP,//72	0x48	Keyboard Pause
KEY88_INS,//73	0x49	Keyboard Insert
KEY88_HOME,//74	0x4A	Keyboard Home
KEY88_ROLLUP,//75	0x4B	Keyboard PageUp
0xff,//76	0x4C	Keyboard Delete Forward
0xff,//77	0x4D	Keyboard End
KEY88_ROLLDOWN,//78	0x4E	Keyboard PageDown
KEY88_RIGHT,//79	0x4F	Keyboard RightArrow
KEY88_LEFT,//80	0x50	Keyboard LeftArrow
KEY88_DOWN,//81	0x51	Keyboard DownArrow
KEY88_UP,//82	0x52	Keyboard UpArrow
0xff,//83	0x53	Keypad Num Lock and Clear
KEY88_KP_DIVIDE,//84	0x54	Keypad /
KEY88_KP_MULTIPLY,//85	0x55	Keypad *
KEY88_KP_SUB,//86	0x56	Keypad -
KEY88_KP_ADD,//87	0x57	Keypad +
KEY88_RETURNR,//88	0x58	Keypad ENTER
KEY88_KP_1,//89	0x59	Keypad 1 and End
KEY88_KP_2,//90	0x5A	Keypad 2 and Down Arrow
KEY88_KP_3,//91	0x5B	Keypad 3 and PageDn?
KEY88_KP_4,//92	0x5C	Keypad 4 and Left Arrow
KEY88_KP_5,//93	0x5D	Keypad 5
KEY88_KP_6,//94	0x5E	Keypad 6 and Right Arrow
KEY88_KP_7,//95	0x5F	Keypad 7 and Home
KEY88_KP_8,//96	0x60	Keypad 8 and Up Arrow
KEY88_KP_9,//97	0x61	Keypad 9 and PageUp?
KEY88_KP_0,//98	0x62	Keypad 0 and Insert
KEY88_KP_PERIOD,//99	0x63	Keypad . and Delete
0xff,//100	0x64	Keyboard Non-US \ and ｜
0xff,//101	0x65	Keyboard Application
0xff,//102	0x66	Keyboard Power
KEY88_KP_EQUAL,//103	0x67	Keypad =
0xff,//104	0x68	Keyboard F13
0xff,//105	0x69	Keyboard F14
0xff,//106	0x6A	Keyboard F15
0xff,//107	0x6B	Keyboard F16
0xff,//108	0x6C	Keyboard F17
0xff,//109	0x6D	Keyboard F18
0xff,//110	0x6E	Keyboard F19
0xff,//111	0x6F	Keyboard F20
0xff,//112	0x70	Keyboard F21
0xff,//113	0x71	Keyboard F22
0xff,//114	0x72	Keyboard F23
0xff,//115	0x73	Keyboard F24
0xff,//116	0x74	Keyboard Execute
0xff,//117	0x75	Keyboard Help
0xff,//118	0x76	Keyboard Menu
0xff,//119	0x77	Keyboard Select
0xff,//120	0x78	Keyboard Stop
0xff,//121	0x79	Keyboard Again
0xff,//122	0x7A	Keyboard Undo
0xff,//123	0x7B	Keyboard Cut
0xff,//124	0x7C	Keyboard Copy
0xff,//125	0x7D	Keyboard Paste
0xff,//126	0x7E	Keyboard Find
0xff,//127	0x7F	Keyboard Mute
0xff,//128	0x80	Keyboard Volume Up
0xff,//129	0x81	Keyboard Volume Down
0xff,//130	0x82	Keyboard Locking Caps Lock
0xff,//131	0x83	Keyboard Locking Num Lock
0xff,//132	0x84	Keyboard Locking Scroll Lock
0xff,//133	0x85	Keypad Comma
0xff,//134	0x86	Keypad Equal Sign
KEY88_UNDERSCORE,//135	0x87	Keyboard International1
0xff,//136	0x88	Keyboard International2
KEY88_YEN,//137	0x89	Keyboard International3
KEY88_HENKAN,//138	0x8A	Keyboard International4 変換
KEY88_KETTEI,//139	0x8B	Keyboard International5 無変換
0xff,//140	0x8C	Keyboard International6 
0xff,//141	0x8D	Keyboard International7
0xff,//142	0x8E	Keyboard International8
0xff,//143	0x8F	Keyboard International9
0xff,//144	0x90	Keyboard LANG1
0xff,//145	0x91	Keyboard LANG2
0xff,//146	0x92	Keyboard LANG3
0xff,//147	0x93	Keyboard LANG4
0xff,//148	0x94	Keyboard LANG5
0xff,//149	0x95	Keyboard LANG6
0xff,//150	0x96	Keyboard LANG7
0xff,//151	0x97	Keyboard LANG8
0xff,//152	0x98	Keyboard LANG9
0xff,//153	0x99	Keyboard Alternate Erase
0xff,//154	0x9A	Keyboard SysReq?/Attention
0xff,//155	0x9B	Keyboard Cancel
0xff,//156	0x9C	Keyboard Clear
0xff,//157	0x9D	Keyboard Prior
0xff,//158	0x9E	Keyboard Return
0xff,//159	0x9F	Keyboard Separator
0xff,//160	0xA0	Keyboard Out
0xff,//161	0xA1	Keyboard Oper
0xff,//162	0xA2	Keyboard Clear/Again
0xff,//163	0xA3	Keyboard CrSel?/Props
0xff,//164	0xA4	Keyboard ExSel?
0xff,//165    0xA5    レイヤー指定
0xff,//166    0xA6    レイヤー指定
0xff,//167    0xA7    レイヤー切り替え
0xff,//168    0xA8    未定義
0xff,//169    0xA9    ユーザ定義マクロ(Long)1
0xff,//170    0xAA     ユーザ定義マクロ(Long)2
0xff,//171    0xAB     ユーザ定義マクロ(Long)3
0xff,//172    0xAC     ユーザ定義マクロ(Long)4
0xff,//173    0xAD     ユーザ定義マクロ(Long)5
0xff,//174    0xAE     ユーザ定義マクロ(Long)6
0xff,//175    0xAF    ユーザ定義マクロ(Long)7
0xff,//176    0xB0    ユーザ定義マクロ(Short)1
0xff,//177    0xB1    ユーザ定義マクロ(short)2
0xff,//178    0xB2    ユーザ定義マクロ(short)3
0xff,//179    0xB3    ユーザ定義マクロ(short)4
0xff,//180    0xB4    ユーザ定義マクロ(short)5
0xff,//181    0xB5    ユーザ定義マクロ(short)6
0xff,//182    0xB6    ユーザ定義マクロ(short)7
0xff,//183    0xB7    ユーザ定義マクロ(short)8
0xff,//184    0xB8    ユーザ定義マクロ(short)9
0xff,//185    0xB9    ユーザ定義マクロ(short)10
0xff,//186    0xBA    ユーザ定義マクロ(short)11
0xff,//187    0xBB    ユーザ定義マクロ(short)12
0xff,//188    0xBC    ユーザ定義マクロ(short)13
0xff,//189    0xBD    ユーザ定義マクロ(short)14
0xff,//190    0xBE    未定義
0xff,//191    0xBF    未定義
0xff,//192    0xC0    未定義
0xff,//193    0xC1    未定義
0xff,//194    0xC2    未定義
0xff,//195    0xC3    未定義
0xff,//196    0xC4    未定義
0xff,//197    0xC5    未定義
0xff,//198    0xC6    未定義
0xff,//199    0xC7    未定義
0xff,//200    0xC8    未定義
0xff,//201    0xC9    未定義
0xff,//202    0xCA    未定義
0xff,//203    0xCB    未定義
0xff,//204    0xCC    未定義
0xff,//205    0xCD    未定義
0xff,//206    0xCE    未定義
0xff,//207    0xCF    未定義
0xff,//208    0xD0    システムコントロール Power Down
0xff,//209    0xD1    システムコントロール Sleep
0xff,//210    0xD2    未定義
0xff,//211    0xD3    未定義
0xff,//212    0xD4    未定義
0xff,//213    0xD5    未定義
0xff,//214    0xD6    組み込みマクロ テンキー =　(注1)
0xff,//215    0xD7    組み込みマクロ テンキー , (カンマ) (注1)
0xff,//216    0xD8    組み込みマクロ テンキー 000
0xff,//217    0xD9    組み込みマクロ Ctrl & Alt & Del
0xff,//218    0xDA    組み込みマクロ Ctrl & Z
0xff,//219    0xDB    組み込みマクロ Ctrl & X
0xff,//220    0xDC    組み込みマクロ Ctrl & C
0xff,//221    0xDD    組み込みマクロ Ctrl & V
0xff,//222    0xDE    組み込みマクロ Win & Shift & N
0xff,//223    0xDF    組み込みマクロ Ctrl & Shift & Esc
0xff,//KEY88_CTRL,//224	0xE0	Keyboard LeftControl //SHIFT,CTRL,ALT は mods で判定
0xff,//KEY88_SHIFTL,//225	0xE1	Keyboard LeftShift
0xff,//KEY88_GRAPH,//226	0xE2	Keyboard LeftAlt
0xff,//227	0xE3	Keyboard Left GUI
0xff,//KEY88_CTRL,//228	0xE4	Keyboard RightControl
0xff,//KEY88_SHIFTR,//229	0xE5	Keyboard RightShift
0xff,//KEY88_GRAPH,//230	0xE6	Keyboard RightAlt
0xff,//231	0xE7	Keyboard Right GUI
0xff//232-65535	0xE8-0xFFFF	Reserved
};
static int _last_key = 0;
//https://wiki.onakasuita.org/pukiwiki/?HID%2F%E3%82%AD%E3%83%BC%E3%82%B3%E3%83%BC%E3%83%89
static void keyboard(const uint8_t* d, int len)
{
    int mods = d[1];          // can we use hid mods instead of SDL? TODO
    int key_code = d[3];      // only care about first key
    //shift Key KEY88_SHIFTR
    if((mods & 0b00100010) > 0){
        quasi88_key(KEY88_SHIFT, true);
    }else{
        quasi88_key(KEY88_SHIFT, false);
    }
    if((mods & 0b00010001) > 0){
        quasi88_key(KEY88_CTRL, true);
    }else{
        quasi88_key(KEY88_CTRL, false);
    }
    if((mods & 0b01000100) > 0){
        quasi88_key(KEY88_GRAPH, true);
    }else{
        quasi88_key(KEY88_GRAPH, false);
    }
    if (key_code != _last_key) {
        //Serial1.printf("mods:%d keyCode:%d\n",mods,key_code);
        //if(key_code < 232){
        //    Serial1.printf("hidKey:%d\n",hid_to_emu[key_code]);
        //}

        //かなキーは特別扱い。押すたびにON/OFFトグル
        if (hid_to_emu[key_code] == KEY88_KANA)
        {
            if (kanaMode == false)
            {
                quasi88_key(KEY88_KANA, true);
                kanaMode = true;
            }
            else
            {
                quasi88_key(KEY88_KANA, false);
                kanaMode = false;
            }
            return;
        }

        if(0 < _last_key && _last_key < 232){
          quasi88_key(hid_to_emu[_last_key],false);
        }
        if (key_code) {
            _last_key = key_code;
        } else {
            _last_key = 0;
        }

        if(0 < key_code && key_code < 232){
          quasi88_key(hid_to_emu[key_code],true);
        }        
    }
}
void keyboard_transfer_cb(usb_transfer_t *transfer)
{
  if (Device_Handle == transfer->device_handle) {
    isKeyboardPolling = false;
    if (transfer->status == 0) {
      if (transfer->actual_num_bytes == 8) {
        uint8_t *const p = transfer->data_buffer;
        //bluetoothキーボードとあわせる。[1]にmodiry[3]にキーコード
        uint8_t keyData[4];
        keyData[1] = p[0];
        keyData[3] = p[2];
        
        //M5.Display.setCursor(0,0);
        //M5.Display.setTextColor(WHITE,BLACK);
        //M5.Display.printf("HID report: %02x %02x %02x %02x %02x %02x %02x %02x",
         //   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

            keyboard(keyData,4);

     }
    }
  }
}
void check_interface_desc_boot_keyboard(const void *p)
{
  const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

  if ((intf->bInterfaceClass == USB_CLASS_HID) &&
      (intf->bInterfaceSubClass == 1) &&
      (intf->bInterfaceProtocol == 1)) {
    isKeyboard = true;
    ESP_LOGI("", "Claiming a boot keyboard!");
    esp_err_t err = usb_host_interface_claim(Client_Handle, Device_Handle,
        intf->bInterfaceNumber, intf->bAlternateSetting);
    if (err != ESP_OK) ESP_LOGI("", "usb_host_interface_claim failed: %x", err);
  }
}

void prepare_endpoint(const void *p)
{
  const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
  esp_err_t err;

  // must be interrupt for HID
  if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT) {
    ESP_LOGI("", "Not interrupt endpoint: 0x%02x", endpoint->bmAttributes);
    return;
  }
  if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
    err = usb_host_transfer_alloc(KEYBOARD_IN_BUFFER_SIZE, 0, &KeyboardIn);
    if (err != ESP_OK) {
      KeyboardIn = NULL;
      ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
      return;
    }
    KeyboardIn->device_handle = Device_Handle;
    KeyboardIn->bEndpointAddress = endpoint->bEndpointAddress;
    KeyboardIn->callback = keyboard_transfer_cb;
    KeyboardIn->context = NULL;
    isKeyboardReady = true;
    KeyboardInterval = endpoint->bInterval;
    ESP_LOGI("", "USB boot keyboard ready");
  }
  else {
    ESP_LOGI("", "Ignoring interrupt Out endpoint");
  }
}

#endif

#ifdef _ATOMS3R

void selectDiskAtomS3R()
{
    waitDrawing();
    graph_stopDrawing();
    delay(100);
    File d88FileRoot;
    String fileList[MAX_DISK_FILES];
    
    lcd.startWrite();
    lcd.fillScreen(0);
    lcd.endWrite();
    String diskDir = String(osd_dir_disk());
    if (diskDir.endsWith("/") == true)
    {
        diskDir = diskDir.substring(0, diskDir.length() - 1);
    }

    d88FileRoot = SPIFFS.open(diskDir);
    int fileListCount = 0;

    while (1)
    {
        File entry = d88FileRoot.openNextFile();
        if (!entry)
        { // no more files
            break;
        }
        //ファイルのみ取得
        if (!entry.isDirectory())
        {
            String fullFileName = entry.name();
            String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
            fileList[fileListCount] = fileName;
            fileListCount++;
            //Serial.println(fileName);
        }
        entry.close();
    }
    d88FileRoot.close();

    lcd.startWrite();
    delay(10);

    lcd.setTextSize(1);

    int startIndex = 0;
    int endIndex = startIndex + 10;
    if (endIndex > fileListCount)
    {
        endIndex = fileListCount;
    }

    sortList(fileList, fileListCount);

    boolean needRedraw = true;
    boolean longPressB = false;
    int selectIndex = 0;
    String curDiskFile = String(file_disk[0]);
    curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
    curDiskFile.trim();

    if (curDiskFile.length() > 0)
    {
        for (int index = 0; index < fileListCount; index++)
        {
            if (fileList[index].compareTo(curDiskFile) == 0)
            {
                selectIndex = index + 2;
                break;
            }
        }
    }

    while (true)
    {

        if (needRedraw == true)
        {
            lcd.fillScreen(0);
            lcd.setCursor(0, 0);
            startIndex = selectIndex - 5;
            if (startIndex < 0)
            {
                startIndex = 0;
            }
            endIndex = startIndex + 12;
            if (endIndex + 1 > fileListCount)
            {
                endIndex = fileListCount + 1;
                startIndex = endIndex - 12;
                if(startIndex < 0){
                    startIndex = 0;
                }
            }

            for (int index = startIndex; index < endIndex + 1; index++)
            {
                if (index == selectIndex)
                {
                    if(longPressB==true){
                        lcd.setTextColor(TFT_RED);
                    }else{
                        lcd.setTextColor(TFT_GREEN);   
                    }
                }
                else
                {
                    lcd.setTextColor(TFT_WHITE);
                }
                if (index == 0)
                {
                    lcd.println("[BACK]");
                }
                else if (index == 1)
                {
                    lcd.println("[EJECT]");
                }
                else
                {
                    lcd.println(fileList[index - 2]);
                }
            }
            lcd.setTextColor(TFT_WHITE);

            needRedraw = false;
        }
        
        extBtn.read();

        //長押し・決定
        //押し・下
        if (extBtn.wasReleasefor(500)){

            if (selectIndex == 0)
            {
                //何もせず戻る
                lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }
            else if (selectIndex == 1)
            {
                quasi88_disk_eject_all();
                lcd.fillScreen(TFT_BLACK);
                lcd.setCursor(0, 0);
                lcd.println("Disk Eject");
            }
            else
            {
                delay(10);
                //Set Disk
                lcd.startWrite();
                lcd.fillScreen(TFT_BLACK);
                lcd.setCursor(0, 0);
                lcd.endWrite();
                int driveIndex = DRIVE_1;
                //if (curDiskFile.compareTo(fileList[selectIndex - 2]) != 0)
                //{ //変更されている場合のみセット
                    delay(10);
                    //quasi88_disk_eject_all();
                    //delay(10);
                    String fileName = diskDir + "/" + fileList[selectIndex - 2];
                    //もし txt ファイルだった場合は、autoKey設定を行う。
                    if(fileName.endsWith(".txt")){
                        //autoKey設定
                        setAutoKey(fileName);
                        lcd.fillScreen(TFT_BLACK);
                        delay(10);
                        graph_updateDrawFlag();
                        graph_restartDrawing();
                        return;
                    }
                    const char *cFileName = fileName.c_str();
                    
                    if(quasi88_disk_insert(driveIndex, cFileName, 0, false) == true)
                    //if (quasi88_disk_insert_all(cFileName, false) == true) //DRIVE1とDRIVER2
                    {
                        lcd.startWrite();
                        lcd.print("Set Disk Image\nDrive:");
                        lcd.print("1");
                        lcd.println("");
                    }
                    else
                    {
                        lcd.startWrite();
                        lcd.print("Set Disk FAIL!:");
                    }
                //}
                //else
            // {
            //    lcd.print("Not Change:");
            //}
                lcd.println(fileList[selectIndex -2]);
                lcd.endWrite();
                //イメージが複数ある場合、イメージのセレクト
                if (disk_image_num(driveIndex) > 1)
                {
                    selectDiskImageAtomS3R(driveIndex);
                }
            }
            delay(2000);
            lcd.fillScreen(TFT_BLACK);
            delay(10);
            graph_updateDrawFlag();
            graph_restartDrawing();
            return;
        
        }

        if (extBtn.wasReleased()) {
            selectIndex++;
            if (selectIndex > fileListCount + 1)
            {
                selectIndex = 0;
            }
            needRedraw = true;
        }
    
        if(extBtn.pressedFor(500) && longPressB ==false){
            longPressB = true;
            needRedraw = true;
        }
        
        delay(100);
    }
}

void selectDiskImageAtomS3R(int driveIndex)
{
    //if (disk_image_exist(driveIndex) == false)
    if (disk_image_exist(driveIndex) == null)
    {
        return;
    }
    int imageCount = disk_image_num(driveIndex);

    boolean needUpdate = true;
    boolean longPressB = false;
    int selectIndex = 0;
    while (true)
    {        
        if(needUpdate){
            lcd.startWrite();
            lcd.fillScreen(TFT_BLACK);
            lcd.setCursor(0, 0);
            lcd.setTextColor(TFT_WHITE);
            for (int imageIndex = 0; imageIndex < imageCount; imageIndex++)
            {
                lcd.setTextColor(TFT_WHITE);
                if(selectIndex == imageIndex){
                    if(longPressB == true){
                        lcd.setTextColor(TFT_RED);
                    }else{
                        lcd.setTextColor(TFT_GREEN);
                    }  
                }
                lcd.printf("%d :", imageIndex + 1);
                lcd.println(drive[driveIndex].image[imageIndex].name);
            }
            lcd.setTextColor(TFT_WHITE);
            lcd.endWrite();
            needUpdate = false;
        }


        extBtn.read();

        //長押し・決定
        //押し・下
        
        if (extBtn.wasReleasefor(500)){
            quasi88_disk_image_select(driveIndex, selectIndex);    
	            break;        	
        }

        if (extBtn.wasReleased()){
            selectIndex = selectIndex + 1;
            if(selectIndex >= imageCount ){
                selectIndex = 0;
            }
            needUpdate = true;
        }

        if (extBtn.pressedFor(500)&&longPressB==false){
            longPressB = true;
            needUpdate = true;
        }
        delay(100);
    }
    lcd.startWrite();
    lcd.fillScreen(TFT_BLACK);
    lcd.setCursor(0, 0);
    lcd.println("Set Disk Image");
    
    for (int driveIndex = 0; driveIndex < 2; driveIndex++)
    {
        lcd.printf("DRIVE %d:", driveIndex + 1);
        String curDiskFile = String(file_disk[driveIndex]);
        curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
        lcd.println(curDiskFile);
        lcd.print(" ");
        lcd.println(drive[driveIndex].image[drive[driveIndex].selected_image].name);
    }
    lcd.endWrite();
}

#define MENU_ITEM_COUNT 5
void systemMenuAtomS3R()
{

    static String menuItem[] =
        {
            "[BACK]",
            "RESET:N88-BASIC V2",
            "RESET:N88-BASIC V1H",
            "RESET:N88-BASIC V1S",
            "RESET:N-BASIC",
            ""};

    waitDrawing();
    graph_stopDrawing();
    delay(100);
    lcd.fillScreen(TFT_BLACK);
    delay(10);
    lcd.setTextSize(1);
    bool needRedraw = true;
    boolean longPressB = false;
    int menuItemCount = 0;
    while(menuItem[menuItemCount] != ""){
        menuItemCount++;
    }

    int selectIndex = 0;
    while (true)
    {
        if (needRedraw == true)
        {
            lcd.fillScreen(0);
            lcd.setCursor(0, 0);
            for (int index = 0; index < menuItemCount; index++)
            {
                if (index == selectIndex)
                {
                    if(longPressB==true){
                        lcd.setTextColor(TFT_RED);
                    }else{
                        lcd.setTextColor(TFT_GREEN);
                    }
                }
                else
                {
                    lcd.setTextColor(TFT_WHITE);
                }
                lcd.println(menuItem[index]);
            }
            lcd.setTextColor(TFT_WHITE);
            lcd.println("");

            for (int driveIndex = 0; driveIndex < 2; driveIndex++)
            {
                lcd.printf("DRIVE %d:", driveIndex + 1);
                if (drive[driveIndex].empty == true)
                {
                    lcd.println("[EMPTY]");
                    lcd.println("");
                }
                else
                {
                    String curDiskFile = String(file_disk[driveIndex]);
                    curDiskFile = curDiskFile.substring(curDiskFile.lastIndexOf("/") + 1);
                    lcd.println(curDiskFile);
                    lcd.print(" ");
                    lcd.println(drive[driveIndex].image[drive[driveIndex].selected_image].name);
                }
            }

            lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
            lcd.setCursor(35, 240 - 17);
            lcd.print("U P");
            lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
            lcd.setCursor(125, 240 - 17);
            lcd.print("SELECT");
            lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            lcd.setCursor(245, 240 - 17);
            lcd.print("DOWN");

            needRedraw = false;
        }
        extBtn.read();

        if (extBtn.wasReleasefor(500))
        {
            if (selectIndex == 0)
            {
                lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }

            T_RESET_CFG cfg;
            quasi88_get_reset_cfg(&cfg);
            String basicMode = "";
            switch (selectIndex)
            {
            case 1:
                cfg.boot_basic = BASIC_V2;
                basicMode = "N88-BASIC V2";
                break;
            case 2:
                cfg.boot_basic = BASIC_V1H;
                basicMode = "N88-BASIC V1H";
                break;
            case 3:
                cfg.boot_basic = BASIC_V1S;
                basicMode = "N88-BASIC V1S";
                break;
            case 4:
                cfg.boot_basic = BASIC_N;
                basicMode = "N-BASIC";
                break;
            default:
                lcd.fillScreen(TFT_BLACK);
                delay(10);
                graph_updateDrawFlag();
                graph_restartDrawing();
                return;
            }
            lcd.fillScreen(TFT_BLACK);
            lcd.setCursor(0, 0);
            lcd.println("Reset QUASI88...");
            lcd.print("MODE : ");
            lcd.print(basicMode);
            delay(1000);
            lcd.fillScreen(TFT_BLACK);
            lcd.endWrite();
            quasi88_reset(&cfg);
            //lcd.startWrite();
            graph_updateDrawFlag();
            graph_restartDrawing();
            
            return;
        }
        
        if (extBtn.wasReleased())
        {
            selectIndex++;
            if (selectIndex >= menuItemCount)
            {
                selectIndex = 0;
            }
            needRedraw = true;
        }
        if(extBtn.pressedFor(500) && longPressB ==false){
            longPressB = true;
            needRedraw = true;
        }

        delay(100);
    }
}
#endif

String autoKeyData = ""; //簡単のため Stringで管理してます
int autoKeyIndex = 0;
int autoKeyLineDelayMillis = 100; //\n 入力時の待ち時間
long autoKeyNextMillis = 0;

bool isAutoKeyHeadChar = true; //行の先頭文字

void checkAutoKeyboard(){
    //ファイルから読み込んだテキストファイル順にキーを押下する。
    //読み込みテキストが無いなら何もしない
    if(autoKeyData == ""){
        return;
    }
    //前回の入力から指定秒経過しているかチェック
    long nowMillis = millis();
    if(nowMillis < autoKeyNextMillis){
        return;
    }
    char inputKey = autoKeyData.charAt(autoKeyIndex);

    //先頭#のみの行は入力しない。指定された時間待ち
    if( inputKey == '#' && isAutoKeyHeadChar == true && (autoKeyIndex + 1 < autoKeyData.length())){
        char inputNextKey = autoKeyData.charAt(autoKeyIndex + 1);
        if(inputNextKey == '\r'){ //改行時は指定された時間待ち    
            autoKeyNextMillis = nowMillis + autoKeyLineDelayMillis;
            autoKeyIndex = autoKeyIndex + 2;
            return;
        }
    }
    if(inputKey == '\r'){ //改行時は指定された時間待ち
        autoKeyNextMillis = nowMillis + autoKeyLineDelayMillis;
        isAutoKeyHeadChar=true;
    }else{
        autoKeyNextMillis = nowMillis + 100;//通常の待ち
        isAutoKeyHeadChar=false;
    }
    
    keyPress(inputKey);

    autoKeyIndex++;
    if(autoKeyIndex > autoKeyData.length()){
        //delete autoKeyData;
        autoKeyData = "";
        autoKeyIndex = 0;
    }   
}

void setAutoKey(String fileName){
    //1行目の先頭が# の場合、次の数字が入力待ち秒数。なかった場合はデフォルト値とする。

#ifdef _ATOMS3R
    File autoKeyFile = SPIFFS.open(fileName, FILE_READ);
#else
    File autoKeyFile = SD.open(fileName, FILE_READ);
#endif
    autoKeyData = "";
    autoKeyIndex = 0;
    autoKeyLineDelayMillis = 100;
    if(!autoKeyFile){
        return;
    }
    while (autoKeyFile.available()) {
        char keyData = char(autoKeyFile.read());
        autoKeyData += keyData;     
    }

    // \r\n⇒ \r
    //PC-8001の改行は chr$(13) = \r
    autoKeyData.replace("\r\n","\r");  
    autoKeyData.replace("\n","\r"); 

    if(autoKeyData.startsWith("#")){
        //最初の改行を探す
        int lineEnd = autoKeyData.indexOf("\r");
        if(lineEnd > 0){
            autoKeyLineDelayMillis = autoKeyData.substring(1,lineEnd).toInt() * 1000;
            autoKeyIndex = lineEnd + 1;
        }
    }

    isAutoKeyHeadChar = true;
    autoKeyFile.close();
}