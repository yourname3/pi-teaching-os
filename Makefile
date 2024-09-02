include .config

CFLAGS = -g -ffreestanding -nostdlib -nostdinc -nostartfiles -Wall

# TODO: Machine-dependent makefile setup
SRCS = \
	kern/arch/aarch64/boot.S \
	kern/main/main.c \
	drivers/rpi4b/devices.c \
	drivers/rpi4b/rpi4os_uart.c 

OBJS = $(SRCS:%=compile/%.o)

DIRS = $(sort $(dir $(SRCS)))
DIRS := $(DIRS:%=compile/%)

LINK = kern/arch/aarch64/link.ld

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary $< $@

kernel8.elf: $(OBJS)
	$(CC) $^ -o $@ -ffreestanding -nostdlib -T $(LINK)

compile/%.c.o: %.c | $(DIRS)
	$(CC) -c $< -o $@ $(CFLAGS)

compile/%.S.o: %.S | $(DIRS)
	$(CC) -c $< -o $@ -ffreestanding

$(DIRS):
	mkdir -p $@

clean:
#	find . -name "*.o" -exec rm {} \;
	rm -rf compile
	rm -f kernel8.img
	rm -f kernel8.elf

qemu: kernel8.img
	$(QEMU) -machine raspi4b -kernel kernel8.img -serial null -serial stdio

.PHONY: clean qemu