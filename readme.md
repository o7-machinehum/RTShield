# Mycrobez Shield
The Mycrobez sheild is a Raspberry PI hat containing an MCU to serve as a "realtime" companion. The PI is connected to the RP2350 over a UART link, and all sensors are wired into the RP2350 itself. Space permitting, maybe we can consider adding Arduino headers for additional shields.

## IOs
- 4x Analog output (0-3.3V, 12bit)
- 8x Analog input (0-3.3V, 12bit)
- IO's (3.3V)
- 2x Relays, for dry contacts
- Switches, for config
- LEDs

## Flashing RP2350 from RPI
```
sudo apt-get update
sudo apt-get install -y build-essential git autoconf automake libtool pkg-config \
    libusb-1.0-0-dev libhidapi-dev libjim-dev

git clone --depth=1 https://github.com/raspberrypi/openocd.git
cd openocd
./bootstrap
./configure --enable-bcm2835gpio --enable-sysfsgpio
make -j$(nproc)
sudo make install

### Wiring
GPIO25 (pin 22) → SWCLK
GPIO24 (pin 18) → SWDIO
GPIO18 (pin 12) → nRESET (optional)

### Connect
sudo openocd -f interface/raspberrypi-swd.cfg -f target/rp2350.cfg \
  -c "adapter speed 400; init; reset init"

### Or pogram
sudo openocd -f interface/raspberrypi-swd.cfg -f target/rp2350.cfg \
  -c "adapter speed 1000; init; program build/your_app.elf verify reset exit"

### Install pico sdk
sudo apt install cmake gcc-arm-none-eabi
mkdir -p ~/pico
cd ~/pico
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ..

git clone -b master https://github.com/raspberrypi/pico-examples.git

### Build blinky
export PICO_SDK_PATH=$HOME/pico/pico-sdk
cd ~/pico/pico-examples
mkdir build && cd build
cmake -DPICO_PLATFORM=rp2350 ..
cd blink
make -j8

blink.elf   # load with OpenOCD
blink.uf2   # drag & drop via USB boot
```
## License
The project is licensed under the Creative Commons (4.0 International License) Attribution—Noncommercial—Share Alike license. This allows sharing and adapting material for non-commercial purposes, provided credit is given to the creator and adaptations are shared under the same terms. The material can be used in any format with necessary technical modifications, but no warranties are provided. The license prohibits imposing additional restrictions and ensures the rights are irrevocable as long as the terms are followed.

Basically... If you make boards for yourself: great, make boards for your friends: great, but **please do not sell hundreds on them on Aliexpress**.

Files in `docs/` 3rd party reference material are not covered by this licence.
