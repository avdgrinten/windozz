
bootmgr.bin:
	@echo "\033[0;1;35m[ AS ]\e[0m stage1/mbr.asm"
	@nasm -f bin stage1/mbr.asm -o stage1/mbr.bin
	@echo "\033[0;1;35m[ AS ]\e[0m stage1/echfs.asm"
	@nasm -f bin stage1/echfs.asm -o stage1/echfs.bin
	@echo "\033[0;1;35m[ AS ]\e[0m stage2/bootmgr.asm"
	@nasm -f bin stage2/bootmgr.asm -o bootmgr.bin

clean:
	@rm -f stage1/*.bin *.bin
