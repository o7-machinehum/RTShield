#include <pybind11/pybind11.h>

#include "rtshield/rtshield.hpp"

namespace py = pybind11;

PYBIND11_MODULE(rtshield, m) {
    py::class_<rtshield::Shield>(m, "Shield")
        .def(py::init<const std::string&, int>(), py::arg("device") = "/dev/serial0", py::arg("baud") = 115200)
        .def("ping", &rtshield::Shield::ping)
        .def("read_analog_raw", &rtshield::Shield::read_analog_raw)
        .def("read_analog_voltage", &rtshield::Shield::read_analog_voltage)
        .def("write_analog_raw", &rtshield::Shield::write_analog_raw)
        .def("write_analog_voltage", &rtshield::Shield::write_analog_voltage)
        .def("read_digital", &rtshield::Shield::read_digital)
        .def("write_digital", &rtshield::Shield::write_digital)
        .def("set_digital_input", &rtshield::Shield::set_digital_input)
        .def("set_digital_output", &rtshield::Shield::set_digital_output)
        .def("set_led", &rtshield::Shield::set_led)
        .def("read_switch", &rtshield::Shield::read_switch)
        .def("set_relay", &rtshield::Shield::set_relay);
}
