#include <focuser.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdio.h>

std::vector<libusb_device *>
Focuser::find_device(libusb_device **devs,
                     int cnt,
                     uint16_t vid, uint16_t pid,
                     const std::string& vendor, const std::string& product)
{
	std::vector<libusb_device *> founded;
	ssize_t i;									// for iterating through the list
	for (i = 0; i < cnt; i++)
	{
		auto dev = devs[i];
		libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			continue;

		if (desc.idVendor == vid && desc.idProduct == pid)
		{
			founded.push_back(dev);
		}
	}

	return founded;
}

Focuser::Focuser() : vendor("vladtcvs"), product("focuser"), vid(0x16c0), pid(0x3e08)
{
	int r = libusb_init(&ctx);
	if (r < 0)
	{
        libusb_exit(ctx);
		throw 0;
	}
	
    size_t cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0)
	{
        libusb_free_device_list(devs, 1);
	    libusb_exit(ctx);
		throw 0;
	}

    auto devices = find_device(devs, cnt, vid, pid, vendor, product);
	if(devices.size() == 0)
    {
        libusb_free_device_list(devs, 1);
	    libusb_exit(ctx);
        throw 0;
    }

    for (int i = 0; i < devices.size(); i++)
    {
        auto dev = devices[i];
        libusb_device_handle *handle;
        auto err = libusb_open(dev, &handle);
        if (err)
        {
            for (int j = 0; j < i; j++)
                libusb_close(handles[j]);

		    libusb_free_device_list(devs, 1);
	        libusb_exit(ctx);
            throw 0;
	    }
        handles.push_back(handle);
    }

    active_device = 0;
}

Focuser::~Focuser()
{
    for (int i = 0; i < handles.size(); i++)
        libusb_close(handles[i]);

    libusb_free_device_list(devs, 1);
	libusb_exit(ctx);
}

size_t Focuser::num_devices()
{
    return handles.size();
}

void Focuser::select_device(int id)
{
    if (id < 0 || id >= handles.size())
        throw 1;
    active_device = id;
}

void Focuser::device_set_led(bool on)
{
	uint8_t requestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT;
	uint8_t request = on ? 1 : 0;
	uint16_t wValue = 0;
	uint16_t wIndex = 0;
	uint8_t data[1];
	uint16_t wLength = 0;
	int timeout = 1000;

	int result = libusb_control_transfer(handles[active_device], requestType, request, wValue, wIndex, data, wLength, timeout);
	if (result < 0)
		throw 2;
}

std::string Focuser::device_send_command(std::string cmd)
{
	uint8_t requestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT;
	uint8_t request = 2;
	uint16_t wValue = 0;
	uint16_t wIndex = 0;
	uint16_t wLength = cmd.length();
	
    std::cout << "Send command: " << cmd << std::endl;

	uint8_t data[1024] = {0};
	memcpy(data, cmd.c_str(), wLength);
	int timeout = 1000;

	int result = libusb_control_transfer(handles[active_device], requestType, request, wValue, wIndex, data, wLength, timeout);
	if (result < 0)
		throw 2;

    memset(data, 0, sizeof(data));

    requestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT;
    request = 3;
    wLength = 8;
    result = libusb_control_transfer(handles[active_device], requestType, request, wValue, wIndex, data, wLength, timeout);
	if (result < 0)
		throw 2;
	
    std::cout << "Response: [" << data << "] " << result << std::endl;
	return std::string((const char*)data, wLength);
}

// Convert from 28-bit signed int
static int decode(const std::string& str)
{
    uint32_t v;
    sscanf(str.c_str(), "%X", &v);
    if (v >= 0x8000000UL)
        v |= 0xFF000000UL;
    int32_t vv = (int32_t)v;
    return vv;
}

// Convert int to 28-bit signed int
static std::string encode(int val)
{
    int32_t v = val;
    uint32_t vv = v;
    vv <<= 4;
    vv >>= 4;
    char buf[8];
    snprintf(buf, 8, "%07X", vv);
    buf[7] = 0;
    return std::string(buf);
}

void Focuser::device_move_with_speed(bool dir)
{
    device_send_command(std::string("MD") + (dir ? "T" : "F"));
}

void Focuser::device_move_to_target(int pos_um)
{
    std::ostringstream os;
    os << "T" << encode(pos_um);
    device_send_command(os.str());
}

void Focuser::device_set_speed(int um_per_sec)
{
    std::ostringstream os;
    os << "SS" << um_per_sec;
    device_send_command(os.str());
}

int  Focuser::device_get_position_um()
{
    auto resp = device_send_command("GP");
    if (resp[0] != 'P')
        throw 4;
    resp.erase(0,1);
    return decode(resp);
}

bool Focuser::device_is_busy()
{
    auto resp = device_send_command("GB");
    return (resp[0] == 'T');
}

void Focuser::device_set_position_um(int pos_um)
{
    std::ostringstream os;
    os << "P" << encode(pos_um);
    device_send_command(os.str());
}

void Focuser::device_stop()
{
    device_send_command("B");
}

void Focuser::device_set_bounds(int min_um, int max_um)
{
    std::ostringstream os1;
    os1 << "N" << encode(min_um);
    device_send_command(os1.str());
    
    std::ostringstream os2;
    os2 << "X" << encode(max_um);
    device_send_command(os2.str());
}

void Focuser::device_unforce()
{
    device_send_command("U");
}
