
#include <DMXSerial.h>

#define T_RISE 60       // [s], rise time when potentiometer is at full right
#define T_FALL 60       // [s], fall time when potentiometer is at full right
#define START_DELAY 6   // [?]

// Arduino Uno pin definitions
#define PIN_ANALOG_ATTACK_IN 0
#define PIN_ANALOG_RELEASE_IN 1
#define PIN_MAIN_BYPASS_IN 8
#define PIN_MAIN_EXPONENTIAL_IN 9
#define PIN_ALARM_IN 10
#define PIN_TEST_IN 11
#define PIN_DMX_OUT 1

// Example definition of DMX channels 1..5 for a simple RGBW fixture.
// Currently unused, see very end of main loop.
#define DMX_CH_DIM 1 // main dimmer of the fixture
#define DMX_CH_R 2
#define DMX_CH_G 3
#define DMX_CH_B 4
#define DMX_CH_W 5

// Implement potentiometer averaging to filter out noise
class Averaged_Analog_Pin {
  private:
    size_t pin;
    uint16_t* buf;
    size_t buf_size;
    size_t head;
    
  public:
    Averaged_Analog_Pin(size_t pin, size_t n) {
      this->pin = pin;
      buf = (uint16_t*) calloc(sizeof(uint16_t), n);
      buf_size = n;
      head = 0;
    }

    ~Averaged_Analog_Pin(void) {
      free(buf);
    }

    uint16_t get_value(void) {
      buf[head] = analogRead(pin);
      if (++head == buf_size) {
        head = 0;
      }

      uint32_t sum = 0;
      for (size_t i = 0; i < buf_size; i++) {
        sum += buf[i];
      }
      return sum / buf_size;
    }
};

Averaged_Analog_Pin* attack;
Averaged_Analog_Pin* rlease;

void setup() {

  // Configure all pins
  pinMode(PIN_MAIN_BYPASS_IN, INPUT_PULLUP);
  pinMode(PIN_MAIN_EXPONENTIAL_IN, INPUT_PULLUP);
  pinMode(PIN_ALARM_IN, INPUT);
  pinMode(PIN_TEST_IN, INPUT_PULLUP);
  pinMode(PIN_DMX_OUT, OUTPUT);

  attack = new Averaged_Analog_Pin(PIN_ANALOG_ATTACK_IN, 10);
  rlease = new Averaged_Analog_Pin(PIN_ANALOG_RELEASE_IN, 10);

  DMXSerial.init(DMXController, PIN_DMX_OUT);

  // Set constant channels
  // DMXSerial.write(DMX_CH_DIM, 255); // Set main dimmer to full intensity
} // setup


uint8_t intensity = 0;
uint16_t n_samples_over = 0;

void loop() {

  float attack_value = (float)attack->get_value();
  float rlease_value = (float)rlease->get_value();

  float attack_dly = 0;
  float rlease_dly = 0;

  if (digitalRead(PIN_MAIN_BYPASS_IN) == LOW) { // BYPASS/OFF
    
    n_samples_over = 0;
    intensity = 0;
    
  } else if (digitalRead(PIN_MAIN_EXPONENTIAL_IN) == LOW) { // EXPONENTIAL increase/decrease

    if (digitalRead(PIN_TEST_IN) == LOW || digitalRead(PIN_ALARM_IN) == HIGH) {
      // Trigger by test button or alarm input.
      if (digitalRead(PIN_TEST_IN) == HIGH && n_samples_over++ < START_DELAY * 800) {
        // Implement an initial delay, except when test button is pressed.
        delay(1);
      } else {
        n_samples_over = START_DELAY * 800;
        if (intensity++ == 255) {
          intensity = 255;
        } else {
          attack_dly = (T_RISE - 0.53) / 4.91 * attack_value * (log((float)intensity + 2) - log((float)intensity + 1));
          delay(round(attack_dly));
        }
      }
    } else {
      // Test button or alarm input released.
      n_samples_over = 0;
      if (intensity-- == 0) {
        intensity = 0;
      } else {
        rlease_dly = (T_FALL - 0.53) / 4.91 * rlease_value * (log((float)intensity + 2) - log((float)intensity + 1));
        delay(round(rlease_dly));
      }
    }

  } else { // LINEAR increase/decrease

    if (digitalRead(PIN_TEST_IN) == LOW || digitalRead(PIN_ALARM_IN) == HIGH) {
      if (digitalRead(PIN_TEST_IN) == HIGH && n_samples_over++ < START_DELAY * 800) {
        delay(1);
      } else {
        n_samples_over = START_DELAY * 800;
        if (intensity++ == 255) {
          intensity = 255;
        } else {
          attack_dly = (T_RISE - 0.359) / 0.250 / 1022 * attack_value;
          delay(round(attack_dly));
        }
      }
    } else {
      n_samples_over = 0;
      if (intensity-- == 0) {
        intensity = 0;
      } else {
        rlease_dly = (T_FALL - 0.359) / 0.250 / 1022 * rlease_value;
        delay(round(rlease_dly));
      }
    }

  }

  // Blast DMX channels 1..255
  for (uint16_t i = 1u; i <= 255; i++) {
    DMXSerial.write(i, intensity);
  }

  // Assuming Light fixture with Dimmer fixed full:
  //DMXSerial.write(DMX_CH_R, intensity);
  //DMXSerial.write(DMX_CH_G, intensity);
  //DMXSerial.write(DMX_CH_B, intensity);

} // loop
