#!/bin/sh
### BEGIN INIT INFO
# Provides:          samba
# Required-Start:  $local_fs $syslog $network
# Required-Stop:   $local_fs $syslog $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start Algobot daemon
### END INIT INFO

start()
{
     # Mettez ici le d..marrage de votre programme
        mosquitto&
        sleep 1;
        ws-tcp-bridge --method=ws2tcp --lport=9001 --rhost=127.0.0.1:1883&
        sleep 1;
        ws-tcp-bridge --method=tcp2ws --lport=1883 --rhost=ws://127.0.0.1:9001&
        #sleep 1;
        ./root/algobot/algobot_onionomega&
		sleep 10;
 		./root/algobotmanager&
		
        echo "ALGOBOT LAUNCHER -> Algobot application started !"
}
 
stop()
{
     # Mettez ici un kill sur votre programme
        killall algobot_onionomega
        echo "ALGOBOT LAUNCHER -> Algobot application stopped !"
}
 
 
restart()
{
    stop;
    sleep 2;
    start;
}
case $1 in
start) 
      start;;
stop)
      stop;;
restart)
      restart;;
*)
      start;;
esac