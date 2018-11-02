/*
* mm.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#include <mm.h>

void mm_init()
{
	pmm_init();
	vmm_init();
}