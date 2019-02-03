
all:
	@make -C echfs-utils
	@make -C bootmgr
	@make -C kernel

	@dd status=none if=/dev/zero bs=512 conv=notrunc count=20480 of=part.hdd
	@./echfs-utils/echfs-utils part.hdd format 2048

	@./echfs-utils/echfs-utils part.hdd import kernel/winkern winkern

	@dd status=none if=bootmgr/stage1/echfs.bin bs=1 count=4 conv=notrunc of=part.hdd
	@dd status=none if=bootmgr/stage1/echfs.bin bs=1 count=480 conv=notrunc skip=36 seek=36 of=part.hdd
	@dd status=none if=bootmgr/bootmgr.bin bs=512 conv=notrunc seek=4 of=part.hdd

	@dd status=none if=/dev/zero bs=512 conv=notrunc count=20608 of=disk.hdd
	@dd status=none if=part.hdd bs=512 conv=notrunc count=20480 seek=63 of=disk.hdd
	@dd status=none if=bootmgr/stage1/mbr.bin bs=512 count=1 conv=notrunc of=disk.hdd
	@rm -f part.hdd

run:
	@qemu-system-x86_64 -drive file=disk.hdd,format=raw -debugcon stdio -smp 8

clean:
	@rm -f disk.hdd
	@make clean -C echfs-utils
	@make clean -C bootmgr
	@make clean -C kernel
