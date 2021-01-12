EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:Conn_01x10_Female J1
U 1 1 60018D12
P 2850 3600
F 0 "J1" H 2742 2875 50  0000 C CNN
F 1 "Sensor_Conn" H 2742 2966 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 2850 3600 50  0001 C CNN
F 3 "~" H 2850 3600 50  0001 C CNN
	1    2850 3600
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x03_Female J2
U 1 1 6001AAFF
P 4150 2700
F 0 "J2" V 4088 2512 50  0000 R CNN
F 1 "SENSOR_LIGHT" V 3997 2512 50  0000 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 4150 2700 50  0001 C CNN
F 3 "~" H 4150 2700 50  0001 C CNN
	1    4150 2700
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J4
U 1 1 6001C3E7
P 5200 2700
F 0 "J4" V 5138 2512 50  0000 R CNN
F 1 "SENSOR_BATHROOM" V 5047 2512 50  0000 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 5200 2700 50  0001 C CNN
F 3 "~" H 5200 2700 50  0001 C CNN
	1    5200 2700
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J6
U 1 1 6001CA27
P 6300 2700
F 0 "J6" V 6238 2512 50  0000 R CNN
F 1 "SENSOR_BEDROOM" V 6147 2512 50  0000 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 6300 2700 50  0001 C CNN
F 3 "~" H 6300 2700 50  0001 C CNN
	1    6300 2700
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J3
U 1 1 6001D103
P 4150 4400
F 0 "J3" V 3996 4548 50  0000 L CNN
F 1 "SENSOR_OUTSIDE" V 4087 4548 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 4150 4400 50  0001 C CNN
F 3 "~" H 4150 4400 50  0001 C CNN
	1    4150 4400
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J5
U 1 1 6001DE6B
P 5200 4400
F 0 "J5" V 5046 4548 50  0000 L CNN
F 1 "SENSOR_WARDROBE" V 5137 4548 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 5200 4400 50  0001 C CNN
F 3 "~" H 5200 4400 50  0001 C CNN
	1    5200 4400
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J7
U 1 1 6001E275
P 6300 4400
F 0 "J7" V 6146 4548 50  0000 L CNN
F 1 "SENSOR_STAIRS" V 6237 4548 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 6300 4400 50  0001 C CNN
F 3 "~" H 6300 4400 50  0001 C CNN
	1    6300 4400
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x03_Female J8
U 1 1 6001E6B3
P 6800 3600
F 0 "J8" H 6828 3626 50  0000 L CNN
F 1 "SENSOR_KITCHEN" H 6828 3535 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 6800 3600 50  0001 C CNN
F 3 "~" H 6800 3600 50  0001 C CNN
	1    6800 3600
	1    0    0    -1  
$EndComp
Text GLabel 3050 4000 2    50   Input ~ 0
5V
Text GLabel 3050 3100 2    50   Input ~ 0
GND
Text GLabel 4050 4200 1    50   Input ~ 0
5V
Text GLabel 5100 4200 1    50   Input ~ 0
5V
Text GLabel 6200 4200 1    50   Input ~ 0
5V
Text GLabel 6400 2900 3    50   Input ~ 0
5V
Text GLabel 5300 2900 3    50   Input ~ 0
5V
Text GLabel 4250 2900 3    50   Input ~ 0
5V
Text GLabel 4050 2900 3    50   Input ~ 0
GND
Text GLabel 5100 2900 3    50   Input ~ 0
GND
Text GLabel 6200 2900 3    50   Input ~ 0
GND
Text GLabel 6400 4200 1    50   Input ~ 0
GND
Text GLabel 5300 4200 1    50   Input ~ 0
GND
Text GLabel 4250 4200 1    50   Input ~ 0
GND
Wire Wire Line
	3050 3900 4150 3900
Wire Wire Line
	4150 3900 4150 4200
Wire Wire Line
	3050 3800 5200 3800
Wire Wire Line
	5200 3800 5200 4200
Wire Wire Line
	3050 3700 6300 3700
Wire Wire Line
	6300 3700 6300 4200
Wire Wire Line
	3050 3300 4150 3300
Wire Wire Line
	4150 3300 4150 2900
Wire Wire Line
	3050 3400 5200 3400
Wire Wire Line
	5200 3400 5200 2900
Wire Wire Line
	3050 3500 6300 3500
Wire Wire Line
	6300 3500 6300 2900
Wire Wire Line
	3050 3600 6600 3600
Text GLabel 6600 3700 0    50   Input ~ 0
5V
Text GLabel 6600 3500 0    50   Input ~ 0
GND
NoConn ~ 3050 3200
$EndSCHEMATC
