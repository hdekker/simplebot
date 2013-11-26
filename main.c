#include <signal.h>
#include <unistd.h>  // sleep
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc
#include <pthread.h>
#include "lms2012.h"

#include "motors.h"
#include "sensors.h"
#include "robot.h"
#include "common.h"
#include "command.h"
#include "maze.h"
#include "ball.h"


typedef void(*game_func)(bool*);


typedef struct function_args
{
  game_func function;
  bool * keep_running;
} function_args;


// Global boolean to indicate whether we can continue, or have to terminate.
static bool keep_running = true;


void intHandler(int dummy)
{
  // This is called when CTRL-C is pressed on keyboard.
  keep_running = false;
}


void* run(void* args)
{
  // This runs in thread context. Set some thread parameters, so we can be terminated when necessary.
  function_args* functionargs = (function_args*) args;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  functionargs->function(functionargs->keep_running);
  return NULL;
}


void execute(game_func gamefunc)
{
  //Execute in thread, to be able to terminate when behaving not correct.
  pthread_t thread;
  function_args* args = (function_args*) malloc(sizeof(function_args));
  args->function = gamefunc;
  args->keep_running = &keep_running;
  if (keep_running)
  {
    pthread_create(&thread, NULL, run, (void*)args);
    // Wait while thread is running, or has to stop
    while (keep_running and (pthread_kill(thread, 0)==0)) sleep(1);
    pthread_cancel(thread);
    pthread_join(thread, NULL);
  }
  free(args);
}


int main(int argc, const char* argv[])
{
  signal(SIGINT, intHandler);
    
  sensors_initialize(&keep_running);
  motors_initialize();
  command_initialize();
  ball_initialize();
  sleep(5);
  
  if (sensors_is_button_pressed(BUTTON_CODE_LEFT))
  {
    // Run through maze
    execute(maze_execute);
  }
  
  if (sensors_is_button_pressed(BUTTON_CODE_RIGHT))
  {
    // Throw balls
    execute(ball_execute);
  }
  
  ball_terminate();
  command_terminate();
  motors_terminate();
  sensors_terminate(&keep_running);
  
  return 0;
}
