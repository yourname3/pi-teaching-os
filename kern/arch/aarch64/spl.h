#ifndef AARCH64_SPL_H
#define AARCH64_SPL_H

int splhigh();
int splx(int spl);
int spl0();

/* TODO: Make this per-cpu */
extern int curspl;

#endif