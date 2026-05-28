#pragma once

#include <cstdint>
#include <string>

namespace rtshield {

class Shield {
public:
    explicit Shield(const std::string& device = "/dev/serial0", int baud = 115200);
    ~Shield();

    Shield(const Shield&) = delete;
    Shield& operator=(const Shield&) = delete;

    std::string ping();

    int read_analog_raw(int channel);
    double read_analog_voltage(int channel);
    void write_analog_raw(int channel, int value);
    void write_analog_voltage(int channel, double volts);

    bool read_digital(int pin);
    void write_digital(int pin, bool value);
    void set_digital_input(int pin);
    void set_digital_output(int pin);

    void set_led(int led, bool on);
    bool read_switch(int sw);
    void set_relay(int relay, bool on);

private:
    int fd_ = -1;

    std::string command(const std::string& line);
    int command_int(const std::string& line);
};

}  // namespace rtshield
