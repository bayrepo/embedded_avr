#include <stdio.h>
#include <stdint.h>


#define V180 568
#define V205 647
#define V235 742
#define V260 821
#define V100 315

#define V10 16 //V100/20

#ifndef byte
#define byte unsigned char
#endif

#define UNDEF 15000
#define END 15001

int ind = 0;

uint16_t convert(int val){
	return (val * 573 / 1810);
}

uint16_t array[] = { 
	200, 250, 315, 340, 500, 560, 450, 320, 150, 100, //177
	200, 250, 315, 340, 500, 568, 450, 320, 150, 100, //180
	200, 250, 315, 340, 500, 555, 450, 320, 150, 100, //175
	200, 250, 315, 340, 500, 540, 450, 320, 150, 100, //171
	200, 250, 315, 340, 500, 560, 450, 320, 150, 100, //177
	200, 250, 315, 340, 500, 648, 450, 320, 150, 100, //205
	200, 250, 315, 340, 500, 638, 450, 320, 150, 100, //200
	200, 250, 315, 340, 500, 650, 450, 320, 150, 100, //205
	200, 250, 315, 340, 500, 600, 450, 320, 150, 100, //205
	END
	};

uint16_t readADC(uint8_t adc_num){
    return array[ind++];
}

uint16_t adc_temp = 0, adc_temp2 = 0;
uint8_t flag = 0;

//For debug
int TmpLineIn = 0;
int TmpTemper = 0;
int Tmpcounter = 0;
uint8_t beg = 0;
uint16_t waveMax = 0;
uint16_t tmpMax = 0;
uint16_t prevVal = 0;

uint16_t getWaveMax(uint16_t in_data){
	if (in_data == END) return END;
    if (!beg){
        if (in_data>V100 && !waveMax){
            waveMax = in_data;
            } else if (in_data>V100 && prevVal<=in_data){
            if (waveMax < in_data) waveMax = in_data;
            beg = 1;
            } else {
            waveMax = 0;
        }
        } else {
        if (waveMax<in_data){
            waveMax = in_data;
            } else {
            if (in_data<=V100){
                tmpMax = waveMax;
                waveMax = 0;
                beg = 0;
                prevVal = 0;
                return tmpMax;
            }
        }
    }
    prevVal = in_data;
    return UNDEF;
}

int cond = 0; //0 < 180, 1 < 205, 2 < 235, 3 < 260, 4 > 260

uint16_t getLimit(uint16_t val){
    if (val == V180){
        if (cond == 1) return (V180 - V10);
    } else if (val == V205) {
        if (cond == 2) return (V205 - V10);
    } else if (val == V235) {
        if (cond == 3) return (V235 - V10);
    } else if (val == V260) {
        if (cond == 4) return (V260 - V10);
    }
    return val;
}

int main(void)
{  
    cond = 0;

	adc_temp = 0;
    //WorkMode
    while (1)
    {   
        adc_temp = getWaveMax(readADC(1)); //LineIn
        if (adc_temp == END) break;
        if (adc_temp!=UNDEF) {
            if ((adc_temp< getLimit(V180)) || (adc_temp>getLimit(V260))){
                if (adc_temp < getLimit(V180)) {
					cond = 0;
					printf("%d) Volt %d cond %d\n", ind-1, convert(adc_temp), cond);
				}
                if (adc_temp > getLimit(V260)) {
					cond = 4;
					printf("%d) Volt %d cond %d\n", ind-1, convert(adc_temp), cond);
				}
                continue;
            }
        }
        if (adc_temp==UNDEF){
			printf("%d) Volt UNDEF=%d\n", ind-1, convert(array[ind-1]));
            continue;
        }
        if (adc_temp<getLimit(V205)){
            cond = 1;
			printf("%d) Volt %d cond %d\n", ind-1, convert(adc_temp), cond);
            continue;
        }
        if (adc_temp>getLimit(V235)){
            cond = 3;
			printf("%d) Volt %d cond %d\n", ind-1, convert(adc_temp), cond);
            continue;
        }
        cond = 2;
		printf("%d) Volt %d cond %d\n", ind-1, convert(adc_temp), cond);
    }
}

