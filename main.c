#include "reg52.h"
#include "intrins.h"

typedef unsigned char u8;
typedef unsigned int u16;

typedef enum  {
    discrete_press, 
    discrete_release, 
    continuous, 
    trigger_delay_continuous, 
    none_trigger
}botton_mode;

#define KEY_PRESSED     0
#define KEY_RELEASE     1

#define LED_ON          0
#define LED_OFF         1

#define SMG_A_DP_PORT       P0
#define KEY_MATRIX_PORT     P1  

sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;
sbit BEEP = P2^5;
sbit K1 = 0xb1;
sbit K2 = 0xb0;
sbit K3 = 0xb2;     // 归零
sbit K4 = 0xb3;     // 复位


extern u8 gsmg_code[17] = {
    0x00,
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};
static u16 digital_display_array[8] = {
    0, 0, 0, 0,
    0, 0, 0, 0
};
static u16 digital_display_history[8] = {
    0, 0, 0, 0,
    0, 0, 0, 0
};
const u16 digital_display_origin[8] = {
    0, 0, 0, 0,
    0, 0, 0, 0
};

/*  ----------------PROTOTYPES--------------------  */

void delay_10us(u16 time_10us);
u8 key_matrix_ranks_scan(void);
u8 key_matrix_flip_scan(void);
void LS_select(u8);
void display();
void buzzer(u16 frequency, u16 lasting_time);       // to detect whether bottom is pressed
void reset();
void set_zero();

/*  ----------------MAIN--------------------------- */

void main() {
    u8 key = 0;
    u8 i;
    while(1){
        display();
        reset();
        set_zero();
        if(key = key_matrix_flip_scan()) {
            buzzer(5000, 8e3);
            for(i = 0; i < 7; i++) {
                digital_display_array[i] = digital_display_array[i + 1];
            }
            digital_display_array[7] = key;
        }
    }
}

/*  -----------------FUNCTIONS--------------------   */

u8 key_matrix_flip_scan(void) {
    static u8 key_value = 0;
    static botton_mode key_matrix_mode = discrete_release;
    KEY_MATRIX_PORT = 0x0f;
    delay_10us(2);
    if(KEY_MATRIX_PORT != 0x0f && key_matrix_mode == discrete_release) {
        delay_10us(1e3);
        if(KEY_MATRIX_PORT != 0x0f){
            key_matrix_mode = none_trigger;
            KEY_MATRIX_PORT = 0x0f;
            switch(KEY_MATRIX_PORT) {
                case 0x07: key_value = 1; break;
                case 0x0b: key_value = 2; break;
                case 0x0d: key_value = 3; break;
                case 0x0e: key_value = 4; break;
            }

            KEY_MATRIX_PORT = 0xf0;
            delay_10us(1);
            switch(KEY_MATRIX_PORT) {
                case 0x70: key_value = key_value; break;
                case 0xb0: key_value = key_value + 4; break;
                case 0xd0: key_value = key_value + 8; break;
                case 0xe0: key_value = key_value + 12; break;
            }
        }
        return 0;
    }
    else if(KEY_MATRIX_PORT != 0x0f && key_matrix_mode == none_trigger) {
        return 0;
    }
    else if(KEY_MATRIX_PORT == 0x0f && key_matrix_mode == discrete_release) {
        return 0;
    }
    else if(KEY_MATRIX_PORT == 0x0f && key_matrix_mode == none_trigger) {
        key_matrix_mode = discrete_release;
    }
    return key_value;
}

u8 key_matrix_ranks_scan(void) {
    u8 key_value = 0;
    KEY_MATRIX_PORT = 0xf7;
    if(KEY_MATRIX_PORT != 0xF7) {
        delay_10us(1e3);
        switch(KEY_MATRIX_PORT) {
            case 0x77:
                key_value = 1;
                break;
            case 0xb7:
                key_value = 5;
                break;
            case 0xd7:
                key_value = 9;
                break;
            case 0xe7:
                key_value = 13;
                break;
            default:
                key_value = 0;
                break;
        }
    }
    while(KEY_MATRIX_PORT != 0xf7);

    KEY_MATRIX_PORT = 0xfb;
    if(KEY_MATRIX_PORT != 0xfb) {
        delay_10us(1e3);
        switch(KEY_MATRIX_PORT) {
            case 0x7b:
                key_value = 2;
                break;
            case 0xbb:
                key_value = 6;
                break;
            case 0xdb:
                key_value = 10;
                break;
            case 0xeb:
                key_value = 14;
                break;
            default:
                key_value = 0;
                break;
        }
    }
    while(KEY_MATRIX_PORT != 0xfb);

    KEY_MATRIX_PORT = 0xfd;
    if(KEY_MATRIX_PORT != 0xfd) {
        delay_10us(1e3);
        switch(KEY_MATRIX_PORT) {
            case 0x7d:
                key_value = 3;
                break;
            case 0xbd:
                key_value = 7;
                break;
            case 0xdd:
                key_value = 11;
                break;
            case 0xed:
                key_value = 15;
                break;
            default:
                key_value = 0;
                break;
        }
    }
    while(KEY_MATRIX_PORT != 0xfd);

    KEY_MATRIX_PORT = 0xfe;
    if(KEY_MATRIX_PORT != 0xfe) {
        delay_10us(1e3);
        switch(KEY_MATRIX_PORT) {
            case 0x7e:
                key_value = 4;
                break;
            case 0xbe:
                key_value = 8;
                break;
            case 0xde:
                key_value = 12;
                break;
            case 0xee:
                key_value = 16;
                break;
            default:
                key_value = 0;
                break;
        }
    }
    while(KEY_MATRIX_PORT != 0xfe);

    return key_value;
}

void delay_10us(u16 time_10us)	//@11.0592MHz
{
	unsigned char data i;
    while(time_10us--) {
        i = 2;
        while (--i);
    }
}

void LS_select(u8 i) {
    switch(i) {
        case 0:
            LSC = 1;
            LSB = 1;
            LSA = 1;
            break;
        case 1:
            LSC = 1;
            LSB = 1;
            LSA = 0;
            break;
        case 2:
            LSC = 1;
            LSB = 0;
            LSA = 1;
            break;
        case 3:
            LSC = 1;
            LSB = 0;
            LSA = 0;
            break;
        case 4:
            LSC = 0;
            LSB = 1;
            LSA = 1;
            break;
        case 5:
            LSC = 0;
            LSB = 1;
            LSA = 0;
            break;
        case 6:
            LSC = 0;
            LSB = 0;
            LSA = 1;
            break;
        case 7:
            LSC = 0;
            LSB = 0;
            LSA = 0;
            break;
        default:
            P0 = 0xff;
    }
}

void display() {
    u8 i;
    // if(strcmp(digital_display_array, digital_display_history)) {
    //     for(i = 0; i < 8; i++){
    //         LS_select(i);
    //         SMG_A_DP_PORT = 0x00;
    //     }
    //     for(i = 0; i < 8; i++) {
    //         digital_display_history[i] = digital_display_array[i];
    //     }
    //     delay_10us(10);
    // }
    for(i = 0; i < 8 ; i++) {
        SMG_A_DP_PORT = 0x00;
        delay_10us(5);
        LS_select(i);
        SMG_A_DP_PORT = gsmg_code[digital_display_array[i]];
        delay_10us(5);
    }
}

void buzzer(u16 frequency, u16 lasting_time) {
	u16 T = 10e5 / frequency;
	u16 vibrations = lasting_time / T;
	while(vibrations--) {
		BEEP = !BEEP;
		delay_10us(T / 2);
		BEEP = !BEEP;
		delay_10us(T / 2);
	}
}

void reset() {
    if(K4 == 0) {
        u8 i = 0;
        delay_10us(1e3);
        if(K4 == 0) {
            for(i = 0; i < 8; i ++) {
                digital_display_array[i] = 0;
            } 
            delay_10us(2);
        }
    }
}

void set_zero() {
    if(K3 == 0) {
        u8 i;
        delay_10us(1e3);
        if(K3 == 0) {
            for (i = 0; i < 7; i++) {
                digital_display_array[i] = 0;
            }
            digital_display_array[7] = 1;
        }
        delay_10us(2);
    }
}