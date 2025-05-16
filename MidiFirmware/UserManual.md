# USB Midi HID User Manual

This user manual uses Linux examples with Grandorgue. For other instruments and Operating Systems similar commands will exist.

## Midi Messages

|  Control   |   Midi Message  | Direction |
|:----------:|:---------------:|:---------:|
| Keyboard   | Midi Note 32-96 | To Comp   |
| Pedalboard | Midi Note 32-64 | To Comp   |
| Buttons    | Midi Note 0-127 | To   Comp |  
| Button LEDs| Midi Note 0-127 | From Comp |
| Expression | Midi CC 32-40   | To   Comp |
| LED 7Seg   | Midi Sysex 32   | From Comp |
| LCD Display| Midi Sysex      | From Comp |




## The USB Connection
To discover whether the midi interface is successfully connected to the computer type 'lsusb' at the command prompt. The midi interfaces and the configured midi channel numbers for keyboard/pedalboard and illuminated buttons should show up in the response.
```
$ lsusb
   ...
   Bus 001 Device 094: ID 1eaf:0031 Leaflabs VPO Console Midi Ch1, 9
   Bus 001 Device 095: ID 1eaf:0031 Leaflabs VPO Console Midi Ch2, 10
    ...
```
# Monitoring Midi Messages
Use 'midisnoop' to monitor messages sent between the keyboard/pedalboard and the computer. Use Alsa unless you have Jack configured.
# Making Midi Connections
Use 'qjackctl' graph function to view or connect midi outputs to midi inputs
Connect grandorgue output to midisnoop input using the qjacktctl graph

## Configuring the Midi Interface
A USB serial connection is used to communicate with the Midi Interface.
Connect one STM32 BluePill to the computer using a USB cable.
Install 'minicom' and configure it to connect to /dev/ttyACM0.
Once connected successfully expand the minicom window and press the spacebar.
You should see a display that shows which pin functions are assigned.
Select what you require using the A-J keys, then press 'S' to save.
```
STM32 Blue Pill Assigned Pin Functions
IP_SR_DATA    PB12   USB   GND    
Scan In  0    PB13         GND    
Scan In  1    PB14         3V3    
Scan In  2    PB15         NRST   
Scan In  3    PA_8         PB11   OP_SR_CLOCK
Scan In  4    PA_9         PB10   TM1637_CK
Scan In  5    PA10         PB_1   Scan In  8
USB-          PA11         PB_0   Scan In  9
USB+          PA12         PA_7   Scan In  a
Scan In  6    PA15         PA_6   Scan In  b
Scan In  7    PB_3         PA_5   Scan In  c
Scan Out 4    PB_4         PA_4   Scan In  d
Scan Out 5    PB_5         PA_3   Scan In  e
LCD  I2C_SCL  PB_6         PA_2   Scan In  f
ShareI2C_SDA  PB_7         PA_1   Scan Out 3
Scan Out 6    PB_8         PA_0   Scan Out 2
Scan Out 7    PB_9         PC15   Scan Out 1
               +5V         PC14   Scan Out 0
               GND         PC13   OP_LED
               3V3 | | | | VBAT   
                   | | | |
SWDIO         PA13---+ +---PA14   SWCLK
BOOT1         PB_2 (on Jumper via 100K resistor)

Configuration Menu
A [3] Key/Pedal Midi Channel, Button channel=11
B [ ] Pedalboard & Expression Pedals (ADC inputs)
C [0]   Number of ADC Inputs PA2-PB1
D [✓] Keyboard Velocity Reporting
E [✓] PCF8574 I2C LCDs 16x2
F [✓] TM1637 7 segment, 6 digit display
G [✓] 74HC164 Shift register LED buttons
H [✓] 74HC164 Shift register LED invert
I [ ] Use Debug Connector for WS2812 RGB LEDs PA13,PA14
J [ ] Event Log to USB Serial (may slow response time)
Cfg.Word=3E02
Enter A-L,a-l To adjust cfg value, S to Save, ? - Menu:
```
The event log is usefule to monitor midi messages being sent to the computer. However
using the event log may slow down performance and so should not be enabled during normal use.
# Hardware Interface Menu
To monitor activity on attached devices type '?' to view the hardware interface menu.
```
Midi Interface Menu
A-J Adjust config value
K - Keyboard Contacts
L - LCD check
M - Flash memory summary
O - Shift Register IO
P - Profile
S - Save New Configuration
T - Test IO Pins
R - Restore Configuration
V - Version Info
Z - Display Status
```
### 'K' Keyboard scan result.
The MAudio Keystation 61 Mk3 music keyboard has two contacts per key to sense velocity. Keyboard scanning uses 8 outputs and 16 inputs. When pedalboard is selected the top 8 keyboard input lines are re-assigned to be used as expression pedal (ADC) inputs. The pedalboard scans 32 contacts using 8 outputs and 8 inputs. Alternate inputs are unused when there is no velocity sensing.
```
Keyboard Scan Result Midi Channel 3

0000000000000000 OP0  CN13 PIN4
0000000000000000 OP1  CN13 PIN3
0000000000000000 OP2  CN13 PIN2
0000000000000000 OP3  CN13 PIN1
0000000000000000 OP4  CN14 PIN4
0000000000000000 OP5  CN14 PIN3
0000000000000000 OP6  CN14 PIN2
0000000000000000 OP7  CN14 PIN1
........******** IP *=CN13 input pins 5-6,7=nc,8-13
********........ IP *=CN14 input pins 5-12
```

### 'L' LCD Check

-  I2C Display addresses are configured by PCF8574 solder bridge inputs on the LCD modules, these addresses are then configured in Grandorgue and map through the blue pill. Expect address 0x38-0x3f for Philips/NXP ICs and 0x20-0x27 for TI. The highest address is used when no solder bridges are fitted. Note that Grandorgue specifies the addresses in Base10.

### 'M' Flash Memory Summary
Not really a user feature.  Provided for testing the 'S'ave command.
```
Flash Memory Summary

0800C850 0000 0000 0000 0000 0000 0000 A678 0800
0800C860 FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
...
0801F7F0 FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
0801F800 DEAD 7F58 0000 1000 1E00 1E01 1E03 1E02
...
0801F820 BE72 3E12 3E02 FFFF FFFF FFFF FFFF FFFF
0801F830 FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
...
```

```
Shift Register Midi Channel 11 Note Numbers

SR 1   00  i01o  02   03   04   05   06   07  
SR 2   08   09   10   11   12   13   14   15  
SR 3   16   17   18   19   20   21   22   23  
SR 4   24   25   26   27   28   29   30   31  
SR 5   32   33   34   35   36   37   38   39  

i = input on, o = output on
```

### 'Z' Pin Status
This screen may be useful for viewing the 12 bit ADC results before filtering and conversion to 7 bit midi values.
```
USB Midi Interface Status
id  port   function     kbd  count input error fault
 0  VBAT                 0     0     0     0    ' ' 
 1  PC13       OP_LED    0     0     0     0    ' ' 
 2  PC14   Scan Out 0    0     0     0     0    ' ' 
 3  PC15   Scan Out 1    0     1     0     0    ' ' 
 4  PA_0   Scan Out 2    0     2     0     0    ' ' 
 5  PA_1   Scan Out 3    0     3     0     0    ' ' 
 6  PA_2    IP_ADC  0    0     0    e0    e0    ' ' 
 7  PA_3    IP_ADC  1    0     1   36e   37a    ' ' 
 8  PA_4    IP_ADC  2    0     2   509   51e    ' ' 
 9  PA_5    IP_ADC  3    0     3   749   75c    ' ' 
10  PA_6    IP_ADC  4    0     4   8b4   8a1    ' ' 
11  PA_7    IP_ADC  5    0     5   8d8   8c0    ' ' 
12  PB_0    IP_ADC  6    0     6   a59   a43    ' ' 
13  PB_1    IP_ADC  7    0     7   e10   e1b    ' ' 
14  PB10    TM1637_CK    0     0     0     0    ' ' 
15  PB11  OP_SR_CLOCK    0    60     0     0    ' ' 
16  NRST                 0     0     0     0    ' ' 
17   3V3                 0     0     0     0    ' ' 
18   GND                 0     0     0     0    ' ' 
19   GND                 0     0     0     0    ' ' 
20  PB12   IP_SR_DATA    0     0     0     0    ' ' 
21  PB13   Scan In  0    0     0     0     0    ' '
22  PB14   Scan In  1    0     1     0     0    ' ' 
23  PB15   Scan In  2    0     2     0     0    ' ' 
24  PA_8   Scan In  3    0     3     0     0    ' ' 
25  PA_9   Scan In  4    0     4     0     0    ' ' 
26  PA10   Scan In  5    0     5     0     0    ' ' 
27  PA11         USB-    0     0     0     0    ' ' 
28  PA12         USB+    0     0     0     0    ' ' 
29  PA15   Scan In  6    0     6     0     0    ' ' 
30  PB_3   Scan In  7    0     7     0     0    ' ' 
31  PB_4   Scan Out 4    0     4     0     0    ' ' 
32  PB_5   Scan Out 5    0     5     0     0    ' ' 
33  PB_6 LCD  I2C_SCL    0     0     0     0    '!' 
34  PB_7 ShareI2C_SDA    0     0     0     0    '!' 
35  PB_8   Scan Out 6    0     6     0     0    ' ' 
36  PB_9   Scan Out 7    0     7     0     0    ' ' 
37   +5V                 0     0     0     0    ' ' 
38   GND                 0     0     0     0    ' ' 
39   3V3                 0     0     0     0    ' ' 
40  PA13  WS2812 LEDs    0     0     0     0    ' ' 
41  PA14  WS2812 LEDs    0     0     0     0    ' ' 
42  PB_2        BOOT1    0     0     0     0    ' ' 

```
