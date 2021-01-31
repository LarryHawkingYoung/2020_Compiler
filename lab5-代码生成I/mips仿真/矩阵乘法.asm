.data
x1:.space 256
x2:.space 256
x:.space 256
space:.asciiz" "
enter:.asciiz"\n"

.macro index(%ans,%i,%j,%n)
subi %ans,%i,1
mult %ans,%n
mflo %ans
add %ans,%ans,%j
sll %ans,%ans,2
subi %ans,%ans,4
.end_macro

.text
li $v0,5
syscall
move $s0,$v0	#s0=n
mult $s0,$s0
mflo $s1	#s1=n*n

li $t0,0
li $t1,0
input_loop1:
li $v0,5
syscall
sw $v0,x1($t1)
addi $t1,$t1,4
addi $t0,$t0,1
beq $t0,$s1,end_input_loop1
j input_loop1
end_input_loop1:

li $t0,0
li $t1,0
input_loop2:
li $v0,5
syscall
sw $v0,x2($t1)
addi $t1,$t1,4
addi $t0,$t0,1
beq $t0,$s1,end_input_loop2
j input_loop2
end_input_loop2:

addi $s1,$s0,1	#s1=n+1

li $t1,1	#t1=i
loop1:
li $t2,1	#t2=j
loop2:
li $a0,0
li $t3,1
loop:
index($t5,$t1,$t3,$s0)
lw $a1,x1($t5)
index($t6,$t3,$t2,$s0)
lw $a2,x2($t6)
mult $a1,$a2
mflo $a3
add $a0,$a0,$a3
index($t4,$t1,$t2,$s0)
sw $a0,x($t4)
addi $t3,$t3,1
beq $t3,$s1,end_loop
j loop
end_loop:
addi $t2,$t2,1
beq $t2,$s1,end_loop2
j loop2
end_loop2:
addi $t1,$t1,1
beq $s1,$t1,end_loop1
j loop1
end_loop1:

li $t1,1
print_loop1:
li $t2,1
print_loop2:
index($t3,$t1,$t2,$s0)
lw $a0,x($t3)
li $v0,1
syscall
la $a0,space
li $v0,4
syscall
addi $t2,$t2,1
beq $t2,$s1,end_print_loop2
j print_loop2
end_print_loop2:
la $a0,enter
li $v0,4
syscall
addi $t1,$t1,1
beq $t1,$s1,end_print_loop1
j print_loop1
end_print_loop1:

li $v0,10
syscall

