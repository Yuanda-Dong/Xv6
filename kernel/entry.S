        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0 # load address of stack0 to sp
        li a0, 1024*4 # load immediate a0 = 4096
        csrr a1, mhartid // read csr (Control and Status Reg), the hart id, and stores in a1;
        addi a1, a1, 1 // a1 = a1 + 1
        mul a0, a0, a1 // a0 = a0 * a1 = 4096 * (hart_id + 1)
        add sp, sp, a0 // sp = sp + a0 = stack0 + a0
        // sp points to the top of the stack of the current hart
        // the stack size is 4096 
        # jump to start() in start.c
        call start
spin:
        j spin
