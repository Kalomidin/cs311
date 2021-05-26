	.data
	.text
main:
	addu	$2, $4, $5 // 0x00
	addu	$2, $6, $7 // 0x04
	subu	$9, $3, $2 // 0x08
lab1:
	and	$11, $11, $0 // 0x0c
	addiu	$10, $10, 0x1 // 0x10
	or	$6, $6, $0 // 0x14
	jal	lab3 // 0x18
	addu	$0, $0, $0 // 0x1c
lab3:
	sll	$7, $6, 2 // 0x20
	srl	$5, $4, 2 // 0x24
	sltiu	$9, $10, 100 // 0x28
	beq	$9, $0, lab4 // 0x2c
	jr	$31 // 0x30
lab4:
	sltu	$4, $2, $3 // 0x34
	bne	$4, $0, lab5 // 0x38
	j	lab1 // 0x3c
lab5:
	ori	$16, $16, 0xf0f0 // 0x40
