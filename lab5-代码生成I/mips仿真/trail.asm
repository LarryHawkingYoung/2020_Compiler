.data
str1: .asciiz "18182653"
str2: .asciiz "the bigger is m"
str3: .asciiz "m is "
str4: .asciiz "check"
str5: .asciiz "right"
str6: .asciiz "middle"
str7: .asciiz "1181007016"
str8: .asciiz " "
str9: .asciiz "end"
.text
	li $v0 , 12
	syscall
	move $a0, $v0
	li $v0, 11
	syscall
	li $v0, 5
	syscall
	move $a0, $v0
	li $v0, 1
	syscall
