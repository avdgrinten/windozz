/*
* cpu.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#pragma once

#include <stdint.h>

void write_cr0(uint64_t);
void write_cr3(uint64_t);
void write_cr4(uint64_t);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
uint64_t read_cr4();

void write_msr(uint32_t, uint64_t);
uint64_t read_msr(uint32_t);