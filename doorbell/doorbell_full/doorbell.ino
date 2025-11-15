
//
// OUTPUT PINS
//
// These are the pin numbers for the outputs (lights and bell)
int bell_out = 32;
int front_door_out = 46;
int side_door_out = 47;
int kitchen_out = 48;
int living_room_out = 49;
int b1_out = 50;
int b2_out = 51;
int b4_out = 52; // this is labelled bedroom 3 on the unit
int bathroom_out = 53;

// Lights that are internal
int internal_out[6] = {kitchen_out, living_room_out, b1_out, b2_out, b4_out, bathroom_out};

// Lights that are external
int external_out[2] = {front_door_out, side_door_out};

//
// INPUT PINS
//

// A pin that can be used to switch on all lights for testing
int all_in_det = 21;

// These are the pin numbers for the inputs (connected to bell pushes)
const int front_door_in = 22;
const int side_door_in = 23;
const int kitchen_in = 24;
const int living_room_in = 25;
const int b1_in = 26;
const int b2_in = 27;
const int b4_in = 28;
const int bathroom_in = 29;

// All internal inputs
int internal_in[6] = {kitchen_in, living_room_in, b1_in, b2_in, b4_in, bathroom_in};

// All external inputs
int external_in[2] = {front_door_in, side_door_in};

// 
// Helper functions
//

int* all_out(int* x) {
    // Returns all outputs
    for (int i = 0; i < 6; i++) {
        x[i] = internal_out[i];
    }
    for (int i = 0; i < 2; i++) {
        x[6 + i] = external_out[i];
    }
    return x;
}

int* all_in(int* x) {
    // Returns all inputs
    for (int i = 0; i < 6; i++) {
        x[i] = internal_in[i];
    }
    for (int i = 0; i < 2; i++) {
        x[6 + i] = external_in[i];
    }
    return x;
}

int detect(void) {
    // Detects whether a button has been pressed and returns the pin number that was pressed
    int inputs[8];
    all_in(inputs);

    int which_on = -1;
    for (int i = 0; i < 8; i++) {
        int val = digitalRead(inputs[i]);
        if (val == LOW) {
        which_on = inputs[i];
        }
    }

    return which_on;
}

void setup() {
    // start serial connection
    Serial.begin(9600);

    //
    // Configure inputs
    //

    // Configure the test pin
    pinMode(all_in_det, INPUT_PULLUP);

    // Configure all input pins
    int x_in[8];
    all_in(x_in);
    for (int i = 0; i < 8; i++) {
        pinMode(x_in[i], INPUT_PULLUP);
    }

    //
    // Configure outputs
    //

    // Configure all light output pins
    int x_out[8];
    all_out(x_out);
    for (int i = 0; i < 8; i++) {
        pinMode(x_out[i], OUTPUT);
        digitalWrite(x_out[i], LOW);
    }

    // Configure the bell
    pinMode(bell_out, OUTPUT);
    digitalWrite(bell_out, LOW);

    Serial.println("setup done");
}

void loop() {
    // Desired funcitonality

    // If any switch is pushed then the corresponding light is illuminated and goes off after t_light_on_sec sec.
    // If an external switch is pushed then the buzzer sounds for the length of time that the button is pushed.
    // If an internal switch is pushed then the buzzer sounds for 2 sharp buzzes, and after that does not sound again for t_break_sec sec.

    // There is a switch, internal_bell, that sets whether the internal bells sound.
    // If that switch is off then buzzer for the internal bells does not sound.
    //int sensorVal = digitalRead(all_in);
    //int bellVal = digitalRead(bell_in);

    // Used to store output of millis(). Wraps after about 50 days.
    int last_on = 0;
    const int time_on = 6000;

    const int internal_cycle_time_ms = 300;

    // Populate array of all inputs
    int inputs[8];
    all_in(inputs);

    // Populate array of all outputs
    int outputs[8];
    all_out(outputs);

    int internal_off = false;
    // Serial.println("x")

    while (true) {

    int which_input = detect();

    Serial.println(which_input);

    switch (which_input) {
        case front_door_in:
            digitalWrite(front_door_out, HIGH);
            break;
        case side_door_in:
            digitalWrite(side_door_out, HIGH);
            break;
        case kitchen_in:
            digitalWrite(kitchen_out, HIGH);
            break;
        case living_room_in:
            digitalWrite(living_room_out, HIGH);
            break;
        case b1_in:
            digitalWrite(b1_out, HIGH);
            break;
        case b2_in:
            digitalWrite(b2_out, HIGH);
            break;
        case b4_in:
            digitalWrite(b4_out, HIGH);
            break;
        case bathroom_in:
            digitalWrite(bathroom_out, HIGH);
            break;
    }

    if (which_input > -1) {
        // Something has been pressed
        if (which_input == front_door_in || which_input == side_door_in) {
            digitalWrite(bell_out, HIGH);
            // delay(1000);
        } else {
            digitalWrite(bell_out, HIGH);
            delay(internal_cycle_time_ms);
            digitalWrite(bell_out, LOW);
            delay(internal_cycle_time_ms);
            digitalWrite(bell_out, HIGH);
            delay(internal_cycle_time_ms);
            digitalWrite(bell_out, LOW);
        }
    }

    delay(500);
  }
}
