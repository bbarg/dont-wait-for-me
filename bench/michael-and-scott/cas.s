        .globl cas
        .ent cas 2
cas:
        .option O1
L1:
        ll	$8, 0($4)
        bne	$8, $5, L2
	or	$9, $6, $0
	sc	$9, 0($4)
	beqz	$9, L1
	or	$2, $9, $0
	j	$31
L2:
	or	$2, $0, $0
	j	$31
        .end cas
