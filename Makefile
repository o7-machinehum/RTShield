PICO_SDK_PATH ?= $(CURDIR)/third_party/pico-sdk
BUILD_TYPE ?= Release

FIRMWARE_ELF := firmware/build/rtshield_fw.elf
PY_EXT := $(shell python3-config --extension-suffix)
PY_MODULE := python/build/rtshield$(PY_EXT)
PY_SITE := $(shell python3 -m site --user-site)

.PHONY: all apt-deps firmware cpp python flash install clean

all: firmware cpp python

apt-deps:
	sudo apt install -y build-essential cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi python3-dev python3-pybind11 openocd

firmware:
	cmake --fresh -S firmware -B firmware/build -GNinja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
	cmake --build firmware/build

cpp:
	cmake -S cpp -B cpp/build -GNinja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build cpp/build

python:
	cmake -S python -B python/build -GNinja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build python/build

flash: firmware
	sudo openocd -f interface/raspberrypi-swd.cfg -f target/rp2350.cfg -c "adapter speed 1000; init; program $(FIRMWARE_ELF) verify reset exit"

install: python
	install -d "$(PY_SITE)"
	install -m 755 "$(PY_MODULE)" "$(PY_SITE)/rtshield$(PY_EXT)"

clean:
	cmake --build firmware/build --target clean 2>/dev/null || true
	cmake --build cpp/build --target clean 2>/dev/null || true
	cmake --build python/build --target clean 2>/dev/null || true
