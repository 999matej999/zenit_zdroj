#include "scpi.h"
#include "variables.h"
#include <string.h>

enum class CHANNEL { CH1 = 0, CH2, CH3, CH4, ALL, NONE, ERR };

CHANNEL selected = CHANNEL::CH1;

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

	for(uint8_t tmp_i=0; tmp_i < tmp_separators_count; tmp_i++)
	{
		//remove everything before separator (if the string contain separator)
		while(*tmp_new_string != tmp_separators[tmp_i] && *tmp_new_string != 0x00){tmp_new_string++;}

		//in case we found separator, return pointer to it
		if(*tmp_new_string == tmp_separators[tmp_i])
		{
			return tmp_new_string;
		//in case that we haven't found separator restore pointer to start of string
		}
		else
		{
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

CHANNEL string_to_channel(char *tmp_string)
{
	if(strlen(tmp_string) == 0) return CHANNEL::NONE;
	else if(compare_strings(tmp_string, "CH1")) return CHANNEL::CH1;
	else if(compare_strings(tmp_string, "CH2")) return CHANNEL::CH2;
	else if(compare_strings(tmp_string, "CH3")) return CHANNEL::CH3;
	else if(compare_strings(tmp_string, "CH4")) return CHANNEL::CH4;
	else if(compare_strings(tmp_string, "ALL")) return CHANNEL::ALL;
	else return CHANNEL::ERR;
}

CHANNEL dec_to_channel(int8_t ch)
{
	if(ch == 5) return CHANNEL::NONE;
	else if(ch == 0) return CHANNEL::CH1;
	else if(ch == 1) return CHANNEL::CH2;
	else if(ch == 2) return CHANNEL::CH3;
	else if(ch == 3) return CHANNEL::CH4;
	else if(ch == 4) return CHANNEL::ALL;
	else return CHANNEL::ERR;
}

int8_t channel_to_dec(CHANNEL tmp)
{
	if(tmp == CHANNEL::ALL) return 4;
	else if(tmp == CHANNEL::CH1) return 0;
	else if(tmp == CHANNEL::CH2) return 1;
	else if(tmp == CHANNEL::CH3) return 2;
	else if(tmp == CHANNEL::CH4) return 3;
	//else if(tmp == CHANNEL::NONE) return 5;
	else return -1;
}

int8_t string_to_channel_dec(char *tmp_string)
{
	CHANNEL tmp = string_to_channel(tmp_string);
	return channel_to_dec(tmp);
}

float string_to_float(char *tmp_string)
{
	//parameters (string, pointer to end of number (not used in case of NULL))
	return strtod(tmp_string, NULL);
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

	tmp_string = buffer + 1;

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
	else if(does_string_start_with(tmp_string, "CAL"))
	{
		
	}
	else if(does_string_start_with(tmp_string, "INST"))
	{
		tmp_string = remove_to_separator(tmp_string, ':');
		if(does_string_start_with(tmp_string, "NSEL"))
		{
			char tmp_separators[] = {' ', '?'};
			uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);

			if(*tmp_string == '?')
			{
				mySerial.println(channel_to_dec(selected)+1);
			}
			else if(*tmp_string == ' ')
			{
				++tmp_string;
				int8_t ch = atoi(tmp_string);
				selected = dec_to_channel(ch-1);
				mySerial.println("OK");
			}
		}
		else
		{
			if(does_string_start_with(tmp_string, "SEL"))
			{
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}
			else
			{
				tmp_string = buffer + 1;
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}

			if(*tmp_string == '?')
			{
				switch(selected)
				{
					case CHANNEL::CH1: mySerial.println("CH1"); break;
					case CHANNEL::CH2: mySerial.println("CH2"); break;
					case CHANNEL::CH3: mySerial.println("CH3"); break;
					case CHANNEL::CH4: mySerial.println("CH4"); break;
					default: break;
				}
			}
			else if(*tmp_string == ' ')
			{
				++tmp_string;
				CHANNEL ch = string_to_channel(tmp_string);
				if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
				{
					selected = ch;
					mySerial.println("OK");
				}
			}
		}
	}
	else if(does_string_start_with(tmp_string, "MEAS"))
	{
		tmp_string = remove_to_separator(tmp_string, ':');
		if(does_string_start_with(tmp_string, "ALL"))
		{
			tmp_string = remove_to_separator(tmp_string, ':');
		
			if(does_string_start_with(tmp_string, "DC"))
			{
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}
			else
			{
				tmp_string = buffer + 1;
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}

			if(*(tmp_string - 1) == '?' || *tmp_string == '?')
			{
				++tmp_string;

				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				float valueU = 0, valueI = 0;
				switch(ch)
				{
					case CHANNEL::CH1: valueU = Ch1Enabled ? U1meas : 0; valueI = Ch1Enabled ? I1meas : 0; break;
					case CHANNEL::CH2: valueU = Ch2Enabled ? U2meas : 0; valueI = Ch2Enabled ? I2meas : 0; break;
					case CHANNEL::CH3: valueU = Ch3Enabled ? U3meas : 0; valueI = Ch3Enabled ? I3meas : 0; break;
					case CHANNEL::CH4: valueU = Ch4Enabled ? U4meas : 0; valueI = Ch4Enabled ? I4meas : 0; break;
					default: break;
				}

				if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
				{
					mySerial.print(valueU, 3);
					mySerial.print(",");
					mySerial.println(valueI, 3);
				}
			}
		}
		else if(does_string_start_with(tmp_string, "CURR"))
		{
			tmp_string = remove_to_separator(tmp_string, ':');
		
			if(does_string_start_with(tmp_string, "DC"))
			{
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}
			else
			{
				tmp_string = buffer + 1;
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}

			if(*(tmp_string - 1) == '?' || *tmp_string == '?')
			{
				++tmp_string;

				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				float valueI = 0;
				switch(ch)
				{
					case CHANNEL::CH1: valueI = Ch1Enabled ? I1meas : 0; break;
					case CHANNEL::CH2: valueI = Ch2Enabled ? I2meas : 0; break;
					case CHANNEL::CH3: valueI = Ch3Enabled ? I3meas : 0; break;
					case CHANNEL::CH4: valueI = Ch4Enabled ? I4meas : 0; break;
					default: break;
				}

				if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
				{
					mySerial.println(valueI, 3);
				}
			}
		}
		else
		{
			tmp_string = buffer + 1;

			if(does_string_start_with(tmp_string, "MEAS"))
			{
				char tmp_separators[] = {':', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':') ++tmp_string;
			}

			if(does_string_start_with(tmp_string, "VOLT"))
			{
				char tmp_separators[] = {':', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':') ++tmp_string;
			}
		
			if(does_string_start_with(tmp_string, "DC"))
			{
				char tmp_separators[] = {':', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':') ++tmp_string;
			}

			if(*(tmp_string - 1) == '?' || *tmp_string == '?')
			{
				if(*tmp_string == ' ') ++tmp_string;

				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				float valueU = 0;
				switch(ch)
				{
					case CHANNEL::CH1: valueU = Ch1Enabled ? U1meas : 0; break;
					case CHANNEL::CH2: valueU = Ch2Enabled ? U2meas : 0; break;
					case CHANNEL::CH3: valueU = Ch3Enabled ? U3meas : 0; break;
					case CHANNEL::CH4: valueU = Ch4Enabled ? U4meas : 0; break;
					default: break;
				}

				if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
				{
					mySerial.println(valueU, 3);
				}
			}
		}
	}
	else if(does_string_start_with(tmp_string, "OUTP"))
	{
		tmp_string = remove_to_separator(tmp_string, ':');
		if(does_string_start_with(tmp_string, "CVCC") || does_string_start_with(tmp_string, "MODE"))
		{
			tmp_string = remove_to_separator(tmp_string, ' ');
			CHANNEL ch = string_to_channel(tmp_string);
			if(ch == CHANNEL::NONE) ch = selected;

			bool state = false;
			switch(ch)
			{
				case CHANNEL::CH1: state = Ch1Ilimit; break;
				case CHANNEL::CH2: state = Ch2Ilimit; break;
				case CHANNEL::CH3: state = Ch3Ilimit; break;
				case CHANNEL::CH4: state = Ch4Ilimit; break;
				default: break;
			}

			if(ch != CHANNEL::ALL && ch!= CHANNEL::ERR)
			{
				mySerial.println(state ? "CC" : "CV");
			}
		}
		else if(does_string_start_with(tmp_string, "OCP"))
		{
			tmp_string = remove_to_separator(tmp_string, ':');
			if(does_string_start_with(tmp_string, "ALAR") || does_string_start_with(tmp_string, "QUES"))
			{
				tmp_string = remove_to_separator(tmp_string, ' ');
				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				bool state = false;
				switch(ch)
				{
					case CHANNEL::CH1: state = Fuse1Trip; break;
					case CHANNEL::CH2: state = Fuse2Trip; break;
					case CHANNEL::CH3: state = Fuse3Trip; break;
					case CHANNEL::CH4: state = Fuse4Trip; break;
					default: break;
				}

				if(ch != CHANNEL::ALL && ch!= CHANNEL::ERR)
				{
					mySerial.println(state ? "YES" : "NO");
				}
			}
			else if(does_string_start_with(tmp_string, "CLEAR"))
			{
				tmp_string = remove_to_separator(tmp_string, ' ');
				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				switch(ch)
				{
					case CHANNEL::CH1: Ch1Enabled = false; Fuse1Reset = true; break;
					case CHANNEL::CH2: Ch2Enabled = false; Fuse2Reset = true; break;
					case CHANNEL::CH3: Ch3Enabled = false; Fuse3Reset = true; break;
					case CHANNEL::CH4: Ch4Enabled = false; Fuse4Reset = true; break;
					default: break;
				}

				if(ch != CHANNEL::ALL && ch!= CHANNEL::ERR)
				{
					mySerial.println("OK");
				}
			}
			else
			{
				if(does_string_start_with(tmp_string, "STAT"))
				{
					char tmp_separators[] = {' ', '?'};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				}
				else
				{
					tmp_string = buffer + 1;
					char tmp_separators[] = {' ', '?'};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				}

				if(*(tmp_string - 1) == '?' || *tmp_string == '?')
				{
					++tmp_string;

					CHANNEL ch = string_to_channel(tmp_string);
					if(ch == CHANNEL::NONE) ch = selected;

					bool state = false;
					switch(ch)
					{
						case CHANNEL::CH1: state = Fuse1Ena; break;
						case CHANNEL::CH2: state = Fuse2Ena; break;
						case CHANNEL::CH3: state = Fuse3Ena; break;
						case CHANNEL::CH4: state = Fuse4Ena; break;
						default: break;
					}

					if(ch != CHANNEL::ALL && ch!= CHANNEL::ERR)
					{
						mySerial.println(state ? "ON" : "OFF");
					}
				}
				else if(*tmp_string == ' ')
				{
					++tmp_string;

					if(*tmp_string == 'C' || *tmp_string == 'O')
					{
						CHANNEL ch = CHANNEL::NONE;
						if(*tmp_string == 'C')
						{
							char tmp_ch[4] = {};
							strncpy(tmp_ch, tmp_string, 3);
							ch = string_to_channel(tmp_ch);
							tmp_string += 4;
						}
						if(*tmp_string == 'O')
						{
							if(ch == CHANNEL::NONE) ch = selected;

							int8_t state = -1;

							if(compare_strings(tmp_string, "ON")) state = 1;
							else if(compare_strings(tmp_string, "OFF")) state = 0;

							if(state != -1)
							{
								switch(ch)
								{
									case CHANNEL::CH1: Fuse1Ena = state; break;
									case CHANNEL::CH2: Fuse2Ena = state; break;
									case CHANNEL::CH3: Fuse3Ena = state; break;
									case CHANNEL::CH4: Fuse4Ena = state; break;
									default: break;
								}

								if(ch != CHANNEL::ALL && ch!= CHANNEL::ERR)
								{
									mySerial.println("OK");
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if(does_string_start_with(tmp_string, "STAT"))
			{
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}
			else
			{
				tmp_string = buffer + 1;
				char tmp_separators[] = {' ', '?'};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			}

			if(*(tmp_string - 1) == '?' || *tmp_string == '?')
			{
				++tmp_string;

				CHANNEL ch = string_to_channel(tmp_string);
				if(ch == CHANNEL::NONE) ch = selected;

				bool state = false;
				switch(ch)
				{
					case CHANNEL::CH1: state = Ch1Enabled; break;
					case CHANNEL::CH2: state = Ch2Enabled; break;
					case CHANNEL::CH3: state = Ch3Enabled; break;
					case CHANNEL::CH4: state = Ch4Enabled; break;
					case CHANNEL::ALL: state = OutEnabled; break;
					default: break;
				}

				if(ch != CHANNEL::ERR)
				{
					mySerial.println(state ? "ON" : "OFF");
				}
			}
			else if(*tmp_string == ' ')
			{
				++tmp_string;

				if(*tmp_string == 'C' || *tmp_string == 'A' || *tmp_string == 'O')
				{
					CHANNEL ch = CHANNEL::NONE;
					if(*tmp_string == 'C' || *tmp_string == 'A')
					{
						char tmp_ch[4] = {};
						strncpy(tmp_ch, tmp_string, 3);
						ch = string_to_channel(tmp_ch);
						tmp_string += 4;
					}
					if(*tmp_string == 'O')
					{
						if(ch == CHANNEL::NONE) ch = selected;

						int8_t state = -1;

						if(compare_strings(tmp_string, "ON")) state = 1;
						else if(compare_strings(tmp_string, "OFF")) state = 0;

						if(state != -1)
						{
							if(state == 1)
							{
								switch(ch)
								{
									case CHANNEL::CH1: Ch1Enabled = state; break;
									case CHANNEL::CH2: Ch2Enabled = state; break;
									case CHANNEL::CH3: Ch3Enabled = state; break;
									case CHANNEL::CH4: Ch4Enabled = state; break;
									case CHANNEL::ALL: OutEnabled = state; updateOutEnabled = true; break;
									default: break;
								}
							}
							else if(state == 0)
							{
								switch(ch)
								{
									case CHANNEL::CH1: Ch1Enabled = state; Fuse1Reset = true; break;
									case CHANNEL::CH2: Ch2Enabled = state; Fuse2Reset = true; break;
									case CHANNEL::CH3: Ch3Enabled = state; Fuse3Reset = true; break;
									case CHANNEL::CH4: Ch4Enabled = state; Fuse4Reset = true; break;
									case CHANNEL::ALL: OutEnabled = state; updateOutEnabled = true; break;
									default: break;
								}
							}

							if(ch != CHANNEL::ERR)
							{
								mySerial.println("OK");
							}
						}
					}
				}
			}
		}
	}
	else
	{
		CHANNEL ch = selected;
		if(does_string_start_with(tmp_string, "SOUR"))
		{
			tmp_string = remove_to_separator(tmp_string, ':');
			char ch_ = *(tmp_string - 2);
			ch_ -= '1';
			if(ch_ >= 0 && ch_ <= 3) ch = dec_to_channel(ch_);
		}

		if(does_string_start_with(tmp_string, "CURR"))
		{
			char tmp_separators[] = {':', '?', ' '};
			uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;

			if(does_string_start_with(tmp_string, "PROT"))
			{
				tmp_string = remove_to_separator(tmp_string, ':');
				if(does_string_start_with(tmp_string, "CLE"))
				{
					switch(ch)
					{
						case CHANNEL::CH1: Ch1Enabled = false; Fuse1Reset = true; break;
						case CHANNEL::CH2: Ch2Enabled = false; Fuse2Reset = true; break;
						case CHANNEL::CH3: Ch3Enabled = false; Fuse3Reset = true; break;
						case CHANNEL::CH4: Ch4Enabled = false; Fuse4Reset = true; break;
						default: break;
					}

					if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
					{
						mySerial.println("OK");
					}
				}
				else if(does_string_start_with(tmp_string, "STAT"))
				{
					char tmp_separators[] = {' ', '?'};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);

					if(*tmp_string == '?')
					{
						bool state = false;
						switch(ch)
						{
							case CHANNEL::CH1: state = Fuse1Ena; break;
							case CHANNEL::CH2: state = Fuse2Ena; break;
							case CHANNEL::CH3: state = Fuse3Ena; break;
							case CHANNEL::CH4: state = Fuse4Ena; break;
							default: break;
						}

						if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
						{
							mySerial.println(state ? "ON" : "OFF");
						}
					}
					else if(*tmp_string == ' ')
					{
						++tmp_string;

						int8_t state = -1;

						if(compare_strings(tmp_string, "ON")) state = 1;
						else if(compare_strings(tmp_string, "OFF")) state = 0;

						if(state != -1)
						{
							switch(ch)
							{
								case CHANNEL::CH1: Fuse1Ena = state; break;
								case CHANNEL::CH2: Fuse2Ena = state; break;
								case CHANNEL::CH3: Fuse3Ena = state; break;
								case CHANNEL::CH4: Fuse4Ena = state; break;
								default: break;
							}

							if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
							{
								mySerial.println("OK");
							}
						}
					}
				}
				else if(does_string_start_with(tmp_string, "TRIP"))
				{
					if(tmp_string[strlen(tmp_string) - 1] == '?')
					{
						bool state = false;
						switch(ch)
						{
							case CHANNEL::CH1: state = Fuse1Trip; break;
							case CHANNEL::CH2: state = Fuse2Trip; break;
							case CHANNEL::CH3: state = Fuse3Trip; break;
							case CHANNEL::CH4: state = Fuse4Trip; break;
							default: break;
						}

						if(ch != CHANNEL::ALL && ch!= CHANNEL::NONE && ch!= CHANNEL::ERR)
						{
							mySerial.println(state ? "YES" : "NO");
						}
					}
				}
			}
			else
			{
				if(does_string_start_with(tmp_string, "LEV"))
				{
					char tmp_separators[] = {':', '?', ' '};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
					if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
				}

				if(does_string_start_with(tmp_string, "IMM"))
				{
					char tmp_separators[] = {':', '?', ' '};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
					if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
				}
			
				if(does_string_start_with(tmp_string, "AMPL"))
				{
					char tmp_separators[] = {':', '?', ' '};
					uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
					tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
					if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
				}

				if(*(tmp_string - 1) == '?' || *tmp_string == '?')
				{
					if(*tmp_string == ' ') ++tmp_string;
					
					float valueI = -10;

					if(strlen(tmp_string) == 0)
					{
						switch(ch)
						{
							case CHANNEL::CH1: valueI = I1setpoint; break;
							case CHANNEL::CH2: valueI = I2setpoint; break;
							case CHANNEL::CH3: valueI = I3setpoint; break;
							case CHANNEL::CH4: valueI = I4setpoint; break;
							default: break;
						}
					}
					else if(compare_strings(tmp_string, "MIN")) valueI = Imin;
					else if(compare_strings(tmp_string, "MAX")) valueI = Imax;

					if(valueI > -1)
					{
						mySerial.println(valueI, 3);
					}
				}
				else if(*(tmp_string - 1) == ' ')
				{
					float valueI = -10;

					if(compare_strings(tmp_string, "MIN"))
					{
						switch(ch)
						{
							case CHANNEL::CH1: I1setpoint = Imin; break;
							case CHANNEL::CH2: I2setpoint = Imin; break;
							case CHANNEL::CH3: I3setpoint = Imin; break;
							case CHANNEL::CH4: I4setpoint = Imin; break;
							default: break;
						}
					}
					else if(compare_strings(tmp_string, "MAX")){
						switch(ch)
						{
							case CHANNEL::CH1: I1setpoint = Imax; break;
							case CHANNEL::CH2: I2setpoint = Imax; break;
							case CHANNEL::CH3: I3setpoint = Imax; break;
							case CHANNEL::CH4: I4setpoint = Imax; break;
							default: break;
						}
					}
					else
					{
						valueI = string_to_float(tmp_string);
						switch(ch)
						{
							case CHANNEL::CH1: I1setpoint = valueI; break;
							case CHANNEL::CH2: I2setpoint = valueI; break;
							case CHANNEL::CH3: I3setpoint = valueI; break;
							case CHANNEL::CH4: I4setpoint = valueI; break;
							default: break;
						}
					}

					if(valueI > -1)
					{
						mySerial.println("OK");
					}
				}
			}
		}
		
		if(does_string_start_with(tmp_string, "VOLT"))
		{
			char tmp_separators[] = {':', '?', ' '};
			uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
			if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;

			if(does_string_start_with(tmp_string, "LEV"))
			{
				char tmp_separators[] = {':', '?', ' '};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
			}

			if(does_string_start_with(tmp_string, "IMM"))
			{
				char tmp_separators[] = {':', '?', ' '};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
			}
		
			if(does_string_start_with(tmp_string, "AMPL"))
			{
				char tmp_separators[] = {':', '?', ' '};
				uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);
				tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);
				if(*tmp_string == '?' || *tmp_string == ':' || *tmp_string == ' ') ++tmp_string;
			}

			if(*(tmp_string - 1) == '?' || *tmp_string == '?')
			{
				if(*tmp_string == ' ') ++tmp_string;
				
				float valueU = -10;

				if(strlen(tmp_string) == 0)
				{
					switch(ch)
					{
						case CHANNEL::CH1: valueU = U1setpoint; break;
						case CHANNEL::CH2: valueU = U2setpoint; break;
						case CHANNEL::CH3: valueU = U3setpoint; break;
						case CHANNEL::CH4: valueU = U4setpoint; break;
						default: break;
					}
				}
				else if(compare_strings(tmp_string, "MIN")) valueU = Umin;
				else if(compare_strings(tmp_string, "MAX")) valueU = Umax;

				if(valueU > -1)
				{
					mySerial.println(valueU, 3);
				}
			}
			else if(*(tmp_string - 1) == ' ')
			{
				float valueU = -10;

				if(compare_strings(tmp_string, "MIN"))
				{
					switch(ch)
					{
						case CHANNEL::CH1: U1setpoint = Umin; break;
						case CHANNEL::CH2: U2setpoint = Umin; break;
						case CHANNEL::CH3: U3setpoint = Umin; break;
						case CHANNEL::CH4: U4setpoint = Umin; break;
						default: break;
					}
				}
				else if(compare_strings(tmp_string, "MAX")){
					switch(ch)
					{
						case CHANNEL::CH1: U1setpoint = Umax; break;
						case CHANNEL::CH2: U2setpoint = Umax; break;
						case CHANNEL::CH3: U3setpoint = Umax; break;
						case CHANNEL::CH4: U4setpoint = Umax; break;
						default: break;
					}
				}
				else
				{
					valueU = string_to_float(tmp_string);
					switch(ch)
					{
						case CHANNEL::CH1: U1setpoint = valueU; break;
						case CHANNEL::CH2: U2setpoint = valueU; break;
						case CHANNEL::CH3: U3setpoint = valueU; break;
						case CHANNEL::CH4: U4setpoint = valueU; break;
						default: break;
					}
				}

				if(valueU > -1)
				{
					mySerial.println("OK");
				}
			}
		}
	}
}

void to_upper_case(char *buf)
{
	for(size_t i = 0; i < strlen(buf); ++i)
	{
		if(buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= ('a' - 'A');
	}
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
		}
	}
}
