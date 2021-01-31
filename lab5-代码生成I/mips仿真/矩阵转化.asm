.data
matrix:.space 10000
space:.asciiz" "
enter:.asciiz"\n"

.macro index(%ans,%i,%j)
li $t7,50
mult $t7,%i
mflo %ans
add %ans,%ans,%j
sll %ans,%ans,2
.end_macro

.text
li $v0,5
syscall
move $s0,$v0	#s0=n
li $v0,5
syscall
move $s1,$v0	#s1=m
add $s2,$s0,1
add $s3,$s1,1
li $t0,1
input_loop1:
li $t1,1
input_loop2:
li $v0,5
syscall
index($t2,$t0,$t1)
sw $v0,matrix($t2)
addi $t1,$t1,1
beq $t1,$s3,end_loop2
j input_loop2
end_loop2:
addi $t0,$t0,1
beq $t0,$s2,end_loop1
j input_loop1
end_loop1:
##################### ‰»ÎÕÍ±œ
move $t0,$s0
output_loop1:
move $t1,$s1
output_loop2:
index($t2,$t0,$t1)
lw $t2,matrix($t2)
beq $t2,0,end
li $v0,1
move $a0,$t0
syscall
li $v0,4
la $a0,space
syscall
li $v0,1
move $a0,$t1
syscall
li $v0,4
la $a0,space
syscall
li $v0,1
move $a0,$t2
syscall
li $v0,4
la $a0,enter
syscall
end:
subi $t1,$t1,1
beq $t1,0,end_output_loop2
j output_loop2
end_output_loop2:
subi $t0,$t0,1
beq $t0,0,end_output_loop1
j output_loop1
end_output_loop1:

li $v0,10
syscall


