.data
map:.space 400
vis:.space 400

.macro index(%ans,%i,%j,%n)
addi %ans,%n,1
mult %ans,%i
mflo %ans	#ans=(n+1)*i
add %ans,%ans,%j#ans=(n+1)*i+j
sll %ans,%ans,2
.end_macro

.text
li $v0,5
syscall
move $s0,$v0	#s0=n
li $v0,5
syscall
move $s1,$v0	#s1=m
addi $s2,$s0,1
addi $s3,$s1,1
li $t0,1
input_loop1:
li $t1,1
input_loop2:
li $v0,5
syscall
index($t2,$t0,$t1,$s1)
sw $v0,map($t2)
addi $t1,$t1,1
beq $t1,$s3,end_input_loop2
j input_loop2
end_input_loop2:
addi $t0,$t0,1
beq $t0,$s2,end_input_loop1
j input_loop1
end_input_loop1:

li $v0,5
syscall
move $s2,$v0	#s2=ax
li $v0,5
syscall
move $s3,$v0	#s3=ay
li $v0,5
syscall
move $s4,$v0	#s4=bx
li $v0,5
syscall
move $s5,$v0	#s5=by
####################### 输入完成
#######################	s6用来存储答案
li $s6,0
move $a0,$s2
move $a1,$s3	#a0,a1是参数

jal dfs

li $v0,1
move $a0,$s6
syscall
li $v0,10
syscall

dfs:
beq $a0,$0,return
beq $a1,$0,return
sub $t0,$a0,$s0
bgtz $t0,return
sub $t0,$a1,$s1
bgtz $t0,return
li $t1,1
index($t0,$a0,$a1,$s1)
lw $t2,map($t0)
beq $t2,$t1,return
lw $t2,vis($t0)
beq $t2,$t1,return

j end_return
return:
jr $31
end_return:

li $t0,0
li $t2,2
beq $a0,$s4,judge1
end_judge1:
beq $a1,$s5,judge2
end_judge2:
beq $t0,$t2,return1
j end_return1
return1:
addi $s6,$s6,1
jr $31
end_return1:

index($t0,$a0,$a1,$s1)
li $t1,1
sw $t1,vis($t0)	#vis[x][y]=1
j work1
end_work1:
j work2
end_work2:
j work3
end_work3:
j work4
end_work4:

index($t0,$a0,$a1,$s1)
sw $0,vis($t0)

jr $31

work1:
sw $ra,0($sp)
subi $sp,$sp,4
sw $a0,0($sp)
subi $sp,$sp,4
sw $a1,0($sp)
subi $sp,$sp,4
subi $a0,$a0,1

jal dfs

addi $sp,$sp,4
lw $a1,0($sp)
addi $sp,$sp,4
lw $a0,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work1

work2:
sw $ra,0($sp)
subi $sp,$sp,4
sw $a0,0($sp)
subi $sp,$sp,4
sw $a1,0($sp)
subi $sp,$sp,4
subi $a1,$a1,1

jal dfs

addi $sp,$sp,4
lw $a1,0($sp)
addi $sp,$sp,4
lw $a0,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work2

work3:
sw $ra,0($sp)
subi $sp,$sp,4
sw $a0,0($sp)
subi $sp,$sp,4
sw $a1,0($sp)
subi $sp,$sp,4
addi $a1,$a1,1

jal dfs

addi $sp,$sp,4
lw $a1,0($sp)
addi $sp,$sp,4
lw $a0,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work3

work4:
sw $ra,0($sp)
subi $sp,$sp,4
sw $a0,0($sp)
subi $sp,$sp,4
sw $a1,0($sp)
subi $sp,$sp,4
addi $a0,$a0,1

jal dfs

addi $sp,$sp,4
lw $a1,0($sp)
addi $sp,$sp,4
lw $a0,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work4

judge1:
addi $t0,$t0,1
j end_judge1

judge2:
addi $t0,$t0,1
j end_judge2