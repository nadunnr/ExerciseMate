#include <avr/io.h>           // AVR standard I/O definitions
#include <avr/interrupt.h>    // AVR interrupt handling
#include <stdbool.h>          // Standard boolean definitions
#include <string.h>           // String manipulation functions
#include <util/delay.h>       // Delay functions
#include "nrf24l01.h"         // NRF24L01 module header
#include "lcd.h"              // LCD display header

volatile bool rf_interrupt = false;  // Flag to indicate NRF24L01 interrupt

// Function to set up NRF24L01 module
nRF24L01 *setup_rf(void) {
    nRF24L01 *rf = nRF24L01_init();  // Initialize NRF24L01 module
    // Set up pins for SPI communication
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
    EICRA |= _BV(ISC01);    // Configure interrupt on falling edge of INT0 (PD2)
    EIMSK |= _BV(INT0);     // Enable INT0 interrupt
    nRF24L01_begin(rf);     // Begin NRF24L01 module
    return rf;              // Return pointer to NRF24L01 instance
}

// Function to set up SSD1306 OLED display
void setup_display(void) {
    ssd1306_init();                // Initialize SSD1306 display
    ssd1306_clear_display();       // Clear display
    ssd1306_set_cursor(0, 0);      // Set cursor to top-left corner
    ssd1306_print_string("Select exercise mode:");  // Print initial message
}

// Function to get the exercise mode string
const char* get_exercise_mode_string(uint8_t mode) {
    switch (mode) {
        case 0: return "Walking";
        case 1: return "Jumping jacks";
        case 2: return "Side shoulder stretch";
        case 3: return "Side shoulder stretch";
        case 4: return "Overhead shoulder stretch";
        default: return "Unknown mode";
    }
}

// Main function
int main(void) {
    uint8_t address[5] = {0x01, 0x01, 0x01, 0x01, 0x01};  // Address for NRF24L01 communication
    sei();  // Enable global interrupts
    nRF24L01 *rf = setup_rf();  // Set up NRF24L01 module
    setup_display();  // Set up SSD1306 display
    nRF24L01_listen(rf, 0, address);  // Start listening on NRF24L01 with given address

    while (1) {
        if (rf_interrupt) {  // Check if NRF24L01 interrupt occurred
            rf_interrupt = false;  // Clear interrupt flag

            if (nRF24L01_data_received(rf)) {  // Check if data received
                nRF24L01Message msg;  // NRF24L01 message container
                nRF24L01_read_received_data(rf, &msg);  // Read received data
                uint8_t exercise_mode = msg.data[0];  // Extract exercise mode from message
                uint16_t exercise_count = (msg.data[1] << 8) | msg.data[2];  // Extract exercise count

                // Print exercise mode and count on SSD1306 display
                ssd1306_set_cursor(0, 16);
                ssd1306_print_string("Exercise mode: ");
                ssd1306_print_string(get_exercise_mode_string(exercise_mode));

                ssd1306_set_cursor(0, 32);
                ssd1306_print_string("Exercise count: ");
                ssd1306_print_int(exercise_count);

                ssd1306_update_display();  // Update SSD1306 display
            }
        }
    }

    return 0;  // Return statement (never actually reached)
}

// Interrupt Service Routine (ISR) for INT0 vector (PD2)
ISR(INT0_vect) {
    rf_interrupt = true;  // Set interrupt flag indicating data received from NRF24L01
}
