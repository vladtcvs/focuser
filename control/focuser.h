#pragma once

#include <libusb.h>
#include <stdint.h>
#include <vector>
#include <string>

class Focuser
{
    private:
        const std::string vendor;
        const std::string product;
        const uint16_t vid;
        const uint16_t pid;

        libusb_context *ctx;
        libusb_device **devs;
        std::vector<libusb_device_handle *> handles;
        int active_device;
    private:
        std::vector<libusb_device *> find_device(libusb_device **devs,
                                                 int cnt,
                                                 uint16_t vid, uint16_t pid,
                                                 const std::string& vendor, const std::string& product);
        std::string device_send_command(std::string cmd);
    public:
        Focuser();
        ~Focuser();
        Focuser operator = (const Focuser &) = delete;
        Focuser operator = (const Focuser &&) = delete;
    public:
        void select_device(int id);
        size_t num_devices();
        void device_set_led(bool on);
        void device_move_with_speed(bool dir);
        void device_move_to_target(int pos_um);
        void device_set_speed(int um_per_sec);
        int  device_get_position_um();
        void device_set_position_um(int um);
        void device_set_bounds(int min_um, int max_um);
        bool device_is_busy();
        void device_stop();
        void device_unforce();
};
