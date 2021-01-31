.data
jj:.space 400
jjh:.space 400
res:.space 400
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
move $s0,$v0	#s0=m1
li $v0,5
syscall
move $s1,$v0	#s1=n1
li $v0,5
syscall
move $s2,$v0	#s2=m2
li $v0,5
syscall 
move $s3,$v0	#s3=n2

mult $s0,$s1
mflo $t1	#t1=m1*n1
li $t0,0
li $t2,0
input_loop1:
li $v0,5
syscall
sw $v0,jj($t2)
addi $t2,$t2,4
addi $t0,$t0,1
beq $t1,$t0,input_loop1_end
j input_loop1
input_loop1_end:

mult $s2,$s3
mflo $t1
li $t0,0
li $t2,0
input_loop2:
li $v0,5
syscall
sw $v0,jjh($t2)
addi $t2,$t2,4
addi $t0,$t0,1
beq $t1,$t0,input_loop2_end
j input_loop2
input_loop2_end:
##########################输入完成，开始计算
sub $s4,$s0,$s2
addi $s4,$s4,1
sub $s5,$s1,$s3
addi $s5,$s5,1	#s4=m1-m2+1	s5=n1-n2+1	s4为要计算的卷积的行数  s5为要计算的卷积的列数
addi $t8,$s4,1
addi $t9,$s5,1

li $t0,1	#t0代表行数
calc_loop1:
li $t1,1	#t1代表列数
calc_loop2:

li $a0,0	#a0保存在这个位置上的计算结果
li $t2,0	#t2代表要计算的卷积的行数
calc_loop3:
add $t4,$t0,$t2	#t4=t0+t2 	t4表示在原卷积上的行数
li $t3,0	#t3代表要计算的卷积的列数
calc_loop4:
add $t5,$t1,$t3	#t5=t1+t3	t5表示在原卷积上的列数
index($t6,$t4,$t5,$s1)
lw $a1,jj($t6)
add $k0,$t2,1
add $k1,$t3,1
index($t6,$k0,$k1,$s3)
lw $a2,jjh($t6)
mult $a1,$a2
mflo $a1
add $a0,$a0,$a1
addi $t3,$t3,1
beq $t3,$s3,end_loop4
j calc_loop4
end_loop4:
addi $t2,$t2,1
beq $t2,$s2,end_loop3
j calc_loop3
end_loop3:

index($t6,$t0,$t1,$s5)
sw $a0,res($t6)
addi $t1,$t1,1
beq $t9,$t1,end_loop2
j calc_loop2
end_loop2:
addi $t0,$t0,1
beq $t8,$t0,end_loop1
j calc_loop1
end_loop1:


###########################计算完毕，开始输出
addi $s6,$s4,1
addi $s7,$s5,1
li $t0,1
print_loop1:
li $t1,1
print_loop2:
index($t3,$t0,$t1,$s5)
lw $a0,res($t3)
li $v0,1
syscall
la $a0,space
li $v0,4
syscall
addi $t1,$t1,1
beq $t1,$s7,end_print_loop2
j print_loop2
end_print_loop2:
la $a0,enter
li $v0,4
syscall
addi $t0,$t0,1
beq $t0,$s6,end_print_loop1
j print_loop1
end_print_loop1:

li $v0,10
syscall
