
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

#define MAX_PROCESSES           512
#define MAX_THREADS             32

#define THREAD_STACK            131072

#define THREAD_NONE             0
#define THREAD_IDLE             1
#define THREAD_BUSY             2
#define THREAD_WAIT             3

typedef struct thread_t
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t rip;
    uint64_t rsp;
    uint64_t rflags;
    uint64_t cs;
    uint64_t ds;

    int status;
}__attribute__((packed)) thread_t;

typedef struct process_t
{
    int valid;
    thread_t **threads;
    size_t thread_count;

    uint64_t cr3;
} process_t;

int sys_ready;
void kmain_late();
void sched_init();
int create_process(uintptr_t);
void resched(thread_t *);
void load_context(thread_t *);
int get_pid();
int get_tid();
int create_thread(uintptr_t);

