/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/
#include <M5Stack.h>
#include "quasi88.h"
#include "device.h"
#include "initval.h"

#include "event.h"
#include "file-op.h"

extern "C"
{
#include "keyboard.h"
#include "drive.h"
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
void keyPress(int keyCode);
void selectDisk();
void selectDiskImage();
void systemMenu();
void sortList(String fileList[], int fileListCount); 


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
}

/*
 * 約 1/60 毎に呼ばれる
 */
void event_update(void)
{
    M5.update();
    if (M5.BtnB.wasReleased())
    {
        selectDisk();
    }
    if (M5.BtnC.wasReleased())
    {
        systemMenu();
    }
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
}

//--------------------------------------------------------------
// I2C Keyboard Logic
//--------------------------------------------------------------
#define CARDKB_ADDR 0x5F
void checkI2cKeyboard()
{
    int i2cKeyCode = 0;
    if (Wire.requestFrom(CARDKB_ADDR, 1))
    { // request 1 byte from keyboard
        while (Wire.available())
        {
            i2cKeyCode = Wire.read(); // receive a byte as
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
void selectDisk()
{
    File d88FileRoot;
    String fileList[MAX_DISK_FILES];

    delay(100);
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

    delay(10);
    M5.Lcd.fillScreen(TFT_BLACK);
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
            M5.Lcd.drawCentreString("SELECT", 159, 240 - 17, 1);
            //M5.Lcd.setCursor(125, 240 - 17);
            //M5.Lcd.print("SELECT");
            M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("DOWN", 266, 240 - 17, 1);
            //M5.Lcd.setCursor(245, 240 - 17);
            //M5.Lcd.print("DOWN");
            needRedraw = false;
        }
        M5.update();
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

                if (curDiskFile.compareTo(fileList[selectIndex - 2]) != 0)
                { //変更されている場合のみセット
                    quasi88_disk_eject_all();
                    const char *cFileName = (diskDir + "/" + fileList[selectIndex - 2]).c_str();
                    //if(quasi88_disk_insert(DRIVE_1, cFileName, 0, false) == true){
                    if (quasi88_disk_insert_all(cFileName, false) == true)
                    {
                        M5.Lcd.print("Set Disk:");
                    }
                    else
                    {
                        M5.Lcd.print("Set Disk FAIL!:");
                    }
                }
                else
                {
                    M5.Lcd.print("Not Change:");
                }
                M5.Lcd.println(fileList[selectIndex -2]);
                //イメージが複数ある場合、イメージのセレクト
                if (disk_image_num(0) > 1)
                {
                    selectDiskImage();
                }
            }
            delay(2000);
            M5.Lcd.fillScreen(TFT_BLACK);
            delay(10);
            return;
        }
        delay(100);
    }
}

void selectDiskImage()
{
    if (disk_image_exist(0) == false)
    {
        return;
    }
    int imageCount = disk_image_num(0);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.lcd.println("Select Drive:1");
    for (int imageIndex = 0; imageIndex < imageCount; imageIndex++)
    {
        M5.Lcd.printf("%d :", imageIndex + 1);
        M5.Lcd.println(drive[0].image[imageIndex].name);
    }

    M5.Lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
    M5.Lcd.drawCentreString("1", 53, 240 - 17, 1);
    //M5.Lcd.setCursor(35, 240 - 17);
    //M5.Lcd.print(" 1 ");
    M5.Lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
    M5.Lcd.drawCentreString("2", 159, 240 - 17, 1);
    //M5.Lcd.setCursor(140, 240 - 17);
    //M5.Lcd.print(" 2 ");
    if (imageCount >= 3)
    {
        M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
        M5.Lcd.drawCentreString("3", 266, 240 - 17, 1);
        //M5.Lcd.setCursor(250, 240 - 17);
        //M5.Lcd.print(" 3 ");
    }

    int drive1Image = 0;
    int drive2Image = 0;
    while (true)
    {        
        M5.update();
        if (M5.BtnA.wasReleased())
        {
            drive1Image = 0;
            drive2Image = 1;
            break;
        }
        if (M5.BtnB.wasReleased())
        {
            drive1Image = 1;
            if (imageCount >= 3)
            {
                drive2Image = 2;
            }
            else
            {
                drive2Image = 0;
            }
            break;
        }
        if (M5.BtnC.wasReleased())
        {
            if (imageCount >= 3)
            {
                drive1Image = 2;
                drive2Image = 0;
                break;
            }
        }
        delay(100);
    }
    
    quasi88_disk_image_select(0, drive1Image);
    quasi88_disk_image_select(1, drive2Image);

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

    delay(10);
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
                return;
            }
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Reset QUASI88...");
            M5.Lcd.print("MODE : ");
            M5.Lcd.print(basicMode);
            delay(1000);
            quasi88_reset(&cfg);
            return;
        }
        delay(100);
    }
}
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