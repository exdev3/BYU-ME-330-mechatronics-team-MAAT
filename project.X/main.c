/*
 * File:   main.c
 * Author: mccda
 *
 * Created on October 10, 2017, 1:18 PM
 */

#pragma config ICS = PGx3 //this allows debugging.... use PGC/D pair 3 (pins 9,10) for debugging
#pragma config FNOSC = FRCDIV // select 8MHz oscillator with postscaler
#pragma config OSCIOFNC = OFF
#pragma config SOSCSRC = DIG
#pragma config FWDTEN = OFF// Watchdog Timer Enable bits (WDT disabled in hardware; SWDTEN bit disabled)


#include "xc.h"
#define FCY 4000000
#include <libpic30.h>

//Configurations------------------------------------
float motorPwmDutyCycle = 2000;
float launcherPwmDutyCycle = 2000;
float turretPwmPeriod = 20000;
float turretPwmDutyCycle = 1500;
int numberOfBalls = 11;
int maxIrSensorAnalogInput = 2048;
int irSensorAnalogThreshold = 800;
//--------------------------------------------------

float irInput1 = 0;
float irInput1_old = 0;
float irInput2 = 0;
float blackBallInput = 0;

int haveBallz = 0;

void initInputOutput();
void initDigitalPorts();
void initAnalogPorts();
void initPwmPorts();
void moveBase(char baseDirection, float speed);
void moveLauncher(float speed);
void moveTurret(char turretDirection);
void moveFeeder();
void flashLight(float delayMsAmount);
void stopBase();
void stopLauncher();
void stopTurret();
void stopFeeder();
void speedUpBase(char baseDirection, float newMotorSpeedPercent);
void slowDownBase(char baseDirection, float oldMotorSpeedPercent);

int main(void) {
    
    _RCDIV = 0b010; // postscale oscillator (divide-by-4))
    
    /* PIC 24 PINOUT
    Pin 1/RA5: --
    Pin 2/RA0: digital output for LED
    Pin 3/RA1: digital output for Launch feeder
    Pin 4/RB0: PWM (Turret motor)
    Pin 5/RB1: Launch motor digital output/PWM
    Pin 6/RB2: Analog input for ROS
    Pin 7/RA2: Sleep
    Pin 8/RA3: Touch sensor(s) input
    Pin 9/RB4: --
    Pin 10/RA4: --
    Pin 11/RB7: --
    Pin 12/RB8: --
    Pin 13/RB9: --
    Pin 14/RA6: PWM for H-Bridge
    Pin 15/RB12: Dir 1
    Pin 16/RB13: Dir 2
    Pin 17/RB14: Analog input for Light sensor 1
    Pin 18/RB15: Analog input for Light sensor 2
    Pin 19/VSS: Vss
    Pin 20/VDD: Vdd
     * */
    
    initInputOutput();
    initDigitalPorts();
    initPwmPorts();
    initAnalogPorts();
    
    flashLight(100);
    flashLight(100);
    flashLight(100);
    
    float i = 0;
    float j = 0;
    int trys = 0;
    
    char goal = 'M';
    
    stopBase();
    stopLauncher();
    stopTurret();
    stopFeeder();
    
    irInput1 = ADC1BUF10;
    irInput1_old = irInput1;
    
    
    //    moveLauncher(.9);
    while(1){
        
        
        blackBallInput = ADC1BUF4;
        if(blackBallInput>30){
            _LATA0 = 0;
        }else{
            _LATA0 = 1;
        }
         
    }
    
    while(1){
        
        speedUpBase('F',1);
        __delay_ms(100);
        moveBase('B',1);
        __delay_ms(110);
        
        //////////////////////LOOKING FOR DISPENSOR////////////////////////////////////////
        speedUpBase('L',.9);
        //todo: add logic for input 2 in while loop and if statement
        irInput1 = ADC1BUF10;
        while(irInput1<irSensorAnalogThreshold){irInput1 = ADC1BUF10;}//wait for IR input
        slowDownBase('L',.9);
        speedUpBase('R',.9);
        __delay_ms(320);
        while(1){
            //////////////////////MOVING BASE TWARD DISPENSOR////////////////////////////////////////
            speedUpBase('B',1);
            __delay_ms(420);
            speedUpBase('B',.6);
            i = 0;
            while(!_RA3 || i<6000){i += 1;}//wait for touch sensor
            slowDownBase('B',.6);
            speedUpBase('L',.055);
            __delay_ms(100);
            speedUpBase('R',.055);
            __delay_ms(100);
            speedUpBase('B',.055);
            __delay_ms(100);
            stopBase();
            __delay_ms(100);
            //////////////////////GETTING BALLZ////////////////////////////////////////
            i = 0;
            while(i<11){
                _LATA0 = 1;
                moveBase('B',.06);
                __delay_ms(100);
                _LATA0 = 0;
                moveBase('F',.06);
                __delay_ms(100);
                i += 1;
            }
            _LATA0 = 0;
            haveBallz = 1;
            //////////////////////MOVING BASE AWAY FROM DISPENSOR////////////////////////////////////////
            speedUpBase('F',.8);
            __delay_ms(660);
            trys = 0;
            while(haveBallz && trys<3){
                //////////////////////ROTATE BASE TWARD GOAL////////////////////////////////////////
                irInput1 = ADC1BUF10;
                irInput2 = ADC1BUF9;
                if(irInput1>irSensorAnalogThreshold){
                    goal = 'M';
                    __delay_ms(10);
                    slowDownBase('F',.8);
                }
                else if(irInput2>irSensorAnalogThreshold){
                    goal = 'L';
                    moveBase('R',.7);
                    __delay_ms(215);//this must match below
                    slowDownBase('R',.7);
                }
                else{
                    goal = 'R';
                    moveBase('L',.7);
                    __delay_ms(210);//this must match below
                    slowDownBase('L',.7);
                }
                irInput1 = ADC1BUF10;
                if(irInput1>irSensorAnalogThreshold){
                    //////////////////////FINE TUNE GOAL////////////////////////////////////////
                    i = 0;
                    j = 0;
                    irInput1_old = irInput1;
                    irInput1 = ADC1BUF10;
                    moveBase('L',.1);
                    while( !(irInput1<irSensorAnalogThreshold && irInput1_old>irSensorAnalogThreshold) ){irInput1_old = irInput1;irInput1 = ADC1BUF10;}
                    moveBase('R',.1);
                    __delay_ms(30);
                    while( !(irInput1<irSensorAnalogThreshold && irInput1_old>irSensorAnalogThreshold) ){irInput1_old = irInput1;irInput1 = ADC1BUF10;i=i+1;}

                    moveBase('L',.1);
                    while(j<i/2.0){j += 1;}
                    stopBase();
                    //////////////////////SHOOT////////////////////////////////////////
                    moveLauncher(.9);
                    __delay_ms(400);
                    moveFeeder();
                    __delay_ms(1800);
                    stopFeeder();
                    stopLauncher();
                    haveBallz = 0;
                }else{
                    trys += 1;
                }
                //////////////////////ROTATE BASE AWAY FROM GOAL////////////////////////////////////////
                if(goal == 'L'){
                    moveBase('L',.7);
                    __delay_ms(215);//this must match above
                    slowDownBase('L',.7);
                }else if(goal == 'R'){
                    moveBase('R',.7);
                    __delay_ms(190);//this must match above
                    slowDownBase('R',.7);
                }
                
                //If you havent found the goal, go back to the corner and re-align yourself
                if(trys>2){
                    //////////////////////MOVING BASE TWARD DISPENSOR////////////////////////////////////////
                    speedUpBase('B',1);
                    __delay_ms(420);
                    speedUpBase('B',.6);
                    i = 0;
                    while(!_RA3 || i<6000){i += 1;}//wait for touch sensor
                    slowDownBase('B',.6);
                    speedUpBase('L',.05);
                    __delay_ms(50);
                    speedUpBase('R',.05);
                    __delay_ms(50);
                    stopBase();
                    //////////////////////MOVING BASE AWAY FROM DISPENSOR////////////////////////////////////////
                    speedUpBase('F',.8);
                    __delay_ms(660);
                    trys = 0;
                }
            }
        }
    
    }
    
    return 0;
}

void initInputOutput(){
    TRISAbits.TRISA0 = 0;//Pin 2/RA0: digital output for LED
    TRISAbits.TRISA1 = 0;//Pin 3/RA1: digital output for Launch feeder
    TRISBbits.TRISB0 = 0;//Pin 4/RB0: PWM (Turret motor)
    TRISBbits.TRISB1 = 0;//Pin 4/RB0: Launch motor digital output/PWM
    TRISBbits.TRISB2 = 1;//Pin 6/RB2: Analog input for ROS
    TRISAbits.TRISA2 = 0;//Pin 7/RA2: Sleep
    TRISAbits.TRISA3 = 1;//Pin 8/RA3: Touch sensor(s) input
    //TRISBbits.TRISB4 = 0;
    //TRISAbits.TRISA4 = 0;
    //TRISBbits.TRISB7 = 0;
    //TRISBbits.TRISB8 = 0;
    //TRISBbits.TRISB9 = 0;
    TRISAbits.TRISA6 = 0;//Pin 14/RA6: PWM for H-Bridge
    TRISBbits.TRISB12 = 0;//Pin 15/RB7: Dir 1
    TRISBbits.TRISB13 = 0;//Pin 16/RB13: Dir 2
    TRISBbits.TRISB14 = 1;//Pin 17/RB14: Analog input for Light sensor 1
    TRISBbits.TRISB15 = 1;//Pin 18/RB15: Analog input for Light sensor 2
}

void initDigitalPorts(){
    ANSAbits.ANSA0 = 0;//Pin 2/RA0: digital output for LED
    ANSAbits.ANSA1 = 0;//Pin 3/RA1: digital output for Launch feeder
    ANSBbits.ANSB0 = 0;//Pin 4/RB0: PWM (Turret motor)
    ANSBbits.ANSB1 = 0;//Pin 4/RB0: Launch motor digital output/PWM
    ANSBbits.ANSB2 = 1;//Pin 6/RB2: Analog input for ROS
    ANSAbits.ANSA2 = 0;//Pin 7/RA2: Sleep
    ANSAbits.ANSA3 = 0;//Pin 8/RA3: Touch sensor(s) input
    ANSBbits.ANSB4 = 0;
    //ANSAbits.ANSA4 = 0;
    //ANSBbits.ANSB7 = 0;
    //ANSBbits.ANSB8 = 0;
    //ANSBbits.ANSB9 = 0;
    //ANSAbits.ANSA6 = 0;//Pin 14/RA6: PWM for H-Bridge
    ANSBbits.ANSB12 = 0;//Pin 15/RB7: Dir 1
    ANSBbits.ANSB13 = 0;//Pin 16/RB13: Dir 2
    ANSBbits.ANSB14 = 1;//Pin 17/RB14: Analog input for Light sensor 1
    ANSBbits.ANSB15 = 1;//Pin 18/RB15: Analog input for Light sensor 2
}

void initAnalogPorts(){

	/*** Select Voltage Reference Source ***/
	// use AVdd for positive reference
	_PVCFG = 00;		// AD1CON2<15:14>, pg. 212-213 datasheet
	// use AVss for negative reference
	_NVCFG = 0;			// AD1CON2<13>


	/*** Select Analog Conversion Clock Rate ***/
	// make sure Tad is at least 600ns, see Table 29-41 datasheet
	_ADCS = 0x02;	// AD1CON3<7:0>, pg. 213 datasheet


	/*** Select Sample/Conversion Sequence ***/
	// use auto-convert
	_SSRC = 0b0111;		// AD1CON1<7:4>, pg. 211 datasheet
	// use auto-sample
	_ASAM = 1;			// AD1CON1<2>
	// choose a sample time >= 1 Tad, see Table 29-41 datasheet
	_SAMC = 0b0001;		// AD1CON3<12:8>


	/*** Choose Analog Channels to be Used ***/
	// scan inputs
	_CSCNA = 1;			// AD1CON2<10>
	// choose which channels to scan, e.g. for ch AN12, set _CSS12 = 1;
	_CSS4 = 1;			// AD1CSSH/L, pg. 217
	_CSS9 = 1;			// AD1CSSH/L, pg. 217
	_CSS10 = 1;			// AD1CSSH/L, pg. 217

	/*** Select How Results are Presented in Buffer ***/
	// set 12-bit resolution
	_MODE12 = 1;		// AD1CON1<10>
	// use absolute decimal format
	_FORM = 00;			// AD1CON1<9:8>
	// load results into buffer determined by converted channel, e.g. ch AN12 
    // results appear in ADC1BUF12
	_BUFREGEN = 1;		// AD1CON2<11>


	/*** Select Interrupt Rate ***/
	// interrupt rate should reflect number of analog channels used, e.g. if 
    // 5 channels, interrupt every 5th sample
	_SMPI = 2;		// AD1CON2<6:2>  ??mccann is this right??


	/*** Turn on A/D Module ***/
	_ADON = 1;			// AD1CON1<15>
    
    
    //*****************How to read the analog buffer******************
    //float blackBallInput = ADC1BUF4;  ------> pin6
    //float irInput1 = ADC1BUF10;  ------> pin18
    //float irInput2 = ADC1BUF9;  ------> pin17
}

void initPwmPorts(){
    
    //-----------------------------------------------------------
    // CONFIGURE PWM1 USING OC1 (on pin 14)
    
    // Clear control bits initially
    OC1CON1 = 0;
    OC1CON2 = 0;
   
    // Set period and duty cycle
    OC1R = motorPwmDutyCycle;                // Set Output Compare value to achieve
                                // desired duty cycle. This is the number
                                // of timer counts when the OC should send
                                // the PWM signal low. The duty cycle as a
                                // fraction is OC1R/OC1RS.
    OC1RS = motorPwmDutyCycle*2;               // Period of OC1 to achieve desired PWM 
                                // frequency, FPWM. See Equation 15-1
                                // in the datasheet. For example, for
                                // FPWM = 1 kHz, OC1RS = 3999. The OC1RS 
                                // register contains the period when the
                                // SYNCSEL bits are set to 0x1F (see FRM)
    
    // Configure OC1
    OC1CON1bits.OCTSEL = 0b111; // System (peripheral) clock as timing source
    OC1CON2bits.SYNCSEL = 0x1F; // Select OC1 as synchronization source
                                // (self synchronization) -- Although we
                                // selected the system clock to determine
                                // the rate at which the PWM timer increments,
                                // we could have selected a different source
                                // to determine when each PWM cycle initiates.
                                // From the FRM: When the SYNCSEL<4:0> bits
                                // (OCxCON2<4:0>) = 0b11111, they make the
                                // timer reset when it reaches the value of
                                // OCxRS, making the OCx module use its
                                // own Sync signal.
    OC1CON2bits.OCTRIG = 0;     // Synchronizes with OC1 source instead of
                                // triggering with the OC1 source
    OC1CON1bits.OCM = 0b110;    // Edge-aligned PWM mode    
    //-----------------------------------------------------------
    //-----------------------------------------------------------
    // CONFIGURE PWM2 USING OC2 (on pin 4)
    
    // Clear control bits initially
    OC2CON1 = 0;
    OC2CON2 = 0;
   
    // Set period and duty cycle
    OC2R = 0;                // Set Output Compare value to achieve
                                // desired duty cycle. This is the number
                                // of timer counts when the OC should send
                                // the PWM signal low. The duty cycle as a
                                // fraction is OC1R/OC1RS.
    OC2RS = 800;               // Period of OC1 to achieve desired PWM 
                                // frequency, FPWM. See Equation 15-1
                                // in the datasheet. For example, for
                                // FPWM = 1 kHz, OC1RS = 3999. The OC1RS 
                                // register contains the period when the
                                // SYNCSEL bits are set to 0x1F (see FRM)
    
    // Configure OC1
    OC2CON1bits.OCTSEL = 0b111; // System (peripheral) clock as timing source
    OC2CON2bits.SYNCSEL = 0x1F; // Select OC1 as synchronization source
                                // (self synchronization) -- Although we
                                // selected the system clock to determine
                                // the rate at which the PWM timer increments,
                                // we could have selected a different source
                                // to determine when each PWM cycle initiates.
                                // From the FRM: When the SYNCSEL<4:0> bits
                                // (OCxCON2<4:0>) = 0b11111, they make the
                                // timer reset when it reaches the value of
                                // OCxRS, making the OCx module use its
                                // own Sync signal.
    OC2CON2bits.OCTRIG = 0;     // Synchronizes with OC1 source instead of
                                // triggering with the OC1 source
    OC2CON1bits.OCM = 0b110;    // Edge-aligned PWM mode    
    //-----------------------------------------------------------
    //-----------------------------------------------------------
    // CONFIGURE PWM2 USING OC3 (on pin 5)
    
    // Clear control bits initially
    OC3CON1 = 0;
    OC3CON2 = 0;
   
    // Set period and duty cycle
    
    OC3R = 0;
    OC3RS = launcherPwmDutyCycle*2;
    
    // Configure OC1
    OC3CON1bits.OCTSEL = 0b111; // System (peripheral) clock as timing source
    OC3CON2bits.SYNCSEL = 0x1F; // Select OC1 as synchronization source
                                // (self synchronization) -- Although we
                                // selected the system clock to determine
                                // the rate at which the PWM timer increments,
                                // we could have selected a different source
                                // to determine when each PWM cycle initiates.
                                // From the FRM: When the SYNCSEL<4:0> bits
                                // (OCxCON2<4:0>) = 0b11111, they make the
                                // timer reset when it reaches the value of
                                // OCxRS, making the OCx module use its
                                // own Sync signal.
    OC3CON2bits.OCTRIG = 0;     // Synchronizes with OC1 source instead of
                                // triggering with the OC1 source
    OC3CON1bits.OCM = 0b110;    // Edge-aligned PWM mode    
    //-----------------------------------------------------------
    
    //todo do we need these?
    //_LATB0 = 1;
    //_LATB1 = 1;
    //_LATA6 = 1;
}

void moveBase(char baseDirection, float speed){
    switch (baseDirection){
        case 'F':
            _LATB13 = 0;
            _LATB12 = 0;
            break;
        case 'B':
            _LATB13 = 1;
            _LATB12 = 1;
            break;
        case 'L':
            _LATB13 = 1;
            _LATB12 = 0;
            break;
        case 'R':
            _LATB13 = 0;
            _LATB12 = 1;
            break;
        default:
            stopBase();
            break;
    }
    OC1R = motorPwmDutyCycle*(1.0/speed);
    OC1RS = motorPwmDutyCycle*(1.0/speed)*2;
    _LATA2 = 1;//sleep
}
void stopBase(){
    _LATA2 = 0;//sleep
    OC1R = 0;
    OC1RS = 9999;
}
void moveLauncher(float speed){
    OC3R = launcherPwmDutyCycle*speed;
    OC3RS = launcherPwmDutyCycle;
    _LATB1 = 1;
}
void stopLauncher(){
    _LATB1 = 0;
    OC3R = 0;
    OC3RS = 9999;
}
void moveFeeder(){
    //_LATA1 = 0;
    
                _LATB0 = 1;
                OC2R = 200;
                OC2RS = 800;
                
}
void stopFeeder(){
    //_LATA1 = 1;
    
                _LATB0 = 1;
                OC2R = 0;
                OC2RS = 800;
                
}
void moveTurret(char turretDirection){
    switch (turretDirection){
        case 'L':
            turretPwmDutyCycle = 1750;
            break;
        case 'R':
            turretPwmDutyCycle = 1250;
            break;
        case 'M':
            turretPwmDutyCycle = 1500;
            break;
        default:
            break;
    }
    OC2R = turretPwmDutyCycle;
    OC2RS = turretPwmPeriod;
    _LATB0 = 1;
}
void stopTurret(){
    OC2R = turretPwmDutyCycle;
    OC2RS = turretPwmPeriod;
    _LATB0 = 0;
}
void speedUpBase(char baseDirection, float newMotorSpeedPercent){
    float motorSpeedPercent = 0;
    while(motorSpeedPercent<newMotorSpeedPercent){
        motorSpeedPercent += .01;
        moveBase(baseDirection,motorSpeedPercent);
    }
    moveBase(baseDirection, newMotorSpeedPercent);
}
void slowDownBase(char baseDirection, float oldMotorSpeedPercent){
    float motorSpeedPercent = oldMotorSpeedPercent;
    while(motorSpeedPercent>0){
        moveBase(baseDirection,motorSpeedPercent);
        motorSpeedPercent -= .01;
    }
    moveBase(baseDirection, 0);
    stopBase();
}
void flashLight(float delayMsAmount){
    _LATA0 = 1;
    __delay_ms(delayMsAmount);
    _LATA0 = 0;
    __delay_ms(delayMsAmount);
}