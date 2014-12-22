/* Runs an "operating system" on a 16x2 HD44780 character LCD using
* a msp430g2231 launchpad. Two main functions that it utilizes is a
* simple single-digit calculator and a 24-hour clock. User input
* comes in the form of a single push button. Pin connections are as follows:
*
* (all pins except P1.7 connect to the character LCD)
*
* P1.0  ----> Data output 1
* P1.1  ----> Data output 2
* P1.2  ----> Data output 3
* P1.3  ----> Data output 4
* P1.4  ----> Register select
* P1.5  ----> R/~W select
* P1.6  ----> Enable select
* P1.7  ----> Push button
*
*/

#include "LCD_MSP430.h"
#define BTN           BIT7
#define TRUE          1
#define FALSE         0
#define CLK_CONSTANT  10200
#define WAIT_TIME     300000


char* intToStr(int i, char str[], int length);
char digitToChar(int i);
void runClock();
void runCalc();
int getNumFromScreen(int max, char choices[], char message[]);
__interrupt void CCR0_ISR(void);

int seconds;
int minutes;
int hours;
 
void main(void) {
  WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer
  
  P1REN |=  BTN;  // Enable Port 1 P1.7 pull-up resistor
  P1SEL &= ~BTN;  // Select Port 1 P1.7, 0 selects
 
  lcd_init();
  send_string("Welcome to LISOS");
  send_command(0xC0); 
  send_string("Push to start");
 
  P1DIR &= ~BTN;
  while ((P1IN & BTN) == 0);  // wait for button press

  send_command(0x01);

  delay(500);

  send_command(0x80); 
  send_string("choose activity:");

  send_command(0xC1);
  send_string("calc");

  send_command(0xC8);
  send_string("clock");

  send_command(0xC1);
  int selection = TRUE;
  long count = 0;

  //configure push button as input to toggle between calc and clock activites
  P1DIR &= ~BTN;
  while(count < WAIT_TIME) {
    if ((P1IN & BTN) == BTN) {
      if(selection) {
        selection = FALSE;
        send_command(0xC8);
      } else {
        selection = TRUE;
        send_command(0xC1);
      }
      delay(200);
      count = 0;
    }
    count++;
  }

  send_command(0x01);
  if(selection) {
    runCalc();
  } else {
    runClock();
  }
   

}

void runCalc() {
  while(TRUE) {
    //Get numbers and operation from user
    int num1 = getNumFromScreen(9, "0123456789", "first number:");
    
    int operation = getNumFromScreen(3, "+-*/", "operation:");
    
    int num2 = getNumFromScreen(9, "0123456789", "second number:");
    
    int answer;
    
    //Check for the divide by 0 error
    if(!(operation == 3 && num2 == 0)) {
      switch(operation) {
      case 0: 
        answer = num1 + num2;
        break;
      case 1: 
        answer = num1 - num2;
        break;
      case 2: 
        answer = num1 * num2;
        break;
      case 3:
        answer = num1 / num2;
        break;
      default: answer = 0;
      }
      
      //Print answer to screen
      send_command(0x80);
      send_string("answer: ");
      send_command(0x88);
      char str[3];
      send_string(intToStr(answer, str, 3));
      
    } else {
      send_command(0x80);
      send_string("Divide by 0 :(");
    }
    
    long count = 0;
    while(count < WAIT_TIME) {
      count++;
    }
    send_command(0x01);
  }
}

//Presents the user with a number of options to pick, and 
//returns an integer representation of the user choice
int getNumFromScreen(int max, char choices[], char message[]) {
  send_command(0x81);
  send_string(message);
 
  send_command(0xC2);
  send_string(choices);
  
  int cursorPos = 0;
  send_command(0xC2);
  
  //cycle through the options
  long count = 0;
  while(count < WAIT_TIME) {
    if ((P1IN & BTN) == BTN) {

      if(cursorPos == max) {
        cursorPos = 0;
      } else {
        cursorPos++;
      }
      send_command(0xC2 + cursorPos);
      delay(200);
      count = 0;
    }
    count++;
     
   }
  send_command(0x01);
  return cursorPos;
}

void runClock() {
  //User can input 4 numbers for the hour and minute of a 24-hr clock
  int hour1 = getNumFromScreen(2, "012", "hour num 1:");
  int hour2 = getNumFromScreen(9, "0123456789", "hour num 2:");
  
  hours = (hour1 * 10) + hour2;
  hours = hours > 23 ? 23 : hours;
  
  int minute1 = getNumFromScreen(5, "012345", "minute num 1:");
  int minute2 = getNumFromScreen(9, "0123456789", "minute num 2:");
  
  minutes = (minute1 * 10) + minute2;
  
  seconds = 0;
  
  send_command(0x82); 
  send_string("24-hr clock");
  
  BCSCTL1 = CALBC1_1MHZ; // Running at 1 MHz
  DCOCTL = CALDCO_1MHZ;
  BCSCTL3 |= LFXT1S1;     // VLO mode

  TACCR0 = CLK_CONSTANT;  
         
  TACCTL0 = CCIE;         // Enable interrupts for CCR0 - trigger them
                          // approximately every second
  TACTL = TASSEL_1 + MC_1 + TACLR;  // ACLK, up mode, clear timer
 
  _BIS_SR(LPM3_bits + GIE);    // Enter LPM3 and enable interrupts
  
  while(TRUE){}
}

//Triggered every second to update the clock
#pragma vector = TIMERA0_VECTOR
__interrupt void CCR0_ISR(void) {
    seconds++;
    seconds %= 60;
    minutes = seconds == 0 ? minutes + 1 : minutes;
    minutes %= 60;
    hours = (minutes == 0 && seconds == 0) ? hours + 1 : hours;
    hours %= 24;
    
    send_command(0xC3); 
    char str[9] = {digitToChar(hours/10), digitToChar(hours%10), ':', 
                  digitToChar(minutes/10), digitToChar(minutes%10), ':', 
                  digitToChar(seconds/10), digitToChar(seconds%10), '\0'};
    
    send_string(str);
} 

char* intToStr(int i, char str[], int length) {
  if(i == 0) {
    return "0";
  }
   
  //If negative, put negative sign at beginning and 
  //make i positive
  int isNeg = FALSE;
  if(i < 0) {
    isNeg = TRUE;
    i *= -1;
    str[0] = '-';
  }
   
  int place = length - 2;
  str[length - 1] = '\0';
  //Make string backwards by modding and dividing the ones place
  //If negative, don't put anything at index 0
  while(place >= 0 && !(place == 0 && isNeg)) {
    int digit = i % 10;
    
    if(i > 0) {
      str[place] = digitToChar(digit);
    } else {
      str[place] = ' ';
    }
    
    i /= 10;
    place--; 
  }
  return str;
}

char digitToChar(int i) {
  //Use ascii value of 0 to offset char value of the integer
  return (char)(((int)'0') + i);
}
