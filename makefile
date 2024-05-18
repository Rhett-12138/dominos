
BUILD:=build
SRC:=src

ENTRYPOINT:=0x10000
# CPPFLAGS
CPPFLAGS:= -m32
CPPFLAGS+= -fno-use-cxa-atexit
CPPFLAGS+= -nostdlib
CPPFLAGS+= -fno-builtin
CPPFLAGS+= -fno-rtti
CPPFLAGS+= -fno-exceptions
CPPFLAGS+= -fno-leading-underscore 
CPPFLAGS+= -Wno-write-strings

# INCLUDE
INCLUDE:= -Iinclude
INCLUDE+= -Iinclude/kernel
INCLUDE+= -Iinclude/lib
INCLUDE+= -Iinclude/devices

DEBUG:= -g

$(BUILD)/%.bin: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $< -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	$(shell mkdir -p $(dir $@))
	g++ $(CPPFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/interrupts.o \
	$(BUILD)/kernel/inthandlers.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/task_queue.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/gate.o \
	$(BUILD)/kernel/thread.o \
	$(BUILD)/kernel/mutex.o \
	$(BUILD)/devices/clock.o \
	$(BUILD)/devices/keyboard.o \
	$(BUILD)/lib/stdio.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/lib/assert.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/charBuf.o \
	$(BUILD)/lib/bitmap.o \
	$(BUILD)/lib/syscall.o \
	$(BUILD)/lib/list.o \
	$(BUILD)/lib/fifo.o \
	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
 	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map	
# 创建一个 16M 的硬盘镜像
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
# 将boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
# 将loader.bin 写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 测试 system.bin 小于100k，否则需要修改下面的count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"
# 将 system.bin 写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

install: $(BUILD)/master.img

.PHONY: clean
clean:
	rm -rf $(BUILD)/*