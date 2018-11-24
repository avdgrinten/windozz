# Makefile
# Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
# Use of this source code is governed by a license that can be
# found in the LICENSE.md file, in the root directory of
# the source package.

bootmgr.bin:
	@echo "\033[0;1;35m[ AS ]\e[0m stage1/mbr.asm"
	@nasm -f bin stage1/mbr.asm -o stage1/mbr.bin
	@echo "\033[0;1;35m[ AS ]\e[0m stage1/echfs.asm"
	@nasm -f bin stage1/echfs.asm -o stage1/echfs.bin
	@echo "\033[0;1;35m[ AS ]\e[0m stage2/bootmgr.asm"
	@nasm -f bin stage2/bootmgr.asm -o bootmgr.bin

clean:
	@rm -f stage1/*.bin *.bin