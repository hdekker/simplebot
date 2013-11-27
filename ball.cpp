#include "lms2012.h"
#include "unistd.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
//#include "fastmath.h"
#include "common.h"
#include "command.h"
#include "motors.h"
#include "robot.h"
#include "sensors.h"
#include "ball.h"


void CThrower::calibrate()
{
  printf("Calibrate throw engine start\n");
  motors_set_speed(ROBOT_BALL_THROW_PORT, 20);
  SBYTE motor_speeds[4];
  int sum_motor_speeds = 0;
  for (int i=0; i<4; i++) motor_speeds[i] = 1;
  int i = 0;
  time_t starttime = time(NULL);
  bool timeout = false;
  do
  {
    sleep_ms(100);
    timeout = difftime(time(NULL), starttime) > 4.0; // Open arms shouldn't take more than 4 seconds, otherwise mechanical problems
    motor_speeds[(i++)%4] = motors_get_motor_speed(ROBOT_BALL_THROW_PORT);
    printf("calibrate %d %d %d %d (timeout=%d)\n", motor_speeds[0], motor_speeds[1], motor_speeds[2], motor_speeds[3], timeout);
    sum_motor_speeds = abs(motor_speeds[0]) + abs(motor_speeds[1]) + abs(motor_speeds[2]) + abs(motor_speeds[3]);
  } while ((not timeout) and (sum_motor_speeds > 0));
  motors_reset_angle(ROBOT_BALL_THROW_PORT);
  motors_stop(ROBOT_BALL_THROW_PORT);
  is_down = false;
  is_half_down = false;
  is_up = false;
  printf("Calibrate throw engine done\n");
}
  
void CThrower::up()
{
  if ((not is_up) and (not is_down) and (not is_half_down))
  {
    calibrate();
  }
  
  if (not is_up)
  {
    printf("Throw engine UP start\n");
    int setpoint_angle = (is_down) ? 180 : -180;
    motors_move_to_angle(ROBOT_BALL_THROW_PORT, 30, setpoint_angle, 1);
    printf("Throw engine UP done\n");
  }
  
  is_up = true;
  is_down = false;
  is_half_down = false;
}
  
  
void CThrower::down()
{
  if ((not is_up) and (not is_down) and (not is_half_down))
  {
    calibrate();
  }
  
  if (not is_down)
  {
    printf("Throw engine DOWN start\n");
    int setpoint_angle = (is_half_down) ? -340 : -350;
    motors_move_to_angle(ROBOT_BALL_THROW_PORT, 30, setpoint_angle, 1);
    printf("Throw engine DOWN done\n");
  }
  
  is_up = false;
  is_down = true;
  is_half_down = false;
}


void CThrower::half_down()
{
  if ((not is_up) and (not is_down) and (not is_half_down))
  {
    calibrate();
  }
  
  if (not is_half_down)
  {
    printf("Throw engine HALF_DOWN start\n");
    int setpoint_angle = (is_down) ? 60 : -300;
    motors_move_to_angle(ROBOT_BALL_THROW_PORT, 15, setpoint_angle, 1);
    printf("Throw engine HALF_DOWN done\n");
  }
  
  is_up = false;
  is_down = false;
  is_half_down = true;
}


void CThrower::shoot()
{
  printf("Throw engine SHOOT start\n");
  down();
  printf("SHOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOT");
  motors_set_speed(ROBOT_BALL_THROW_PORT, -100);
  sleep_ms(500);
  motors_stop(ROBOT_BALL_THROW_PORT);
  sleep_ms(100);
  is_up = false;
  is_down = false;
  is_half_down = false;
  printf("Throw engine SHOOT done\n");
}



bool CCatcher::open()
{
  printf("Catch engine OPEN start\n");
  bool is_ok = true;
  if (not is_open)
  {
    int try_count = 0;
    is_ok = false;
    while ((not is_ok) and (try_count++ < 2))
    {
      motors_set_speed(ROBOT_BALL_CATCH_PORT, direction * 30);
      SBYTE motor_speeds[4];
      int sum_motor_speeds = 0;
      for (int i=0; i<4; i++) motor_speeds[i] = 1;
      int i = 0;
      int current_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
      time_t starttime = time(NULL);
      bool timeout = false;
      do
      {
        sleep_ms(100);
        timeout = difftime(time(NULL), starttime) > 3.0; // Open arms shouldn't take more than 3 seconds, otherwise mechanical problems
        motor_speeds[(i++)%4] = motors_get_motor_speed(ROBOT_BALL_CATCH_PORT);
        printf("CCatcher.open %d %d %d %d (timeout=%d)\n", motor_speeds[0], motor_speeds[1], motor_speeds[2], motor_speeds[3], timeout);
        sum_motor_speeds = abs(motor_speeds[0]) + abs(motor_speeds[1]) + abs(motor_speeds[2]) + abs(motor_speeds[3]);
      } while ((not timeout) and (sum_motor_speeds > 0));
      int reached_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
      motors_reset_angle(ROBOT_BALL_CATCH_PORT);
      motors_stop(ROBOT_BALL_CATCH_PORT);
      // is_ok also true when started with half open arms
      int rotation = abs(current_angle - reached_angle);
      is_ok = (timeout) ? false : ((is_closed) ? (rotation > 170) : (rotation > 10));
      if (not is_ok and (try_count == 1))
      {
        printf("Trying to open arms with motor in reverse direction!\n");
        direction *= -1; // Try once more with motor in another direction
        sleep(1);
      }
    }
  }
  is_open = true;
  is_closed = false;
  printf("Catch engine OPEN result %d\n", is_ok);
  return is_ok;
}


bool CCatcher::close()
{
  bool is_ok = true;
  printf("Catch engine CLOSE start\n");
  
  if (not is_open)
  {
    is_ok = open();
  }
  
  if (is_ok)
  {
    int current_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
    motors_move_to_angle(ROBOT_BALL_CATCH_PORT, 80, -direction * 180, 1);
    int reached_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
    if (abs(reached_angle - current_angle) > 170)
    {
      is_open = false;
      is_closed = true;
    }
    else
    {
      is_ok = false;
    }
  }
  printf("Catch engine CLOSE result %d\n", is_ok);
  return is_ok;
}



void CBallHandler::init()
{
  printf("CBallHandler.init\n");
  printf("\nOPEN\n"); open();
  printf("\nUP\n"); up();
  printf("\nDOWN\n"); down();
  printf("\nCLOSE\n"); close();
}


bool CBallHandler::trycatch(COLOR_CODE& color)
{
  printf("trycatch start\n");
  color = COLOR_CODE_TRANSPARENT;
  bool success = false;
  down();
  for (int i=0; i<3 and not success; i++)
  {
    success = open();
    success = close() and success;
    color = sensors_get_color(ROBOT_COLOR_SENSOR_PORT);
    success = success and (color==COLOR_CODE_BLUE or color==COLOR_CODE_GREEN or 
                            color==COLOR_CODE_YELLOW or color==COLOR_CODE_RED);
  }
  printf("trycatch done (success=%d, color=%d)\n", success, color);
  return success;
}


bool CBallHandler::tryshoot()
{
  printf("tryshoot start\n");
  bool retval = false;
  
  // Shoot
  if (open())
  {
    retval = true;
    shoot();
    down();
    close();
  }
  
  printf("tryshoot done\n");
  return retval;
}


void CWheelHandler::move_until_collision()
{
  printf("move_until_collision start\n");
  time_t starttime = time(NULL);
  time_t currenttime;
  int elapsed = 0;
  int distance = 0;
  int percentage = 0;
  int velocity = -40;
  int actual_velocity_left = 0;
  int actual_velocity_right = 0;
  motors_set_speed(ROBOT_WHEEL_RIGHT_PORT, velocity);
  motors_set_speed(ROBOT_WHEEL_LEFT_PORT, velocity);
  do
  {
    sleep(1);
    distance = sensors_get_us_distance_mm(ROBOT_ULTRASONIC_SENSOR_PORT);
    percentage = sensors_get_ir_distance(ROBOT_INFRARED_SENSOR_PORT);
    actual_velocity_right = motors_get_motor_speed(ROBOT_WHEEL_RIGHT_PORT);
    actual_velocity_left = motors_get_motor_speed(ROBOT_WHEEL_LEFT_PORT);
    currenttime = time(NULL);
    elapsed = static_cast<int>(difftime(currenttime, starttime));
    printf("move_until_collision dist=%d, perc=%d, velocity=%d, left=%d, right=%d, elapsed=%d\n", distance, percentage, velocity, actual_velocity_left, actual_velocity_right, elapsed);
  } while ((elapsed < 30) and /*((distance<120) or (distance>180)) and*/ ((abs(actual_velocity_left)>0) or (abs(actual_velocity_right)>0)));
  motors_stop(ROBOT_WHEEL_RIGHT_PORT);
  motors_stop(ROBOT_WHEEL_LEFT_PORT);
  sleep(1);
  printf("move_until_collision done\n");
}
  
  
bool CWheelHandler::is_target_collision()
{
  
  // Drive a little bit backward, and turn, with sensors along target
  command_move_distance(100, 50);
  command_turn_angle(-90, 50);
  int distance = sensors_get_us_distance_mm(ROBOT_ULTRASONIC_SENSOR_PORT);
  bool is_target = ((distance > 260) and (distance < 340));
  printf("\n\nis_target_collision=%d, distance=%d\n\n", is_target, distance);
  return is_target;
}


void CWheelHandler::move_until_target()
{
  printf("move_until_target start\n");
  bool reached_target = false;
  while (not reached_target)
  {
    move_until_collision();
    if (is_target_collision())
    {
      command_turn_angle(90, 50);
      command_move_distance(200, 50);
      reached_target = true;
    }
    else
    {
      command_turn_angle(90, 50);
      command_move_distance(200, 50);
      command_turn_angle(135, 50);
    }
  }
  printf("move_until_target done\n");
}


static CBallHandler* ballhandler = NULL;
static CWheelHandler* wheelhandler = NULL;


void ball_initialize()
{
  ballhandler = new CBallHandler();
  wheelhandler = new CWheelHandler();
  ballhandler->init();
}


void ball_terminate()
{
  delete wheelhandler;
  delete ballhandler;
  motors_stop(ROBOT_WHEEL_RIGHT_PORT);
  motors_stop(ROBOT_WHEEL_LEFT_PORT);
  motors_stop(ROBOT_BALL_THROW_PORT);
  motors_stop(ROBOT_BALL_CATCH_PORT);
}


void ball_execute(bool* keep_running)
{
  printf("\nBALL started\n\n");
  COLOR_CODE color;
  
  /*
  ballhandler->open();
  sleep(5);
  printf("\n\nHALF");
  ballhandler->half_down();
  sleep(5);
  printf("\n\nDOWN");
  ballhandler->down();
  sleep(5);
  printf("\n\nHALF");
  ballhandler->half_down();
  sleep(5);
  printf("\n\nUP");
  ballhandler->up();
  sleep(5);
  printf("\n\nHALF");
  ballhandler->half_down();
  sleep(5);
  printf("\n\nUP");
  ballhandler->up();
  sleep(5);
  printf("\n\nDOWN");
  ballhandler->down();
  sleep(5);
  ballhandler->close();
  sleep(5);
  */
  
  while(1)
  {
    wheelhandler->move_until_target();
    command_turn_angle(180, 50);
    ballhandler->open();
    ballhandler->half_down();
    command_move_distance(300, 40);
    
    if (ballhandler->trycatch(color))
    {
      int angle;
      switch (color)
      {
        case COLOR_CODE_GREEN:
          angle = 180;
          break;
        case COLOR_CODE_YELLOW:
          angle = 170;
          break;
        case COLOR_CODE_RED:
          angle = 160;
          break;
        case COLOR_CODE_BLUE:
          angle = 150;
          break;
        default:
          angle = 180;
      }
      if (ballhandler->open())
      {
        // Raise bar a bit, otherwise get stuck when driving backward
        ballhandler->half_down();
      }
      // Drive a little bit backward
      command_move_distance(-200, 50);
      command_turn_angle(angle, 50);
      
      ballhandler->tryshoot();
      
      // turn with side (and ultrasonic sensor) along target
      command_turn_angle(90-angle, 50);
      // move 200mm further
      command_move_distance(-200, 50);
      // turn with front (IR sensor) to target
      command_turn_angle(90, 50);
    }
    else // No ball
    {
      if (ballhandler->open())
      {
        // Raise bar a bit, otherwise get stuck when driving backward
        ballhandler->half_down();
      }
      // Drive a little bit backward
      command_move_distance(-200, 50);
  
      // turn with side (and ultrasonic sensor) along target
      command_turn_angle(-90, 50);
      // move 200mm further
      command_move_distance(-200, 50);
      // turn with front (IR sensor) to target
      command_turn_angle(90, 50);
    }
  }
  
  printf("\nBALL finished\n\n");
}
