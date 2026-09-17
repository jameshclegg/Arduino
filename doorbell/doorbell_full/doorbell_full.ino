
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

const int internal_doorbell_count = 6;
const int external_doorbell_count = 2;
const int doorbell_count = internal_doorbell_count + external_doorbell_count;

// These arrays are kept in the same order as internal_in so each input maps to
// the output at the same array index.
const int internal_out[internal_doorbell_count] = {
    kitchen_out, living_room_out, b1_out, b2_out, b4_out, bathroom_out
};

// These arrays are likewise aligned with external_in.
const int external_out[external_doorbell_count] = {front_door_out, side_door_out};

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

// Grounding this active-low input isolates (disables) the internal bell.
const int internal_isolation_in = 38;

// Internal inputs correspond positionally to internal_out.
const int internal_in[internal_doorbell_count] = {
    kitchen_in, living_room_in, b1_in, b2_in, b4_in, bathroom_in
};

// External inputs correspond positionally to external_out.
const int external_in[external_doorbell_count] = {front_door_in, side_door_in};

// 
// Other constants
//
// How often to read inputs
const unsigned long loop_delay_ms = 10;

// An input must remain unchanged for this long before its new state is used.
const unsigned long input_debounce_ms = 30;

// Internal bell pattern
const unsigned long internal_bell_on_ms = 500;
const unsigned long internal_bell_off_ms = 150;

// Sets how long the LEDs stay on after the last button is pushed
const unsigned long led_timer_delay_ms = 4000;

// Sets how long the buzzer is blocked for after an internal bell is pushed
const unsigned long internal_block_ms = 10000;

// The internal bell uses a non-blocking state machine so inputs can still be
// read while the two-buzz pattern is playing.
enum InternalBellStage {
    INTERNAL_BELL_IDLE,
    INTERNAL_BELL_FIRST_ON,
    INTERNAL_BELL_FIRST_OFF,
    INTERNAL_BELL_SECOND_ON,
    INTERNAL_BELL_SECOND_OFF
};

struct DebouncedInput {
    bool raw_active;
    bool active;
    unsigned long raw_changed_ms;
};

// Declare this explicitly because Arduino's automatic prototype generation
// can place prototypes before user-defined types.
bool read_active_low_input(int pin, DebouncedInput& state, unsigned long now_ms);

// 
// Helper functions
//

void all_out(int* pins) {
    // Internal pins come first, followed by the external pins.
    for (int i = 0; i < internal_doorbell_count; i++) {
        pins[i] = internal_out[i];
    }
    for (int i = 0; i < external_doorbell_count; i++) {
        pins[internal_doorbell_count + i] = external_out[i];
    }
}

void all_in(int* pins) {
    // Keep the same ordering as all_out so inputs[i] controls outputs[i].
    for (int i = 0; i < internal_doorbell_count; i++) {
        pins[i] = internal_in[i];
    }
    for (int i = 0; i < external_doorbell_count; i++) {
        pins[internal_doorbell_count + i] = external_in[i];
    }
}

bool internal_bell_stage_is_on(InternalBellStage stage) {
    return stage == INTERNAL_BELL_FIRST_ON || stage == INTERNAL_BELL_SECOND_ON;
}

bool read_active_low_input(int pin, DebouncedInput& state, unsigned long now_ms) {
    bool raw_active = digitalRead(pin) == LOW;

    if (raw_active != state.raw_active) {
        state.raw_active = raw_active;
        state.raw_changed_ms = now_ms;
    }

    if (state.active != state.raw_active
            && now_ms - state.raw_changed_ms >= input_debounce_ms) {
        state.active = state.raw_active;
    }

    return state.active;
}

void setup() {
    // Start serial connection.
    Serial.begin(115200);

    //
    // Configure inputs
    //

    // Configure the test pin
    pinMode(all_in_det, INPUT_PULLUP);

    // INPUT_PULLUP makes HIGH the normal enabled state and LOW isolated.
    pinMode(internal_isolation_in, INPUT_PULLUP);

    // Bell pushes are active-low: pressing one connects its pin to ground.
    int x_in[doorbell_count];
    all_in(x_in);
    for (int i = 0; i < doorbell_count; i++) {
        pinMode(x_in[i], INPUT_PULLUP);
    }

    //
    // Configure outputs
    //

    // Configure all light output pins
    int x_out[doorbell_count];
    all_out(x_out);
    for (int i = 0; i < doorbell_count; i++) {
        pinMode(x_out[i], OUTPUT);
        digitalWrite(x_out[i], LOW);
    }

    // Configure the bell
    pinMode(bell_out, OUTPUT);
    digitalWrite(bell_out, LOW);

    // Configure built-in LED.
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("setup done");
}

void loop() {
    // Each call samples the inputs, updates latched indicator lights, advances
    // the internal two-buzz pattern, and sets the shared buzzer output.

    // State is static so it persists between calls from the Arduino runtime.
    // Unsigned subtraction keeps elapsed-time checks safe when millis() wraps.
    static unsigned long last_on_ms = 0;
    static unsigned long internal_last_on_ms = 0;
    static bool internal_bell_has_sounded = false;
    static InternalBellStage internal_bell_stage = INTERNAL_BELL_IDLE;
    static unsigned long internal_bell_stage_started_ms = 0;

    // Previous states let internal bells trigger on press edges rather than
    // repeatedly while a button is held.
    static bool input_was_pressed[doorbell_count] = {};
    static DebouncedInput input_states[doorbell_count] = {};
    static DebouncedInput test_input_state = {};
    static DebouncedInput isolation_input_state = {};

    // inputs[i] and outputs[i] always describe the same door or room.
    int inputs[doorbell_count];
    all_in(inputs);
    int outputs[doorbell_count];
    all_out(outputs);

    delay(loop_delay_ms);

    unsigned long now_ms = millis();
    bool input_pressed[doorbell_count];
    bool any_input_pressed = false;
    bool internal_press_started = false;
    bool external_input_pressed = false;
    bool test_input_pressed = read_active_low_input(
        all_in_det, test_input_state, now_ms);
    bool internal_bell_isolated = read_active_low_input(
        internal_isolation_in, isolation_input_state, now_ms);

    // Debounce every input independently so contact bounce cannot create
    // false press edges. The final two entries are the external doors.
    for (int i = 0; i < doorbell_count; i++) {
        input_pressed[i] = read_active_low_input(
            inputs[i], input_states[i], now_ms);
        if (input_pressed[i]) {
            any_input_pressed = true;
            if (i < internal_doorbell_count) {
                internal_press_started = internal_press_started || !input_was_pressed[i];
            }
            else {
                external_input_pressed = true;
            }
        }
    }
    any_input_pressed = any_input_pressed || test_input_pressed;

    if (any_input_pressed) {
        // Extend the indicator timeout for as long as any input is held.
        last_on_ms = now_ms;
    }
    else {
        // Indicators latch on until there has been no input for the full
        // timeout. Subtraction is safe across millis() rollover.
        if (now_ms - last_on_ms >= led_timer_delay_ms) {
            for (int i = 0; i < doorbell_count; i++) {
                digitalWrite(outputs[i], LOW);
            }
        }
    }

    //
    // Indicator LED control
    //
    // The test input lights every indicator but deliberately does not
    // activate the buzzer.
    for (int i = 0; i < doorbell_count; i++) {
        if (input_pressed[i] || test_input_pressed) {
            digitalWrite(outputs[i], HIGH);
        }
    }

    //
    // Buzzer control
    //
    // Advance at most one timed stage per poll. Unlike delay(), this keeps
    // external buttons and indicator lights responsive during the pattern.
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

    bool internal_bell_enabled = !internal_bell_isolated;
    bool internal_bell_is_idle = internal_bell_stage == INTERNAL_BELL_IDLE;
    bool internal_bell_cooldown_complete = !internal_bell_has_sounded
        || now_ms - internal_last_on_ms >= internal_block_ms;
    bool start_internal_bell = internal_press_started
        && internal_bell_enabled
        && internal_bell_is_idle
        && internal_bell_cooldown_complete;

    if (start_internal_bell) {
        internal_bell_stage = INTERNAL_BELL_FIRST_ON;
        internal_bell_stage_started_ms = now_ms;
        Serial.println("Internal bell started");
    }
    else if (internal_press_started) {
        if (!internal_bell_enabled) {
            Serial.println("Internal bell ignored: isolated");
        }
        else if (!internal_bell_is_idle) {
            Serial.println("Internal bell ignored: pattern already playing");
        }
        else {
            Serial.println("Internal bell ignored: cooldown");
        }
    }

    // External buttons sound continuously while held. Internal buttons
    // sound only during the two ON stages of their pattern.
    bool internal_bell_on = internal_bell_stage_is_on(internal_bell_stage);
    bool bell_on = external_input_pressed || internal_bell_on;
    digitalWrite(bell_out, bell_on ? HIGH : LOW);

    // Save this sample for edge detection during the next poll.
    for (int i = 0; i < doorbell_count; i++) {
        input_was_pressed[i] = input_pressed[i];
    }
}
