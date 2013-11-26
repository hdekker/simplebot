simplebot
=========

Lego EV3 robot

Instructions:

WiFi Router
- Switch on, it is configured already
- The USB WiFi dongles get IP addresses 192.168.3.197 or 192.168.3.198

EV3 (start WiFi)
- EV3 menu most right tab (configuration)
- with down-button nagivate to WiFi
- press center-button
- with down-button nagivate to WiFi again
- press center-button again to activate WiFi
- select "Connections", and select the "AWESOME" wifi router.
- often line above must be done twice to see wifi routers in the list

EV3 (setup ssh, to communication between laptop and EV3)
- "telnet 192.168.3.197" (or 192.168.3.198 if other WiFi USB dongle)
- leave password empty (root has not password)
- type "dropbear" to start ssh server

LAPTOP (test connection between laptop and EV3)
- type 'ssh root@192.168.3.197'
- again leave password for root empty
- you should get a "root@EV3:~#" prompt, then you successfully setup the connection

LAPTOP (build and run)
- Login as user 'lego' (password is also 'lego')
- Go to directory /home/lego/simplebot
- type commands:
  To clean old files:
  % make clean
  To build all files:
  % make
  To copy program to EV3:
  % sshpass -p ' ' scp program root@192.168.3.197:/media/card/program
  To run program at EV3:
  % sshpass -p ' ' ssh root@192.168.3.197:/media/card/program

  Note: all these commands can be done at once by typing "./lego.sh"

