        .globl fai
        .ent fai 2
fai:
        .option O1
L1:
        ll	$8, 0($4)
        addiu	$9, $8, 1
	sc	$9, 0($4)
	beqz	$9, L1
	or	$2, $8, $0
	j	$31
        .end fai
