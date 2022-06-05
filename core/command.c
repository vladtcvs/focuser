#include <command.h>
#include <movement.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define cmdcmp(cmd, len, command) (((len) >= sizeof(command) - 1) && !memcmp(cmd, command, sizeof(command) - 1))

static int32_t read_int(const char *s, size_t len)
{
    int sign = 1;
    int32_t val = 0;
    if (len == 0)
        return 0;

    if (*s == '-')
    {
        sign = -1;
        s++;
        len--;
    }
    else if (*s == '+')
    {
        sign = 1;
        s++;
        len--;
    }
    while (len > 0 && isdigit(*s))
    {
        val *= 10;
        val += (*s - '0');
        len--;
        s++;
    }

    return sign * val;
}

static bool read_bool(const char *s, size_t len)
{
    if (s[0] == 'T' || s[0] == 't')
        return true;
    return false;
}

static void (*response)(const char *msg);

void command_init(void (*resp)(const char *msg))
{
    response = resp;
}

void command_process(struct focuser_descriptor_s *desc, const char *cmd, size_t len)
{
    if (cmdcmp(cmd, len, "MT"))
    {
        int32_t pos_um = read_int(cmd+2, len-2);
        focuser_move_to_target_um(desc, pos_um);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "MD"))
    {
        bool dir = read_bool(cmd+2, len-2);
        focuser_move_with_speed(desc, dir);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "SP"))
    {
        int32_t pos_um = read_int(cmd+2, len-2);
        focuser_set_position_um(desc, pos_um);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "SS"))
    {
        int32_t um_per_sec = read_int(cmd+2, len-2);
        focuser_set_speed(desc, um_per_sec);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "GP"))
    {
        int32_t pos_um = focuser_get_position_um(desc);
        char buf[20];
        snprintf(buf, 20, "P%06d", pos_um);
        response(buf);
    }
    else if (cmdcmp(cmd, len, "GB"))
    {
        bool busy = focuser_is_busy(desc);
        if (busy)
            response("T");
        else
            response("F");
    }
    else
    {
        response("UC");
    }
}
