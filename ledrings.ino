#include "Adafruit_NeoPixel.h"
//#include <inttypes.h>
#include "millisDelay.h"
#include "buttons.h"

int right = 100;
int left = 200;
int both = 300;


#define Cin  A0
#define Din  A1
#define E1in A2
#define E2in A3
#define Fin  A4
#define Gin  A5

volatile bool Cin_state  = 0;
volatile bool Din_state  = 0;
volatile bool E1in_state = 0;
volatile bool E2in_state = 0;
volatile bool Fin_state  = 0;
volatile bool Gin_state  = 0;
volatile bool trip_state = 0;

buttons::Button Cin_but (Cin_state );
buttons::Button Din_but (Din_state );
buttons::Button E1in_but(E1in_state);
buttons::Button E2in_but(E2in_state);
buttons::Button Fin_but (Fin_state );
buttons::Button Gin_but (Gin_state );
buttons::Button trip_but(trip_state);

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s16 int16_t


#define NUM_PIXELS  93

#define NEOPIXEL_PIN_LEFT 2
Adafruit_NeoPixel LEFT(NUM_PIXELS, NEOPIXEL_PIN_LEFT, NEO_GRB + NEO_KHZ800);

#define NEOPIXEL_PIN_RIGHT 3
Adafruit_NeoPixel RIGHT(NUM_PIXELS, NEOPIXEL_PIN_RIGHT, NEO_GRB + NEO_KHZ800);


int f = 0;
int dly;
u8 colors = 0x00;
u32 colorW(u8 colorz);
int inputs = 2;

//////////////////////////////////////////////////////////////////////

// states
//  animation one
//  ILLUMINATE,     // 0
//  WAIT,           // 1
//  SLOWFADE,       // 2
//  
//  animation two
//  TRAIL,          // 3
//  FLASH,          // 4
//  FASTFADE,       // 5

int illuminateDly = 30;  // speed of illumination phase of animation one, bigger numbers = slower
u8 illuminateStartBrightness = 0x00;
u8 illuminateMaxBrightness = 0x40; // brightness reached in animation one.  0x00-0xff
int illuminateDlyAcc = 1; // acceleration of illumination phase of animation one, bigger numbers = slower

int waitDly = 1000; // duration of wait phase of animation one, bigger numbers = longer

int slowfadeDly = 20; // speed of slow fade phase of animation one, bigger numbers = slower

u8 trailBrightness = 0xb0; // brightness reached in animation two.  0x00-0xff
int trailDly = 0; // speed of trail phase of animation two, bigger numbers = slower

u8 flashBrightness = 0xb0;// brightness of flash in animation two.  0x00-0xff

int fastfadeDly = 5; // speed of trail fast fade of animation two, bigger numbers = slower

int flashDly = 250; // duration of flash phase of animation two, bigger numbers = longer

int MAX_BRIGHTNESS = 50; 
int refreshDly = 20; // 20 milliseconds = 50hz framerate

u8 fivepercent = 0x0f; //value associated with five percent brightness, 0x00-0xff
u8 fifteenpercent = 0x30; //value associated with fifteen percent brightness, 0x00-0xff
u8 fortypercent = 0x50; //value associated with forty percent brightness, 0x00-0xff

//////////////////////////////////////////////////////////////////////

millisDelay mainLoopDelay;
millisDelay dlyDelay;

millisDelay tripDelay;
int tripFlag = 0;
int tripCount = 0;

// States
enum States {
    ILLUMINATE,     // 0
    WAIT,           // 1
    SLOWFADE,       // 2
    TRAIL,          // 3
    FLASH,          // 4
    FASTFADE,       // 5
    POLL,           // 6
};

States state;
bool enteringState;

void setState(States newState);
void setState(States newState) {
    enteringState = true;
    state = newState;
    Serial.println(state);
    Serial.println(inputs);
    Serial.print(Cin_state   );
    Serial.print(Din_state   );
    Serial.print(E1in_state  );
    Serial.print(E2in_state  );
    Serial.print(Fin_state   );
    Serial.print(Gin_state   );
    Serial.print(trip_state  );
    Serial.print(" ");
    Serial.print(colors);
    Serial.println("x");
    dlyDelay.start(1);
}

void stateMachine(int side) {
  switch (state) {
    case ILLUMINATE:

        if (enteringState) {
            colors = illuminateStartBrightness;
            f = 0;
            enteringState = false;
            dly = illuminateDly;
            dlyDelay.start(dly);
            break;
        }

        if (f < 108 && side == left) {
            f++;
            dly = dly - illuminateDlyAcc;
            if (dly<0) dly=0;
        }
        else if (side == left) {
            setState(WAIT);
            break;
        }

        for (u16 l = 0; l < f / 2.65; l++) {
            if (side==left) LEFT.setPixelColor(l + 53, colorW(colors));
            if (side==right) RIGHT.setPixelColor(l + 53, colorW(colors));
        }
        for (u16 i = 0; i < f / 2; ++i) {
            if (side==left) LEFT.setPixelColor(i, colorW(colors));
            if (side==right) RIGHT.setPixelColor(i, colorW(colors));
        }

        
        dlyDelay.start(dly);
        if (colors == illuminateMaxBrightness)colors = illuminateMaxBrightness - 1;
        if (side == left) colors++;
        break;

    case WAIT:

        if (enteringState) {
            enteringState = false;
            dly = waitDly;
            dlyDelay.start(waitDly);
            break;
        }

        if (side == right && (inputs == 7 || inputs == 15)) {
            setState(FASTFADE);
        }
        else if (side == left) {
            setState(SLOWFADE);
        }
        dlyDelay.start(10);
        break;

    case SLOWFADE:

        if (enteringState) {
            enteringState = false;
            dly = slowfadeDly;
            dlyDelay.start(dly);
            Serial.print(" ");
            Serial.print(colors);
            Serial.print("y");
            break;
        }

        if (side == left && colors == 0) { setState(POLL); break; }
        if (inputs == 8 && side == right) { setAllMin(side, fivepercent, colors); }
        else if (inputs == 12 && side == left) { setAllMin(side, fivepercent, colors); }
        else {setAll(side, colors);}
        if (side == left) colors--;
        dlyDelay.repeat();
        
        dlyDelay.start(dly);
        break;

    case TRAIL:

        if (enteringState) {
            colors = trailBrightness;
            f = 1;
            enteringState = false;
            dlyDelay.start(trailDly);
            break;
        }
        if (f < 93 && side == left) {
            f++;
        }
        //if (f < 53 && side == left) {
        //    f++;
        //}
        else if(side == left){
            setState(FLASH);
            break;
        }

        for (int i = 0; i < f; ++i) {
            if (side==left) LEFT.setPixelColor(i - 1, colorW(colors));
            if (side==right) RIGHT.setPixelColor(i - 1, colorW(colors));
            for (int j = i; j > i-6; j--) {
                if (side==left) LEFT.setPixelColor(j - 1, colorW(180 - ((i - j) * 50)));
                if (side==right) RIGHT.setPixelColor(j - 1, colorW(180 - ((i - j) * 50)));
                if ((180 - ((i - j) * 50)) < 0) {
                    if (side==left) LEFT.setPixelColor(j - 1, 0);
                    if (side==right) RIGHT.setPixelColor(j - 1, 0);
                }
            }
        }
        if (side==left) LEFT.setPixelColor(52, 0);
        if (side==right) RIGHT.setPixelColor(52, 0);
        dlyDelay.repeat();

        break;

    case FLASH:

        if (enteringState) {
            colors = flashBrightness;
            f = 0;
            enteringState = false;
            dlyDelay.start(flashDly);
            break;
        }
        for (int i = 0; i < 93; ++i) {
            if (side==left) LEFT.setPixelColor(i, colorW(colors));
            if (side==right) RIGHT.setPixelColor(i, colorW(colors));
        }
        if (side == left) setState(FASTFADE);
        break;

    case FASTFADE:

        if (enteringState) {
            enteringState = false;
            dlyDelay.start(fastfadeDly);
            break;
        }
        if (side == left && colors == 0) setState(POLL);

        if (inputs == 8 && side == right) { setAllMin(side, fivepercent, colors); }
        else if (inputs == 11 && side == left) { setAllMin(side, fivepercent, colors); }
        else if (inputs == 15) { setAllMin(side, fifteenpercent, colors); }
        else if (inputs == 16) { setAllMin(side, fifteenpercent, colors); }
        else if (inputs == 30) { setAllMin(side, fifteenpercent, colors); }
        else if (inputs == 46) { setAllMin(side, fifteenpercent, colors); }
        else if (inputs == 50 && tripFlag == 0) { setAllMin(side, fortypercent, colors); }
        else if (inputs == 62) { setAllMin(side, fortypercent, colors); }
        else { setAll(side, colors); }
        if (side == left) colors--;
        dlyDelay.repeat();

        break;
    case POLL:

        if (enteringState) {
            dlyDelay.start(1);
            enteringState = false;
            break;
        }
        dlyDelay.start(1);
        break;
  }
}


void setup() {
    Serial.begin(9600);
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);
    pinMode(A4, INPUT_PULLUP);
    pinMode(A5, INPUT_PULLUP);
    setAll(left, 0);
    setAll(right, 0);

    LEFT.begin();
    LEFT.setBrightness(MAX_BRIGHTNESS);
    LEFT.show();

    RIGHT.begin();
    RIGHT.setBrightness(MAX_BRIGHTNESS);
    RIGHT.show();

    mainLoopDelay.start(refreshDly);
    dlyDelay.start(30);
    setState(POLL);
    stateMachine(left);
    stateMachine(right);
    //LEFT.setPixelColor(93, 0);
    //RIGHT.setPixelColor(93, 0);
    setAll(right, 0);
    setAll(left, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {


    if (mainLoopDelay.justFinished()) {
        LEFT.show();
        RIGHT.show();
        poll_buttons(left);
        poll_buttons(right);
        mainLoopDelay.restart();
    }
    if (dlyDelay.justFinished()) {
        stateMachine(left);
        stateMachine(right);
    }            

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void setAllMin(int side, u8 min, u8 colorbs) {
    if (min > colorbs) setAll(side, colorbs);
    if (min < colorbs) setAll(side, min);
}

//turn all three white LEDs on
u32 colorW(u8 colorz) {
    u32 colorW = 0x000000;
    colorW = colorW + colorz;
    colorW = colorW << 8;
    colorW = colorW + colorz;
    colorW = colorW << 8;
    colorW = colorW + colorz;
    return colorW;
}


void setAll(int side, u8 brightness) {
    for (u16 i = 0; i < 93; ++i) {
        if (side == left || side == both) LEFT.setPixelColor(i, colorW(brightness)); 
        if (side == right || side == both) RIGHT.setPixelColor(i, colorW(brightness)); 
    }
}


void poll_buttons(int side) {
    //Serial.print(Cin_state);
    //Serial.print(Din_state);
    //Serial.print(E1in_state);
    //Serial.print(E2in_state);
    //Serial.print(Fin_state);
    //Serial.print(Gin_state);
    //Serial.print(trip_state);
    //Serial.println("x");

    Cin_state  = !digitalRead(Cin);
    Din_state  = !digitalRead(Din);
    E1in_state = !digitalRead(E1in);
    E2in_state = !digitalRead(E2in);
    Fin_state  = !digitalRead(Fin);
    Gin_state  = !digitalRead(Gin);
    trip_state = (digitalRead(E1in) && digitalRead(E2in));



    Cin_but.update();
    Din_but.update();
    E1in_but.update();
    E2in_but.update();
    Fin_but.update();
    Gin_but.update();
    trip_but.update();

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 2
        ) {
        setState(POLL);
        setAll(both, 0);
        inputs = 2;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        Gin_but.isHeld(100)  &&
        state == POLL &&
        inputs != 3
        ) {
        setState(ILLUMINATE);
        inputs = 3;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        Fin_but.isHeld(100)  &&
        !Gin_but.isHeld(100) &&
        state != SLOWFADE &&
        inputs != 4
        ) {
        setState(SLOWFADE);
        inputs = 4;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 6
        ) {
        colors = fivepercent;
        setAll(right, colors);
        inputs = 6;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        Gin_but.isHeld(100)  &&
        state == POLL &&
        inputs != 7
        ) {
        setState(ILLUMINATE);
        inputs = 7;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        E2in_but.isHeld(100) &&
        Fin_but.isHeld(100)  &&
        !Gin_but.isHeld(100) &&
        state != SLOWFADE &&
        inputs != 8
        ) {
        setState(SLOWFADE);
        inputs = 8;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 10
        ) {
        colors = fivepercent;
        setAll(left, colors);
        inputs = 10;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        Gin_but.isHeld(100)  &&
        state == POLL &&
        inputs != 11
        ) {
        setState(ILLUMINATE);
        inputs = 11;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        !E2in_but.isHeld(100)&&
        Fin_but.isHeld(100)  &&
        !Gin_but.isHeld(100) &&
        state != SLOWFADE &&
        inputs != 12
        ) {
        setState(SLOWFADE);
        inputs = 12;
    }
    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        //state == POLL &&
        state != ILLUMINATE &&
        state != TRAIL &&
        inputs != 14
        ) {
        state == POLL;
        colors = fifteenpercent;
        setAll(both, colors);
        inputs = 14;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        Gin_but.isHeld(100)  &&
        state == POLL &&
        inputs != 15
        ) {
        setState(ILLUMINATE);
        inputs = 15;
    }

    if (
        !Cin_but.isHeld(100) &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        Fin_but.isHeld(100)  &&
        !Gin_but.isHeld(100) &&
        state != FASTFADE &&
        inputs != 16
        ) {
        setState(FASTFADE);
        inputs = 16;
        colors == fifteenpercent;
    }

    if (
        !Cin_but.isHeld(100) &&
        Din_but.isHeld(100)  &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 18
        ) {
        setState(TRAIL);
        inputs = 18;
    }

    if (
        !Cin_but.isHeld(100) &&
        Din_but.isHeld(100)  &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 30
        ) {
        setState(TRAIL);
        inputs = 30;
    }

    if (
        Cin_but.isHeld(100)  &&
        !Din_but.isHeld(100) &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        //state == POLL &&
        state != ILLUMINATE &&
        state != TRAIL &&
        inputs != 34
        ) {
        setState(POLL);
        if (tripFlag == 0) {
            colors = fortypercent;
            setAll(both, colors);
        }
        inputs = 34;
    }

    if (
        Cin_but.isHeld(100)  &&
        !Din_but.isHeld(100) &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 46
        ) {
        //setState(FASTFADE);
        inputs = 46;
    }

    if (
        Cin_but.isHeld(100)  &&
        Din_but.isHeld(100)  &&
        !E1in_but.isHeld(100)&&
        !E2in_but.isHeld(100)&&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 50
        ) {
        setState(TRAIL);
        inputs = 50;
    }

    if (
        Cin_but.isHeld(100)  &&
        Din_but.isHeld(100)  &&
        E1in_but.isHeld(100) &&
        E2in_but.isHeld(100) &&
        !Fin_but.isHeld(100) &&
        !Gin_but.isHeld(100) &&
        state == POLL &&
        inputs != 62
        ) {
        setState(TRAIL);
        inputs = 62;
    }

    if (trip_but.isPressed()) {
        tripDelay.start(3000);
        tripCount++;
        Serial.println("tripCount++");
    }
    if (trip_but.isPressed() && tripCount == 2) {
        tripFlag = !tripFlag;
        Serial.println(tripFlag);
        tripCount = 0;
    }
    
    
    
    if (tripDelay.justFinished()) {
        tripCount = 0;
    }

}
