#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "sensors.h"
#include "common.h"
#include "command.h"
#include "ball.h"

void test_move(bool* keep_running)
{
  printf("\nTEST_MOVE started\n");
  printf("\n*****************************************************\n");
  printf("UP:     Increase 'speed'\n");
  printf("DOWN:   Decrease 'speed'\n");
  printf("LEFT:   Move backward\n");
  printf("RIGHT:  Move forward\n");
  printf("*****************************************************\n\n");
  
  sensors_clear_buttons_pressed();
  int speed = 50;
  while (*keep_running)
  {
    
    if (sensors_is_button_pressed(BUTTON_CODE_UP))
    {
      // Increase speed
      sensors_clear_buttons_pressed();
      speed += 10; // Increase speed, max value is 100
      speed = (speed > 100) ? 100 : speed;
      printf("new speed is %d\n", speed);
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_DOWN))
    {
      // Decrease speed
      sensors_clear_buttons_pressed();
      speed -= 10; // Decrease speed, min value is 10
      speed = (speed < 10) ? 10 : speed;
      printf("new speed is %d\n", speed);
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_LEFT))
    {
      printf("move 20 cm backward\n");
      sensors_clear_buttons_pressed();
      command_move_distance(200, speed);
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_RIGHT))
    {
      printf("move 20 cm forward\n");
      sensors_clear_buttons_pressed();
      command_move_distance(200, -speed);
    }
    
    sleep_ms(10);
  }
  
  printf("\nTEST_MOVE finished\n\n");
}


void test_rotate(bool* keep_running)
{
  printf("\nTEST_ROTATE started\n");
  printf("\n*****************************************************\n");
  printf("LEFT:   Change 'angle'\n");
  printf("RIGHT:  Change 'speed'\n");
  printf("UP:     Rotate anti clockwise\n");
  printf("DOWN:   Rotate clockwise\n");
  printf("*****************************************************\n\n");
  
  sensors_clear_buttons_pressed();
  int angle = 180;
  int speed = 40;
  while (*keep_running)
  {
    
    if (sensors_is_button_pressed(BUTTON_CODE_LEFT))
    {
      // Change angle
      sensors_clear_buttons_pressed();
      angle += 10; // Change angle, range is 10..360
      angle = (angle > 360) ? 10 : angle;
      printf("new angle is %d\n", angle);
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_RIGHT))
    {
      // Change speed
      sensors_clear_buttons_pressed();
      speed += 10; // Change speed, range is 10..100
      speed = (speed > 100) ? 10 : speed;
      printf("new speed is %d\n", speed);
    }

    if (sensors_is_button_pressed(BUTTON_CODE_UP))
    {
      printf("rotate anti clockwise (angle=%d, speed=%d)\n", angle, speed);
      sensors_clear_buttons_pressed();
      command_turn_angle(-angle, speed);
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_DOWN))
    {
      printf("rotat clockwise (angle=%d, speed=%d)\n", angle, speed);
      sensors_clear_buttons_pressed();
      command_turn_angle(angle, speed);
    }
    
    sleep_ms(10);
  }

  printf("\nTEST_ROTATE finished\n\n");
}


void test_shoot(bool* keep_running)
{
  printf("\nTEST_SHOOT started\n");
  printf("\n*****************************************************\n");
  printf("LEFT:   Catch ball\n");
  printf("RIGHT:  Shoot ball\n");
  printf("UP:     Catch and Shoot, until UP pressed again\n");
  printf("DOWN:   Move until target found\n");
  printf("*****************************************************\n\n");
  
  sensors_clear_buttons_pressed();
  CBallHandler ballhandler;
  CWheelHandler wheelhandler;
  
  while (*keep_running)
  {
    if (sensors_is_button_pressed(BUTTON_CODE_LEFT))
    {
      printf("Try catching ball\n");
      sensors_clear_buttons_pressed();
      COLOR_CODE color;
      bool ball_detected = ballhandler.trycatch(color);
      if (ball_detected)
      {
        printf("\nCaught ball. Color=%d\n\n", color);
      }
      else
      {
        printf("\nNo blue, green, yellow, or red ball. Color=%d\n\n", color);
      }
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_RIGHT))
    {
      printf("Try catching ball\n");
      sensors_clear_buttons_pressed();
      bool shot = ballhandler.tryshoot();
      printf("\nShoot result %d\n\n", shot);
    }

    if (sensors_is_button_pressed(BUTTON_CODE_UP))
    {
      sensors_clear_buttons_pressed();
      while (not sensors_is_button_pressed(BUTTON_CODE_UP))
      {
        COLOR_CODE color;
        bool ball_detected = ballhandler.trycatch(color);
        if (ball_detected)
        {
          printf("Caught ball. Color=%d\n", color);
          if (ballhandler.tryshoot())
          {
            printf("Shot the ball. Bulls eye?\n");
          }
          else
          {
            printf("Shooting failed\n");
          }
        }
        else
        {
          printf("No blue, green, yellow, or red ball. Color=%d\n", color);
        }
        sleep(3);
      }
    }
    
    if (sensors_is_button_pressed(BUTTON_CODE_DOWN))
    {
      sensors_clear_buttons_pressed();
      wheelhandler.move_until_target();
    }
    
    sleep_ms(10);
  }
  
  printf("\nTEST_SHOOT finished\n\n");
}
