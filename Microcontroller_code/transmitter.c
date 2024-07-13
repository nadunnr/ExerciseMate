#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "i2c.h"
#include "mpu6050.h"
#include "nrf24l01.h"
#include <util/delay.h>

// Define threshold constants
#define UPPER_THRESHOLD 1.1
#define LOWER_THRESHOLD 0.8
#define DROP_THRESHOLD 0.8

// Global variables
volatile uint8_t exercise_mode = 0; // 0-4 for 5 exercise modes
volatile uint16_t exercise_count = 0;
volatile bool rf_interrupt = false;
volatile bool transmit_data = false;
int16_t accel_buff[3]; // Buffer to store accelerometer data

// Setup the nRF24L01 module
nRF24L01 *setup_rf(void) {
    nRF24L01 *rf = nRF24L01_init();
    rf->ss.port = &PORTB;
    rf->ss.pin = PB2;
    rf->ce.port = &PORTB;
    rf->ce.pin = PB1;
    rf->sck.port = &PORTB;
    rf->sck.pin = PB5;
    rf->mosi.port = &PORTB;
    rf->mosi.pin = PB3;
    rf->miso.port = &PORTB;
    rf->miso.pin = PB4;
    EICRA |= _BV(ISC01); // Interrupt on falling edge of INT0 (PD2)
    EIMSK |= _BV(INT0);  // Enable INT0
    nRF24L01_begin(rf);
    return rf;
}

// Setup the MPU6050 accelerometer
void setup_mpu6050(void) {
    i2c_init();
    mpu6050_init();
}

// Setup the button input
void setup_button(void) {
    DDRB &= ~_BV(PB0); // Set PB0 as input
    PORTB |= _BV(PB0); // Enable pull-up resistor on PB0
}

// Variables to store previous accelerometer values
double ax_prev = -20, ay_prev = -20, az_prev = -20;

// Walking mode state enumeration
typedef enum {
    BELOW,
    EXCEEDED
} State;

State state = BELOW; // Initial state

int main(void) {
    sei(); // Enable global interrupts
    nRF24L01 *rf = setup_rf(); // Initialize RF module
    setup_mpu6050(); // Initialize MPU6050
    setup_button(); // Initialize button

    while (1) {
        // Check if the button is pressed
        if (bit_is_clear(PINB, PB0)) {
            _delay_ms(50); // Debounce delay
            exercise_mode = (exercise_mode + 1) % 5; // Cycle through exercise modes
            exercise_count = 0; // Reset exercise count
            transmit_data = true; // Flag to transmit data
        }

        // Transmit data if flagged
        if (transmit_data) {
            transmit_data = false;
            nRF24L01Message msg;
            msg.data[0] = exercise_mode;
            msg.data[1] = exercise_count >> 8;
            msg.data[2] = exercise_count & 0xFF;
            msg.length = 3;
            nRF24L01_transmit(rf, (uint8_t[5]){0x01, 0x01, 0x01, 0x01, 0x01}, &msg);
        }

        // Check RF interrupt flag
        if (rf_interrupt) {
            rf_interrupt = false;
            int success = nRF24L01_transmit_success(rf);
            if (success != 0)
                nRF24L01_flush_transmit_message(rf);
        }

        // Read accelerometer data
        double ax, ay, az;
        mpu6050_read_accel_ALL(accel_buff);

        // Convert accelerometer data to g
        ax = accel_buff[0] * 2.0 / 32768.0;
        ay = accel_buff[1] * 2.0 / 32768.0;
        az = accel_buff[2] * 2.0 / 32768.0;

        // Exercise counting logic based on exercise mode
        if (exercise_mode == 0) {
            switch (state) {
                case BELOW:
                    if (ax > UPPER_THRESHOLD) {
                        exercise_count++;
                        state = EXCEEDED;
                    }
                    break;
                case EXCEEDED:
                    if (ax < DROP_THRESHOLD) {
                        state = BELOW;
                    }
                    break;
            }
        } else if (exercise_mode == 1) {
            switch (state) {
                case BELOW:
                    if (ay <= 0) {
                        exercise_count++;
                        state = EXCEEDED;
                    }
                    break;
                case EXCEEDED:
                    if (ay > 0.5) {
                        state = BELOW;
                    }
                    break;
            }
        } else if (exercise_mode == 2) {
            switch (state) {
                case BELOW:
                    if (az >= 0.5) {
                        exercise_count++;
                        state = EXCEEDED;
                    }
                    break;
                case EXCEEDED:
                    if (az < 0) {
                        state = BELOW;
                    }
                    break;
            }
        } else if (exercise_mode == 3) {
            switch (state) {
                case BELOW:
                    if (ay >= 0.5 && az <= 0) {
                        exercise_count++;
                        state = EXCEEDED;
                    }
                    break;
                case EXCEEDED:
                    if (az > 0.25) {
                        state = BELOW;
                    }
                    break;
            }
        } else if (exercise_mode == 4) {
            switch (state) {
                case BELOW:
                    if (ax >= 0.5) {
                        exercise_count++;
                        state = EXCEEDED;
                    }
                    break;
                case EXCEEDED:
                    if (ax < 0.25) {
                        state = BELOW;
                    }
                    break;
            }
        }

        // Update previous accelerometer values
        ax_prev = ax;
        ay_prev = ay;
        az_prev = az;
    }

    return 0;
}

// nRF24L01 interrupt handler
ISR(INT0_vect) {
    rf_interrupt = true;
}
