#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	uint32_t instruction, opcode, rs, rt, rd, sa, immediate, function, target, offset;
	uint64_t product;
	instruction = mem_read_32(CURRENT_STATE.PC);

	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	opcode = (instruction & 0xFC000000) >> 26;
	rs = (instruction & 0x3E00000) >> 21;
	rt = (instruction & 0x1F0000) >> 16;
	rd = (instruction & 0xF800) >> 11;
	sa = (instruction & 0x7C0) >> 6;
	immediate = (instruction & 0xFFFF);
	function = (instruction & 0x3F);
	offset = (instruction & 0x3FFFFFF);
	
	if(opcode == 0x00){
	
		switch(function){
			
			case 0x0C:
				//syscall
				RUN_FLAG = FALSE;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20:
				//add
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x21:
				//addu
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x22:
				//sub
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x23:
				//subu
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x18:
				//mult
				product = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
				NEXT_STATE.HI = product >> 32;
				NEXT_STATE.LO = product & 0xFFFFFFFF;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x19:
				//multu 
				product = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
				NEXT_STATE.HI = product >> 32;
				NEXT_STATE.LO = product & 0xFFFFFFFF;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1A:
				//div
				NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1B:
				//divu
				NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x24:
				//and
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x25:
				//or
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x26:
				//xor
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x27:
				//nor
				NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x2A:
				//slt
				if(CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]){
					NEXT_STATE.REGS[rd] = 0x00000001;
				}
				else{
					NEXT_STATE.REGS[rd] = 0x00000000;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x00:
				//sll
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x02:
				//srl
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03:
				//sra
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08:
				//jr
				NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09:
				//jalr
				NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
				NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x10:
				//mfhi
				NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x11:
				//mthi
				NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x12:
				//mflo
				NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x13:
				//mtlo
				NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
				print_instruction(CURRENT_STATE.PC);
				break;

		}
	
	}
	else{
		switch(opcode){
			case 0x01:
							//confused
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x02:
				//j
				target = offset << 2;
				NEXT_STATE.PC = ((CURRENT_STATE.PC >> 28) << 28) + target;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03:
				//jal
				target = offset << 2;
				NEXT_STATE.PC = ((CURRENT_STATE.PC >> 28) << 28) + target;
				NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x04:
				//beq
				target = ((int32_t)((int16_t)immediate)) << 2;
				if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
					NEXT_STATE.PC = CURRENT_STATE.PC + target;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x05:
				//bne
				target = ((int32_t)((int16_t)immediate)) << 2;
				if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
					NEXT_STATE.PC = CURRENT_STATE.PC + target;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x06:
				//blez
				target = ((int32_t)((int16_t)immediate)) << 2;
				if (((int32_t)CURRENT_STATE.REGS[rs]) <= 0){
					NEXT_STATE.PC = CURRENT_STATE.PC + target;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x07:
				//bgtz
				target = ((int32_t)((int16_t)immediate)) << 2;
				if (((int32_t)CURRENT_STATE.REGS[rs]) > 0){
					NEXT_STATE.PC = CURRENT_STATE.PC + target;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08:
				//ADDI
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + (int32_t)((int16_t)immediate);
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09:
				//ADDIU
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + (uint32_t)((uint16_t)immediate);
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0C:
				//ANDI
				NEXT_STATE.REGS[rt] = immediate & CURRENT_STATE.REGS[rs] & 0xFFFF;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0D:
				//ori
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | (uint32_t)((uint16_t)immediate);
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0E:
				//xori
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ (uint32_t)((uint16_t)immediate);
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0A:
				//slti
				if(CURRENT_STATE.REGS[rs] < (int32_t)((int16_t)immediate)){
					NEXT_STATE.REGS[rt] = 0x00000001;
				}
				else{
					NEXT_STATE.REGS[rt] = 0x00000000;
				}
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20:
                                //lb
								//confused
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x23:
                                //lw
				target = ((uint32_t)((uint16_t)immediate)) + CURRENT_STATE.REGS[rs];
				NEXT_STATE.REGS[rt] = mem_read_32(target);
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x21:
                                //lh
								//confused
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x2B:
                                //sw
								//confused
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x28:
                                //sb
								//confused
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x29:
                                //sh
								//confused
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x0F:
                                //lui
				NEXT_STATE.REGS[rt] = ((uint32_t)immediate) << 16;
				print_instruction(CURRENT_STATE.PC);
                                break;		
		}
	}
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
	uint32_t instruction, opcode, rs, rt, rd, sa, immediate, function, target, offset;
	
	instruction = mem_read_32(addr);

	opcode = (instruction & 0xFC000000) >> 26;
        rs = (instruction & 0x3E00000) >> 21;
        rt = (instruction & 0x1F0000) >> 16;
        rd = (instruction & 0xF800) >> 11;
        sa = (instruction & 0x7C0) >> 6;
        immediate = (instruction & 0xFFFF);
        function = (instruction & 0x3F);
        offset = (instruction & 0x3FFFFFF);

	//printf("From print: %x", instruction);

	if(opcode == 0x00){
		switch(function){

                        case 0x0C:
                                //syscall
				printf("SYSCALL\n");
                                break;
                        case 0x20:
                                //add
                                printf("\nADD  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
			case 0x21:
                                //addu
                                printf("\nADD  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;

                        case 0x22:
                                //sub
				printf("\nSUB  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x23:
                                //subu
				printf("\nSUBU  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x18:
                                //mult
				printf("\nMULT  ");
				printf("$%d ", rs);
				printf("$%d \n", rt);

                                break;
                        case 0x19:
                                //multu
				printf("\nMULTU  ");
				printf("$%d ", rs);
				printf("$%d ", rt);

                                break;
                        case 0x1A:
                                //div
				printf("\nDIV  ");
				printf("$%d ", rs);
				printf("$%d ", rt);
                                break;
                        case 0x1B:
                                //divu
				printf("\nDIVU  ");
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x24:
                                //and
				printf("\nAND  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x25:
                                //or
				printf("\nOR  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x26:
                                //xor
				printf("\nXOR  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x27:
                                //nor
				printf("\nNOR  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x2A:
                                //slt
				printf("\nSLT  ");
				printf("$%d ", rd);
				printf("$%d ", rs);
				printf("$%d\n", rt);
                                break;
                        case 0x00:
                                //sll
				printf("\nSLL  ");
				printf("$%d ", rd);
				printf("$%d ", rt);
				printf("$%d\n", sa);
                                break;
			case 0x02:
                                //srl
				printf("\nSRL  ");
				printf("$%d ", rd);
				printf("$%d ", rt);
				printf("$%d\n", sa);
                                break;
                        case 0x03:
                                //sra
				printf("\nSRA  ");
				printf("$%d ", rd);
				printf("$%d ", rt);
				printf("$%d\n", sa);
                                break;
                        case 0x08:
                                //jr
				printf("\nJR  ");
				printf("$%d\n", rs);
                                break;
                        case 0x09:
                                 //jalr
				printf("\nJALR  ");
				printf("$%d \n", rs);
				printf("JALR $%d ", rd);
				printf("$%d\n", rs);
                                break;
			case 0x10:
                                //mfhi
				printf("\nMFHI  ");
				printf("$%d \n", rd);
                                break;
                        case 0x11:
                                //mthi
				printf("\nMTHI  ");
				printf("$%d \n", rs);
                                break;
                        case 0x12:
                                //mflo
				printf("\nMFLO  ");
				printf("$%d \n", rd);
                                break;
                        case 0x13:
                                //mtlo
				printf("\nMTLO  ");
				printf("$%d \n", rs);
                                break;
                }
	                                      

        }
        else{
                switch(opcode){
                        case 0x02:
				//J
                                printf("\nJ  ");
				printf("%d\n", offset);
                                break;
                        case 0x03:
				//JAL
                                printf("\nJAL  ");
				printf("%d\n", offset);
                                break;
                        case 0x04:
                                //beq
				printf("\nBEQ  ");
				printf("$%d ", rs);
				printf("$%d ", rt);
				printf("%d\n", immediate);
                                break;
                        case 0x05:
                                //bne
				printf("\nBNE  ");
				printf("$%d ", rs);
				printf("$%d ", rt);
				printf("%d\n", immediate);
                                break;
                        case 0x06:
                                //blez
				printf("\nBLEZ  ");
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x07:
                                //bgtz
				printf("\nBGTZ  ");
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x08:
                                //ADDI
				printf("\nADDI  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x09:
                                //ADDIU
				printf("\nADDIU  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x0C:
                                //ANDI
				printf("\nANDI  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x0D:
                                //ori
				printf("\nORI  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x0E:
                                //xori
				printf("\nXORI  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x0A:
                                //slti
				printf("\nSLTI  ");
				printf("$%d ", rt);
				printf("$%d ", rs);
				printf("%d\n", immediate);
                                break;
                        case 0x20:
                                //lb
				printf("\nLB  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d \n", rs);
				break;
                        case 0x23:
                                //lw
				printf("\nLW  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d \n", rs);
                                break;
                        case 0x21:
                                //lh
				printf("\nLH  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d \n", rs);
                                break;
                        case 0x2B:
                                //sw
				printf("\nSW  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d \n", rs);
                                break;
                        case 0x28:
                                //sb
				printf("\nSB  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d \n", rs);
                                break;
                        case 0x29:
                                //sh
				printf("\nSH  ");
				printf("$%d ", rt);
				printf("%d ", immediate);
				printf("$%d\n", rs);
                                break;
                        case 0x0F:
                                //lui
				printf("\nLUI  ");
				printf("$%d ", rt);
				printf("%d\n", immediate);
                                break;
                }
        }
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
