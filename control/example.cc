#include <libusb.h>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>

using namespace std;

const unsigned VENDOR_ID = 0x16c0;
const unsigned PRODUCT_ID = 0x3e08;

#include <focuser.h>

int	main()
{
	Focuser *focuser = new Focuser;

	//focuser->device_set_led(false);
	focuser->device_stop();
	focuser->device_set_bounds(-10000, -8000);
	//focuser->device_set_position_um(-9000);

	focuser->device_set_speed(50);
	focuser->device_move_with_speed(true);
	std::cout << focuser->device_is_busy() << std::endl;
	std::cout << "Pos: " << focuser->device_get_position_um() << std::endl;

	focuser->device_unforce();

	delete focuser;
	return 0;
}
