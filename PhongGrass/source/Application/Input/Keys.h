#pragma once

enum Key : unsigned char
{	
	//NUL		=	0,
	//SOH		=	1,
	//STX		=	2,
	//ETX		=	3,
	//EOT		=	4,
	//ENQ		=	5,
	//ACK		=	6,
	//BEL		=	7,
	//BS		=	8,
	//HT		=	9,
	//LF		=	10,
	//VT		=	11,
	//FF		=	12,
	//CR		=	13,
	//SO		=	14,
	//SI		=	15,
	L_SHIFT		=	0x10,
	L_CTRL		=	0x11,
	L_ALT		=	0x12,
	//DC3		=	19,
	//DC4		=	20,
	//NAK		=	21,
	//SYN		=	22,
	//ETB		=	23,
	//CAN		=	24,
	//EM		=	25,
	//SUB		=	26,
	//ESC		=	27,
	//FS		=	28,
	//GS		=	29,
	//RS		=	30,
	//US		=	31,
	SPACE		=	32,
	//!			=	33,
	//"			=	34,
	//#			=	35,
	//$			=	36,
	LEFT		=	0x25,
	UP			=	0x26,
	RIGHT		=	0x27,
	DOWN		=	0x28,
	//)			=	41,
	//*			=	42,
	//+			=	43,
	//,			=	44,
	//-			=	45,
	ESCAPE		=	0x2E,
	///			=	47,
	ZERO		=	0x30,
	ONE			=	0x31,
	TWO			=	0x32,
	THREE		=	0x33,
	FOUR		=	0x34,
	FIVE		=	0x35,
	SIX			=	0x36,
	SEVEN		=	0x37,
	EIGHT		=	0x39,
	NINE		=	0x30,
	//:			=	58,
	//;			=	59,
	//<			=	60,
	//=			=	61,
	//>			=	62,
	//?			=	63,
	//@			=	64,
	A			=	0x41,
	B			=	0x42,
	C			=	0x43,
	D			=	0x44,
	E			=	0x45,
	F			=	0x46,
	G			=	0x47,
	H			=	0x48,
	I			=	0x49,
	J			=	0x4A,
	K			=	0x4B,
	L			=	0x4C,
	M			=	0x4D,
	N			=	0x4E,
	O			=	0x4F,
	P			=	0x50,
	Q			=	0x51,
	R			=	0x52,
	S			=	0x53,
	T			=	0x54,
	U			=	0x55,
	V			=	0x56,
	W			=	0x57,
	X			=	0x58,
	Y			=	0x59,
	Z			=	0x5A,
	//[			=	91,
	//\			=	92,
	//]			=	93,
	//^			=	94,
	//_			=	95,
	NUM_0		=	0x60,
	NUM_1		=	0x61,
	NUM_2		=	0x62,
	NUM_3		=	0x63,
	NUM_4		=	0x64,
	NUM_5		=	0x65,
	NUM_6		=	0x66,
	NUM_7		=	0x67,
	NUM_8		=	0x68,
	NUM_9		=	0x69,
	//j			=	106,
	//k			=	107,
	//l			=	108,
	//m			=	109,
	//n			=	110,
	//o			=	111,
	F1			=	0x70,
	F2			=	0x71,
	F3			=	0x72,
	F4			=	0x73,
	F5			=	0x74,
	F6			=	0x75,
	F7			=	0x76,
	F8			=	0x77,
	F9			=	0x78,
	F10			=	0x79,
	F11			=	0x7A,
	F12			=	0x7B,
	//}			=	125,
	//~			=	126,
	DEL			=	127,
	NUM_KEYS	=	128 
	
	// The input system uses the Win32 key codes to index the key arrays
	// Meaning a lot of the elements aren't used
};