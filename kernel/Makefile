
CC = clang -target x86_64-elf-none
CFLAGS = -Wall -m64 -mcmodel=large -mno-red-zone -O2 -ffreestanding -fomit-frame-pointer -nodefaultlibs -fno-builtin -nostdlib -nostdinc -I./src/include -I./src/acpi/lai/src -I./src/acpi/lai/src/windozz -mno-sse -mno-sse2 -DCOM1_DEBUG
LD = ld
LFLAGS = -O2 -nostdlib -nostartfiles -nodefaultlibs

SRC := $(shell find ./src -type f -name '*.c')
OBJ := $(SRC:.c=.o)

ASRC := $(shell find ./src -type f -name '*.asm')
AOBJ := $(ASRC:.asm=.o)

A16SRC := $(shell find ./src -type f -name '*.a16')
A16OBJ := $(A16SRC:.a16=.bin)

winkern: $(A16OBJ) $(AOBJ) $(OBJ)
	@echo "\e[0m\e[1m\e[34m[ LD ]\e[0m winkern"
	@$(LD) $(LFLAGS) -T kernel.ld -o winkern $(OBJ) $(AOBJ)

%.o: %.c
	@echo "\e[0m\e[1m\e[34m[ CC ]\e[0m $@"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo "\e[0m\e[1m\e[34m[ AS ]\e[0m $@"
	@nasm -f elf64 $< -o $@

%.bin: %.a16
	@echo "\e[0m\e[1m\e[34m[ AS ]\e[0m $@"
	@nasm -f bin $< -o $@

clean:
	@rm -f $(A16OBJ) $(OBJ) $(AOBJ) winkern
