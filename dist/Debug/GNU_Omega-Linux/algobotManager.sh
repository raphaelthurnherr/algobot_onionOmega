#!/bin/sh
### BEGIN INIT INFO
# Provides:          samba
# Required-Start:  $local_fs $syslog $network
# Required-Stop:   $local_fs $syslog $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start Algobot daemon
### END INIT INFO

# ALGOBOT MANAGER VERSION 19.07.2018

checkApp(){
	
        rm /root/update/*

        status=0

## TRY TO DOWNLOAD MD5 FILE
        echo "Start download application MD5 from server..."
        CMD=`wget -P /root/update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`

        if [ $? -eq 0 ];
        then
                echo "- Download MD5 application file:  OK"
                                               
	   ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE                      
		cmp  /root/update/algobot_onionomega.md5 /root/algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
       	        if [ $resultat -eq 0 ];                                         
               	then                                                            
                       	echo "Algobot application firmware version is last, no update !"            
                       	status=11                                           
                elif [ $resultat -eq 1 ];                                       
       	        then                                                            
               	        echo "New application firmware found !"                            
			status=10 
                else                                                            
       	                echo "Algobot application MD5 file is missing, please update application"
			status=12
               	fi                      
        else
                echo "- Download application MD5 : ERROR"
		status=1
        fi
	return $status
}

checkManager(){
	
        rm /root/update/*

        status=0

## TRY TO DOWNLOAD MD5 FILE
        echo "Start download manager application MD5 from server..."
        CMD=`wget -P /root/update/ -q https://github.com/raphaelthurnherr/algobot_manager/blob/master/dist/Debug/GNU_Omega-Linux/algobotmanager.md5`

        if [ $? -eq 0 ];
        then
                echo "- Download MD5 manager file:  OK"
                                               
	   ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE                      
		cmp  /root/update/algobotmanager.md5 /root/algobotmanager.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
       	        if [ $resultat -eq 0 ];                                         
               	then                                                            
                       	echo "Algobot manager firmware version is last, no update !"            
                       	status=11                                           
                elif [ $resultat -eq 1 ];                                       
       	        then                                                            
               	        echo "New manager firmware found !"                            
			status=10 
                else                                                            
       	                echo "Algobot manager MD5 file is missing, please update application"
			status=12
               	fi                      
        else
                echo "- Download manager MD5 : ERROR"
		status=1
        fi
	return $status
}


install_update_app(){                                                                                                                                                                  
                                                                                                                                                                            
    echo "Starting application firmware upgrade... ";                                                                                                                                   
    killall algobot_onionomega;                                                                                                                                             
    sleep 1;                                                                                                                                                                
    rm /root/algobot/*                                                                                                                                                           
    cp /root/update/algobot_onion* /root/algobot/                                                                                                                                                    
    chmod +x /root/algobot/algobot_onionomega                                                                                                                                     
} 

install_update_manager(){                                                                                                                                                                  
                                                                                                                                                                            
    echo "Starting manager firmware upgrade... ";                                                                                                                                   
    killall algobotmanager;                                                                                                                                             
    sleep 1;                                                                                                                                                                
    rm /root/algobotmanager*                                                                                                                                                       
    cp /root/update/algobotmanager* /root/                                                                                                                                                    
    chmod +x /root/algobotmanager                                                                                                                                   
} 


restart(){                                                                              
    killall algobot_onionomega;
    killall algobotmanager;
    sleep 2;                                                                            
    echo "Restarting application and manager... ";

    cd /root/
    ./algobotmanager&
    
    cd /root/algobot/
    ./algobot_onionomega&
} 




updateApp(){                                                                                                                                                      
                                                                                                                                            
## CHECK FOR UPDATE
  	checkApp
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
			echo "- Download binary application file: OK"

			install_update_app
			restart
			updateResult=0
		else
			echo "- Download binary application file: ERROR"
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

updateMan(){                                                                                                                                                      
                                                                                                                                            
## CHECK FOR UPDATE
  	checkManager
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
        	echo "Start download binary manager firmware from server..."                                                                                                                 
	        CMD=`wget -P /root/update/ -q https://github.com/raphaelthurnherr/algobot_manager/raw/master/dist/Debug/GNU_Omega-Linux/algobotmanager`
		if [ $? -eq 0 ];
		then			
			echo "- Download binary manager file: OK"

			install_update_manager
			restart
			updateResult=0
		else
			echo "- Download binary manager file: ERROR"
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
 return $result
}

install_npm_ws(){                                                                                                                      
 opkg install npm                                                                                                                 
 result=$?                                                                                                                                                                                                                                                    
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
    
	echo "Configuring RC launcher file..."
	#CMD=`wget -P /root/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobotLauncher.sh`
	if [ $? -eq 0 ];
	then			
		echo "- Configuring rc.local file"
		sed -i '3 a # CALL THE ALGOBOT LAUNCHER APPLICATION' /etc/rc.local
                sed -i '4 a ws-tcp-bridge --method=ws2tcp --lport=9001 --rhost=127.0.0.1:1883&' /etc/rc.local
                sed -i '5 a ws-tcp-bridge --method=tcp2ws --lport=1883 --rhost=ws://127.0.0.1:9001&' /etc/rc.local
                sed -i '6 a sh /root/algobotManager.sh restart >> /root/autostartLog.txt 2>&1' /etc/rc.local

		
		$result=$?
	else
		echo "- Configuring launcher file: ERROR"
		$result=$?
	fi
    
    
    echo "- Adding algobot files to root..."
    # Update the manager bash file    
    rm /root/algobotManager.sh  
    cp /tmp/mounts/SD-P1/algobotManager.sh /root/

    # Update the manager application
    rm /root/algobotmanager
    rm /root/algobotmanager.md5
    cp /tmp/mounts/SD-P1/bin/algobotmanager /root/
    cp /tmp/mounts/SD-P1/bin/algobotmanager.md5 /root/

    # Update the algobot application
    rm /root/algobot/algobot_onionomega.md5
    rm /root/algobot/algobot_onionomega
    cp /tmp/mounts/SD-P1/bin/algobot/algobot_onionomega /root/algobot
    cp /tmp/mounts/SD-P1/bin/algobot/algobot_onionomega.md5 /root/algobot

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

 if [ -f /root/algobotmanager.md5 ]; then
  echo "MD5 file is present"
 else 
  touch /root/algobotmanager.md5
 fi
                                                                                                                                                               
# Execution des parametres recus                                                                                                                               
                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
for param in "$@"                                                                                                                                              
do                                                                                                                                                             
        if [ "$param" = "check" ];                                                                                                                             
        then                   
		checkApp                                                                                                                                
                actionResult=$?                                                                                                                                 
        fi
        
        if [ "$param" = "checkman" ];                                                                                                                             
        then                   
		checkManager                                                                                                                                
                actionResult=$?                                                                                                                                 
        fi 
                                                                                                                                                               
        if [ "$param" = "update" ];                                                                                                                            
        then                                                                                                                                                   
                updateApp        
		actionResult=$?                                                                                                                                 
        fi                                                                                                                                                     
          
        if [ "$param" = "updateman" ];                                                                                                                            
        then                                                                                                                                                   
                updateMan        
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

echo "End of installation algobot and dependecies"
exit $actionResult 

