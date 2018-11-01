# Makefile
# Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
# Use of this source code is governed by a license that can be
# found in the LICENSE.md file, in the root directory of
# the source package.

bootmgr.bin:
	@nasm -f bin stage1/mbr.asm -o stage1/mbr.bin
	@nasm -f bin stage1/echfs.asm -o stage1/echfs.bin
	@nasm -f bin stage2/bootmgr.asm -o bootmgr.bin

clean:
	@rm -f stage1/*.bin *.bin


