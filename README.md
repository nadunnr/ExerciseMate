# ExerciseMate

ExerciseMate is a wearable device designed to track exercises, paired with a stationary display. This repository contains all project files, including documentation, PCB designs, SolidWorks files, and microcontroller code. ExerciseMate is a comprehensive system designed to assist users in tracking their exercise routines with a wearable device and a stationary display unit.
 
## Folder Structure

### Enclosure
This folder contains SolidWorks files for the enclosure design.
- `Enclosure/`

### PCB
This folder contains Altium Designer files for schematic and PCB design.
- `PCB/`

### Microcontroller_code
This folder contains the microcontroller code developed for the project.

- `Microcontroller_code/receiver.c` - Microcontroller code for the central display device.
- `Microcontroller_code/transmitter.c` - Microcontroller code for the hand-wearable device.
- `Microcontroller_code/mpu6050` - Source code for the MPU6050 sensor, open-sourced from [YifanJiangPolyU/MPU6050](https://github.com/YifanJiangPolyU/MPU6050).
- `Microcontroller_code/nrf24l01` - Source code for the nRF24L01 module (wireless transmitter/receiver), open-sourced from [antoineleclair/avr-nrf24l01](https://github.com/antoineleclair/avr-nrf24l01).
- `Microcontroller_code/ssd1306` - Source code for the SSD1306 OLED display, open-sourced from [Sylaina/oled-display](https://github.com/Sylaina/oled-display).

### Documentation
This folder contains the project's documentation files.
- `DesignDocumentation_ExerciseMate.pdf` - Detailed design documentation for ExerciseMate.
- `DesignMethodologyReport_ExerciseMate.pdf` - Methodology report for the project's design process.
