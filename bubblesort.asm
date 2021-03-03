# psuedo code
#
#

.data
	myArray: .space 40

.text
	addi $t0, $zero, 5	#putting array into registers
	addi $t1, $zero, 3
	addi $t2, $zero, 6
	addi $t3, $zero, 8
	addi $t4, $zero, 9
	addi $t5, $zero, 1
	addi $t6, $zero, 4
	addi $t7, $zero, 7
	addi $s0, $zero, 2
	addi $s1, $zero, 10
	
	addi $a3, $zero, 0	#index
	
	sw $t0, myArray($a3)	#putting registers into data
		addi $a3,$a3,4
	sw $t1, myArray($a3)
		addi $a3,$a3,4	
	sw $t2, myArray($a3)
		addi $a3,$a3,4		
	sw $t3, myArray($a3)
		addi $a3,$a3,4			
	sw $t4, myArray($a3)
		addi $a3,$a3,4				
	sw $t5, myArray($a3)
		addi $a3,$a3,4					
	sw $t6, myArray($a3)
		addi $a3,$a3,4		
	sw $t7, myArray($a3)
		addi $a3,$a3,4
	sw $s0, myArray($a3)
		addi $a3,$a3,4
	sw $s1, myArray($a3)
		addi $a3,$a3,4		

addi $a3, $zero, 0	#clears index to 0
addi $a1, $zero, 0	#temp num 1
addi $a2, $zero, 0	#temp num 2
addi $s6, $zero, 0	#flag

sort:
lw $a1, myArray($a3)	#puts temp value into temp
addi $a3,$a3,4		#increments the array
lw $a2, myArray($a3)	#puts temp value into temp
beqz $a2, endOfArray
bgt $a1, $a2, swap	#branches if left number greater than right (needs to be swapped)
beqz $zero, sort

swap:
sw $a1, myArray($a3)
addi $a3,$a3,-4
sw $a2, myArray($a3)
addi $a3,$a3,4
li $s6, 1	#flags that swap has happened
beqz $zero, sort

endOfArray:
beqz $s6 end
addi $s6, $zero, 0	#resets flag
beqz $s6, loop


loop:
addi $a3, $zero, 0	#clears index to 0
beqz $a3, sort


end:
li $v0, 1       
li $a0, 12       # $integer to print to confirm function
syscall		 # prints coonfirmation "12" to show 

#END END END END END END END END END END END END END