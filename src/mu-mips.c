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
	uint32_t instruction, opcode, rs, rt, rd, sa, immediate, function, target;
	instruction = mem_read_32(CURRENT_STATE.PC);
	printf("Instruction: %x\n",instruction);

	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	opcode = (instruction & 0xFC000000) >> 26;
	rs = (instruction & 0x3E00000) >> 21;
	rt = (instruction & 0x1F0000) >> 16;
	rd = (instruction & 0xF800) >> 11;
	sa = (instruction & 0x7C0) >> 6;
	immediate = (instruction & 0xFFFF);
	function = (instruction & 0x3F);
	target = (instruction & 0x3FFFFFF);
	/*
	printf("I- Type: \n");
	printf("opcode: %x\n",opcode);
	printf("RS: %x\n", rs);
	printf("RT: %x\n", rt);
	printf("immediate %x\n", immediate);
	printf("\nJ- Type: \n");
	printf("opcode: %x\n",opcode);
	printf("Target: %x\n", target);
	printf("\nR-Type: \n");
	printf("RS: %x\n", rs);
        printf("RT: %x\n", rt);
	printf("RD: %x\n", rd);
	printf("SA: %x\n", sa);
	printf("function: %x\n", function);
	printf("\n");
	*/
	if(opcode == 0x00){
	
		switch(function){
			
			case 0x0C:
				RUN_FLAG = FALSE;
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20:
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x21:
				//addu
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x22:
				//sub
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x23:
				//subu
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x18:
				//mult
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x19:
				//multu 
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1A:
				//div
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1B:
				//divu
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x24:
				//and
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x25:
				//or
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x26:
				//xor
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x27:
				//nor
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x2A:
				//slt
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x00:
				//sll
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x02:
				//srl
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03:
				//sra
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08:
				//jr
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09:
				//jalr
				print_instruction(CURRENT_STATE.PC);
				break;

		}
	
	}
	else{
		switch(opcode){
			case 0x02:
				//j
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03:
				//jal
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x04:
				//beq
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x05:
				//bne
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x06:
				//blez
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x07:
				//bgtz
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08:
				//ADDI
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09:
				//ADDIU
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0C:
				//ANDI
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0D:
				//ori
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0E:
				//xori
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0A:
				//slti
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x10:
				//mfhi
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x11:
				//mthi
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x12:
				//mflo
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x13:
				//mtlo
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20:
                                //lb
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x23:
                                //lw
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x21:
                                //lh
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x2B:
                                //sw
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x28:
                                //sb
				print_instruction(CURRENT_STATE.PC);
                                break;
                        case 0x29:
                                //sh
				print_instruction(CURRENT_STATE.PC);
                                break;
			case 0x0F:
                                //lui
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
	uint32_t instruction, opcode, rs, rt, rd, sa, immediate, function, target;
	
	instruction = mem_read_32(addr);

	opcode = (instruction & 0xFC000000) >> 26;
        rs = (instruction & 0x3E00000) >> 21;
        rt = (instruction & 0x1F0000) >> 16;
        rd = (instruction & 0xF800) >> 11;
        sa = (instruction & 0x7C0) >> 6;
        immediate = (instruction & 0xFFFF);
        function = (instruction & 0x3F);
        target = (instruction & 0x3FFFFFF);

	//printf("From print: %x", instruction);
	
	char RT[10];
	char RS[10];
	char RD[10];
	
		switch(rd){
			case 0x00:
				strcpy(RS, "zero");
				break;
			case 0x02:
                                strcpy(RD, "v0");
                                break;
                        case 0x03:
				strcpy(RD, "v1");
                                break;
                        case 0x04:
                                strcpy(RD, "a0");
                                break;
                        case 0x05:
				strcpy(RD, "a1");
                                break;
                        case 0x06:
				strcpy(RD, "a2");
                                break;
                        case 0x07:
                                strcpy(RD, "a3");
                                break;
			case 0x08:
                                strcpy(RD, "t0");
                                break;
                        case 0x09:
                                strcpy(RD, "t1");
                                break;
                        case 0x0A:
                                strcpy(RD, "t2");
                                break;
                        case 0x0B:
                                strcpy(RD, "t3");
                                break;
			case 0x0C:
                                strcpy(RD, "t4");
                                break;
                        case 0x0D:
                                strcpy(RD, "t5");
                                break;
                        case 0x0E:
                                strcpy(RD, "t6");
                                break;
                        case 0x0F:
                                strcpy(RD, "t7");
                                break;
                        case 0x10:
                                strcpy(RD, "s0");
                                break;
                        case 0x11:
                                strcpy(RD, "s1");
                                break;
                        case 0x12:
                                strcpy(RD, "s2");
                                break;
                        case 0x13:
                                strcpy(RD, "s3");
                                break;
			case 0x14:
                                strcpy(RD, "s4");
                                break;
                        case 0x15:
                                strcpy(RD, "s5");
                                break;
                        case 0x16:
                                strcpy(RD, "s6");
                                break;
                        case 0x17:
                                strcpy(RD, "s7");
                                break;
                        case 0x18:
                                strcpy(RD, "t8");
                                break;
                        case 0x19:
                                strcpy(RD, "t9");
                                break;
                        case 0x1A:
                                strcpy(RD, "k0");
                                break;
                        case 0x1B:
                                strcpy(RD, "k1");
                                break;
                }
	
		switch(rs){
			case 0x00:
				strcpy(RS, "zero");
				break;
			case 0x02:
                                strcpy(RS, "v0");
                                break;
                        case 0x03:
				strcpy(RS, "v1");
                                break;
                        case 0x04:
                                strcpy(RS, "a0");
                                break;
                        case 0x05:
				strcpy(RS, "a1");
                                break;
                        case 0x06:
				strcpy(RS, "a2");
                                break;
                        case 0x07:
                                strcpy(RS, "a3");
                                break;
			case 0x08:
                                strcpy(RS, "t0");
                                break;
                        case 0x09:
                                strcpy(RS, "t1");
                                break;
                        case 0x0A:
                                strcpy(RS, "t2");
                                break;
                        case 0x0B:
                                strcpy(RS, "t3");
                                break;
			case 0x0C:
                                strcpy(RS, "t4");
                                break;
                        case 0x0D:
                                strcpy(RS, "t5");
                                break;
                        case 0x0E:
                                strcpy(RS, "t6");
                                break;
                        case 0x0F:
                                strcpy(RS, "t7");
                                break;
                        case 0x10:
                                strcpy(RS, "s0");
                                break;
                        case 0x11:
                                strcpy(RS, "s1");
                                break;
                        case 0x12:
                                strcpy(RS, "s2");
                                break;
                        case 0x13:
                                strcpy(RS, "s3");
                                break;
			case 0x14:
                                strcpy(RS, "s4");
                                break;
                        case 0x15:
                                strcpy(RS, "s5");
                                break;
                        case 0x16:
                                strcpy(RS, "s6");
                                break;
                        case 0x17:
                                strcpy(RS, "s7");
                                break;
                        case 0x18:
                                strcpy(RS, "t8");
                                break;
                        case 0x19:
                                strcpy(RS, "t9");
                                break;
                        case 0x1A:
                                strcpy(RS, "k0");
                                break;
                        case 0x1B:
                                strcpy(RS, "k1");
                                break;
                }
	
		switch(rt){
			case 0x00:
				strcpy(RS, "zero");
				break;
			case 0x02:
                                strcpy(RT, "v0");
                                break;
                        case 0x03:
				strcpy(RT, "v1");
                                break;
                        case 0x04:
                                strcpy(RT, "a0");
                                break;
                        case 0x05:
				strcpy(RT, "a1");
                                break;
                        case 0x06:
				strcpy(RT, "a2");
                                break;
                        case 0x07:
                                strcpy(RT, "a3");
                                break;
			case 0x08:
                                strcpy(RT, "t0");
                                break;
                        case 0x09:
                                strcpy(RT, "t1");
                                break;
                        case 0x0A:
                                strcpy(RT, "t2");
                                break;
                        case 0x0B:
                                strcpy(RT, "t3");
                                break;
			case 0x0C:
                                strcpy(RT, "t4");
                                break;
                        case 0x0D:
                                strcpy(RT, "t5");
                                break;
                        case 0x0E:
                                strcpy(RT, "t6");
                                break;
                        case 0x0F:
                                strcpy(RT, "t7");
                                break;
                        case 0x10:
                                strcpy(RT, "s0");
                                break;
                        case 0x11:
                                strcpy(RT, "s1");
                                break;
                        case 0x12:
                                strcpy(RT, "s2");
                                break;
                        case 0x13:
                                strcpy(RT, "s3");
                                break;
			case 0x14:
                                strcpy(RT, "s4");
                                break;
                        case 0x15:
                                strcpy(RT, "s5");
                                break;
                        case 0x16:
                                strcpy(RT, "s6");
                                break;
                        case 0x17:
                                strcpy(RT, "s7");
                                break;
                        case 0x18:
                                strcpy(RT, "t8");
                                break;
                        case 0x19:
                                strcpy(RT, "t9");
                                break;
                        case 0x1A:
                                strcpy(RT, "k0");
                                break;
                        case 0x1B:
                                strcpy(RT, "k1");
                                break;
                }


	if(opcode == 0x00){
		switch(function){

                        case 0x0C:
                                //syscall
                                break;
                        case 0x20:
                                //add
                                printf("\nADD  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
			case 0x21:
                                //addu
                                printf("\nADD  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;

                        case 0x22:
                                //sub
				printf("\nSUB  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);

                                break;
                        case 0x23:
                                //subu
				printf("\nSUBU  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);

                                break;
                        case 0x18:
                                //mult
				printf("\nMULT  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);

                                break;
                        case 0x19:
                                //multu
				printf("\nMULTU  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);

                                break;
                        case 0x1A:
                                //div
				printf("\nDIV  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);

                                break;
                        case 0x1B:
                                //divu
				printf("\nDIVU  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x24:
                                //and
				printf("\nAND  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x25:
                                //or
				printf("\nOR  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x26:
                                //xor
				printf("\nXOR  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x27:
                                //nor
				printf("\nNOR  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x2A:
                                //slt
				printf("\nSLT  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x00:
                                //sll
				printf("\nSLL  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
			case 0x02:
                                //srl
				printf("\nSRL  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x03:
                                //sra
				printf("\nSRA  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x08:
                                //jr
				printf("\nJR  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;
                        case 0x09:
                                 //jalr
				printf("\nJALR  ");
				printf("%s ", RD);
				printf("%s ", RS);
				printf("%s\n", RT);
                                break;

                }
	                             
printf("\n");           

        }
        else{
                switch(opcode){
                        case 0x02:
				//J
                                printf("\nJ  ");
				printf("0x%x\n", target);
                                break;
                        case 0x03:
				//JAL
                                printf("\nJAL  ");
				printf("0x%x\n", target);
                                break;
                        case 0x04:
                                //beq
				printf("\nBEQ  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("0x%x\n", immediate);
                                break;
                        case 0x05:
                                //bne
				printf("\nBNE  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("0x%x\n", immediate);
                                break;
                        case 0x06:
                                //blez
				printf("\nBLEZ  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("0x%x\n", immediate);
                                break;
                        case 0x07:
                                //bgtz
				printf("\nBGTZ  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("0x%x\n", immediate);
                                break;
                        case 0x08:
                                //ADDI
				printf("\nADDI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x09:
                                //ADDIU
				printf("\nADDIU  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x0C:
                                //ANDI
				printf("\nANDI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x0D:
                                //ori
				printf("\nORI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x0E:
                                //xori
				printf("\nXORI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x0A:
                                //slti
				printf("\nSLTI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x10:
                                //mfhi
				printf("\nMFHI  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("%s ", RD);
				printf("0x%x", sa);
				printf("0x%x\n", function);
                                break;
                        case 0x11:
                                //mthi
				printf("\nMTHI  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("%s ", RD);
				printf("0x%x", sa);
				printf("0x%x\n", function);
                                break;
                        case 0x12:
                                //mflo
				printf("\nMFLO  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("%s ", RD);
				printf("0x%x", sa);
				printf("0x%x\n", function);
                                break;
                        case 0x13:
                                //mtlo
				printf("\nMTLO  ");
				printf("%s ", RS);
				printf("%s ", RT);
				printf("%s ", RD);
				printf("0x%x", sa);
				printf("0x%x\n", function);
                                break;
                        case 0x20:
                                //lb
				printf("\nLB  ");
				printf("%s ", RT);
				printf("0x%x ", immediate);
				printf("%s \n", RS);
				break;
                        case 0x23:
                                //lw
				printf("\nLW  ");
				printf("%s ", RT);
				printf("0x%x ", immediate);
				printf("%s \n", RS);
                                break;
                        case 0x21:
                                //lh
				printf("\nLH  ");
				printf("%s ", RT);
				printf("0x%x ", immediate);
				printf("%s \n", RS);
                                break;
                        case 0x2B:
                                //sw
				printf("\nSW  ");
				printf("%s ", RT);
				printf("0x%x ", immediate);
				printf("%s \n", RS);
                                break;
                        case 0x28:
                                //sb
				printf("\nSB  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x29:
                                //sh
				printf("\nSH  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
                                break;
                        case 0x0F:
                                //lui
				printf("\nLUI  ");
				printf("%s ", RT);
				printf("%s ", RS);
				printf("0x%x\n", immediate);
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
