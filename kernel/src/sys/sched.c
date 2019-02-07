
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE          "sys"

#include <sys.h>
#include <debug.h>
#include <cpu.h>
#include <apic.h>
#include <mutex.h>
#include <mm.h>
#include <stddef.h>
#include <string.h>

process_t *processes;
mutex_t sched_mutex = MUTEX_FREE;
int sys_ready = 0;
size_t running_processes = 0;
int last_process = 0;

int get_pid()
{
    return get_cpu()->pid;
}

int get_tid()
{
    return get_cpu()->tid;
}

void sched_init()
{
    processes = kcalloc(sizeof(process_t), MAX_PROCESSES);
    if(!processes)
    {
        ERROR("unable to allocate memory.\n");
        while(1);
    }

    create_process((uintptr_t)&kmain_late);

    sys_ready = 1;
}

static int find_process()
{
    for(int i = 0; i < MAX_PROCESSES; i++)
        if(!processes[i].valid) return i;

    return -1;
}

int create_process(uintptr_t entry)
{
    acquire(&sched_mutex);

    int pid = find_process();
    if(pid == -1)
    {
        ERROR("unable to create process; no free slots available.\n");
        release(&sched_mutex);
        return -1;
    }

    processes[pid].cr3 = pmm_alloc_page();
    if(!processes[pid].cr3)
    {
        ERROR("unable to create process; out of memory.\n");
        release(&sched_mutex);
        return -1;
    }

    uintptr_t *cr3 = (uintptr_t *)((uintptr_t)processes[pid].cr3 + PHYSICAL_MEMORY);
    cr3[256] = bsp_pml4[256];
    cr3[257] = bsp_pml4[257];
    cr3[258] = bsp_pml4[258];

    /* TODO: more CR3 setup here for userland processes */

    processes[pid].thread_count = 1;
    processes[pid].threads = kcalloc(sizeof(thread_t *), MAX_THREADS);
    if(!processes[pid].threads)
    {
        ERROR("unable to create process; out of memory.\n");
        pmm_free_page(processes[pid].cr3);
        release(&sched_mutex);
        return -1;
    }

    processes[pid].threads[0] = kcalloc(sizeof(thread_t), 1);
    if(!processes[pid].threads[0])
    {
        ERROR("unable to create process; out of memory.\n");
        pmm_free_page(processes[pid].cr3);
        kfree(processes[pid].threads);
        release(&sched_mutex);
        return -1;
    }

    thread_t *thread = processes[pid].threads[0];
    thread->rflags = 0x202;         /* interrupts */
    thread->rip = (uint64_t)entry;  /* entry point */

    if(!running_processes)
    {
        thread->rsp = (uint64_t)kmalloc(THREAD_STACK);
        if(!thread->rsp)
        {
            ERROR("unable to create process; out of memory.\n");
            pmm_free_page(processes[pid].cr3);
            kfree(thread);
            kfree(processes[pid].threads);
            release(&sched_mutex);
            return -1;
        }

        thread->rsp += THREAD_STACK;
        thread->cs = KCODE_SEGMENT;
        thread->ds = KDATA_SEGMENT;
    }

    thread->status = THREAD_IDLE;
    processes[pid].valid = 1;
    processes[pid].thread_count = 1;

    running_processes++;
    DEBUG("created process %d at entry point 0x%016lX\n", pid, entry);

    release(&sched_mutex);
    return pid;
}

static thread_t *find_idle(cpu_t *cpu)
{
    int pid = cpus[0].pid;
    int tid = cpus[0].tid;
    int tries = 0;

start:
    for(int i = pid; i < MAX_PROCESSES; i++)
    {
        if(processes[i].valid)
        {
            for(int j = tid; j < MAX_THREADS; j++)
            {
                if(processes[i].threads[j] &&
                    processes[i].threads[j]->status == THREAD_IDLE)
                {
                    cpu->pid = i;
                    cpu->tid = j;
                    write_cr3(processes[i].cr3);
                    return processes[i].threads[j];
                }
            }
        }
    }

    tries++;
    pid = 0;
    tid = 0;

    if(tries < 2)
        goto start;

    return NULL;
}

void resched(thread_t *old)
{
    /* save BSP state */
    if(cpus[0].sys_ready)
    {
        if(running_processes == 1 && processes[0].thread_count == 1)
            return;

        /* save the state */
        int pid = cpus[0].pid;
        int tid = cpus[0].tid;

        memcpy(processes[pid].threads[tid], old, 160);  /* not the entire fucking thing */
        processes[pid].threads[tid]->status = THREAD_IDLE;

        cpus[0].tid++;
    }

    cpus[0].sys_ready = 1;

    /* load new context */
    thread_t *new = find_idle(&cpus[0]);
    if(new)
    {
        new->status = THREAD_BUSY;
        lapic_eoi();
        load_context(new);
    }
}

static int find_thread(process_t *process)
{
    for(int i = 0; i < MAX_THREADS; i++)
        if(!processes->threads[i] || !processes->threads[i]->status) return i;

    return -1;
}

int create_thread(uintptr_t entry)
{
    acquire(&sched_mutex);

    int pid = get_pid();
    int tid = find_thread(&processes[pid]);

    if(tid == -1)
    {
        ERROR("unable to create thread, no free slots available.\n");
        release(&sched_mutex);
        return -1;
    }

    processes[pid].threads[tid] = kcalloc(sizeof(thread_t), 1);
    if(!processes[pid].threads[tid])
    {
        ERROR("unable to create thread, no free slots available.\n");
        release(&sched_mutex);
        return -1;
    }

    thread_t *thread = processes[pid].threads[tid];

    if(pid == 0)
    {
        thread->cs = KCODE_SEGMENT;
        thread->ds = KDATA_SEGMENT;
        thread->rsp = (uint64_t)kmalloc(THREAD_STACK);
        if(!thread->rsp)
        {
            ERROR("unable to create thread, out of memory.\n");
            kfree(thread);
            release(&sched_mutex);
            return -1;
        }

    } else
    {
        thread->cs = UCODE_SEGMENT;
        thread->ds = UDATA_SEGMENT;
    }

    thread->rsp += THREAD_STACK;
    thread->rflags = 0x202;
    thread->rip = (uint64_t)entry;

    thread->status = THREAD_IDLE;
    processes[pid].thread_count++;

    release(&sched_mutex);
    return 0;
}
