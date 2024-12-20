include .config

CFLAGS = -g -ffreestanding -nostdlib -nostdinc -nostartfiles -Wall -std=gnu11 -I.

# TODO: Machine-dependent makefile setup
SRCS = \
	kern/arch/aarch64/boot.S \
	kern/arch/aarch64/switch.S \
	kern/arch/aarch64/pcb.c \
	kern/mem/mem.c \
	kern/task/task.c \
	kern/main/main.c \
	kern/console/console.c \
	kern/console/menu.c \
	drivers/rpi4b/devices.c \
	drivers/rpi4b/rpi4os_uart.c \
	drivers/generic/con_vt100.c

OBJS = $(SRCS:%=compile/%.o)

DIRS = $(sort $(dir $(SRCS)))
DIRS := $(DIRS:%=compile/%)

LINK = kern/arch/aarch64/link.ld

kernel8.img: kernel8.elf
	@$(OBJCOPY) -O binary $< $@
	@echo "OBJCOPY $<"

kernel8.elf: $(OBJS)
	@$(CC) $^ -o $@ -ffreestanding -nostdlib -T $(LINK)
	@echo "LD      $<"

compile/%.c.o: %.c | $(DIRS)
	@$(CC) -c $< -o $@ $(CFLAGS)
	@echo "CC      $<"

compile/%.S.o: %.S | $(DIRS)
	@$(CC) -c $< -o $@ -ffreestanding
	@echo "AS      $<"

$(DIRS):
	@mkdir -p $@
	@echo "MKDIR   $@"

clean:
#	find . -name "*.o" -exec rm {} \;
	rm -rf compile
	rm -f kernel8.img
	rm -f kernel8.elf

qemu: kernel8.img
	$(QEMU) -machine raspi4b -kernel kernel8.img -serial null -serial stdio

qemu-debug: kernel8.img
	$(QEMU) -s -S -machine raspi4b -kernel kernel8.img -serial null -serial stdio

gdb: kernel8.img
	$(GDB) kernel8.elf

.PHONY: clean qemu qemu-debug