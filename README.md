lissa-OS
========

lissa-OS, or more simply LISOS, is an "operating system" created with a
msp-430g2231 launchpad microcontroller and a HD44780 character LCD. Users interface 
with it through a single push-button, which controls input for two OS "activites" -
a single-digit calculator, and a 24-hour clock. For the calculator, users input 
two digits, each 0-9, and the operator they wish to use on them. For the clock, 
users input the time, and the msp430 timers do the rest. 

Pin connections I used are listed in the main.c file. Credit for the LCD_MSP430.h file, which provides
some basic functions for interfacing the HD44780 with the msp430 in 4-bit mode,
goes to Manpreet Singh Minhas, in his article here:
http://learningmsp430.wordpress.com/2013/11/16/16x2-lcd-interfacing-in-4-bit-mode/

I'll be posting a video soon of the final working product :)
