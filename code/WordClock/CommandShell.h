#ifndef SURREALITYLABS_COMMANDSHELL
#define SURREALITYLABS_COMMANDSHELL

#include <Arduino.h>

/* UART command buffer size */
#define COMMANDSHELL_BUF_SIZE   32

/* UART command max length */
#define COMMANDSHELL_CMD_LENGH_MAX (COMMANDSHELL_BUF_SIZE - 1)

/* UART command max parameter count */
#define COMMANDSHELL_PARA_CNT_MAX 4

/* UART command shell prompt */
#define COMMANDSHELL_PROMPT ">>"

/* type defines */
/* UART command function prototype */
typedef int (*commandshell_cmd_func)(char * args[], char num_args);

/* UART command structure */
typedef struct commandshell_cmd_struct
{
  char *name;
  char *help;
  commandshell_cmd_func do_cmd;
} commandshell_cmd_struct_t;

class CommandShell {
    public:
        void runService();
        void init(Stream* outputStream);
        commandshell_cmd_struct_t *commandTable;
    protected:
        Stream* myStream; 
        /* UART command buffer */
        unsigned char commandshell_buf[COMMANDSHELL_BUF_SIZE];
        /* UART command buffer write pointer */
        unsigned char commandshell_wptr;
        /* UART command parameter pointer */
        char * commandshell_para_ptr[COMMANDSHELL_PARA_CNT_MAX];
        int printHelp(char * args[], char num_args);
        void printPrompt(void);
        void executeCommand(void);
        void flushBuffer(void);
};

#endif

