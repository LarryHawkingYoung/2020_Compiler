.data
str:.space 24

.text
li $v0,5
syscall
move $s0,$v0	#s0=n

li $t0,0
li $t1,1
input_loop:
li $v0,12
syscall
sb $v0,str($t1)
addi $t1,$t1,1
addi $t0,$t0,1
beq $t0,$s0,end_input_loop
j input_loop
end_input_loop:

li $a0,0
li $t0,0
li $t1,2
div $s0,$t1
mflo $s1	#s1=n/2
loop:
beq $t0,$s1,end_loop
addi $t3,$t0,1
sub $t4,$s0,$t0
lb $t5,str($t3)
lb $t6,str($t4)
beq $t5,$t6,sum
end_sum:
addi $t0,$t0,1
j loop
end_loop:

beq $a0,$s1,end1
j end0

sum:
addi $a0,$a0,1
j end_sum

end1:
li $v0,1
li $a0,1
syscall
li $v0,10
syscall

end0:
li $v0,1
li $a0,0
syscall
li $v0,10
syscall
