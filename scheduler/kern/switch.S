/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:

	# restaurar el estado del environment a correr

    	mov 4(%esp), %esp    # cargo el %esp con el Trapframe.

    	popal            # restauro registros de propósito general
    	popl %es
    	popl %ds         # sigo restaurando registros según su orden
    	addl $4, %esp    # skip trapno
    	addl $4, %esp    # skip err


    	# realizar el cambio de contexto a modo usuario

    	iret             # modifico conjuntamente los registros cs, eip y esp de forma atómica, tomando valores desde el stack.

	# Should never get here, but in case we do, just spin.

spin:
	jmp spin
