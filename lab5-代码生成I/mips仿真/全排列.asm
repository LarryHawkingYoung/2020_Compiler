.data
sym:.space 32
arr:.space 32
space:.asciiz" "
enter:.asciiz"\n"

.text
li $v0,5
syscall
move $s0,$v0	#s0=n
li $a1,0	#a1ÊÇ²ÎÊý
jal dfs
li $v0,10
syscall

dfs:

#move $a0,$a1
#li $v0,1
#syscall
#la $a0,enter
#li $v0,4
#syscall

sub $t0,$a1,$s0	#t0=index-n
bgez $t0,print
end_print:
li $t0,0
dfs_loop:
sll $t1,$t0,2
lw $t2,sym($t1)	#t2=symbol[i]
bne $t2,$0,end
sll $t3,$a1,2
addi $t4,$t0,1
sw $t4,arr($t3)	#array[index]=i+1
sll $t3,$t0,2
li $t4,1
sw $t4,sym($t3)	#symbol[i]=1
j work
end_work:
sll $t3,$t0,2
sw $0,sym($t3)	#symbol[i]=0
end:
addi $t0,$t0,1
beq $t0,$s0,end_dfs_loop
j dfs_loop
end_dfs_loop:
jr $ra

print:
li $t0,0
print_loop:
sll $t1,$t0,2
lw $a0,arr($t1)
li $v0,1
syscall
la $a0,space 
li $v0,4
syscall
addi $t0,$t0,1
beq $t0,$s0,end_print_loop
j print_loop
end_print_loop:
la $a0,enter
li $v0,4
syscall
jr $31


work:
sw $ra,0($sp)
subi $sp,$sp,4
sw $a1,0($sp)
subi $sp,$sp,4
sw $t0,0($sp)
subi $sp,$sp,4
addi $a1,$a1,1
jal dfs

addi $sp,$sp,4
lw $t0,0($sp)
addi $sp,$sp,4
lw $a1,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work
