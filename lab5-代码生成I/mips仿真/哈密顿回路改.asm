.data
map:.space 196
vis:.space 40

.macro index(%ans,%x,%y,%xx)
subi %ans,%x,1
mult %ans,%xx
mflo %ans
add %ans,%ans,%y
subi %ans,%ans,1
sll %ans,%ans,2
.end_macro

.text
li $v0,5
syscall
move $s1,$v0	#s1=n
li $v0,5
syscall
move $s2,$v0	#s2=m
li $t0,0	#t0=0
li $t1,1	#t1=1

input_loop:
li $v0,5
syscall
move $s3,$v0
li $v0,5
syscall
move $s4,$v0
index($t2,$s3,$s4,$s1)
sw $t1,map($t2)
index($t2,$s4,$s3,$s1)
sw $t1,map($t2)
addi $t0,$t0,1
beq $t0,$s2,end_input
j input_loop
end_input:

li $s0,0	#s0是答案
li $t0,0
li $t4,4
li $a0,1
li $t9,2
addi $k0,$s1,1	#k0=n+1 控制循环
move $s3,$s1	#s3=sum=n
#################	s1=n  s2=m  t0=0  t1=1  t4=4  s0是答案  a0是参数
jal dfs

#end
move $a0,$s0
li $v0,1
syscall
li $v0,10
syscall
###################	dfs
dfs:

subi $t3,$a0,1	#t3=a0-1
sll $t3,$t3,2
lw $t5,vis($t3)	#t5=vis[a]
addi $t5,$t5,1
sw $t5,vis($t3)	#vis[a]++
subi $s3,$s3,1	#sum--

addi $t8,$s3,1	#t8=sum+1
beq $t8,$t0,judge
end_judge:

li $t2,1	#t2=i
dfs_loop:
index($t7,$a0,$t2,$s1)
lw $t3,map($t7)	#t3=map[a][i]
li $s7,0
bne $t3,$t1,next#map[a][i]!=1,next
move $t3,$t2
subi $t3,$t3,1
sll $t3,$t3,2
lw $t3,vis($t3)	#t3=vis[i]

beq $t2,$t1,judge1#i==1
end_judge1:

bne $t2,$t1,judge2#i!=1
end_judge2:

beq $s7,$t1,work
end_work:
next:
addi $t2,$t2,1
beq $t2,$k0,end_dfs_loop
j dfs_loop
end_dfs_loop:

move $t5,$a0
subi $t5,$t5,1
sll $t5,$t5,2
lw $t6,vis($t5)	#t6=vis[a]
subi $t6,$t6,1
sw $t6,vis($t5)	#vis[a]--
addi $s3,$s3,1	#sum++

jr $31
####################### dfs end

work:

sw $ra,0($sp)
subi $sp,$sp,4
sw $a0,0($sp)
subi $sp,$sp,4
sw $t2,0($sp)
subi $sp,$sp,4
move $a0,$t2
jal dfs

addi $sp,$sp,4
lw $t2,0($sp)
addi $sp,$sp,4
lw $a0,0($sp)
addi $sp,$sp,4
lw $ra,0($sp)
j end_work

####################### 判断if条件是否成立
judge:
beq $a0,$t1,ans
end_ans:
j end_judge

ans:
li $a0,1
li $v0,1
syscall
li $v0,10
syscall
j end_ans

judge1:
sub $t7,$t3,$t9#t7=vis[i]-2
slt $s7,$t7,$t0
j end_judge1

judge2:
bne $t3,$t0,endend
addi $s7,$s7,1
endend:
j end_judge2

