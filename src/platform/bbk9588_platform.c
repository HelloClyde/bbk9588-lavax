#include "bbk9588_platform.h"

#include "bda_graphics.h"
#include "bda_input.h"
#include "bda_time.h"
#include "bda_window.h"

#include <stdint.h>
#include <string.h>

#define LOGICAL_WIDTH 320
#define LOGICAL_HEIGHT 240
#define GAME_HEIGHT 160
#define PHYSICAL_WIDTH 240
#define PHYSICAL_HEIGHT 320
#define RAW_EVENTS_PER_TICK 8u
#define EXIT_HOLD_MS 1500u

typedef struct soft_row {
    const char *const *labels;
    const unsigned char *codes;
    unsigned count;
} soft_row_t;

static bda_handle_t g_frame;
static bda_handle_t g_draw;
static bda_handle_t g_draw_owner;
static void *g_draw_object;
static int g_detached;
static int g_timer_started;
static int g_direct;
static int g_close_requested;
static int g_exit_combo;
static u32 g_exit_started;
static u32 g_last_present;
static bda_gui_framebuffer_t g_framebuffer;
static bda_gui_picture_t g_picture;
static uint16_t g_pixels[PHYSICAL_WIDTH * PHYSICAL_HEIGHT]
    __attribute__((aligned(4)));
static unsigned char g_keys[128];
static int g_touch_down;
static int g_touch_key;
static int g_pen_down;
static int g_pen_x;
static int g_pen_y;
static const unsigned char *g_game_indices;
static const unsigned char *g_palette;
static int g_game_width;
static int g_game_height;
static int g_status_mode;
static char g_status_line1[64];
static char g_status_line2[64];
static u32 g_vm_clock;
static u32 g_vm_fraction;

extern volatile unsigned char Hz128;
extern long TickCount;
void bbk9588_hardware_sample(void);

static const char *const g_row0_labels[] = {
    "Q","W","E","R","T","Y","U","I","O","P"
};
static const unsigned char g_row0_codes[] = {
    'q','w','e','r','t','y','u','i','o','p'
};
static const char *const g_row1_labels[] = {
    "A","S","D","F","G","H","J","K","L","ENT"
};
static const unsigned char g_row1_codes[] = {
    'a','s','d','f','g','h','j','k','l',0x0d
};
static const char *const g_row2_labels[] = {
    "Z","X","C","V","B","N","M","SPC","ESC"
};
static const unsigned char g_row2_codes[] = {
    'z','x','c','v','b','n','m',' ',0x1b
};
static const char *const g_row3_labels[] = {
    "F1","F2","F3","F4","HLP","PGU","PGD","LFT","RGT","UP","DN"
};
static const unsigned char g_row3_codes[] = {
    0x1c,0x1d,0x1e,0x1f,0x19,0x13,0x0e,0x17,0x16,0x14,0x15
};
static const soft_row_t g_rows[] = {
    {g_row0_labels,g_row0_codes,10u},
    {g_row1_labels,g_row1_codes,10u},
    {g_row2_labels,g_row2_codes,9u},
    {g_row3_labels,g_row3_codes,11u}
};

static int pointer_valid(const void *pointer)
{
    return pointer && (uint32_t)pointer != 0xffffffffu;
}

static void release_draw_context(void)
{
    if (g_draw && (int32_t)g_draw != -1) bda_gui_end_draw(g_draw);
    g_draw = 0;
    g_draw_owner = 0;
}

static int acquire_draw_context(bda_handle_t owner)
{
    if (g_draw && g_draw_owner == owner) return 1;
    release_draw_context();
    g_draw = bda_gui_current_draw(owner);
    if (!g_draw || (int32_t)g_draw == -1) {
        g_draw = 0;
        return 0;
    }
    g_draw_owner = owner;
    return 1;
}

static int ensure_firmware_renderer(void)
{
    if (!g_frame || !acquire_draw_context(g_frame)) return 0;
    if (!g_draw_object) g_draw_object = bda_gui_draw_object_create(7u);
    return pointer_valid(g_draw_object);
}

static int window_proc(
    bda_handle_t handle, u32 message, u32 wparam, u32 lparam
)
{
    if (message == BDA_MSG_DRAW_CONTEXT_ATTACH) {
        g_frame = handle;
        if (!g_direct) (void)ensure_firmware_renderer();
    } else if (message == BDA_MSG_DRAW_CONTEXT_DETACH) {
        g_direct = 0;
        memset(&g_framebuffer, 0, sizeof(g_framebuffer));
        if (!g_draw_owner || g_draw_owner == handle) release_draw_context();
        g_detached = 1;
    }
    return bda_gui_default_proc(handle, message, wparam, lparam);
}

static void put_logical(int x, int y, uint16_t color)
{
    int physical_x;
    int physical_y;
    if ((unsigned)x >= LOGICAL_WIDTH || (unsigned)y >= LOGICAL_HEIGHT) return;
    physical_x = y;
    physical_y = LOGICAL_WIDTH - 1 - x;
    g_pixels[physical_y * PHYSICAL_WIDTH + physical_x] =
        bda_gui_rgb565_avoid_black_key(color);
}

static void fill_logical(int x, int y, int width, int height, uint16_t color)
{
    int xx, yy;
    for (yy = y; yy < y + height; ++yy)
        for (xx = x; xx < x + width; ++xx) put_logical(xx, yy, color);
}

static const uint8_t *glyph_rows(char character)
{
    static const uint8_t blank[7] = {0,0,0,0,0,0,0};
    static const uint8_t colon[7] = {0,4,4,0,4,4,0};
    static const uint8_t slash[7] = {1,2,4,8,16,0,0};
    static const uint8_t backslash[7] = {16,8,4,2,1,0,0};
    static const uint8_t period[7] = {0,0,0,0,0,6,6};
    static const uint8_t dash[7] = {0,0,0,31,0,0,0};
    static const uint8_t letters[36][7] = {
        {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},
        {14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
        {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
        {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},
        {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},
        {14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
        {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
        {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},
        {17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
        {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
        {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
        {2,6,10,18,31,2,2},{31,16,30,1,1,17,14},
        {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},
        {14,17,17,14,17,17,14},{14,17,17,15,1,2,12}
    };
    if (character >= 'a' && character <= 'z') character -= 'a' - 'A';
    if (character >= 'A' && character <= 'Z') return letters[character - 'A'];
    if (character >= '0' && character <= '9') return letters[26 + character - '0'];
    if (character == ':') return colon;
    if (character == '/') return slash;
    if (character == '\\') return backslash;
    if (character == '.') return period;
    if (character == '-') return dash;
    return blank;
}

static int text_width(const char *text, int scale)
{
    return (int)strlen(text) * 6 * scale;
}

static void draw_text(int x, int y, const char *text, uint16_t color, int scale)
{
    while (*text) {
        const uint8_t *rows = glyph_rows(*text++);
        int row, column, sx, sy;
        for (row = 0; row < 7; ++row) {
            for (column = 0; column < 5; ++column) {
                if (!(rows[row] & (1u << (4 - column)))) continue;
                for (sy = 0; sy < scale; ++sy)
                    for (sx = 0; sx < scale; ++sx)
                        put_logical(x + column * scale + sx,
                            y + row * scale + sy, color);
            }
        }
        x += 6 * scale;
    }
}

static uint16_t palette_rgb565(unsigned index)
{
    const unsigned char *color = g_palette + index * 3u;
    unsigned blue = color[0];
    unsigned green = color[1];
    unsigned red = color[2];
    return (uint16_t)(((red & 0xf8u) << 8) |
        ((green & 0xfcu) << 3) | (blue >> 3));
}

static void draw_game(void)
{
    int scale = (g_game_width == 160 && g_game_height == 80) ? 2 : 1;
    int draw_width = g_game_width * scale;
    int draw_height = g_game_height * scale;
    int offset_x = (LOGICAL_WIDTH - draw_width) / 2;
    int offset_y = (GAME_HEIGHT - draw_height) / 2;
    int x, y, sx, sy;
    if (!g_game_indices || !g_palette || g_game_width <= 0 || g_game_height <= 0)
        return;
    for (y = 0; y < g_game_height; ++y) {
        const unsigned char *line = g_game_indices + y * 256;
        for (x = 0; x < g_game_width; ++x) {
            uint16_t color = palette_rgb565(line[x]);
            for (sy = 0; sy < scale; ++sy)
                for (sx = 0; sx < scale; ++sx)
                    put_logical(offset_x + x * scale + sx,
                        offset_y + y * scale + sy, color);
        }
    }
}

static void draw_keyboard(void)
{
    unsigned row;
    for (row = 0; row < 4u; ++row) {
        const soft_row_t *keys = &g_rows[row];
        unsigned index;
        int top = GAME_HEIGHT + (int)row * 20;
        for (index = 0; index < keys->count; ++index) {
            int left = (int)(index * LOGICAL_WIDTH / keys->count);
            int right = (int)((index + 1u) * LOGICAL_WIDTH / keys->count);
            int pressed = g_keys[keys->codes[index]] != 0;
            uint16_t background = pressed ? 0x2e7fu : 0x18c6u;
            int width = right - left;
            int tx = left + (width - text_width(keys->labels[index], 1)) / 2;
            fill_logical(left + 1, top + 1, width - 2, 18, background);
            draw_text(tx, top + 6, keys->labels[index], 0xffffu, 1);
        }
    }
}

static void submit_pixels(void)
{
    void *old_object;
    if (g_direct) {
        if (bda_gui_framebuffer_present_rgb565(&g_framebuffer, g_pixels) == 0)
            return;
        g_direct = 0;
        memset(&g_framebuffer, 0, sizeof(g_framebuffer));
    }
    if (!ensure_firmware_renderer()) return;
    (void)bda_gui_draw_guard_begin();
    old_object = bda_gui_select_draw_object(g_draw, g_draw_object);
    (void)bda_gui_render_picture(
        g_draw, 0, 0, PHYSICAL_WIDTH, PHYSICAL_HEIGHT, &g_picture
    );
    (void)bda_gui_select_draw_object(g_draw, old_object);
    (void)bda_gui_draw_guard_end();
}

static void render_frame(void)
{
    fill_logical(0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT, 0x0843u);
    if (g_status_mode) {
        draw_text((LOGICAL_WIDTH - text_width("LAVAX FOR BBK 9588", 2)) / 2,
            42, "LAVAX FOR BBK 9588", 0xffffu, 2);
        draw_text((LOGICAL_WIDTH - text_width(g_status_line1, 1)) / 2,
            102, g_status_line1, 0xffc0u, 1);
        draw_text((LOGICAL_WIDTH - text_width(g_status_line2, 1)) / 2,
            124, g_status_line2, 0x07ffu, 1);
        draw_text(56, 205, "HOLD ENTER ESC TO EXIT", 0xffffu, 1);
        submit_pixels();
        return;
    }
    draw_game();
    draw_keyboard();
    submit_pixels();
}

static unsigned char soft_key_at(int logical_x, int logical_y)
{
    unsigned row, index;
    const soft_row_t *keys;
    if ((unsigned)logical_x >= LOGICAL_WIDTH || logical_y < GAME_HEIGHT ||
        logical_y >= LOGICAL_HEIGHT) return 0;
    row = (unsigned)(logical_y - GAME_HEIGHT) / 20u;
    keys = &g_rows[row];
    index = (unsigned)logical_x * keys->count / LOGICAL_WIDTH;
    if (index >= keys->count) index = keys->count - 1u;
    return keys->codes[index];
}

static void update_pen(int logical_x, int logical_y)
{
    int scale = (g_game_width == 160 && g_game_height == 80) ? 2 : 1;
    int offset_x = (LOGICAL_WIDTH - g_game_width * scale) / 2;
    int offset_y = (GAME_HEIGHT - g_game_height * scale) / 2;
    g_pen_x = (logical_x - offset_x) / scale;
    g_pen_y = (logical_y - offset_y) / scale;
    g_pen_down = logical_x >= offset_x && logical_y >= offset_y &&
        g_pen_x >= 0 && g_pen_y >= 0 && g_pen_x < g_game_width &&
        g_pen_y < g_game_height;
}

static void read_touch_position(int *logical_x, int *logical_y)
{
    u16 physical_x = 0, physical_y = 0;
    bda_gui_touch_position(&physical_x, &physical_y);
    if (physical_x >= PHYSICAL_WIDTH) physical_x = PHYSICAL_WIDTH - 1;
    if (physical_y >= PHYSICAL_HEIGHT) physical_y = PHYSICAL_HEIGHT - 1;
    *logical_x = LOGICAL_WIDTH - 1 - (int)physical_y;
    *logical_y = (int)physical_x;
}

static void poll_touch(void)
{
    unsigned count = 0;
    bda_gui_raw_event_t event;
    if (!g_direct) return;
    while (count++ < RAW_EVENTS_PER_TICK &&
           bda_gui_raw_event_fetch(&event) >= 0) {
        int x, y;
        switch ((u32)event.code) {
        case BDA_INPUT_EVENT_TOUCH_DOWN:
            read_touch_position(&x, &y);
            g_touch_down = 1;
            g_touch_key = soft_key_at(x, y);
            if (g_touch_key) g_keys[g_touch_key] = 1;
            else update_pen(x, y);
            break;
        case BDA_INPUT_EVENT_TOUCH_MOVE:
            if (g_touch_down && !g_touch_key) {
                read_touch_position(&x, &y);
                update_pen(x, y);
            }
            break;
        case BDA_INPUT_EVENT_TOUCH_UP:
            if (g_touch_key) g_keys[g_touch_key] = 0;
            g_touch_key = 0;
            g_touch_down = 0;
            g_pen_down = 0;
            break;
        default:
            break;
        }
    }
}

static void poll_physical_keys(void)
{
    bda_gui_input_packet_t packet;
    int touch_code = g_touch_key;
    memset(&packet, 0, sizeof(packet));
    (void)bda_gui_input_packet(&packet);
    memset(g_keys, 0, sizeof(g_keys));
    if (touch_code) g_keys[touch_code] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_LEFT)) g_keys[0x14] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_RIGHT)) g_keys[0x15] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_DOWN)) g_keys[0x17] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_UP)) g_keys[0x16] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_ENTER)) g_keys[0x0d] = 1;
    if (bda_gui_input_packet_key_pressed(&packet, BDA_KEY_ESCAPE)) g_keys[0x1b] = 1;
}

int bbk9588_platform_open(void)
{
    bda_frame_desc_t descriptor;
    if (g_frame) return 1;
    memset(&descriptor, 0, sizeof(descriptor));
    memset(&g_framebuffer, 0, sizeof(g_framebuffer));
    memset(&g_picture, 0, sizeof(g_picture));
    memset(g_pixels, 0, sizeof(g_pixels));
    memset(g_keys, 0, sizeof(g_keys));
    g_detached = g_close_requested = g_exit_combo = 0;
    g_touch_down = g_touch_key = g_pen_down = 0;
    g_status_mode = 0;
    g_draw = g_draw_owner = 0;
    g_draw_object = 0;
    descriptor.title = "LavaX";
    descriptor.wndproc = window_proc;
    descriptor.height = LOGICAL_HEIGHT;
    descriptor.width = LOGICAL_WIDTH;
    g_frame = bda_gui_register_frame_desc(&descriptor);
    if (!g_frame || (int32_t)g_frame == -1) {
        g_frame = 0;
        return 0;
    }
    (void)bda_gui_frame_activate(g_frame, 0x100u);
    if (bda_gui_framebuffer_acquire(&g_framebuffer) == 0) {
        g_direct = 1;
        release_draw_context();
    } else if (!ensure_firmware_renderer()) {
        bbk9588_platform_close();
        return 0;
    }
    g_picture.width = PHYSICAL_WIDTH;
    g_picture.height = PHYSICAL_HEIGHT;
    g_picture.source_pixels = g_pixels;
    g_picture.selected_index = -1;
    bda_gui_millisecond_timer_start();
    g_timer_started = 1;
    g_last_present = bda_gui_millisecond_count();
    g_vm_clock = g_last_present;
    g_vm_fraction = 0;
    render_frame();
    return 1;
}

void bbk9588_platform_close(void)
{
    bda_gui_message_t message;
    unsigned pumps = 0;
    if (g_timer_started) {
        bda_gui_millisecond_timer_stop();
        g_timer_started = 0;
    }
    g_direct = 0;
    memset(&g_framebuffer, 0, sizeof(g_framebuffer));
    if (g_frame) {
        memset(&message, 0, sizeof(message));
        (void)bda_gui_frame_stop(g_frame);
        (void)bda_gui_frame_release(g_frame);
        while (!g_detached && pumps++ < 128u &&
               bda_gui_event_pump_frame_once(&message, g_frame)) {
            bda_sys_delay(1u);
        }
        release_draw_context();
        bda_gui_close_frame(g_frame);
        g_frame = 0;
    }
    g_draw_object = 0;
}

void bbk9588_platform_tick(void)
{
    u32 now = g_timer_started ? bda_gui_millisecond_count() :
        bda_gui_tick_count_25ms() * 25u;
    poll_physical_keys();
    poll_touch();
    {
        u32 elapsed = now - g_vm_clock;
        if (elapsed > 250u) elapsed = 250u;
        g_vm_clock = now;
        g_vm_fraction += elapsed * 256u;
        while (g_vm_fraction >= 1000u) {
            g_vm_fraction -= 1000u;
            ++TickCount;
            ++Hz128;
            if ((Hz128 & 7u) == 0u) bbk9588_hardware_sample();
        }
    }
    if (g_keys[0x0d] && g_keys[0x1b]) {
        if (!g_exit_combo) {
            g_exit_combo = 1;
            g_exit_started = now;
        } else if ((u32)(now - g_exit_started) >= EXIT_HOLD_MS) {
            g_close_requested = 1;
        }
    } else {
        g_exit_combo = 0;
    }
    if ((u32)(now - g_last_present) >= 33u) {
        g_last_present = now;
        render_frame();
    }
}

void bbk9588_platform_idle(void) { bda_sys_delay(1u); }

int bbk9588_platform_should_exit(void)
{
    return g_detached || g_close_requested;
}

int bbk9588_platform_key_state(unsigned code)
{
    return code < sizeof(g_keys) && g_keys[code];
}

void bbk9588_platform_touch(int *pushed, int *x, int *y)
{
    if (pushed) *pushed = g_pen_down;
    if (x) *x = g_pen_x;
    if (y) *y = g_pen_y;
}

void bbk9588_platform_present(
    const unsigned char *indices, int width, int height,
    const unsigned char *palette_bgr
)
{
    g_status_mode = 0;
    g_game_indices = indices;
    g_game_width = width;
    g_game_height = height;
    g_palette = palette_bgr;
}

void bbk9588_platform_status(const char *line1, const char *line2)
{
    (void)strlcpy(g_status_line1, line1 ? line1 : "", sizeof(g_status_line1));
    (void)strlcpy(g_status_line2, line2 ? line2 : "", sizeof(g_status_line2));
    g_status_mode = 1;
    render_frame();
}
