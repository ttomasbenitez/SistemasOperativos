#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

static uint32_t sched_calls = 0;

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	sched_calls++;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	int i, j;

	if (curenv)
		i = (ENVX(curenv->env_id)) + 1;
	else
		i = 0;

	for (j = 0; j < NENV; j++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			env_run(&envs[i]);
			break;
		}

		i++;

		if (i == NENV)
			i -= NENV;
	}

#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities

	int i, j, max_prty;

	if (curenv) {
		i = (ENVX(curenv->env_id)) + 1;
		max_prty = curenv->env_priority + 1;
	} else {
		i = 0;
		max_prty = LOPRTY;
	}

	struct Env *e_next = NULL;


	for (j = 0; j < NENV; j++) {
		if (envs[i].env_status == ENV_RUNNABLE && envs[i].env_priority < max_prty) {
			e_next = &envs[i];
			max_prty = e_next->env_priority;
		}

		i++;

		if (i == NENV)
			i -= NENV;
	}

	if(e_next)
		env_run(e_next);

#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here
		int j, n = 0;

		cprintf("Estadisticas:\n");
		cprintf("    Total de llamadas al scheduler:    %d\n", sched_calls);
		cprintf("    Environments:\n");
		for (j = 0; j < NENV; j++) {
			if (envs[j].env_runs > 0) {
				n++;
				cprintf ("        [%08x]\n", envs[j].env_id);
				cprintf ("            Ejecuciones:               %d\n", envs[j].env_runs);
	// 			cprintf ("            Timer al inicio:           %d\n", envs[j].env_start_time);
	// 			cprintf ("            Timer al finalizar:        %d\n", envs[j].env_end_time);
	// 			cprintf ("            Tiempo total:              %d\n", envs[j].env_end_time - envs[j].env_start_time);
			}
		}
		cprintf("    Total de environments ejecutados:  %d\n", n);

		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();


// 	}

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
