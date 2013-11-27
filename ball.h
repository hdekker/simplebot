#ifndef BALL_H_
#define BALL_H_

void ball_initialize();
void ball_terminate();
void ball_execute(bool* keep_running);


class CThrower
{
  bool is_down = false;
  bool is_half_down = false;
  bool is_up = false;
private:
  void calibrate();
public:
  void up();
  void down();
  void half_down();
  void shoot();
};


class CCatcher
{
  bool is_open = false;
  bool is_closed = false;
  int direction = -1; // must have value '1' or '-1'
public:
  bool open();
  bool close();
};


class CBallHandler: public CThrower, public CCatcher
{
public:
  void init();
  bool trycatch(COLOR_CODE& color);
  bool tryshoot();
};


class CWheelHandler
{
private:
  void move_until_collision();
  bool is_target_collision();
  
public:
  void move_until_target();
};

#endif /* BALL_H_ */
