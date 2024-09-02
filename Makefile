include .config

# TODO: Machine-dependent makefile setup
SRCS = \
	kern/arch/aarch64/boot.S \
	kern/main/main.c

OBJS = $(SRCS:%=%.o)

TARGET = kernel

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ -ffreestanding -nostdlib

%.c.o: %.c
	$(CC) -c $< -o $@ -ffreestanding -nostdlib -nostdinc -g

%.S.o: %.S
	$(CC) -c $< -o $@ -ffreestanding

clean:
	find . -name "*.o" -exec rm {} \;

.PHONY: clean