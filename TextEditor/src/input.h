#pragma once

void platform_Print(const char* msg);

// UTF-8 code point
struct code_point {
	union {
		u32 v;
		u8 bytes[4];
	};
};

#define CodePoint(value) {.v = value}

s32 GetLength(code_point p) {
	// // ascii
	// if ((p.bytes[3] & 0b10000000) == 0)
	// 	return 1;
	
	// // leading code byte
	// if (p.bytes[2] & 0b11000000) {
	// 	s32 res = 1;
	// 	// continuation bytes
	// 	for (s32 i = 2; i >= 0; i--) {
	// 		if ((p.bytes[i] & 0b11000000) == 0b10000000)
	// 			res++;
	// 		else
	// 			break;
	// 	}
		
	// 	return res;
	// }

	// platform_Print("Invalid code point\n");
	// return 0;
	
	for (size_t i = 3; i >= 0; i--)
	{
		u8 byte = p.bytes[i];
		if (byte == 0) continue;
		
		if ((byte & 0b11110000) == 0b11110000) 		return 4;
		if ((byte & 0b11100000) == 0b11100000) 		return 3;
		if ((byte & 0b11000000) == 0b11000000) 		return 2;
		if ((byte & 0b10000000) == 0) return 1; // ascii
	}
	
	platform_Print("Invalid code point\n");
	return 0;
}

enum te_Key {
	Key_None = 0,
	
	Key_0='0', Key_1='1', Key_2='2', Key_3='3', Key_4='4', Key_5='5', Key_6='6', Key_7='7', Key_8='8', Key_9='9',
	
	Key_A = 'A', Key_B = 'B', Key_C = 'C', Key_D = 'D', Key_E = 'E', Key_F = 'F', Key_G = 'G', Key_H = 'H', 
	Key_I = 'I', Key_J = 'J', Key_K = 'K', Key_L = 'L', Key_M = 'M', Key_N = 'N', Key_O = 'O', Key_P = 'P', 
	Key_Q = 'Q', Key_R = 'R', Key_S = 'S', Key_T = 'T', Key_U = 'U', Key_V = 'V', Key_W = 'W', Key_X = 'X', 
	Key_Y = 'Y', Key_Z = 'Z',
	
	Key_ArrowLeft, Key_ArrowRight, Key_ArrowUp, Key_ArrowDown,
	
	Key_ShiftLeft,  Key_ShiftRight,
	Key_CtrlLeft,   Key_CtrlRight,
	Key_AltLeft,    Key_AltRight,
	
	Key_Enter,
	
	// Key_a = 'a', Key_b = 'b', Key_c = 'c', Key_d = 'd', Key_e = 'e', Key_f = 'f', Key_g = 'g', Key_h = 'h', 
	// Key_i = 'i', Key_j = 'j', Key_k = 'k', Key_l = 'l', Key_m = 'm', Key_n = 'n', Key_o = 'o', Key_p = 'p', 
	// Key_q = 'q', Key_r = 'r', Key_s = 's', Key_t = 't', Key_u = 'u', Key_v = 'v', Key_w = 'w', Key_x = 'x', 
	// Key_y = 'y', Key_z = 'z',
};