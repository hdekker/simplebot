#!/bin/sh
make clean && make && sshpass -p ' ' scp program root@192.168.0.108:/media/card/program && echo Done!
