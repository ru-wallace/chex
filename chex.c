#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <argp.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define BG_RED_TXT_WHITE   "\x1b[37;41m"
#define BG_BLUE_TXT_WHITE  "\x1b[34;40m"


/* Program documentation. */
static char doc[] =
"Demo for argp";

/* A description of the arguments we accept. */
static char args_doc[] = "FILE";

/* The options we understand. */
static struct argp_option options[] = {
	{"lines",	 'n', "N_LINES",			0,  "Number of lines to display(default: 10) (default line length: 16 bytes)" },
    {"start",	 's', "START_OFFSET",			0,  "Byte to start at (default: 0)\n(if START_OFFSET begins with '0x' or '0X' it will be parsed as hex). " },
    {"line-bytes",	 'l', "LINE_LEN",			0,  "Number of bytes per line (default: 16)" },
    {"int-tab", 'i', "INTERMEDIATE_TAB_POSITION", 0, "Number of bytes after which to insert tab space (default: 8)"},
    {"decimal", 'd', "DECIMAL_ADDRESSING", OPTION_ARG_OPTIONAL, "Display byte addresses as decimal (default: 0)"},

	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
	char *args[1];                /* table name and input file*/
    size_t start_byte;
	unsigned int n_lines;
    unsigned int intermediate_break;
    unsigned int line_length;
    int decimal_addressing;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
		 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;
    int base;

	switch (key)
	{
		case 'n':
			arguments->n_lines = (unsigned int)atoi(arg);
			break;
        case 's':
            
            
            if (arg[0] == '0' && (arg[1] =='x' || arg[1] == 'X')) {
                base = 16;
                arg = arg + 2;
            } else {
                base = 10;
            } 
            
            arguments->start_byte = (size_t)strtoul(arg, NULL, base);
            break;
        case 'l':
            arguments->line_length = (unsigned int)atoi(arg);
            break;
        case 'i':
            arguments->intermediate_break = (unsigned int)atoi(arg);
            break;
        case 'd':
            arguments->decimal_addressing = 1;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 1)
				/* Too many arguments. */
				argp_usage (state);
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 1)
				/* Not enough arguments. */
				argp_usage (state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

void printLine(unsigned char line[], unsigned int len) {
    //printf(" Line size: %d ", sizeof(line));
    for (int i = 0; i < len; i++){
        unsigned char c = line[i];
        //if (c == '\0') return;
        if (isprint(c)) {
            putchar(c);
        } else if (c < 32) {
            setlocale(LC_CTYPE, "");
            wchar_t specChar = c + 0x2400;
            printf(BG_BLUE_TXT_WHITE "%lc" ANSI_COLOR_RESET, specChar);
            setlocale(LC_ALL, "C");
        } else {
            printf(BG_RED_TXT_WHITE "." ANSI_COLOR_RESET);
        }
    }
}


int main(int argc, char **argv)
{   

    struct arguments arguments;
    setlocale(LC_ALL, "C");

    arguments.n_lines = 0;
    arguments.intermediate_break = 8;
    arguments.line_length = 16;
    arguments.start_byte = 0;
    arguments.decimal_addressing = 0;

    argp_parse (&argp, argc, argv, 0, 0, &arguments);




    if (arguments.line_length <= 0) arguments.line_length = 16;
    if (arguments.n_lines <= 0) arguments.n_lines = 0;
    if (arguments.start_byte <= 0) arguments.start_byte = 0; 
    if (arguments.intermediate_break > arguments.line_length) arguments.intermediate_break = arguments.line_length;
    
    //printf("Reading from '%s':\n", filepath);
    //printf("N_lines: %d\nStart Byte: 0x%04lx\nLine Length: %d\nIntermediate Break: %d\n", arguments.n_lines,
    //arguments.start_byte, arguments.line_length, arguments.intermediate_break);


    FILE *file = fopen( arguments.args[0], "rb" );  

    
    int pos = 0;
    int col = 0;
    int addr=0;
    int row = 0;
    unsigned char line[arguments.line_length];

    int complete = 0;

    
    //printf(" \n0x%04x: ", pos);

    while (1) {

        unsigned char c;
        
        
        if (!complete) 
        {

           //printf("Start\n");
            c = fgetc(file);

            if (addr < arguments.start_byte) {
                addr +=1;
                continue;
            }

            
        } 


        if (col == 0) 
        {

            if (row > 0)
            {
                printf("|");
                printLine(line, arguments.line_length);
                printf("|\n");
                
                memset(line, '0', sizeof(line));
                if (arguments.n_lines && row >= arguments.n_lines) {
                    complete = 1;
                }
            }
            if (complete) break;

            if (arguments.decimal_addressing) 
            {
                printf("%06d: ", addr);
            } else printf("0x%04x: ",  addr);

           
            
        } 
        else if (col % arguments.intermediate_break == 0) 
        {
            printf("  ");
        }

        
        line[col] = c;

        
        col +=1;
        if (col >= arguments.line_length) 
        {
            
            col = 0;
            row +=1;
        }

        addr +=1;

        if (!complete)
        {
            if (feof(file)) 
            {
                line[col] = '\0';
                complete = 1; 
                printf("¬¬ ");

            } else printf("%02x ", c);
            
        } else
        {
            printf("¬¬ ");
        }
        
        
    }


    printf("\n");

}