#include <command.h>
#include <movement.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define cmdcmp(cmd, len, command) (((len) >= sizeof(command) - 1) && !memcmp(cmd, command, sizeof(command) - 1))

static bool ishex(char c)
{
    if (isdigit(c))
        return true;
    if (c >= 'A' && c <= 'F')
        return true;
    return false;
}

static int dechex(char c)
{
    if (c <= '9')
        return (c - '0');
    return (c - 'A' + 10);
}

static int32_t read_int_s28(const char *s, size_t len)
{
    uint32_t val = 0;
    if (len == 0)
        return 0;

    val = dechex(*s);
    len--;
    s++;

    while (len > 0)
    {
        val <<= 4;
        val += dechex(*s);
        len--;
        s++;
    }

    if (val >= 0x8000000UL)
        val |= 0xFF000000UL;

    return (int32_t)val;
}

static int32_t read_uint_dec(const char *s, size_t len)
{
    uint32_t val = 0;
    if (len == 0)
        return 0;
    while (len > 0 && isdigit(*s))
    {
        val *= 10;
        val |= (*s - '0');
        s++;
        len--;
    }
    return val;
}

static void write_int_s28(char *buf, int32_t val)
{
    sprintf(buf, "%07" PRIX32, ((uint32_t)val) & 0x0FFFFFFFUL);
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
    if (cmdcmp(cmd, len, "T"))
    {
        int32_t pos_um = read_int_s28(cmd+1, len-1);
        if (focuser_move_to_target_um(desc, pos_um))
            response("BP");
        else
            response("OK");
    }
    else if (cmdcmp(cmd, len, "MD"))
    {
        bool dir = read_bool(cmd+2, len-2);
        focuser_move_with_speed(desc, dir);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "P"))
    {
        int32_t pos_um = read_int_s28(cmd+1, len-1);
        if (focuser_set_position_um(desc, pos_um))
            response("BP");
        else
            response("OK");
    }
    else if (cmdcmp(cmd, len, "N"))
    {
        int32_t pos_um = read_int_s28(cmd+1, len-1);
        focuser_set_minpos_um(desc, pos_um);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "X"))
    {
        int32_t pos_um = read_int_s28(cmd+1, len-1);
        focuser_set_maxpos_um(desc, pos_um);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "SS"))
    {
        int32_t um_per_sec = read_uint_dec(cmd+2, len-2);
        focuser_set_speed(desc, um_per_sec);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "GP"))
    {
        int32_t pos_um = focuser_get_position_um(desc);
        char buf[10];
        buf[0] = 'P';
        write_int_s28(buf + 1, pos_um);
        buf[9] = 0;
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
    else if (cmdcmp(cmd, len, "B"))
    {
        focuser_stop(desc);
        response("OK");
    }
    else if (cmdcmp(cmd, len, "U"))
    {
        focuser_stop(desc);
        focuser_unforce(desc);
        response("OK");
    }
    else
    {
        response("UC");
    }
}
