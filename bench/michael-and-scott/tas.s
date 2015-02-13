        .globl tas
        .ent tas 2
tas:
        .option O1
L1:
        ll	$8, 0($4)
        li	$9, 1
	sc	$9, 0($4)
	beqz	$9, L1
	or	$2, $8, $0
	j	$31
        .end tas
