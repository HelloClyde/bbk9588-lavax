#include "type.h"
#include "base.h"
#include "bbk9588_platform.h"

#include <stdlib.h>
#include <string.h>

volatile byte lav_key;
byte cur_keyb[128];
byte old_keyb[128];
volatile byte Hz128;
long TickCount;
int s_year, s_month, s_day, s_hour, s_min, s_sec, s_week;

static const byte default_key_code[] = {
    0,0x14,0x15,0x17,0x16,0x0d,0x1b,0x1c,0x1d,0x13,0x0e,0x1f,0x19
};

static byte key_code[] = {
    0,0x14,0x15,0x17,0x16,0x0d,0x1b,0x1c,0x1d,0x13,0x0e,0x1f,0x19,
    0,0,0,0,0,0,28,29,30,31,
    'q','w','e','r','t','y','u','i','o','p',
    'a','s','d','f','g','h','j','k','l',13,
    'z','x','c','v','b','n','m',19,20,14,
    25,26,18,27,'0','.',' ',23,21,22
};

static const char *key_name[] = {
    "","UP","DOWN","LEFT","RIGHT","A","B","SELECT","START",
    "L1","R1","X","Y"
};

struct lava_key_name { const char *name; byte code; };
static const struct lava_key_name lava_key_code[] = {
    {"F1",0x1c},{"F2",0x1d},{"F3",0x1e},{"F4",0x1f},
    {"HELP",0x19},{"SHIFT",0x1a},{"CAPS",0x12},{"ESC",0x1b},
    {"ENTER",0x0d},{"UP",0x14},{"DOWN",0x15},{"LEFT",0x17},
    {"RIGHT",0x16},{"PAGEUP",0x13},{"PAGEDOWN",0x0e},
    {"SPACE",0x20},{"POINT",0x2e}
};

void GetTouch(struct TOUCH *touch)
{
    int pushed = 0, x = 0, y = 0;
    bbk9588_platform_touch(&pushed, &x, &y);
    touch->pushed = (byte)pushed;
    touch->x = touch->rawX = (word)x;
    touch->y = touch->rawY = (word)y;
}

void SaveKeyCode(void)
{
    memcpy(task[task_lev].key_code, key_code, sizeof(default_key_code));
}

void LoadKeyCode(void)
{
    memcpy(key_code, task[task_lev].key_code, sizeof(default_key_code));
}

void SetKeyCode(char *config)
{
    memcpy(key_code, default_key_code, sizeof(default_key_code));
    ReadConfig(config);
}

int ConfigKey(char *name, char *value)
{
    unsigned i;
    int id = 0;
    byte code = 0;
    for (i = 0; i < sizeof(key_name) / sizeof(key_name[0]); ++i) {
        if (!strcmp(name, key_name[i])) { id = (int)i; break; }
    }
    if (!id) return 0;
    for (i = 0; i < sizeof(lava_key_code) / sizeof(lava_key_code[0]); ++i) {
        if (!strcmp(value, lava_key_code[i].name)) {
            code = lava_key_code[i].code;
            break;
        }
    }
    if (!code) {
        if (strlen(value) == 1u) {
            code = (byte)value[0];
            if (code >= 'A' && code <= 'Z') code += 'a' - 'A';
        } else {
            code = (byte)atoi(value + (value[0] == '0'));
        }
    }
    if (!code || code >= 128) return 0;
    key_code[id] = code;
    return 1;
}

int c_keyid(byte key)
{
    unsigned i;
    for (i = 1; i < sizeof(key_code); ++i)
        if (key_code[i] == key) return (int)i;
    return 0;
}

byte c_keyval(byte key)
{
    return key < sizeof(key_code) ? key_code[key] : 0;
}

int CheckExitKey(void) { return bbk9588_platform_should_exit(); }

void bbk9588_hardware_sample(void)
{
    unsigned i;
    memcpy(old_keyb, cur_keyb, sizeof(cur_keyb));
    memset(cur_keyb, 0, sizeof(cur_keyb));
    for (i = 1; i < sizeof(key_code); ++i) {
        cur_keyb[i] = (byte)bbk9588_platform_key_state(key_code[i]);
    }
    if (lav_key < 128) {
        for (i = 1; i < sizeof(key_code); ++i) {
            if (cur_keyb[i] && !old_keyb[i]) {
                lav_key = key_code[i] | 0x80;
                break;
            }
        }
    }
}

void InitNds(void)
{
    memset(cur_keyb, 0, sizeof(cur_keyb));
    memset(old_keyb, 0, sizeof(old_keyb));
    lav_key = 0;
    bbk9588_hardware_sample();
}

void GetLocalTime_os(void)
{
    unsigned long seconds = (unsigned long)TickCount / 256u;
    s_year = 2022;
    s_month = 1;
    s_day = 1 + (int)(seconds / 86400u);
    s_hour = (int)((seconds / 3600u) % 24u);
    s_min = (int)((seconds / 60u) % 60u);
    s_sec = (int)(seconds % 60u);
    s_week = (6 + (int)(seconds / 86400u)) % 7;
}

void SetLocalTime_os(void)
{
    /* The verified SDK has no writable RTC API. Keep the VM-side values. */
}
