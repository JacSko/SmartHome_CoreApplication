# SmartHome CoreApplication

## Description
The idea of the project is to gain simple control over home appliance (like lighting).
Main board was desgined to use STM32F411CEU MCU. It communicates with other boards (list below) using different interfaces (I2C, OneWire, USART).
The main responsibility of this part of project is to ensure informations about current temperature and humidity and appliance state.

![github_core_diagram](https://user-images.githubusercontent.com/47041583/107790916-a00f4300-6d53-11eb-9905-4ffcf12df80a.png)

**Main board is connected with:**
- **Relays board** - equipped with 16 relays, supports either 12VDC and 230VAC switching. Board allows to e.g control light state in all rooms. Connection over I2C bus.
- **Inputs board** - board designed to handle 16 inputs (8x 12VDC, 8x 230VAC). Connection over I2C bus with interrupt fired on each input state changed. The bouncing of inputs state has been resolved either by adding debouce capacitor and programatically, by delaying inputs state readout.
- **LED's board** - board equipped with 16 outputs to control led ambient light on staircase placed near to steps. It allows to achieve some lighting effects since can be controlled independently.
- **Sensors board** - Board, where temperature and humidity sensors (DHT11/DHT22 - type is auto-detected in application), also light intense sensor is connected. The communication is realized over OneWire interface, when light intense sensor signal is send in pure analogue form.

## Details
This module is not able to present gathered data.
There is another project (LINK DO RASPI PORJEKTU) responsible for presenting received data.
The communication with CoreApplication can be done in two ways:
- **Over WiFi** - ESP01 module is onboard, on startup it creates TCP server, where the RaspberryPi can connects and gets the data. Messages are send in raw bytes mode (for protocol specification see notification_manager.h)
- **Over Bluetooth** - this channel is desgined mainly to provide debug traces, but there are several commands that can be issued in string-based, human readable form (see doc/ dirextory for documntation details)

Project build is managed by CMake.
Core application was prepared using CMSIS bare-metal library without dependencies to external libraries (only appropriate header files are required, but those are part of repository).
Project contains also unit tests, where the original MCU header files has been stubbed to allow compilation.

![github_core_sw_diagram](https://user-images.githubusercontent.com/47041583/107791094-ce8d1e00-6d53-11eb-9a1d-f7ec65025457.png)

## Building
### Building for target
To build binary, You need to have:
- the arm-none-eabi toolchain
- The st-linkv2 drivers (can be compiled from sources)
- The ST-FLASH utility installed
Once all of those step are ready:
```
cd <project_dir>
mkdir build && cd build
cmake ..
make
```
After the binary can be flashed by issuing:
```
make write
```
### Building Unit tests
To build unit test, You have to clone googletest repo:
```
cd <project_dir>
cd ext_lib/googletest
git clone https://github.com/google/googletest.git
```
Than run script:
```
./build_and_run_ut.sh
```
### Coverage calculation
Run script:
```
./build_and_run_coverage.sh
```
HTML report will be opened automatically in Firefox, but it can be also found:
```
build_ut/html/coverage.html
```
## TODO
- [ ] EEPROM handling
- [ ] Settings rework to allow read/write settings from EEPROM
- [ ] ADC driver (power supply voltage, light intensity)
- [ ] Flashing over UART (from RaspberryPi module)
