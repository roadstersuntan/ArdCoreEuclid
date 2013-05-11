// ============================================================ 
// 
// Program: Euclid Sequencer

// Author: Stuart Anderson, Euclid code by Tom Whitwell - slightly modified
// 
// Description: Match incoming clock on D0 output but using 
// lookup from array created using Euclid algorithm from Tombola. 
// 
//

// dividing the number back from the pots, max value is 1026, dividing by 171 get you 0-6 range for example
// function is called with beats and total and so 15,34 seems to be the highest sensible value range
// 
// A0 currently set to value/68
// A1 currently set to value/30
//
// This gives us max 16 and 35 but feel free to adjust

// TBD communicate values back to host computer, via serial or talking to Max Object


#include <avr/interrupt.h> 
#include <avr/io.h> 

// Pin 13 has an LED connected on most Arduino boards. 
// give it a name so we can use it when debugging 
int led = 13;

// Set up variables for remote control over serial interface
int control = 0;
int value = 0;


// We'll be toggling it's state so make this easy 
static unsigned int led_state = 0; 

// variables for interrupt handling of the clock input 
int clkState = LOW; 
volatile boolean changed = false; 

// Set up digital output pins 
const int digPin[2] = {3, 4}; // the digital output pins 
const int pinOffset = 5; // the first DAC pin (from 5-12) 

// variables used to control the current DIO output states 
int digState[2] = {LOW, LOW}; // start with both set low 
long digMilli[2] = {0, 0}; // used for trigger timing 

int looper = 0; 
int x; 
int euclidPattern[200]; 
long int result; 
int binaryLength; 


// flag for control changes, set to true first time so it can populate array 
boolean controlChanged = true;
long int newLeftC; 
long int newRightC; 
long int currentLeftC; 
long int currentRightC;

// Set remote change to false
boolean remoteControlChanged = false;

// isrChanging() - quickly handle interrupts from the clock input 
void isrChanging() { 
//led state change indicates interrupt fired 
//led_state = !led_state; 
//digitalWrite(led, led_state); 
changed = true; 
}; 

void setup() { 

// Set up our LED output 
//pinMode(led, OUTPUT); 

// If we need to send data back to our computer, otherwise, comment this line out. 
Serial.begin(9600); 

// set up the digital outputs 
for (int i=0; i<2; i++) { 
pinMode(digPin[i], OUTPUT); 
digitalWrite(digPin[i], LOW); 
} 

// set up the 8-bit DAC output pins 
for (int i=0; i<8; i++) { 
pinMode(pinOffset+i, OUTPUT); 
digitalWrite(pinOffset+i, LOW); 
} 

// variables for interrupt handling of the clock input 
volatile int clkState = LOW; 

// set up clock interrupt 
attachInterrupt(0, isrChanging, CHANGE); 

// force populate array with data 
controlChanged = true; 

} 

void loop() {
// See if remote control has sent any changes
// I want to look for P and either 1 or 2 then value
// parseInt will only pull out the Int values so P1, 255 will give me 1 and 255
if (Serial.find("P")) {
  control = Serial.parseInt();
  value = Serial.parseInt();
  remoteControlChanged = true;
}



// See if controls have changed

// First read the panel controls
newLeftC = (analogRead(0)/68)+1; 
newRightC = (analogRead(1)/30)+1; 
//if (newRightC < newLeftC) newRightC = newLeftC; 
if (newLeftC != currentLeftC) controlChanged = true; 
if (newRightC != currentRightC) controlChanged = true;

// Act on control change 
if (controlChanged || remoteControlChanged) { 

  // Get the control values 
  if (controlChanged) {
    currentLeftC = newLeftC; 
    currentRightC = newRightC;
  }

  if (remoteControlChanged) {
    if (control = 1) {
      currentLeftC = value;
    }
    if (control = 2) {
      currentRightC = value;
    }
  }

result = euclid(currentLeftC,currentRightC); 

binaryLength = findlength(result); 
for (x=binaryLength; x>=0; x--) { 
euclidPattern[x-1] = (result & 0x01); 
result = result >> 1; 
} 
euclidPattern[binaryLength] = 2; 
controlChanged = false; 
looper = 0; 
//Serial.print("E("); 
//Serial.print(currentLeftC); 
//Serial.print(","); 
//Serial.print(currentRightC); 
//Serial.print(") "); 
for (x=0; x<=(binaryLength-1); x++) { 
  Serial.print(euclidPattern[x]); 
  } 
  Serial.println(""); 
} 
// check our state 
if (changed) { 
clkState = digitalRead(2); 
changed = false; 

// service a clock trigger 
if (clkState == HIGH) { 
if (euclidPattern[looper] == 2) { 
looper = 0; 
} 
if ((euclidPattern[looper]) == 1) { 
digitalWrite(digPin[0], HIGH); 
} 
looper++; 
} 
else { 
digitalWrite(digPin[0], LOW); 
} 
} 
} 



//////////////////////////////////////////////////////////////////////   /////// 
// From Tombola's euclidean sequencer.. 
// Credit : Tom Whitwell musicthing.co.uk/modular 
// Euclid calculation function 
unsigned int euclid(int k, int n){ // inputs: n=total, k=beats, o = offset 
int pauses = n-k; 
int pulses = k; 
int per_pulse = pauses/k; 
int remainder = pauses%pulses; 
unsigned int workbeat[n]; 
unsigned int outbeat; 
unsigned int working; 
int workbeat_count=n; 
int a; 
int b; 
int trim_count; 
for (a=0;a<n;a++){ // Populate workbeat with unsorted pulses and pauses 
if (a<pulses){ 
workbeat[a] = 1; 
} 
else { 
workbeat [a] = 0; 
} 
} 

if (per_pulse>0 && remainder <2){ // Handle easy cases where there is no or only one remainer 
for (a=0;a<pulses;a++){ 
for (b=workbeat_count-1; b>workbeat_count-per_pulse-1;b--){ 
workbeat[a] = ConcatBin (workbeat[a], workbeat[b]); 
} 
workbeat_count = workbeat_count-per_pulse; 
} 

outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count 
for (a=0;a < workbeat_count;a++){ 
outbeat = ConcatBin(outbeat,workbeat[a]); 
} 
return outbeat; 
} 

else { 


int groupa = pulses; 
int groupb = pauses; 
int iteration=0; 
if (groupb<=1){ 
} 
while(groupb>1){ //main recursive loop 


if (groupa>groupb){ // more Group A than Group B 
int a_remainder = groupa-groupb; // what will be left of groupa once groupB is interleaved 
trim_count = 0; 
for (a=0; a<groupa-a_remainder;a++){ //count through the matching sets of A, ignoring remaindered 
workbeat[a] = ConcatBin (workbeat[a], workbeat[workbeat_count-1-a]); 
trim_count++; 
} 
workbeat_count = workbeat_count-trim_count; 

groupa=groupb; 
groupb=a_remainder; 
} 


else if (groupb>groupa){ // More Group B than Group A 
int b_remainder = groupb-groupa; // what will be left of group once group A is interleaved 
trim_count=0; 
for (a = workbeat_count-1;a>=groupa+b_remainder;a--){ //count from right back through the Bs 
workbeat[workbeat_count-a-1] = ConcatBin (workbeat[workbeat_count-a-1], workbeat[a]); 

trim_count++; 
} 
workbeat_count = workbeat_count-trim_count; 
groupb=b_remainder; 
} 




else if (groupa == groupb){ // groupa = groupb 
trim_count=0; 
for (a=0;a<groupa;a++){ 
workbeat[a] = ConcatBin (workbeat[a],workbeat[workbeat_count-1-a]); 
trim_count++; 
} 
workbeat_count = workbeat_count-trim_count; 
groupb=0; 
} 

else { 
// Serial.println("ERROR"); 
} 
iteration++; 
} 


outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count 
for (a=0;a < workbeat_count;a++){ 
outbeat = ConcatBin(outbeat,workbeat[a]); 
} 




return outbeat; 

} 
} 

// Function to find the binary length of a number by counting bitwise 
int findlength(unsigned int bnry){ 
boolean lengthfound = false; 
int length=1; // no number can have a length of zero - single 0 has a length of one, but no 1s for the sytem to count 
for (int q=32;q>=0;q--){ 
int r=bitRead(bnry,q); 
if(r==1 && lengthfound == false){ 
length=q+1; 
lengthfound = true; 
} 
} 
return length; 
} 

// Function to concatenate two binary numbers bitwise 
unsigned int ConcatBin(unsigned int bina, unsigned int binb){ 
int binb_len=findlength(binb); 
unsigned int sum=(bina<<binb_len); 
sum = sum | binb; 
return sum; 
} 


// deJitter(int, int) - smooth jitter input 
// ---------------------------------------- 
int deJitter(int v, int test) 
{ 
if (abs(v - test) > 8) { 
return v; 
} 
return test; 
} 
