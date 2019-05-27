#include "CommandShell.h"

/* local function define */
/* flush the UART command buffer */
void CommandShell::flushBuffer(void)
{
    commandshell_wptr = 0;
    memset(commandshell_buf,0,sizeof(commandshell_buf));
    memset(commandshell_para_ptr,0,sizeof(commandshell_para_ptr));
}

/* init the UART command server, should call in setup() */
void CommandShell::init(Stream* outputStream)
{
    myStream = outputStream;
    this->flushBuffer();
    myStream->println(F("\n\n   ---ARDUINO UART COMMAND SHELL---"));
    myStream->println(F("[Jiashu Lin - jiashu.lin@gmail.com]\n\n"));
    this->printPrompt();
}

/* print the command shell prompt */
void CommandShell::printPrompt()
{
  myStream->print('\n');
  myStream->print(COMMANDSHELL_PROMPT);
}

/* help command implementation */
int CommandShell::printHelp(char * args[], char num_args)
{
    char i = 0;

    myStream->println(F("\nARDUINO COMMAND SHELL HELP\n\n\rCommand list"));

    while (commandTable[i].name)
    {
	 myStream->print(commandTable[i].name);
	 myStream->print("\t");
	 myStream->println(commandTable[i].help);
	 i++;
    }
    return 0;
}

/* Execute the command in the command buffer */
void CommandShell::executeCommand(void)
{
    unsigned char i = 0, para_cnt = 0, err = 0;

    while((para_cnt < COMMANDSHELL_PARA_CNT_MAX) && (commandshell_para_ptr[para_cnt]) && (*commandshell_para_ptr[para_cnt]))
    {
	  para_cnt++;
    }
    if(!strcmp("help", (char*)commandshell_buf)) {
          this->printHelp(commandshell_para_ptr, para_cnt);
          this->flushBuffer();
          this->printPrompt();
          return;
    }
    while(0 != (commandTable[i].name))
    {
	  if(!strcmp((char*)commandshell_buf,commandTable[i].name))
	  {
	    myStream->println();
	    err = commandTable[i].do_cmd(commandshell_para_ptr, para_cnt);
	    this->flushBuffer();
            this->printPrompt();
	    return;
	  }
	  i++;
    }
    myStream->println("\nUnknown Command");
    this->flushBuffer();
    this->printPrompt();
}

/* uart command server service routine, should call in loop() */
void CommandShell::runService()
{
    char c = 0;
    char i = 0;
    while(myStream->available())
    {
	// read one byte from serial port
	c = myStream->read();

	// if the first byte is ' ' or '\n', ignore it
	if((0 == commandshell_wptr)&&(' ' == c || '\r' == c))
	{
	  this->executeCommand();
	  continue;
	}

	// if '\n' is read, execute the command
	if('\r' == c)
	{
	  this->executeCommand();
	}
	// if ' ' is read, record the parameter ptr
	else if(' ' == c)
	{
	  // damping the space
	  if(commandshell_buf[commandshell_wptr-1])
	  {
		// replace it with NULL
		commandshell_buf[commandshell_wptr] = 0;

		commandshell_wptr++;

		// record the parameter address
		for(i = 0; i < COMMANDSHELL_PARA_CNT_MAX; i++)
		{
		    if(!commandshell_para_ptr[i])
		    {
			commandshell_para_ptr[i] = (char*)(&commandshell_buf[commandshell_wptr]);
			break;
		    }
		}

		if(COMMANDSHELL_PARA_CNT_MAX == i)
		{
		    this->executeCommand();
		    break;
		}
	  }
	}
        // ignore linefeeds
        else if('\n' == c) {
            // do nothing 
        }
	// other characters, just record it
	else
	{
	    commandshell_buf[commandshell_wptr] = c;
	    commandshell_wptr++;
	    if(commandshell_wptr == COMMANDSHELL_CMD_LENGH_MAX)
	    {
		this->executeCommand();
	    }
	}
    }
}

