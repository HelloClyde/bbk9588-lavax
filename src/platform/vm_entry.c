#include "type.h"
#include "base.h"
#include "bbk9588_platform.h"

#include <stdio.h>
#include <stdlib.h>

extern const unsigned char lav_font[];
extern byte *lav_fonts;
extern byte *ram_base;
extern void main_loop(void);

byte *lav_fonts;
static unsigned long local_ram[256 * 192 / 4] __attribute__((aligned(4)));
static unsigned long lavax_ram[MY_DATA_SIZE / 4] __attribute__((aligned(4)));

static byte *load_lav(FILE *file)
{
    long length;
    byte *program;
    (void)fseek(file, 0, SEEK_END);
    length = ftell(file);
    (void)fseek(file, 0, SEEK_SET);
    if (length <= 0 || length > MY_DATA_SIZE) {
        (void)fclose(file);
        return 0;
    }
    program = (byte *)malloc((size_t)length + 1u);
    if (!program) {
        (void)fclose(file);
        return 0;
    }
    length = (long)fread(program, 1u, (size_t)length, file);
    program[length] = 0;
    (void)fclose(file);
    return program;
}

byte *TaskOpen(char *filename)
{
    byte header[16];
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;
    if (fread(header, 1u, sizeof(header), file) != sizeof(header) ||
        header[0] != 'L' || header[1] != 'A' || header[2] != 'V' ||
        header[3] != 18) {
        (void)fclose(file);
        return 0;
    }
    return load_lav(file);
}

a32 shell_init(void)
{
    filesys_init();
    lRam = VRam;
    task_lev = 0;
    return (a32)TaskOpen("fat:/LavaXOS/System/Shell.sys");
}

static void wait_for_exit(void)
{
    while (!bbk9588_platform_should_exit()) {
        bbk9588_platform_tick();
        bbk9588_platform_idle();
    }
}

__attribute__((section(".text.bda_main")))
int bda_main(void)
{
    byte *probe;
    if (!bbk9588_platform_open()) return 1;
    bbk9588_platform_status("STARTING VM", lavax9588_runtime_root());

    lav_fonts = (byte *)lav_font;
    TickCount = Hz128 = 0;
    ram_base = (byte *)local_ram;
    VRam = (byte *)lavax_ram;
    InitNds();
    Color256Init();

    if (!lavax9588_runtime_available()) {
        bbk9588_platform_status(
            "RUNTIME NOT FOUND", "COPY LAVAXOS TO A: OR B:"
        );
        wait_for_exit();
        bbk9588_platform_close();
        return 2;
    }
    probe = TaskOpen("fat:/LavaXOS/System/Shell.sys");
    if (!probe) {
        bbk9588_platform_status("INVALID SHELL.SYS", lavax9588_runtime_root());
        wait_for_exit();
        bbk9588_platform_close();
        return 3;
    }
    free(probe);
    ReadConfig("fat:/LavaXOS/System/Config.ini");
    main_loop();
    bbk9588_platform_close();
    return 0;
}
