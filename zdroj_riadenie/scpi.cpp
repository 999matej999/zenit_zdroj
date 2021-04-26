#include "scpi.h"
#include "variables.h"
#include <string.h>

enum class CHANNEL { ALL = 0, CH1, CH2, CH3, CH4 };

enum class CMD_TYPE { COMMON = 0, APPLY, INSTRUMENT, MEASURE, OUTPUT, SOURCE, SYSTEM };

enum class ARGUMENT_TYPE { NONE, VALUE, CHANNEL, INTEGER, LOGIC };

uint8_t compare_strings(char *tmp_first, const char *tmp_second){
	return (strcmp(tmp_first, tmp_second) == 0);
}

uint8_t compare_string_to_lenght(char *tmp_first, const char *tmp_second, uint8_t tmp_lenght)
{
	return (strncmp(tmp_first, tmp_second, tmp_lenght) == 0);
}

uint8_t does_string_start_with(char *tmp_string, const char *tmp_start)
{
	return compare_string_to_lenght(tmp_string, tmp_start, strlen(tmp_start));
}

//remove everything before separator (separator is removed too)
char *remove_to_separator(char *tmp_string, const char tmp_separator)
{
	//remove everything before separator (if the string contain separator)
	while(*tmp_string != tmp_separator && *tmp_string != 0x00){tmp_string++;}

	//if there isn't end of string, remove remaining separator
	if(*tmp_string != 0x00){tmp_string++;}

	return tmp_string;
}

//remove everything before separator (separator is not removed)
char *remove_before_separator(char *tmp_string, const char tmp_separator)
{
	//remove everything before separator (if the string contain separator)
	while(*tmp_string != tmp_separator && *tmp_string != 0x00){tmp_string++;}

	return tmp_string;
}

//remove everything before separator, return pointer to separator or NULL in case we haven't found
//any of the separators
char *remove_string_before_separators(char *tmp_string, char* tmp_separators, uint8_t tmp_separators_count)
{

	char *tmp_new_string = tmp_string;

	for(uint8_t tmp_i=0; tmp_i < tmp_separators_count; tmp_i++){
		//remove everything before separator (if the string contain separator)
		while(*tmp_new_string != tmp_separators[tmp_i] && *tmp_new_string != 0x00){tmp_new_string++;}

		//in case we found separator, return pointer to it
		if(*tmp_new_string == tmp_separators[tmp_i]){
			return tmp_new_string;
		//in case that we haven't found separator restore pointer to start of string
		}else{
			tmp_new_string = tmp_string;
		}

	}

	//in case that we haven't found any separator return NULL
	return NULL;
	
}

char *remove_string_from_string(char *tmp_string, const char *tmp_to_remove)
{
	// remove x characters from beginning of string
	// x is lenght of string to remove
	for(uint8_t tmp_i=0; tmp_i < strlen(tmp_to_remove); tmp_i++){
		// we will increase pointer only if we still haven't reached end of string
		if(*tmp_string != 0x00){tmp_string++;}
	}

	return tmp_string;
}

void scpi_parse(RECEIVER r)
{
	char *tmp_string = buffer + 1;
	//char *tmp_out[BUFFER_SIZE] = {};

	if(does_string_start_with(tmp_string, "SYST"))
	{
		tmp_string = remove_to_separator(tmp_string, ':');
		if(does_string_start_with(tmp_string, "LOC"))
		{
			control = CONTROL::LOCAL;
			mySerial.println("OK");
		}
		else if(does_string_start_with(tmp_string, "REM"))
		{
			control = CONTROL::REMOTE;
			mySerial.println("OK");
		}
	}

	if(control == CONTROL::LOCAL) return;

	if(does_string_start_with(buffer, "*IDN?"))
	{
		mySerial.println(DEVICE_NAME);
	}
	else if(does_string_start_with(tmp_string, "SYST"))
	{
		tmp_string = remove_to_separator(tmp_string, ':');
		if(does_string_start_with(tmp_string, "COMM"))
		{
			tmp_string = remove_to_separator(tmp_string, ':');
			if(does_string_start_with(tmp_string, "RS232"))
			{
				tmp_string = remove_to_separator(tmp_string, ':');
				if(does_string_start_with(tmp_string, "BAUD"))
				{
					tmp_string = remove_string_from_string(tmp_string, "BAUD");
					if(*tmp_string == '?')
					{
						mySerial.println(baudrate);
					}
					else if(*tmp_string == ' ')
					{
						long new_baudrate = atol(++tmp_string);
						switch(new_baudrate) //TODO: vyskusat
						{
							case 4800:  baudrate = new_baudrate; break;
							case 9600:  baudrate = new_baudrate; break;
							case 19200: baudrate = new_baudrate; break;
							case 38400: baudrate = new_baudrate; break;
							default: break;
						}
						mySerial.println("OK");
						mySerial.end();
						mySerial.begin(baudrate);
					}
				}
			}
		}
		else if(does_string_start_with(tmp_string, "RWL"))
		{
			char tmp_separators[] = {':', ' '};
			uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			if(*tmp_string == ':')
			{
				++tmp_string;
				tmp_string = remove_before_separator(tmp_string, ' ');
			}
			++tmp_string;
			if(compare_strings(tmp_string, "ON")) rw_lock = true;
			else if(compare_strings(tmp_string, "OFF")) rw_lock = false;
			mySerial.println("OK");
		}
		else if(does_string_start_with(tmp_string, "VERS"))
		{
			tmp_string = remove_before_separator(tmp_string, '?');
			if(*tmp_string == '?')
			{
				mySerial.println(DEVICE_VERSION);
			}
		}
	}
	else if(does_string_start_with(tmp_string, "APPL"))
	{

	}
	else if(does_string_start_with(tmp_string, "INST"))
	{

	}
	else if(does_string_start_with(tmp_string, "MEAS"))
	{
		tmp_string = remove_string_from_string(tmp_string, "MEAS");
		tmp_string = remove_to_separator(tmp_string, ':');
	}
	else if(does_string_start_with(tmp_string, "OUTP"))
	{

	}
	else if(does_string_start_with(tmp_string, "SOUR"))
	{

	}
}

void to_upper_case(char *buf)
{
	for(size_t i = 0; i < strlen(buf); ++i)
	{
		if(buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= ('a' - 'A');
	}
}

void scpi_execute(RECEIVER r)
{

}

void cmd_arrived(RECEIVER r)
{
	if(strlen(buffer) == 0) return;
	else
	{
		if(buffer[0] == '*' || buffer[0] == ':')
		{
			to_upper_case(buffer);
			scpi_parse(r);
			scpi_execute(r);
		}
	}
}
