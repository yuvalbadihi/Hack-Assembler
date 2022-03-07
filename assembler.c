#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#define LENNAME 20
int create_list_from_file(char* filename, char** commands_lst);//function declarations
void clean_string(char* commands_lst);
void clean_array_of_strings(char** commands_lst, int commands_counter);
void create_hack_file(char** commands_lst, int commands_counter, char* filename);
void print_a_instruction(char* command, FILE* f);
void print_c_instruction(char* command, FILE* f);
void handle_a(char* temp_binary, char* command);
void handle_dest(char* temp_binary, char* command);
void handle_comp(char* temp_binary, char* command);
void handle_jmp(char* temp_binary, char* command);

int main()
{//main assembler function, turning asm file into binary file by calling the right functions.
	FILE* fu;
	char Name[LENNAME], * commands_lst[301], new_file_name[21];
	int len1, i = 0, commands_counter;
	printf("Insert assembly file name: ");//asking for file's name.
	scanf("%s", Name);
	strcpy(new_file_name, Name);//changing name type (from .asm to .hack)
	len1 = strlen(new_file_name);
	new_file_name[len1 - 4] = '\0'; 
	strcat(new_file_name, ".hack");
	commands_counter = create_list_from_file(Name, commands_lst);//creating strings list and string counter
	clean_array_of_strings(commands_lst, commands_counter);//cleaning each string
	create_hack_file(commands_lst, commands_counter, new_file_name);//creating file
	printf("Output file %s is now available.", new_file_name);
	for (i = 0; i < commands_counter; i++)
		free(commands_lst[i]);//free allocation.
}

void clean_array_of_strings(char** commands_lst, int commands_counter)
{//this function clears spaces or comments from a string by calling the clean_string function.
	int i;
	for (i = 0; i < commands_counter; i++)
		clean_string(commands_lst[i], commands_counter);
}

void clean_string(char* str)
{//this function clears spaces or comments from a string.
	int i = 0, j = 0;

	while (str[i] != '\0' && str[i] != '/' && str[i] != '\n')//(not space,comment,vertical tab)
	{
		if (str[i] != ' ' && str[i] != '\t')
			str[j++] = str[i];
		i++;
	}
	str[j] = '\0';
}

int create_list_from_file(char* filename, char** commands_lst)
{//this function turns each line in the file to a string in the commands list.
	FILE* f;
	char temp_command[301], * command = NULL;
	int i = 0;
	f = fopen(filename, "r");
	while (fgets(temp_command, 301, f) != NULL)
	{
		command = (char*)(malloc(strlen(temp_command) + 1));
		if (command == NULL)
		{//making sure the allocation worked.
			printf("Failed.");
			free(command);
			exit(1);
		}
		strcpy(command, temp_command);
		commands_lst[i] = command;//filling the list with strings
		i++;
	}
	fclose(f);
	return i;
}
void create_hack_file(char** commands_lst, int commands_counter, char* filename)
{//this function creates the output file, the hack file.
	FILE* f;
	int i;
	char* command;
	f = fopen(filename, "w");
	for (i = 0; i < commands_counter; i++)
	{
		command = commands_lst[i];
		if ((commands_lst[i][0] == '@'))//dealing with a instructions or c instructions.
			print_a_instruction(command, f);
		else
			print_c_instruction(command, f);
	}
	fclose(f);
}

void print_a_instruction(char* command, FILE* f)
{//this function prints a instructions.
	char temp_binary[17];
	int i, num;
	command = command + 1;//we already know the command starts with @, so we skip it.
	num = atoi(command);
	for (i = 0; i < 16; i++)//turning decimal numbers into binary numbers
	{
		if (num % 2 == 1)
			temp_binary[15 - i] = '1';
		else
			temp_binary[15 - i] = '0';
		num = num / 2;
	}
	temp_binary[16] = '\0';
	fputs(temp_binary, f);//printing to file
	fputs("\n", f);
}

void print_c_instruction(char* command, FILE* f)
{//this function prints c instructions.
	char temp_binary[17];
	int i;
	for (i = 0; i < 16; i++)
	{//because its a c instruction. we set 111 in the MSB side of the binary value.
		if (i < 3)
			temp_binary[i] = '1';
		else
			temp_binary[i] = '0';

	}
	handle_a(temp_binary, command);//calling a bit function
	handle_dest(temp_binary, command);//calling dest bits function
	handle_comp(temp_binary, command);//calling comp bits function
	handle_jmp(temp_binary, command);//calling jmp bits function
	temp_binary[16] = '\0';
	if (strcmp(temp_binary, "1110000000000000") != 0 || strcmp(command, "D&A") == 0)
	{//print all but "11100000000000",excluding the specific case of "D&A"(because our default c build is 1110000000000000.)
		fputs(temp_binary, f);
		fputs("\n", f);
	}
}

void handle_a(char* temp_binary, char* command)
{//this function changes 'a' bit according to M in comp.
	int i = 0, found_equal = 0;
	while (i < strlen(command))
	{
		if (command[i] == '=')//checking first set of M conditions(finding 'M' after '='):
		{
			found_equal = 1;//found '=' flag = 1
			while (command[i] != ';' && command[i] != '\0')//this part takes place only after "=", so if we find M, we will change the 'a' sign.
			{
				if (command[i] == 'M')
				{
					temp_binary[3] = '1';//a changed.
					i = strlen(command) - 1;//i=len-1 because we found M, so if i=len we exit the inner loop.
				}
				i++;
			}
		}
		i++;
	}
	if (found_equal == 0)//checking second set of M conditions(finding 'M' and not finding '='):
	{
		i = 0;
		while (command[i] != ';' && command[i] != '\0')
		{
			if (command[i] == 'M')
				temp_binary[3] = '1';
			i++;
		}
	}
}

void handle_dest(char* temp_binary, char* command)
{//this function changes the dest bits according to input.
	char key_dest[7][4] = { "M","D","DM","A","AM","AD","AMD" };//"dictionary"
	char value_key[7][4] = { "001","010","011","100","101","110","111" };
	char dest_command[4];
	int i = 0, j = 0, n = 0, found_it = 0;

	while (i < strlen(command))
	{
		if (command[i] == '=')//search-loop after '='
		{
			found_it = 1;
			i = strlen(command);
		}
		i++;
	}
	if (found_it == 1)//if '=' in command
	{
		i = 0;
		while (command[i] != '=')
		{
			dest_command[i] = command[i];//extracting the dest command
			i++;
		}
		dest_command[i] = '\0';
		for (i = 0; i < 7; i++)
		{
			if (strcmp(dest_command, key_dest[i]) == 0)
			{//checking if the command we built exists in our dictionary, if so, change dest bits.
				for (j = 10; j < 13; j++)
				{
					temp_binary[j] = value_key[i][n];//changing dest bits
					n++;
				}
			}
		}
	}
}

void handle_comp(char* temp_binary, char* command)
{//this function changes the comp bits according to input.
	char key_a[28][4] = { "0","1","-1","D","A","!D","!A","-D","-A","D+1","A+1","D-1","A-1","D+A","D-A","A-D","D&A","D|A","M","!M","-M","M+1","M-1","D+M","D-M","M-D","D&M","D|M" };//"dictionary"
	char value_key[28][7] = { "101010","111111","111010","001100","110000","001101","110001","001111","110011","011111","110111","001110","110010","000010","010011","000111","000000","010101","110000","110001","110011","110111","110010","000010","010011","000111","000000","010101" };
	char comp_command[6];
	int i = 0, j = 0, n, found_equal = 0;
	while (i < strlen(command))
	{
		if (command[i] == '=')//if we found '=', comp part is right after. 
		{
			found_equal = 1;//found equal flag = 1
			while (command[i] != ';' && command[i] != '\0' && command[i] != '\n')
			{
				comp_command[j] = command[++i];//building the comp command.
				j++;
			}
			comp_command[j] = command[--i];//compensates last missing char.
			j--;
			comp_command[j] = '\0';
		}
		i++;
	}
	if (found_equal == 0)//if we didn't find '=', comp part should be at the beginning.
	{
		i = 0;
		while (command[i] != ';' && command[i] != '\0')
		{
			comp_command[i] = command[i];//building comp part
			i++;
		}
		comp_command[i] = '\0';
	}
	n = 0;
	for (i = 0; i < 28; i++)
	{//checking if the command we built exists in our dictionary, if so, change comp bits.
		if (strcmp(comp_command, key_a[i]) == 0)
		{
			for (j = 4; j < 10; j++)
			{
				temp_binary[j] = value_key[i][n];
				n++;
			}
		}
	}
}

void handle_jmp(char* temp_binary, char* command)
{//this function changes the jmp bits according to input.
	char key_jmp[7][4] = { "JGT","JEQ","JGE","JLT","JNE","JLE","JMP" };//dictionary
	char value_key[7][4] = { "001","010","011","100","101","110","111" };
	char jmp_command[4];
	int i = 0, j = 0, n = 0;
	while (i < strlen(command))
	{
		if (command[i] == 'J')//looking for a jump command
		{
			for (j = 0; j < 3; j++)
			{
				jmp_command[j] = command[i + j];//building jmp command
			}
			jmp_command[j] = '\0';
		}
		i++;
	}
	for (i = 0; i < 7; i++)
	{
		if (strcmp(jmp_command, key_jmp[i]) == 0)
		{//checking if the command we built exists in our dictionary, if so, change jmp bits.
			for (j = 13; j < 16; j++)
			{
				temp_binary[j] = value_key[i][n];
				n++;
			}
		}
	}
}