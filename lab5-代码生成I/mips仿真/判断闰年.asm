.text
li $v0,5
syscall
move $s0,$v0

li $t0,400
div $s0,$t0
mfhi $t0
bne $t0,0,end1
li $a0,1
end1:

li $t0,0
li $t1,4
div $s0,$t1
mfhi $t1
bne $t1,0,end2
addi $t0,$t0,1
end2:

li $t1,100
div $s0,$t1
mfhi $t1
beq $t1,0,end3
addi $t0,$t0,1
end3:
beq $t0,2,judge
end_judge:
li $v0,1
syscall
li $v0,10
syscall

judge:
li $a0,1
j end_judge
