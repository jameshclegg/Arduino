
//
// OUTPUT PINS
//
// These are the pin numbers for the outputs (lights and bell)
// Colours represent jumper wires.
const int bell_out = 32;        // red
const int front_door_out = 46;  // purple
const int side_door_out = 47;   // white
const int kitchen_out = 48;     // lt grey (almost white)
const int living_room_out = 49; // black
const int b1_out = 50;          // orange
const int b2_out = 51;          // yellow
const int b4_out = 52;          // green (this is labelled bedroom 3 on the unit)
const int bathroom_out = 53;    // blue

// Lights that are internal
const int internal_out[6] = {kitchen_out, living_room_out, b1_out, b2_out, b4_out, bathroom_out};

// Lights that are external
const int external_out[2] = {front_door_out, side_door_out};

//
// INPUT PINS
//

// A pin that can be used to switch on all lights for testing
const int all_in_det = 35;

// These are the pin numbers for the inputs (connected to bell pushes)
const int front_door_in = 22;   // purple
const int side_door_in = 23;    // white, not working?
const int kitchen_in = 24;      // lt grey
const int living_room_in = 25;  // black
const int b1_in = 26;           // orange
const int b2_in = 27;           // yellow
const int b4_in = 28;           // green
const int bathroom_in = 29;     // blue

// Internal isolation switch
const int internal_insolation_in = 38;

// All internal inputs
const int internal_in[6] = {kitchen_in, living_room_in, b1_in, b2_in, b4_in, bathroom_in};

// All external inputs
const int external_in[2] = {front_door_in, side_door_in};

// 
// Other constants
//
// How often to read inputs
const unsigned long loop_delay_ms = 100;

// Internal bell pattern
const unsigned long internal_bell_on_ms = 500;
const unsigned long internal_bell_off_ms = 150;

// Sets how long the LEDs stay on after the last button is pushed
const unsigned long led_timer_delay_ms = 4000;

// Sets how long the buzzer is blocked for after an internal bell is pushed
const unsigned long internal_block_ms = 10000;

enum InternalBellStage {
    INTERNAL_BELL_IDLE,
    INTERNAL_BELL_FIRST_ON,
    INTERNAL_BELL_FIRST_OFF,
    INTERNAL_BELL_SECOND_ON,
    INTERNAL_BELL_SECOND_OFF
};

// 
// Helper functions
//

void all_out(int* x) {
    // Fills in all ouputs in x
    for (int i = 0; i < 6; i++) {
        x[i] = internal_out[i];
    }
    for (int i = 0; i < 2; i++) {
        x[6 + i] = external_out[i];
    }
}

void all_in(int* x) {
    // Fills in all inputs in x
    for (int i = 0; i < 6; i++) {
        x[i] = internal_in[i];
    }
    for (int i = 0; i < 2; i++) {
        x[6 + i] = external_in[i];
    }
}

void setup() {
    // start serial connection
    Serial.begin(9600);

    //
    // Configure inputs
    //

    // Configure the test pin
    pinMode(all_in_det, INPUT_PULLUP);

    // Configure the internal isolation switch
    pinMode(internal_insolation_in, INPUT_PULLUP);

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

    // Configure builtin LED
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("setup done");
}

void loop() {
    // Desired funcitonality

    // If any switch is pushed then the corresponding light is illuminated and goes off after t_light_on_sec sec.
    // If an external switch is pushed then the buzzer sounds for the length of time that the button is pushed.
    // If an internal switch is pushed then the buzzer sounds for 2 sharp buzzes, and after that does not sound again for t_break_sec sec.

    // There is a switch, internal_bell, that sets whether the internal bells sound.
    // If that switch is off then buzzer for the internal bells does not sound.

    // Used to store output of millis(). Wraps after about 50 days.
    unsigned long last_on_ms = 0;
    unsigned long light_timer_ms = 0;
    unsigned long internal_last_on_ms = 0;
    bool internal_bell_has_sounded = false;
    InternalBellStage internal_bell_stage = INTERNAL_BELL_IDLE;
    unsigned long internal_bell_stage_started_ms = 0;
    bool input_was_pressed[8] = {false, false, false, false, false, false, false, false};

    // Populate array of all inputs
    int inputs[8];
    all_in(inputs);

    // Populate array of all outputs
    int outputs[8];
    all_out(outputs);


    while (true) {
        delay(loop_delay_ms);

        unsigned long now_ms = millis();
        bool input_pressed[8];
        bool any_input_pressed = false;
        bool internal_press_started = false;
        bool external_input_pressed = false;
        bool test_input_pressed = digitalRead(all_in_det) == LOW;
        for (int i = 0; i < 8; i++) {
            input_pressed[i] = digitalRead(inputs[i]) == LOW;
            if (input_pressed[i]) {
                any_input_pressed = true;
                if (i < 6) {
                    internal_press_started = internal_press_started || !input_was_pressed[i];
                }
                else {
                    external_input_pressed = true;
                }
            }
        }
        any_input_pressed = any_input_pressed || test_input_pressed;

        if (any_input_pressed){
            // Something pressed - start the clock
            last_on_ms = now_ms;
        }
        else {
            // Nothing pressed - switch bell off
            digitalWrite(bell_out, LOW);

            // Switch all lights off after some time
            light_timer_ms = now_ms;
            if (light_timer_ms - last_on_ms >= led_timer_delay_ms) {
                for (int i = 0; i < 8; i++) {
                    digitalWrite(outputs[i], LOW);
                }
            }
        }

        //
        // Indicator LED control
        //
        for (int i = 0; i < 8; i++) {
            if (input_pressed[i] || test_input_pressed) {
                digitalWrite(outputs[i], HIGH);
            }
        }

        //
        // Buzzer control
        //
        switch (internal_bell_stage) {
            case INTERNAL_BELL_FIRST_ON:
                if (now_ms - internal_bell_stage_started_ms >= internal_bell_on_ms) {
                    internal_bell_stage = INTERNAL_BELL_FIRST_OFF;
                    internal_bell_stage_started_ms = now_ms;
                }
                break;
            case INTERNAL_BELL_FIRST_OFF:
                if (now_ms - internal_bell_stage_started_ms >= internal_bell_off_ms) {
                    internal_bell_stage = INTERNAL_BELL_SECOND_ON;
                    internal_bell_stage_started_ms = now_ms;
                }
                break;
            case INTERNAL_BELL_SECOND_ON:
                if (now_ms - internal_bell_stage_started_ms >= internal_bell_on_ms) {
                    internal_bell_stage = INTERNAL_BELL_SECOND_OFF;
                    internal_bell_stage_started_ms = now_ms;
                }
                break;
            case INTERNAL_BELL_SECOND_OFF:
                if (now_ms - internal_bell_stage_started_ms >= internal_bell_off_ms) {
                    internal_bell_stage = INTERNAL_BELL_IDLE;
                    internal_last_on_ms = now_ms;
                    internal_bell_has_sounded = true;
                }
                break;
            case INTERNAL_BELL_IDLE:
                break;
        }

        bool bell_on = internal_bell_stage == INTERNAL_BELL_FIRST_ON
            || internal_bell_stage == INTERNAL_BELL_SECOND_ON
            || external_input_pressed;
        if (internal_press_started) {
            int int_val = digitalRead(internal_insolation_in);
            if (int_val
                    && internal_bell_stage == INTERNAL_BELL_IDLE
                    && (!internal_bell_has_sounded || now_ms - internal_last_on_ms >= internal_block_ms)) {
                internal_bell_stage = INTERNAL_BELL_FIRST_ON;
                internal_bell_stage_started_ms = now_ms;
                bell_on = true;
            }
        }
        digitalWrite(bell_out, bell_on ? HIGH : LOW);
        for (int i = 0; i < 8; i++) {
            input_was_pressed[i] = input_pressed[i];
        }
    }
}
