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

class CThrower
{
  bool is_down = false;
  bool is_half_down = false;
  bool is_up = false;
  
private:
  void calibrate()
  {
    printf("Calibrate throw engine start\n");
    motors_set_speed(ROBOT_BALL_THROW_PORT, 30);
    SBYTE motor_speeds[4];
    for (int i=0; i<4; i++) motor_speeds[i] = 1;
    int i = 0;
    do
    {
      sleep_ms(100);
      motor_speeds[(i++)%4] = motors_get_motor_speed(ROBOT_BALL_THROW_PORT);
      printf("%d %d %d %d\n", motor_speeds[0], motor_speeds[1], motor_speeds[2], motor_speeds[3]);
    } while ((abs(motor_speeds[0]) + abs(motor_speeds[1]) + abs(motor_speeds[2]) + abs(motor_speeds[3])) > 0);
    motors_reset_angle(ROBOT_BALL_THROW_PORT);
    motors_stop(ROBOT_BALL_THROW_PORT);
    is_down = false;
    is_half_down = false;
    is_up = false;
    printf("Calibrate throw engine done\n");
  }
  
public:
  
  void up()
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
  
  void down()
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
  
  void half_down()
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
  
  void shoot()
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
};




class CCatcher
{
  bool is_open = false;
  bool is_closed = false;
  
public:
  
  bool open()
  {
    printf("Catch engine OPEN start\n");
    bool is_ok = true;
    if (not is_open)
    {
      motors_set_speed(ROBOT_BALL_CATCH_PORT, 30);
      SBYTE motor_speeds[4];
      for (int i=0; i<4; i++) motor_speeds[i] = 1;
      int i = 0;
      int current_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
      do
      {
        sleep_ms(100);
        motor_speeds[(i++)%4] = motors_get_motor_speed(ROBOT_BALL_CATCH_PORT);
        printf("CCatcher.open %d %d %d %d\n", motor_speeds[0], motor_speeds[1], motor_speeds[2], motor_speeds[3]);
      } while ((abs(motor_speeds[0]) + abs(motor_speeds[1]) + abs(motor_speeds[2]) + abs(motor_speeds[3])) > 0);
      int reached_angle = motors_get_angle(ROBOT_BALL_CATCH_PORT);
      motors_reset_angle(ROBOT_BALL_CATCH_PORT);
      motors_stop(ROBOT_BALL_CATCH_PORT);
      // is_ok also true when started with half open arms
      is_ok = (is_closed) ? (abs(current_angle - reached_angle) > 170) : true;
    }
    is_open = true;
    is_closed = false;
    printf("Catch engine OPEN result %d\n", is_ok);
    return is_ok;
  }
  
  bool close()
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
      motors_move_to_angle(ROBOT_BALL_CATCH_PORT, 80, -180, 1);
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
};



class CBallHandler: public CThrower, public CCatcher
{
  
public:
  CBallHandler()
  {
    open();
    up();
    down();
    close();
  }
  
  bool trycatch(COLOR_CODE& color)
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
};




class CWheelHandler
{
private:
  void move_until_collision()
  {
    printf("move_until_collision start\n");
    time_t starttime = time(NULL);
    time_t currenttime;
    int elapsed = 0;
    int distance = 0;
    int percentage = 0;
    int velocity = -45;
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
  
  
  bool is_target_collision()
  {
    
    // Drive a little bit backward, and turn, with sensors along target
    move_and_turn(200, -90);
    int distance = sensors_get_us_distance_mm(ROBOT_ULTRASONIC_SENSOR_PORT);
    bool is_target = ((distance > 160) and (distance < 240));
    printf("\n\nis_target_collision=%d, distance=%d\n\n", is_target, distance);
    //move_and_turn(0, 90);
    //move_and_turn(-200, 0);
    return is_target;
  }
  
public:
  void move_until_target()
  {
    printf("move_until_target start\n");
    //int distance = 0;
    //int percentage = 0;
    bool reached_target = false;
    while (not reached_target)
    {
      move_until_collision();
      if (is_target_collision())
      {
        reached_target = true;
      }
      else
      {
        //motors_set_speed(ROBOT_WHEEL_RIGHT_PORT, 60);
        //motors_set_speed(ROBOT_WHEEL_LEFT_PORT, 60);
        //sleep(2);
        //motors_stop(ROBOT_WHEEL_RIGHT_PORT);
        //motors_stop(ROBOT_WHEEL_LEFT_PORT);
        sleep(1);
        command_turn_angle(-45);
      }
    }
    printf("move_until_target done\n");
  }

  void move_and_turn(int distance_mm, int degrees)
  {
    printf("move_and_turn start (distance=%d, angle=%d)\n", distance_mm, degrees);
    // Drive backward (positive velocity is moving in direction of shooter)
    if (distance_mm != 0)
    {
      int velocity = (distance_mm>0) ? -60 : 60;
      motors_set_speed(ROBOT_WHEEL_RIGHT_PORT, velocity);
      motors_set_speed(ROBOT_WHEEL_LEFT_PORT, velocity);
      sleep_ms((abs(distance_mm) * 1000)/ 60);
      motors_stop(ROBOT_WHEEL_RIGHT_PORT);
      motors_stop(ROBOT_WHEEL_LEFT_PORT);
      sleep(1);
    }
    if (degrees != 0)
    {
      command_turn_angle(degrees);
    }
    printf("move_and_turn done (distance=%d, angle=%d)\n", distance_mm, degrees);
  }
};

static CBallHandler* ballhandler = NULL;
static CWheelHandler* wheelhandler = NULL;

void ball_initialize()
{
  ballhandler = new CBallHandler();
  wheelhandler = new CWheelHandler();
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
  printf("ball execute start\n");
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
    wheelhandler->move_and_turn(0, -90);
    wheelhandler->move_and_turn(100, 0);
    ballhandler->open();
    ballhandler->half_down();
    wheelhandler->move_and_turn(-200, 0);
    
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
      // Drive a little bit backward, and turn
      wheelhandler->move_and_turn(200, angle);
      // Shoot
      if (ballhandler->open())
      {
        ballhandler->shoot();
      }
      ballhandler->down();
      ballhandler->close();
      // turn with sensors along target
      wheelhandler->move_and_turn(0, 90 - angle);
    }
    else // No ball
    {
      if (ballhandler->open())
      {
        // Raise bar a bit, otherwise get stuck when driving backward
        ballhandler->half_down();
      }
      // Drive a little bit backward, and turn, with sensors along target
      wheelhandler->move_and_turn(200, 90);
    }
  
    // Position robot with sensors pointing to target
    wheelhandler->move_and_turn(200, 90);
  }
  
  printf("ball execute done\n");
  return;
}
