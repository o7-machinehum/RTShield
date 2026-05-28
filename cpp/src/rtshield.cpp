#include "rtshield/rtshield.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace rtshield {
namespace {

speed_t baud_constant(int baud) {
    if (baud == 115200) {
        return B115200;
    }
    throw std::runtime_error("unsupported baud rate");
}

void require_range(int value, int min, int max, const char* name) {
    if (value < min || value > max) {
        throw std::out_of_range(name);
    }
}

}  // namespace

Shield::Shield(const std::string& device, int baud) {
    fd_ = open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0) {
        throw std::runtime_error("open " + device + ": " + std::strerror(errno));
    }

    termios tty{};
    if (tcgetattr(fd_, &tty) != 0) {
        throw std::runtime_error("tcgetattr failed");
    }
    cfmakeraw(&tty);
    cfsetispeed(&tty, baud_constant(baud));
    cfsetospeed(&tty, baud_constant(baud));
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;
    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        throw std::runtime_error("tcsetattr failed");
    }
    tcflush(fd_, TCIOFLUSH);
}

Shield::~Shield() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

std::string Shield::command(const std::string& line) {
    std::string out = line + "\n";
    if (write(fd_, out.data(), out.size()) != static_cast<ssize_t>(out.size())) {
        throw std::runtime_error("serial write failed");
    }

    std::string response;
    char c = 0;
    while (read(fd_, &c, 1) == 1) {
        if (c == '\n') {
            break;
        }
        if (c != '\r') {
            response.push_back(c);
        }
    }
    if (response.rfind("OK", 0) != 0) {
        throw std::runtime_error("device error: " + response);
    }
    if (response.size() > 3) {
        return response.substr(3);
    }
    return {};
}

int Shield::command_int(const std::string& line) {
    return std::stoi(command(line));
}

std::string Shield::ping() {
    return command("PING");
}

int Shield::read_analog_raw(int channel) {
    require_range(channel, 0, 7, "analog channel");
    return command_int("AI " + std::to_string(channel));
}

double Shield::read_analog_voltage(int channel) {
    return read_analog_raw(channel) * 3.3 / 4095.0;
}

void Shield::write_analog_raw(int channel, int value) {
    require_range(channel, 0, 3, "analog output channel");
    require_range(value, 0, 4095, "analog output value");
    command("AO " + std::to_string(channel) + " " + std::to_string(value));
}

void Shield::write_analog_voltage(int channel, double volts) {
    int raw = static_cast<int>(std::clamp(volts, 0.0, 3.3) * 4095.0 / 3.3 + 0.5);
    write_analog_raw(channel, raw);
}

bool Shield::read_digital(int pin) {
    require_range(pin, 2, 9, "digital pin");
    return command_int("DI " + std::to_string(pin)) != 0;
}

void Shield::write_digital(int pin, bool value) {
    require_range(pin, 2, 9, "digital pin");
    command("DO " + std::to_string(pin) + " " + std::to_string(value ? 1 : 0));
}

void Shield::set_digital_input(int pin) {
    require_range(pin, 2, 9, "digital pin");
    command("DM " + std::to_string(pin) + " 0");
}

void Shield::set_digital_output(int pin) {
    require_range(pin, 2, 9, "digital pin");
    command("DM " + std::to_string(pin) + " 1");
}

void Shield::set_led(int led, bool on) {
    require_range(led, 0, 2, "led");
    command("LED " + std::to_string(led) + " " + std::to_string(on ? 1 : 0));
}

bool Shield::read_switch(int sw) {
    require_range(sw, 0, 3, "switch");
    return command_int("SW " + std::to_string(sw)) != 0;
}

void Shield::set_relay(int relay, bool on) {
    require_range(relay, 0, 1, "relay");
    command("REL " + std::to_string(relay) + " " + std::to_string(on ? 1 : 0));
}

}  // namespace rtshield
