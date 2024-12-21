include .config

##############################################################################
#                               Setup CFLAGS.
##############################################################################

# Generate .d files for the build system
CFLAGS=-MMD

# Debugging flags
CFLAGS+=-g

# Freestanding related flags
CFLAGS+=-ffreestanding -nostdlib -nostdinc -nostartfiles

# Warning flags
CFLAGS+=-Wall

# Use C11 and GNU extensions
CFLAGS+=-std=gnu11

# Include path setup
CFLAGS+=-I. -Icompile/symlinks -Ilib/libc

# VERY IMPORTANT: Disable floating point registers for GCC. The code may trap
# if we try to use floating point registers when they haven't been enabled.
# The kernel should not use any floats.
CFLAGS+=-mgeneral-regs-only

# TODO: Machine-dependent makefile setup
SRCS = \
	kern/arch/aarch64/boot.S \
	kern/arch/aarch64/switch.S \
	kern/arch/aarch64/pcb.c \
	kern/arch/aarch64/interrupt.S \
	kern/arch/aarch64/trap.c \
	kern/mem/mem.c \
	kern/task/task.c \
	kern/main/main.c \
	kern/lib/printk.c \
	kern/console/console.c \
	kern/console/menu.c \
	kern/devices/devices.c \
	drivers/rpi4b/devices.c \
	drivers/rpi4b/rpi4os_uart.c \
	drivers/rpi4b/watchdog.c \
	drivers/generic/con_vt100.c \
	drivers/aarch64/el1_physical_timer.S \
	drivers/aarch64/el1_physical_timer.c \
	drivers/rpi4b/bcm2711_irq.c \
	lib/libc/string.c \
	lib/libc/printf.c 

OBJS = $(SRCS:%=compile/%.ko)

DIRS = $(sort $(dir $(SRCS)))
DIRS := $(DIRS:%=compile/%)

LINK = kern/arch/aarch64/link.ld

kernel8.img: kernel8.elf
	@$(OBJCOPY) -O binary $< $@
	@echo "OBJCOPY $@"

compile/symlinks/arch compile/symlinks/arch.lock: .config
	@mkdir -p compile/symlinks
	@touch compile/symlinks/arch.lock
	@ln -s $(abspath kern/arch/$(ARCH)) compile/symlinks/arch
	@echo "GEN     $@"

compile/symlinks/device compile/symlinks/device.lock: .config
	@mkdir -p compile/symlinks
	@touch compile/symlinks/device.lock
	@ln -s $(abspath drivers/$(DEVICE)) compile/symlinks/device
	@echo "GEN     $@"

kernel8.elf: $(LINK) $(OBJS)
	@$(CC) $(OBJS) -o $@ -ffreestanding -nostdlib -T $(LINK)
	@echo "LD      $@"

compile/%.c.ko: %.c | $(DIRS) compile/symlinks/arch compile/symlinks/device
	@$(CC) -c $< -o $@ $(CFLAGS)
	@echo "CC      $<"

compile/%.S.ko: %.S | $(DIRS) compile/symlinks/arch compile/symlinks/device
	@$(CC) -c $< -o $@ -g -ffreestanding
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
	$(QEMU) -machine raspi4b -kernel kernel8.elf -serial null -serial stdio

qemu-debug: kernel8.img
	$(QEMU) -s -S -machine raspi4b -kernel kernel8.elf -serial null -serial stdio

gdb: kernel8.img
	$(GDB) kernel8.elf

.PHONY: clean qemu qemu-debug

-include $(OBJS:.ko=.d)