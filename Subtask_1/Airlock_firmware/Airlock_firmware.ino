#include <Adafruit_NeoPixel.h>

// Pin Definitions
#define PRESENCE_FRONT 12	// IR beam sensor - front zone
#define PRESENCE_MIDDLE 14   // IR beam sensor - middle zone
#define PRESENCE_BACK 27 	// IR beam sensor - back zone
#define GATE_SAFETY_A 26 	// Gate A safety sensor (LOW = obstructed)
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

AirlockState currentState = ST_IDLE;
Adafruit_NeoPixel ledStrip(8, LED_DATA, NEO_GRB + NEO_KHZ800);

// Safety wrapper for gate operations
void operateGate(uint8_t gate, bool open) {
  if(gate == 0) {
	if(digitalRead(GATE_SAFETY_A) == LOW) return;  // LOW means obstructed
	digitalWrite(GATE_REQUEST_A, open);
  } else {
	if(digitalRead(GATE_SAFETY_B) == LOW) return;
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
  pinMode(PRESENCE_FRONT, INPUT);
  pinMode(PRESENCE_MIDDLE, INPUT);
  pinMode(PRESENCE_BACK, INPUT);
  pinMode(GATE_SAFETY_A, INPUT);
  pinMode(GATE_SAFETY_B, INPUT);
 
  pinMode(GATE_REQUEST_A, OUTPUT);
  pinMode(GATE_REQUEST_B, OUTPUT);
  digitalWrite(GATE_REQUEST_A, LOW);
  digitalWrite(GATE_REQUEST_B, LOW);

  ledStrip.begin();
  updateLEDs(ledStrip.Color(0, 255, 0)); // Green = ready
}

void loop() {
  // Reverse logic: 0 means beam is cut (object present)
  bool front = digitalRead(PRESENCE_FRONT) == LOW;
  bool middle = digitalRead(PRESENCE_MIDDLE) == LOW;
  bool back = digitalRead(PRESENCE_BACK) == LOW;
 
  // Check safety sensors first (LOW = obstructed)
  if(digitalRead(GATE_SAFETY_A) == LOW || digitalRead(GATE_SAFETY_B) == LOW) {
	operateGate(0, false);
	operateGate(1, false);
	if(middle) currentState = ST_MIDDLE_OCCUPIED;
	else if(digitalRead(GATE_SAFETY_A) == LOW || digitalRead(GATE_SAFETY_B) == LOW)
		currentState = ST_SAFETY_LOCK;
	else
		currentState = ST_IDLE;
	updateLEDs(ledStrip.Color(255, 0, 0));  // Red
	return;
  }

  switch(currentState) {
	case ST_IDLE:
  	if(front && !middle && !back) {
    	operateGate(0, true);
    	currentState = ST_FRONT_ENTER;
    	updateLEDs(ledStrip.Color(0, 0, 255)); // Blue
  	}
  	else if(back && !middle && !front) {
    	operateGate(1, true);
    	currentState = ST_BACK_ENTER;
    	updateLEDs(ledStrip.Color(255, 165, 0)); // Orange
  	}
  	else if(front && back) { // Conflict
    	operateGate(0, true); // Prioritize front
    	currentState = ST_FRONT_ENTER;
    	updateLEDs(ledStrip.Color(128, 0, 128)); // Purple
  	}
  	break;

	case ST_FRONT_ENTER:
  	if(digitalRead(GATE_MOVING_A) == LOW) { // Gate A fully open
    	if(middle) {
      	operateGate(0, false);
      	currentState = ST_MIDDLE_OCCUPIED;
      	updateLEDs(ledStrip.Color(255, 255, 0)); // Yellow
    	}
  	}
  	break;

	case ST_MIDDLE_OCCUPIED:
  	if(digitalRead(GATE_MOVING_A) == HIGH) { // Gate A closed
    	operateGate(1, true);
    	currentState = ST_BACK_EXIT;
    	updateLEDs(ledStrip.Color(128, 0, 128)); // Purple
  	}
  	break;

	case ST_BACK_EXIT:
  	if(digitalRead(GATE_MOVING_B) == LOW) { // Gate B open
    	if(!back) { // Robot exited
      	operateGate(1, false);
      	currentState = ST_IDLE;
      	updateLEDs(ledStrip.Color(0, 255, 0)); // Green
    	}
  	}
  	break;

	// You can add mirrored states here like ST_BACK_ENTER etc.

	case ST_SAFETY_LOCK:
  	if(digitalRead(GATE_SAFETY_A) == HIGH && digitalRead(GATE_SAFETY_B) == HIGH) {
    	currentState = ST_IDLE;
    	updateLEDs(ledStrip.Color(0, 255, 0));
  	}
  	break;
  }
  delay(100);
}
