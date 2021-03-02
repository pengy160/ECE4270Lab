# psudo code
#
# int fib(n){
#	if (n = 0);
#		exit;
#	else;
#		i += n;
#		n--;
#		fib(n);
# }
# exit: print i;



li $a0, 10	#loads $a0 with fibonici number, in this case 10
li $a1, 0	#loads total reg with 0
jal fib

fib:
beqz $a0, zero		#branch if equal to zero (fib is done)
add $a1, $a1, $a0	#add n value to itself
sub $a0, $a0, 1		#decrement n
jal fib			#call itself (fib) recursivly



zero:
li $v0, 1
move $a0, $a1
syscall 
