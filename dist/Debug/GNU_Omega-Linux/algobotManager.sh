#!/bin/sh
### BEGIN INIT INFO
# Provides:          samba
# Required-Start:  $local_fs $syslog $network
# Required-Stop:   $local_fs $syslog $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start Algobot daemon
### END INIT INFO

## ALGOBOT MANAGER VERSION 19.07.2018

check(){
	
        rm /root/update/*

        status=0

## TRY TO DOWNLOAD MD5 FILE
        echo "Start download MD5 from server..."
        CMD=`wget -P /root/update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`

        if [ $? -eq 0 ];
        then
                echo "- Download MD5 file:  OK"
                                               
	   ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE                      
		cmp  /root/update/algobot_onionomega.md5 /root/algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
       	        if [ $resultat -eq 0 ];                                         
               	then                                                            
                       	echo "Firmware version is last, no update !"            
                       	status=11                                           
                elif [ $resultat -eq 1 ];                                       
       	        then                                                            
               	        echo "New firmware found !"                            
			status=10 
                else                                                            
       	                echo "MD5 file is missing, please update application"
			status=12
               	fi                      
        else
                echo "- Download MD5 : ERROR"
		status=1
        fi
	return $status
}


install(){                                                                                                                                                                  
                                                                                                                                                                            
    echo "Starting firmware upgrade... ";                                                                                                                                   
    killall algobot_onionomega;                                                                                                                                             
    sleep 2;                                                                                                                                                                
    rm /root/algobot/*                                                                                                                                                            
    cp /root/update/* /root/algobot/                                                                                                                                                    
    chmod +x /root/algobot/algobot_onionomega                                                                                                                                     
    echo "Restarting application... ";                                                                                                                                        
    exec /root/algobot/algobot_onionomega                                                                                                                                            
} 

restart(){                                                                              
    killall algobot_onionomega;                                                         
    sleep 2;                                                                            
    echo "Restarting application... ";                                                  
    exec /root/algobot/algobot_onionomega                                                        
} 




update(){                                                                                                                                                      
                                                                                                                                            
## CHECK FOR UPDATE
  	check
	checkResult=$?

	updateResult=0

## TRY TO DOWNLOAD BINARY APPLICATION FILE AND UPDATE
	if [ $checkResult -eq 10 ];
	then

        if [ -f "/root/update/*" ];
        then
                rm /root/update/*
        fi

	## TRY TO DOWNLOAD BINARY                                                                                                                                                    
        	echo "Start download binary firmware from server..."                                                                                                                 
	        CMD=`wget -P /root/update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega`
		if [ $? -eq 0 ];
		then			
			echo "- Download binary file: OK"

			install
			updateResult=0
		else
			echo "- Download binary file: ERROR"
			updateResult=21
		fi
	else
	## FIRMWARE IS UP TO DATE, NO ACTION
		if [ $checkResult -eq 11 ];
		then
			updateResult=$checkResult

		            rm /root/update/*                                                                                    
		else
			updateResult=1
		fi
	fi

	return $updateResult
}         

install_brocker(){
 opkg install mosquitto
 result=$?
 if [ $result -eq 0 ];
 then
  echo "Mosquitto installation OK"
 else
  echo "Mosquitto installation ERROR"
 fi 
 return $result
}

install_npm_ws(){                                                                                                                      
 opkg install npm                                                                                                                 
 result=$?                                                                                                                              
 if [ $result -eq 0 ];                                                                                                                  
 then                                                                                                                                   
  echo "NPM installation OK"                                                                                                      
 else                                                                                                                                   
  echo "NPM installation ERROR"                                                                                                   
 fi                                                                                                                                     
 return $result                                                                                                                         
}

base_install(){                                                                                                                              

## UPDATE PACKAGE                                                                                                                                       
    opkg update
    if [ $? -eq 0 ];
    then
 ## CHECK AND INSTALL MOSQUITTO BROCKER
      echo "Try to install mosquitto... ";                                                                                               
      install_brocker
      if [ $? -eq 0 ];
      then
       echo "Mosquitto successfully installed"
       update
       result=$?
      else
       echo "Error during Mosquitto installation"
      fi

 ## CHECK AND INSTALL NPM
      echo "Try to install NPM... ";
      install_npm_ws
      if [ $? -eq 0 ];                                                                                                                  
      then                                                                                                                              
       echo "NPM installed successfully"                                                                                          
       echo "Installing Websocket gateway..."
       npm install -g ws-tcp-bridge
       if [ $? -eq 0 ];                                                                                                                  
       then                                                                                                                              
         echo "WS gateway successfully installed"                                                                                          
         ws-tcp-bridge --method=ws2tcp --lport=9001 --rhost=127.0.0.1:1883&
	 ws-tcp-bridge --method=tcp2ws --lport=1883 --rhost=ws://127.0.0.1:9001&
       else                                                                                                                              
         echo "Error during WS gateway installation"                                                                                       
       fi 
      else                                                                                                                              
       echo "Error during NPM installation"                                                                                       
      fi
      
    else
     echo "Impossible to update package, please check internet connection !"
     result=0
    fi
    
	echo "Downloading Algobot launcher file..."
	CMD=`wget -P /root/ -q https://github.com/raphaelthurnherr/algobot_onionOmega/blob/master/dist/Debug/GNU_Omega-Linux/algobotLauncher.sh`
	if [ $? -eq 0 ];
	then			
		echo "- Download launcher file: OK"
		echo "Configuring rc.local file"
		sed -i'5 a # CALL THE ALGOBOT LAUNCHER APPLICATION' /etc/rc.local
		sed -i'6 a sh /root/algobotLauncher.sh >> /root/autostartLog.txt 2>&1' /etc/rc.local
		
		$result=$?
	else
		echo "- Download launcher file: ERROR"
		$result=$?
	fi
    
    
    restart
    return $result    
}                                                                                                                                                               

# Test de l'existance des repertoires update et algobot

if [ -d "/root/algobot" ]; then 
 echo "Algobot directory ok"
 if [ -f /root/algobot/algobot_onionomega.md5 ]; then
  echo "MD5 file is present"
 else 
  touch /root/algobot/algobot_onionomega.md5
 fi
else 
 echo "Create directory for application" 
 mkdir /root/algobot
 touch /root/algobot/algobot_onionomega.md5
fi

if [ -d "/root/update" ]; then                                                                                          
 echo "Update directory ok"                                                                                       
else                                                                                                               
 echo "Create directory for update"                                                                           
 mkdir /root/update                                                                                                     
fi 

                                                                                                                                                               
# Execution des parametres recus                                                                                                                               
                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
for param in "$@"                                                                                                                                              
do                                                                                                                                                             
        if [ "$param" = "check" ];                                                                                                                             
        then                   
		check                                                                                                                                
                actionResult=$?                                                                                                                                 
        fi                                                                                                                                                     
                                                                                                                                                               
        if [ "$param" = "update" ];                                                                                                                            
        then                                                                                                                                                   
                update        
		actionResult=$?                                                                                                                                 
        fi                                                                                                                                                     
                                                                                                                                                               
        if [ "$param" = "restart" ]                                                                                                                  
        then                                                                                                                                                   
                restart                                                                                                                                 
        fi                                                                                                                                                     

        if [ "$param" = "install" ]                                                                                                     
        then                                                                                                                            
                base_install                                                                                                                 
		actionResult=$?
        fi
done

exit $actionResult 

