EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Transmitter"
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATtiny:ATtiny13A-PU U1
U 1 1 5E10FBA6
P 5450 2650
F 0 "U1" H 4921 2696 50  0000 R CNN
F 1 "ATtiny13A-PU" H 4921 2605 50  0000 R CNN
F 2 "transmitter:DIP-8_296" H 5450 2650 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/doc8126.pdf" H 5450 2650 50  0001 C CNN
	1    5450 2650
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Female J1
U 1 1 5E11053C
P 7500 2450
F 0 "J1" H 7528 2476 50  0000 L CNN
F 1 "FS1000A" H 7528 2385 50  0000 L CNN
F 2 "transmitter:Socket_3_My" H 7500 2450 50  0001 C CNN
F 3 "~" H 7500 2450 50  0001 C CNN
	1    7500 2450
	1    0    0    -1  
$EndComp
Text Notes 7150 2450 0    50   ~ 0
VCC
Text Notes 7150 2300 0    50   ~ 0
Tx
Text Notes 7150 2650 0    50   ~ 0
GND
$Comp
L Connector:Conn_01x05_Female J2
U 1 1 5E110C1E
P 7500 3400
F 0 "J2" H 7528 3426 50  0000 L CNN
F 1 "KY-023 Joystick" H 7528 3335 50  0000 L CNN
F 2 "transmitter:Socket_5_My" H 7500 3400 50  0001 C CNN
F 3 "~" H 7500 3400 50  0001 C CNN
	1    7500 3400
	1    0    0    -1  
$EndComp
Text Notes 7200 3150 0    50   ~ 0
SW
Text Notes 7200 3300 0    50   ~ 0
VRy
Text Notes 7200 3400 0    50   ~ 0
VRx
Text Notes 7200 3500 0    50   ~ 0
PWR
Text Notes 7200 3600 0    50   ~ 0
GND
$Comp
L power:GND #PWR04
U 1 1 5E1115B3
P 5450 3900
F 0 "#PWR04" H 5450 3650 50  0001 C CNN
F 1 "GND" H 5455 3727 50  0000 C CNN
F 2 "" H 5450 3900 50  0001 C CNN
F 3 "" H 5450 3900 50  0001 C CNN
	1    5450 3900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5E111A9E
P 6700 1850
F 0 "R1" H 6770 1896 50  0000 L CNN
F 1 "10K" H 6770 1805 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 6630 1850 50  0001 C CNN
F 3 "~" H 6700 1850 50  0001 C CNN
	1    6700 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 5E111CF3
P 6350 2000
F 0 "C1" V 6095 2000 50  0000 C CNN
F 1 "100uF" V 6186 2000 50  0000 C CNN
F 2 "transmitter:CP_My" H 6388 1850 50  0001 C CNN
F 3 "~" H 6350 2000 50  0001 C CNN
	1    6350 2000
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5E112685
P 6750 3750
F 0 "R2" H 6820 3796 50  0000 L CNN
F 1 "10K" H 6820 3705 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 6680 3750 50  0001 C CNN
F 3 "~" H 6750 3750 50  0001 C CNN
	1    6750 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5E112ABA
P 7200 4200
F 0 "R3" H 7270 4246 50  0000 L CNN
F 1 "100K" H 7270 4155 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 7130 4200 50  0001 C CNN
F 3 "~" H 7200 4200 50  0001 C CNN
	1    7200 4200
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5E113184
P 6750 4250
F 0 "C2" H 6865 4296 50  0000 L CNN
F 1 "100nF" H 6865 4205 50  0000 L CNN
F 2 "transmitter:R_my_5_08" H 6788 4100 50  0001 C CNN
F 3 "~" H 6750 4250 50  0001 C CNN
	1    6750 4250
	1    0    0    -1  
$EndComp
NoConn ~ 6050 2350
$Comp
L power:VCC #PWR02
U 1 1 5E113C13
P 4150 1800
F 0 "#PWR02" H 4150 1650 50  0001 C CNN
F 1 "VCC" H 4167 1973 50  0000 C CNN
F 2 "" H 4150 1800 50  0001 C CNN
F 3 "" H 4150 1800 50  0001 C CNN
	1    4150 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 1800 4300 1800
Wire Wire Line
	5450 1700 5650 1700
Wire Wire Line
	6700 2000 6500 2000
Wire Wire Line
	6200 2000 6200 3400
Wire Wire Line
	6200 3800 5450 3800
Wire Wire Line
	5450 3800 5450 3900
Wire Wire Line
	5450 3800 5450 3300
Connection ~ 5450 3800
Wire Wire Line
	6700 2000 6700 2850
Wire Wire Line
	6700 2850 6050 2850
Connection ~ 6700 2000
Wire Wire Line
	6950 1700 6700 1700
Connection ~ 6700 1700
Wire Wire Line
	6550 2550 6050 2550
Wire Wire Line
	7300 2550 6950 2550
Wire Wire Line
	6950 2550 6950 3400
Wire Wire Line
	6950 3400 6200 3400
Connection ~ 6200 3400
Wire Wire Line
	6200 3400 6200 3500
Wire Wire Line
	6750 4400 6750 4650
Wire Wire Line
	6750 4650 6200 4650
Wire Wire Line
	6200 4650 6200 3800
Connection ~ 6200 3800
Wire Wire Line
	6750 4100 6750 4000
Wire Wire Line
	7200 4350 8300 4350
Wire Wire Line
	8300 4350 8300 1700
Wire Wire Line
	8300 1700 7150 1700
Connection ~ 6950 1700
Wire Wire Line
	7200 4050 7200 3850
Wire Wire Line
	7200 3850 7050 3850
Wire Wire Line
	7050 3850 7050 3600
Wire Wire Line
	7050 3600 6750 3600
Wire Wire Line
	7300 3200 7050 3200
Wire Wire Line
	7050 3200 7050 3600
Connection ~ 7050 3600
Wire Wire Line
	6750 4000 6600 4000
Wire Wire Line
	6600 4000 6600 2750
Wire Wire Line
	6600 2750 6400 2750
Wire Wire Line
	6400 2750 6400 2450
Wire Wire Line
	6400 2450 6050 2450
Connection ~ 6750 4000
Wire Wire Line
	6750 4000 6750 3900
Wire Wire Line
	7300 3600 7150 3600
Wire Wire Line
	7150 3600 7150 3500
Wire Wire Line
	7150 3500 6200 3500
Connection ~ 6200 3500
Wire Wire Line
	6200 3500 6200 3800
Wire Wire Line
	7300 3500 7200 3500
Wire Wire Line
	7200 3500 7200 3450
Wire Wire Line
	7200 3450 7150 3450
Wire Wire Line
	7150 3450 7150 1700
Connection ~ 7150 1700
Wire Wire Line
	7150 1700 6950 1700
Wire Wire Line
	7300 3400 7000 3400
Wire Wire Line
	7000 3400 7000 2750
Wire Wire Line
	7000 2750 6800 2750
Wire Wire Line
	6800 2750 6800 2700
Wire Wire Line
	6800 2700 6350 2700
Wire Wire Line
	6350 2700 6350 2750
Wire Wire Line
	6350 2750 6050 2750
Wire Wire Line
	7300 3300 7100 3300
Wire Wire Line
	7100 3300 7100 2650
Wire Wire Line
	7100 2650 6050 2650
$Comp
L power:GND #PWR03
U 1 1 5E11AFBF
P 4350 3950
F 0 "#PWR03" H 4350 3700 50  0001 C CNN
F 1 "GND" H 4355 3777 50  0000 C CNN
F 2 "" H 4350 3950 50  0001 C CNN
F 3 "" H 4350 3950 50  0001 C CNN
	1    4350 3950
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR01
U 1 1 5E11B1C4
P 3950 4000
F 0 "#PWR01" H 3950 3850 50  0001 C CNN
F 1 "VCC" H 3968 4173 50  0000 C CNN
F 2 "" H 3950 4000 50  0001 C CNN
F 3 "" H 3950 4000 50  0001 C CNN
	1    3950 4000
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5E11B65F
P 3950 3700
F 0 "#FLG01" H 3950 3775 50  0001 C CNN
F 1 "PWR_FLAG" H 3950 3873 50  0000 C CNN
F 2 "" H 3950 3700 50  0001 C CNN
F 3 "~" H 3950 3700 50  0001 C CNN
	1    3950 3700
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG02
U 1 1 5E11BB2F
P 4350 3700
F 0 "#FLG02" H 4350 3775 50  0001 C CNN
F 1 "PWR_FLAG" H 4350 3873 50  0000 C CNN
F 2 "" H 4350 3700 50  0001 C CNN
F 3 "~" H 4350 3700 50  0001 C CNN
	1    4350 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 3700 4350 3950
Wire Wire Line
	3950 4000 3950 3700
$Comp
L Connector:Conn_01x02_Male J3
U 1 1 5E120CD2
P 3800 2100
F 0 "J3" H 3908 2281 50  0000 C CNN
F 1 "Conn_01x02_Male" H 3908 2190 50  0000 C CNN
F 2 "transmitter:Socket_2_My" H 3800 2100 50  0001 C CNN
F 3 "~" H 3800 2100 50  0001 C CNN
	1    3800 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 2100 4300 2100
Wire Wire Line
	4300 2100 4300 1800
Wire Wire Line
	4000 2200 4000 3300
Wire Wire Line
	4000 3300 5450 3300
Connection ~ 5450 3300
Wire Wire Line
	5450 3300 5450 3250
$Comp
L Device:R R4
U 1 1 5E1162C5
P 8550 1850
F 0 "R4" H 8620 1896 50  0000 L CNN
F 1 "1K" H 8620 1805 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 8480 1850 50  0001 C CNN
F 3 "~" H 8550 1850 50  0001 C CNN
	1    8550 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5E1168B1
P 8550 2600
F 0 "R5" H 8620 2646 50  0000 L CNN
F 1 "2.2K" H 8620 2555 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 8480 2600 50  0001 C CNN
F 3 "~" H 8550 2600 50  0001 C CNN
	1    8550 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5E116ED4
P 8550 3100
F 0 "R6" H 8620 3146 50  0000 L CNN
F 1 "470" H 8620 3055 50  0000 L CNN
F 2 "transmitter:R_my_5_08" V 8480 3100 50  0001 C CNN
F 3 "~" H 8550 3100 50  0001 C CNN
	1    8550 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 1700 5450 2050
$Comp
L Regulator_Linear:LM317_3PinPackage U2
U 1 1 5E117818
P 9250 1450
F 0 "U2" H 9250 1692 50  0000 C CNN
F 1 "LM317_3PinPackage" H 9250 1601 50  0000 C CNN
F 2 "transmitter:Socket_3_My" H 9250 1700 50  0001 C CIN
F 3 "http://www.ti.com/lit/ds/symlink/lm317.pdf" H 9250 1450 50  0001 C CNN
	1    9250 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 1450 4800 2100
Wire Wire Line
	4800 2100 4300 2100
Connection ~ 4300 2100
Wire Wire Line
	8550 2000 8550 2250
Wire Wire Line
	9250 1750 9250 2250
Wire Wire Line
	9250 2250 8550 2250
Connection ~ 8550 2250
Wire Wire Line
	8550 2250 8550 2450
Wire Wire Line
	8550 2750 8550 2950
Wire Wire Line
	6750 4650 8550 4650
Wire Wire Line
	8550 4650 8550 3250
Connection ~ 6750 4650
Wire Wire Line
	9550 1450 9800 1450
Wire Wire Line
	9800 1450 9800 950 
Wire Wire Line
	9800 950  5650 950 
Wire Wire Line
	5650 950  5650 1700
Connection ~ 5650 1700
Wire Wire Line
	5650 1700 6700 1700
Wire Wire Line
	4800 1450 8950 1450
Wire Wire Line
	9800 1450 9800 1950
Wire Wire Line
	9800 1950 8900 1950
Wire Wire Line
	8900 1950 8900 1700
Wire Wire Line
	8900 1700 8550 1700
Connection ~ 9800 1450
Wire Wire Line
	7300 2450 6950 2450
Wire Wire Line
	6950 1700 6950 2450
Wire Wire Line
	7300 2350 6550 2350
Wire Wire Line
	6550 2350 6550 2550
$EndSCHEMATC