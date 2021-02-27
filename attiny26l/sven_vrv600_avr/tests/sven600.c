#include <stdio.h>
#include <stdint.h>

#define V100 300

uint8_t beg = 0;
uint16_t waveMax = 0;
uint16_t tmpMax = 0;
uint16_t prevVal = 0;

#define UNDEF 15000

#define END 15001

uint16_t getWaveMax(uint16_t in_data){
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

/*
 *      /\
 * ----x--x----
 * ====    ====
 */
uint16_t test_array1[] = { 0, 10, 20 ,30, 310, 320, 340, 310, 300, 100, 40, 20, 0, 0, END };
/*
 *      /\_,_
 * ----x-----x--
 * ====       ==
 */
uint16_t test_array2[] = { 0, 10, 20 ,30, 310, 320, 340, 310, 320, 330, 320, 310, 100, 0, END };
/*          /\
 *      /\_/  \
 * ----x-------x
 * ====         ====
 */
uint16_t test_array3[] = { 0, 10, 20 ,30, 310, 320, 340, 310, 320, 330, 340, 350, 320, 310, 100, 0, END };
/*          
 *      /\ /\
 * ----x--x--x
 * ====       ====
 */
uint16_t test_array4[] = { 0, 10, 20 ,30, 310, 320, 340, 310, 290, 330, 330, 325, 320, 310, 100, 0, END };
uint16_t test_array5[] = { 100, 350, 100 ,350, 100, 350, 100, 350, 100, 350, 100, 350, 100, 350, 100, END };
uint16_t test_array6[] = { 100, 350, 350 ,100, 350, 350, 100, 350, 350, 100, 350, 350, 100, END };

uint16_t findUntilEnd(uint16_t array[]){
	int index = 0;
	uint16_t dt = 0;
	uint16_t dt2 = 0;
	while(array[index]!=END){
		dt = getWaveMax(array[index]);
		if (dt!=UNDEF){
			dt2 = dt;
		}
		index++;
	}
	return dt2;
}

uint16_t findUntilUNDEF(uint16_t array[]){
	int index = 0;
	uint16_t dt = 0;
	uint16_t dt2 = 0;
	int begin = 0;
	while(array[index]!=END){
		dt = getWaveMax(array[index]);
		if (dt!=UNDEF){
			dt2 = dt;
			begin = 1;
		} else if (begin == 1){
			break;
		}
		index++;
	}
	return dt2;
}

void printfArray(uint16_t array[], const char *name){
	int index = 0;
	uint16_t dt = 0;
	printf("%s ", name);
	while(array[index]!=END){
		dt = getWaveMax(array[index]);
		if (dt != UNDEF)
			printf("%d=T(%d) ", array[index], dt);
		else
			printf("%d=T(U) ", array[index]);
		index++;
	}
	printf("\n");
	return;
}

int main(){
	printfArray(test_array1, "Test 1. One wave");
	printfArray(test_array2, "Test 2. One distorted wave");
	printfArray(test_array3, "Test 3. One dist wave second huge");
	printfArray(test_array4, "Test 4. Two waves");
	printf("Test 5 %d == 340. One wave\n", findUntilEnd(test_array1));
	printf("Test 6 %d == 340. One distorted wave\n", findUntilEnd(test_array2));
	printf("Test 7 %d == 350. One dist wave second huge\n", findUntilEnd(test_array3));
	printf("Test 8 %d == 330. Two waves, check both\n", findUntilEnd(test_array4));
	printf("Test 9 %d == 340. Two waves, check only first\n", findUntilUNDEF(test_array4));
	printfArray(test_array5, "Test 10. A lot of small waves");
	printfArray(test_array6, "Test 11. A lot of small waves 2");
	return 0;
}
