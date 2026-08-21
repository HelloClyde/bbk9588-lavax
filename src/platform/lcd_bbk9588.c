#include "type.h"
#include "base.h"
#include "bbk9588_platform.h"

#include <string.h>

extern word graph_mode, bgcolor, fgcolor;

int ScreenWidth = 256;
int ScreenHeight = 16;
static unsigned long screen_buffer[LCD_WIDTH * LCD_HEIGHT * 2 / 4]
    __attribute__((aligned(4)));
byte *BmpData = (byte *)screen_buffer;
byte GRAY;
static byte zoom_enabled = 1;
static byte palette[256 * 3];

static const byte system_palette[] = {
    0,0,0,191,191,191, 0,0,0,0,255,0,
    173,193,173,0,0,0, 255,255,255,0,0,0
};
static const byte level9[] = {0,32,64,96,128,160,192,224,255};
static const byte level5[] = {0,64,128,192,255};

void SetZoom(byte value) { zoom_enabled = value; (void)zoom_enabled; }

static void palette_changed(void)
{
    bbk9588_platform_present(BmpData, ScreenWidth, ScreenHeight, palette);
}

static void Palette2(void)
{
    palette[255 * 3] = palette[255 * 3 + 1] = palette[255 * 3 + 2] = 0;
    memcpy(palette, system_palette + (GRAY ? 18 : 12), 6);
    palette_changed();
}

static void Palette16(void)
{
    int i;
    palette[255 * 3] = palette[255 * 3 + 1] = palette[255 * 3 + 2] = 0;
    for (i = 0; i < 16; ++i)
        palette[i * 3] = palette[i * 3 + 1] = palette[i * 3 + 2] =
            (byte)((15 - i) * 0x11);
    palette_changed();
}

static void Palette256(void)
{
    int i;
    palette[0] = palette[1] = palette[2] = 0;
    palette[255 * 3] = palette[255 * 3 + 1] = palette[255 * 3 + 2] = 255;
    for (i = 0; i < 225; ++i) {
        palette[i * 3 + 48] = level5[(i / 5) % 5];
        palette[i * 3 + 49] = level9[i / 25];
        palette[i * 3 + 50] = level5[i % 5];
    }
    palette_changed();
}

void SetPalette(void)
{
    if (graph_mode == 4) Palette16();
    else if (graph_mode == 8) Palette256();
    else Palette2();
}

void lav_setpalette(byte from, int count, byte *address)
{
    int i;
    for (i = from; i < from + count && i < 256; ++i) {
        byte red = *address++;
        byte green = *address++;
        byte blue = *address++;
        ++address;
        palette[i * 3] = blue;
        palette[i * 3 + 1] = green;
        palette[i * 3 + 2] = red;
    }
    palette_changed();
}

void ClearNdsScreen(void)
{
    memset(BmpData, graph_mode == 8 ? 0 : 255, LCD_WIDTH * LCD_HEIGHT * 2);
    palette_changed();
}

void DmaRefresh(void) { palette_changed(); }
void SetWindow(void) { palette_changed(); }

void Color256Init(void)
{
    graph_mode = 8;
    bgcolor = 0;
    fgcolor = 255;
    SetPalette();
}

void Save_Palette(void)
{
    memcpy(task[task_lev].palette, palette, sizeof(palette));
}

void Load_Palette(void)
{
    memcpy(palette, task[task_lev].palette, sizeof(palette));
    palette_changed();
}
