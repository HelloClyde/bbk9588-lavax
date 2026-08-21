#ifndef J2ME9588_SETJMP_H
#define J2ME9588_SETJMP_H

typedef struct j2me9588_jmp_state
{
	unsigned long words[16];
} jmp_buf[1];

#ifdef __cplusplus
extern "C" {
#endif

int setjmp(jmp_buf env) __attribute__((returns_twice));
void longjmp(jmp_buf env, int value) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
