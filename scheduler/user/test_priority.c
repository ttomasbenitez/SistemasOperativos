#include <inc/lib.h>


void umain(int argc, char **argv) {
    envid_t father_envid = thisenv->env_id;
    cprintf("Soy el proceso padre %08x\n", father_envid);
    int priority = sys_env_get_priority();
    cprintf("Mi prioridad es %d\n", priority);


    envid_t pid = fork();
    if (pid < 0) {
        cprintf("Fork failed\n");
    } else if (pid == 0) {
        // Child process

        envid_t child_envid = thisenv->env_id;
        cprintf("Soy el proceso hijo %08x\n", child_envid);
        priority = sys_env_get_priority();

        sys_env_set_priority(child_envid, priority + 1);
        priority = sys_env_get_priority();
        cprintf("Bajé mi prioridad a %d\n", priority);

        sys_yield();

        cprintf("\n\nESTO DEBERÍA MOSTRARSE SEGUNDO\n\n");

    } else {
        sys_yield();

        cprintf("Soy el proceso padre.\n");
        cprintf("\n\nESTO DEBERÍA MOSTRARSE PRIMERO\n\n");

        priority = sys_env_get_priority();
        sys_env_set_priority(father_envid, priority + 1);
        priority = sys_env_get_priority();
        cprintf("Bajé mi prioridad, ahora es %d\n", priority);
        sys_yield();
    }
}

