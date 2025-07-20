#include <Adafruit_NeoPixel.h>

// Pin Definitions
#define PRESENCE_FRONT 12	// IR beam sensor - front zone
#define PRESENCE_MIDDLE 14   // IR beam sensor - middle zone
#define PRESENCE_BACK 27 	// IR beam sensor - back zone
#define GATE_SAFETY_A 26 	// Gate A safety sensor (HIGH = obstructed)
#define GATE_SAFETY_B 25 	// Gate B safety sensor
#define GATE_REQUEST_A 33	// Gate A control (HIGH = open)
#define GATE_REQUEST_B 32	// Gate B control
#define GATE_MOVING_A 35 	// Gate A movement status (HIGH = moving)
#define GATE_MOVING_B 34 	// Gate B movement status
#define LED_DATA 13      	// WS2812B LED strip control

// State Machine
enum AirlockState {
  ST_IDLE,
  ST_FRONT_ENTER,
  ST_MIDDLE_OCCUPIED,
  ST_BACK_EXIT,
  ST_BACK_ENTER,
  ST_FRONT_EXIT,
  ST_SAFETY_LOCK
};

bool lastb[7]={HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
bool b[7]={HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

AirlockState currentState = ST_IDLE;
Adafruit_NeoPixel ledStrip(8, LED_DATA, NEO_GRB + NEO_KHZ800);

// Safety wrapper for gate operations
void operateGate(uint8_t gate, bool open) {
  if(gate == 0) {
	if(!b[1]) return;
	digitalWrite(GATE_REQUEST_A, open);
  } else {
	if(!b[3]) return;
	digitalWrite(GATE_REQUEST_B, open);
  }
}

void updateLEDs(uint32_t color) {
  for(int i=0; i<ledStrip.numPixels(); i++) {
	ledStrip.setPixelColor(i, color);
  }
  ledStrip.show();
}

void setup() {
	Serial.begin(9600);
  pinMode(PRESENCE_FRONT, INPUT_PULLUP);
  pinMode(PRESENCE_MIDDLE, INPUT_PULLUP);
  pinMode(PRESENCE_BACK, INPUT_PULLUP);
  pinMode(GATE_SAFETY_A, INPUT_PULLUP);
  pinMode(GATE_SAFETY_B, INPUT_PULLUP);
	pinMode(GATE_MOVING_A, INPUT_PULLUP);
  pinMode(GATE_MOVING_B, INPUT_PULLUP);
 
  pinMode(GATE_REQUEST_A, OUTPUT);
  pinMode(GATE_REQUEST_B, OUTPUT);
  digitalWrite(GATE_REQUEST_A, LOW);
  digitalWrite(GATE_REQUEST_B, LOW);

  ledStrip.begin();
  updateLEDs(ledStrip.Color(0, 255, 0)); // Green = ready
}

void loop() {
	
	bool currentb[7]={digitalRead(PRESENCE_FRONT),digitalRead(GATE_SAFETY_A),digitalRead(PRESENCE_MIDDLE),digitalRead(GATE_SAFETY_B),digitalRead(PRESENCE_BACK),digitalRead(GATE_MOVING_A),digitalRead(GATE_MOVING_B)};
	
	for(int i=0; i<7; i++){
	if(lastb[i]==HIGH && currentb[i]==LOW){
		b[i]=!b[i];
	}
	lastb[i]=currentb[i];
	}

	Serial.print("PRESENCE_FRONT: ");
	Serial.print(b[0]);
	Serial.print("\n");

	Serial.print("GATE_SAFETY_A: ");
	Serial.print(b[1]);
	Serial.print("\n");
	
	Serial.print("PRESENCE_MIDDLE: ");
	Serial.print(b[2]);
	Serial.print("\n");

	Serial.print("GATE_SAFETY_B: ");
	Serial.print(b[3]);
	Serial.print("\n");

	Serial.print("PRESENCE_BACK: ");
	Serial.print(b[4]);
	Serial.print("\n");

	Serial.print("GATE_MOVING_A: ");
	Serial.print(b[5]);
	Serial.print("\n");

	Serial.print("GATE_MOVING_B: ");
	Serial.print(b[6]);
	Serial.print("\n");
	Serial.print("\n");

	delay(1000);

  bool front = !b[0];
  bool middle = !b[2];
  bool back = !b[4];
 
  // Check safety sensors first
  if(b[1]==0 || b[3]==0) {
	operateGate(0, false);
	operateGate(1, false);
	if(!b[2]) currentState= ST_MIDDLE_OCCUPIED;
	else if(!b[1] || !b[3]) currentState = ST_SAFETY_LOCK;
	else currentState = ST_IDLE;
	updateLEDs(ledStrip.Color(255, 0, 0));
	return;
  }


  switch(currentState) {
	case ST_IDLE:
  	if(front && back) { // Conflict
    	operateGate(0, true); // Prioritize front
			currentState=ST_FRONT_ENTER;
    	updateLEDs(ledStrip.Color(128, 0, 128)); // Purple
  	}

  	else if(back && !middle && !front) {
    	operateGate(1, true);
    	currentState = ST_BACK_ENTER;
    	updateLEDs(ledStrip.Color(255, 165, 0)); // Orange
  	}
  	
		else if(front && !middle && !back) {
    	operateGate(0, true);
    	currentState = ST_FRONT_ENTER;
    	updateLEDs(ledStrip.Color(0, 0, 255)); // Blue
  	}
  	break;

	case ST_FRONT_ENTER:
  	if(!b[5]) { // Gate A fully open
    	if(middle) {
      	operateGate(0, false);
      	currentState = ST_MIDDLE_OCCUPIED;
      	updateLEDs(ledStrip.Color(255, 255, 0)); // Yellow
    	}
  	}
  	break;

	case ST_MIDDLE_OCCUPIED:
  	if(b[5]) { // Gate A closed
    	operateGate(1, true);
    	currentState = ST_BACK_EXIT;
    	updateLEDs(ledStrip.Color(128, 0, 128)); // Purple
  	}
  	break;

	case ST_BACK_EXIT:
  	if(!b[6]) { // Gate B open
    	if(back) { // Robot exited
      	operateGate(1, false);
      	currentState = ST_IDLE;
      	updateLEDs(ledStrip.Color(0, 255, 0));
    	}
  	}
  	break;

	// Mirror states for back entry
	// ...

	  case ST_SAFETY_LOCK:
  	if(b[1] && b[3]) {
    	currentState = ST_IDLE;
    	updateLEDs(ledStrip.Color(0, 255, 0));
  	}
  	break; 
  }
  delay(100);
}

