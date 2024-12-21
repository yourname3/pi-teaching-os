.global read_cntfrq
read_cntfrq:
    mrs x0, cntfrq_el0
    ret

/* TODO: Consider porting these to inline assembly. */

.global setup_timer_tval
setup_timer_tval:
    msr cntp_tval_el0, x0
    ret

.global setup_timer_irq
setup_timer_irq:
    /* turn on ENABLE, turn off IMASK */
    mov x0, #((1 << 0) | (0 << 1))
    mrs x0, cntp_ctl_el0
    ret