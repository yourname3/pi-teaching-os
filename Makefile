include .config

CFLAGS = -g -ffreestanding -nostdlib -nostdinc -nostartfiles -Wall

# TODO: Machine-dependent makefile setup
SRCS = \
	kern/arch/aarch64/boot.S \
	kern/main/main.c

OBJS = $(SRCS:%=%.o)

LINK = kern/arch/aarch64/link.ld

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary $< $@

kernel8.elf: $(OBJS)
	$(CC) $^ -o $@ -ffreestanding -nostdlib -T $(LINK)

%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.S.o: %.S
	$(CC) -c $< -o $@ -ffreestanding

clean:
	find . -name "*.o" -exec rm {} \;
	rm -f kernel8.img
	rm -f kernel8.elf

qemu: kernel8.img
	$(QEMU) -machine raspi4b -kernel kernel8.img

.PHONY: clean qemu