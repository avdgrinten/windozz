
bootmgr.bin:
	@nasm -f bin stage1/mbr.asm -o stage1/mbr.bin
	@nasm -f bin stage1/echfs.asm -o stage1/echfs.bin
	@nasm -f bin stage2/bootmgr.asm -o bootmgr.bin

clean:
	@rm -f stage1/*.bin bootmgr.bin


