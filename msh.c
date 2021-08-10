/*
 
 name: Avijeet Adhikari
 
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports five arguments

//method that handles the signal that user creates from keyboard
static void handle_signal (int sig )
{
    
}

int main()
{
  char *record[25]; //Declaring an arrray that will hold maximum of 25 history, but we will be outputting only 15.
    
  int history = 0;
  int i;
  for (i=0; i<25; i++)
  {
      record[i]=malloc(MAX_COMMAND_SIZE); //allocating memory to store 25 user command.
  }
    
  int PID_store[15]; // Array to store upto 15 PIDs of last 15 processes spawened by shell
  int PID_counter=0;
    
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  
  struct sigaction act;
 
  memset (&act, '\0', sizeof(act)); /*Zero out the sigaction struct*/
 
  act.sa_handler = &handle_signal; /* Set the handler to use the function handle_signal() */
 
  
  if (sigaction(SIGINT , &act, NULL) < 0) /* Install the handler and check the return value. SIGINT for ctrl-c. Used from github code provided sigint.c.*/
  {
    perror ("sigaction: ");
    return 1;
  }
  
  if (sigaction(SIGTSTP , &act, NULL) < 0) /* Install the handler and check the return value. SIGINT for ctrl-z*/ 
  {
    perror ("sigaction: ");
    return 1;
  }

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    strcpy(record[history], cmd_str); //Copy all the commands and store in record array, this will be used for outputting list of history. int history will be incremented at the end.
      
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
    }
    
    if (token[0]!=NULL) // enters only if user enters some command, so that we can compare
    {
      if (token[0][0]=='!')// for !n
      {
          int n;
          //if starts with '!' convert the following string to integer
          //so that we can call that number fromour record array,
          //which contains history of commands.
          n=atoi(&token[0][1]);
          //Max number of displayed history will be upto 15/
          if(n>history || n>15)
          {
              printf("Command not in history.\n");
          }
          else
          {
              // record[n] will be the command that user wants to run using '!' command, we will copy that to cmd_str
              strcpy(cmd_str,record[n]);
             //from strcpy above we will have desired command selected from the history, then just repeat the process
             //provided above that will run the command.
              int   token_count = 0;
              
              char *arg_ptr;
              
              char *working_str  = strdup( cmd_str );
              
              while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
                     (token_count<MAX_NUM_ARGUMENTS))
              {
                  token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
                  if( strlen( token[token_count] ) == 0 )
                  {
                      token[token_count] = NULL;
                  }
                  token_count++;
                  
              }
              
          }
      }
    
     if(cmd_str[0] != '!') //if the command starts with '!' the work will be performed and will skip from this point
     {
    
      if(strcmp(token[0],"history")==0) //if the command is history
        {
            if(history<15) //if number of history is less than 15
            {       int i;
                    for(i=0; i<=history; i++)
                     {
                        printf("%d: %s", i, record[i]); //prints 15 or less history
                     }
                }
            
            else // if user enters more than 15 history
             {
                 int j=0;
                 int k;
                 for(k=history-15; k<=history; k++) // We will print the last 15 history and display them
                 {
                     printf("%d: %s",j, record[k]);
                     j++;
                 }
             }
        }
      
      else if(strcmp(token[0],"exit")==0 || strcmp(token[0],"quit")==0) // for exit and quit command.
      {
         exit(0);
      }
    
      else if (strcmp(token[0],"echo")==0) // for echo command
      {
          int i=0;
          char *echo_commands[15]; //array to store upto 15 string, starting with echo
          char *ptr;
          ptr=strtok(cmd_str," ;\n"); //tokenizing the user command for whitespace, ;, and trailing new line
          while(ptr!=NULL)
          {
              echo_commands[i++]=ptr; //inserting string to an array after removing space, ;, and trailing newline
              ptr=strtok(NULL," ;\n");
          }
          
          //My code runs for both echo foo; echo bar & echo foo ; echo bar
          //meaning that no space between foo and ; or space between foo and ;.
          int k;
          for(k=1; k<i; k+=2)
          {
              //Since we will have more than one echo commands we need to fork as
              //many times the echo command repeats.
              pid_t child_pid =fork();
              PID_store[PID_counter++]=child_pid; //storing pid value in PID_store int array.
              int status;
              if (child_pid == 0) //enters if the fork is succesfull.
              {
                  //my echo_commandswill have {"echo","foo","echo","bar"}
                  //k =1,3,5.... so execlp using echo command for foo and bar.
                  //here, 1st token[0] will be file name in system that is echo, and second token[0]
                  // is command to look for that is echo command in echo file.
                execlp(token[0],token[0],echo_commands[k],NULL);
              }
              waitpid(child_pid,&status,0); //waits for child process to end.
          }
                
        }
        
      else if(strcmp(token[0],"cd")==0) // for cd command
      {
          chdir(token[1]); //if cd is followed by any other command, directory will be changed as per the command.
          //This handles cd .. as well.
      }
      
     else if (strcmp(token[0],"listpids")==0 || strcmp(token[0],"showpids")==0) //for listpids/showpids command
     {
         if(PID_counter<15) // enters if less than 15 process is spawned by the shell.
         {
             int i;
             for (i=0; i<PID_counter; i++)
             {
                 printf("%d: %d\n", i, PID_store[i]);
             }
         }
         else
         {
             int l=0;
             //if more than 15 process are spawned then the following loop will print the last
             //15 PIDs. if there are 17 process then m= 17-15=2 then loop runs from 2 to 17
             // that makes sure that last 15 is printed.
             int m;
             for(m=PID_counter-15; m<PID_counter; m++)
             {
                 printf("%d: %d\n",l, PID_store[m]);
                 l++;
             }
         }
     }
     
     else if(strcmp(token[0],"bg")==0) // if the command is bg, background the suspended process
     {
         //this works with the last process spawned. SIGCONT signal is sent to the
         //very last process in the shell. Idea taken from the professor.
         
         int sigtokill=PID_counter-1; //to catch the last pid value. Below the PID_counter++
                                      //increase the value by one after the last process. To get the
                                      //last pid value just subtract 1 from PID_counter.
         kill(PID_store[sigtokill],SIGCONT);
     }
    
      //if none of the command entered from user is matched to above command, then we are ready to fork.
      //we will create a duplicate process and search for the commands using the path.
     else
     {
         pid_t pid=fork();
         PID_store[PID_counter++]=pid; //storing pid value in PID_store int array.
         int status;
         if( pid == 0 ) //if fork is succesfull, enter this condition.
         {
             //searching command using ececlp. Here first argument token[0] will be the name of the file in the paths
             //available, and the second argument token[0] is the name of the command that needs to searched with in that
             //file. I created up to 10 tokens so that it supports upto 10 command line parameter.
             execlp(token[0],token[0], token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8],token[9],token[10],NULL);
             //if file or command is not found or if exec fails.
             if(execlp(token[0],token[0], token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8],token[9],token[10],NULL)==-1)
             {
                 //if the command entered by user is not valid and also not found in our path.
                 printf("%s: Command not found\n",token[0]);
             }
             exit(EXIT_SUCCESS); //successfull termination of new process.
         }
         waitpid( pid, &status, 0 ); //wating for child process to end.
     }
    
    }
 }
    free( working_root );

    history++; //After one command runs through the system, int history will be incremented and will work as record array index.
}
    for (i=0; i<25; i++)
    {
        free(record[i]); //freeing up the allocated space for history/ record array
    }
    return 0;
}
