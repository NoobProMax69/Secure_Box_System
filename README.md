# Secure_Box_System

  I chose to make a “Secure box” system made of 3 sensors: an RFID module, an IR
receiver and a flame sensor.

  This system is supposed to emulate a secure box that can be accessed by the user in
two different ways:
1. The user can access the secure box system with an RFID tag.
The chosen tag was set manually (hard coded) by using UART communication to
read the UID of the tag using the data visualiser.
2. The user can access the secure box in another way, by typing the code with a
remote controller.
Once again, I used UART communication to directly read the code from for each
button of the remote. 


Being a Secure box, it is also equipped with a fire detector sensor, which presents the
status of a nearby fire emergency on the LCD.

  - - - - - MATERIALS USED - - - - -
          - 2X breadboards (I am using two just to have more freedom with space allocation)
          - Arduino UNO (ATMega328p)
          - RFID Module and tags
          An RFID module is a sensor that uses radio frequency signals to communicate
wirelessly with an RFID tag. The way it works is the module emits a radio signal that powers
passive RFID tags, which then transmits their UID code.
          - IR Receiver with remote
          An IR receiver is a sensor that detects infrared light signals. A remote transfers
infrared light signals to the receiver, which are then converted to electrical signals and
interpreted by the microcontroller to determine which button was pressed.
          - Flame detector. A flame detector is a simple analog sensor that detects the presence of fire or a
very strong light source.
          - 16x02 lcd display
          - 3x buttons
          - 2x LEDs (One blue - OK_LED; One red - ERR_LED)
          - Resistors:  3x 220 ohm
                        1x 10  kohm
                        1x 1   kohm
          - Wires


  - - - - - HARDWARE PIN MAP - - - - -
LCD RS A0 – PC0
LCD E A1 – PC1
LCD D4 D4 – PD4
LCD D5 D5 – PD5
LCD D6 D6 – PD6
LCD D7 D7 – PD7
BUTT1(RFID) D2 – PD2
BUTT2(IR) A5 – PC5
BUTT3(FIRE) A2 – PC2
RFID RST D9 – PB9
RFID NSS D10 – PB10
RFID MOSI D11 – PB11
RFID MISO D12 – PB12
RFID SCK D13 – PB13
LED1 D8 – PB8
LED2 A3 – PC3
IR sensor D3 – PD3
Fire sensor A4 – PC4

