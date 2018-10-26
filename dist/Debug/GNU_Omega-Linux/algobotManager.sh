#!/bin/sh
### BEGIN INIT INFO
# Provides:          samba
# Required-Start:  $local_fs $syslog $network
# Required-Stop:   $local_fs $syslog $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start Algobot daemon
### END INIT INFO

# ALGOBOT MANAGER VERSION 24.10.2018

##-------------------------------------------------------------
## checkApp()
## This function will download the MD5 file of application firmware on the server
## and check (with comparison) if the algobot application firmware is up to date.
## A report is made to the user WITHOUT update !
##
## RETURN VALUES:
## -  1 - ERROR, Impossible to download the MD5 File
## - 10 - New firmware available
## - 11 - No new firware available
## - 12 - No local MD5 file found, impossible to compare the versions
##-------------------------------------------------------------

checkApp(){	
        # Clean the update directory
        rm /root/update/*
        status=0

# TRY TO DOWNLOAD MD5 FILE FROM SERVER
        echo "Start download application MD5 from server..."
        CMD=`wget -P /root/update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`

        if [ $? -eq 0 ];
# MD5 FILE OF THE APPLICATION WAS DOWNLOADED SUCCESSFULLY
        then
                echo "- Download MD5 application file:  OK"
                
        # COMPARE THE MD5 FILE OF THE LOCAL APPLICATION AND THE MD5 FILE ON THE SERVER
        # IF THE FILES ARE THE SAME, NO UPDATE AVAILABLE
		cmp  /root/update/algobot_onionomega.md5 /root/algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
       	        if [ $resultat -eq 0 ];                                         
               	then                                                            
                # MD5 FILE ARE THE SAME, NO UPDATE TO DO
                       	echo "Algobot application firmware version is last, no update found!"            
                       	status=11                       
                elif [ $resultat -eq 1 ];                                       
       	        then                                    
                # MD5 FILE ARE THE DIFFERENT, NEW FIRMWARE FOUND
               	        echo "New application firmware found !"                            
			status=10 
                else                                                            
       	                echo "Algobot application MD5 file is missing, please update application"
			status=12
               	fi                      
        else
# IMPOSSIBLE TO DOWNLOAD THE MD5 OF APPLICATION FILE
                echo "- Download application MD5 : ERROR"
		status=1
        fi
	return $status
}


##-------------------------------------------------------------
## checkManager()
## This function will download the MD5 file of manager application on the server
## and check (with comparison) if the manager application firmware is up to date.
## A report is made to the user WITHOUT update !
##
## RETURN VALUES:
## -  1 - ERROR, Impossible to download the MD5 File
## - 10 - New manager version available
## - 11 - No new manager version available
## - 12 - No local MD5 file found, impossible to compare the versions
##-------------------------------------------------------------
checkManager(){
        # Clean the update directory
        rm /root/update/*
        status=0

# TRY TO DOWNLOAD MD5 FILE OF MANAGER BINARY FROM SERVER
        echo "Start download manager application MD5 from server..."
        CMD=`wget -P /root/update/ -q https://github.com/raphaelthurnherr/algobot_manager/blob/master/dist/Debug/GNU_Omega-Linux/algobotmanager.md5`

        if [ $? -eq 0 ];
# MD5 FILE OF THE APPLICATION WAS DOWNLOADED SUCCESSFULLY
        then
                echo "- Download MD5 manager file:  OK"
                                               
        # COMPARE THE MD5 FILE OF THE LOCAL APPLICATION AND THE MD5 FILE ON THE SERVER
        # IF THE FILES ARE THE SAME, NO UPDATE AVAILABLE                     
		cmp  /root/update/algobotmanager.md5 /root/algobotmanager.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
       	        if [ $resultat -eq 0 ];                                         
               	then                     
                # MD5 FILE ARE THE SAME, NO UPDATE TO DO
                       	echo "Algobot manager firmware version is last, no update !"            
                       	status=11                                           
                elif [ $resultat -eq 1 ];                                       
       	        then                     
                # MD5 FILE ARE THE DIFFERENT, NEW FIRMWARE FOUND
               	        echo "New manager firmware found !"                            
			status=10 
                else                                                            
       	                echo "Algobot manager MD5 file is missing, please update application"
			status=12
               	fi                      
        else
# IMPOSSIBLE TO DOWNLOAD THE MD5 OF MANAGER       
                echo "- Download manager MD5 : ERROR"
		status=1
        fi
	return $status
}

##-------------------------------------------------------------
## install_update_app()
## This function will copy the downloaded application file in the
##  update directory to the application directory
##
## RETURN VALUES: ------
##-------------------------------------------------------------

install_update_app(){                                                                                                                                                                                                                                                                                                            
    echo "Starting application firmware upgrade... ";   
# KILL THE APPLICATION PROCESSUS
    killall algobot_onionomega;                                                                                                                                             
    sleep 1;                    
# REMOVE THE OLD BINARY
    rm /root/algobot/* 
# COPY THE NEW BINARY FILE
    cp /root/update/algobot_onion* /root/algobot/                                                                                                                                                    
# APPLY THE EXECUTION RIGHTS
    chmod +x /root/algobot/algobot_onionomega                                                                                                                                     
} 

##-------------------------------------------------------------
## install_update_MANAGER()
## This function will copy the downloaded manager application file in the
##  update directory to the root directory
##
## RETURN VALUES: ------
##-------------------------------------------------------------
install_update_manager(){   
    echo "Starting manager firmware upgrade... ";                                                                                                                                   
# KILL THE APPLICATION PROCESSUS
    killall algobotmanager;                                                                                                                                             
    sleep 1;                    
# REMOVE THE OLD BINARY
    rm /root/algobotmanager*                
# COPY THE NEW BINARY FILE
    cp /root/update/algobotmanager* /root/     
# APPLY THE EXECUTION RIGHTS    
    chmod +x /root/algobotmanager                                                                                                                                   
} 

##-------------------------------------------------------------
## restart()
## This function will kill the application and manager processus
## and restart it.
## RETURN VALUES: ------
##-------------------------------------------------------------
restart(){                                                                              
    killall algobot_onionomega;
    killall algobotmanager;
    sleep 2;                                                                            
    echo "Restarting algobot application and manager process... ";

    cd /root/
    ./algobotmanager&
    
    cd /root/algobot/
    ./algobot_onionomega&
} 

##-------------------------------------------------------------
## updateApp()
## This function will call the UPDATE CHECK function and try to 
## DOWNLOAD the binary file before INSTALL ti
##
## RETURN VALUES:
## -  0 - FIRMWARE INSTALLATION SUCCESS
## -  1 - NO UPDATE AVAILABLE, RETURN THE STATUS CODE
## - 21 - ERROR, IMPOSSIBLE TO DOWNLOAD APPLICATION FILE
## - OTHER - SEE CHECKMANAGER FUNCTION STATUS
##-------------------------------------------------------------

updateApp(){                                                                                                                                                      
                                                                                                                                            
# DOWNLOAD & CHECK FOR UPDATE
  	checkApp
	checkResult=$?
	updateResult=0

# TEST IF NEW FIRMWARE AVAILABLE
	if [ $checkResult -eq 10 ];
	then
# NEW FIRWARE AVAILABLE, CLEAN THE UPDATE DIRECTORY
                if [ -f "/root/update/*" ];
                then
                        rm /root/update/*
                fi

	# TRY TO DOWNLOAD BINARY FROM SERVER                                                                                                                                                  
        	echo "Start download binary firmware from server..."                                                                                                                 
	        CMD=`wget -P /root/update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega`
		if [ $? -eq 0 ];
		then
                # SUCCESSFULLY DOWNLOAD
			echo "- Download binary application file: OK"
                        # INSTALL THE UPDATES FROM UPDATE DIRECTORY
			install_update_app
			restart
			updateResult=0
		else
                # ERROR, IMPOSSIBLE TO DOWNLOAD APPLICATION FILE
			echo "- Download binary application file: ERROR"
			updateResult=21
		fi
	else
        # NO UPDATE AVAILABLE, CHECK THE STATUS CODE 
		if [ $checkResult -eq 11 ];
		then
                        # NO UPDATE AVAILABLE, RETURN THE STATUS CODE
			updateResult=$checkResult
                        # CLEAN THE UPDATE DIRECTORY
                        rm /root/update/*                                                                                    
		else
                        # NO UPDATE AVAILABLE, RETURN THE STATUS CODE           
			updateResult=1
		fi
	fi
        # RETURN THE STATUS CODE
	return $updateResult
}         

##-------------------------------------------------------------
## updateManager()
## This function will call the UPDATE CHECK function and try to 
## DOWNLOAD the binary file of the manager before INSTALL it
##
## RETURN VALUES:
## -  0 - FIRMWARE INSTALLATION SUCCESS
## -  1 - NO UPDATE AVAILABLE, RETURN THE STATUS CODE
## - 21 - ERROR, IMPOSSIBLE TO DOWNLOAD APPLICATION FILE
## - OTHER - SEE CHECKMANAGER FUNCTION STATUS
##-------------------------------------------------------------
updateManager(){                                                                                                                                                      
                                                                                                                                            
# DOWNLOAD & CHECK FOR UPDATE
  	checkManager
	checkResult=$?

	updateResult=0

# TEST IF NEW MANAGER APPLICATION FILE AVAILABLE
	if [ $checkResult -eq 10 ];
	then
        # NEW MANAGER AVAILABLE, CLEAN THE UPDATE DIRECTORY
        if [ -f "/root/update/*" ];
        then
                rm /root/update/*
        fi

	# TRY TO DOWNLOAD BINARY FROM SERVER                                                                                                                                                     
        	echo "Start download binary manager firmware from server..."                                                                                                                 
	        CMD=`wget -P /root/update/ -q https://github.com/raphaelthurnherr/algobot_manager/raw/master/dist/Debug/GNU_Omega-Linux/algobotmanager`
		if [ $? -eq 0 ];
		then
                # SUCCESSFULLY DOWNLOAD
			echo "- Download binary manager file: OK"

			install_update_manager
			restart
			updateResult=0
		else
                # ERROR, IMPOSSIBLE TO DOWNLOAD APPLICATION FILE
			echo "- Download binary manager file: ERROR"
			updateResult=21
		fi
	else
        # NO UPDATE AVAILABLE, CHECK THE STATUS CODE 
		if [ $checkResult -eq 11 ];
		then
                # NO UPDATE AVAILABLE, RETURN THE STATUS CODE
			updateResult=$checkResult
                # CLEAN THE UPDATE DIRECTORY
                        rm /root/update/*                                                                                    
		else
                # NO UPDATE AVAILABLE, RETURN THE STATUS CODE  
			updateResult=1
		fi
	fi
        # RETURN THE STATUS CODE
	return $updateResult
}         


##-------------------------------------------------------------
## install_brocker()
## This function will install the MQTT mosquitto brocker on the system
##
## RETURN VALUES:
## "opkg status code"
##-------------------------------------------------------------
install_brocker(){
 opkg install mosquitto
 result=$?
 return $result
}

##-------------------------------------------------------------
## install_npm_ws()
## This function will install the NPM installer
## It will be necessary for install the bridge between Mosquitto TCP socket and the Browser WebSocket
##
## RETURN VALUES:
## "opkg status code"
##-------------------------------------------------------------
install_npm_ws(){                                                                                                                      
 opkg install npm                                                                                                                 
 result=$?                                                                                                                                                                                                                                                    
 return $result                                                                                                                         
}

##-------------------------------------------------------------
## base_install()
## This function will install the base applications requiered by the Algobot system.
## It will also configure the configuration file for an autostart
##
## Applications to install
## - Linux update
## - Install the mosquitto brocker
## - Install the NPM application
## -- Install the NPM Websocket/TCP bridge
## -- Configuring etc/rc.local for launch TCP/WS Bridge and algobotManager.sh (this bashfile file)
## - Update the manager application file from SDCARD 
## - Update the algobot application file from SDCARD
## - Update the algobotManager.sh bashfile from SDCARD (This file)
## - Configuring / Copying the WEB APP FILE to EEPROM
##
## RETURN VALUES:
## ---
##-------------------------------------------------------------
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
       npm install -g ws-tcp-bridge ws-tcp-bridge
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
		echo "- Download algobotLaucher and Configuring rc.local file"
		sed -i '3 a # CALL THE ALGOBOT LAUNCHER APPLICATION' /etc/rc.local
                sed -i '4 a ws-tcp-bridge --method=ws2tcp --lport=9001 --rhost=127.0.0.1:1883&' /etc/rc.local
                sed -i '5 a ws-tcp-bridge --method=tcp2ws --lport=1883 --rhost=ws://127.0.0.1:9001&' /etc/rc.local
                sed -i '6 a sh /root/algobotManager.sh restart >> /root/autostartLog.txt 2>&1' /etc/rc.local
		$result=$?
    
    echo "- Adding algobot files to root..."
    # Update the manager bash file from SDCARD
    rm /root/algobotManager.sh  
    cp /tmp/mounts/SD-P1/bin/algobotManager.sh /root/

    # Update the manager application from SDCARD
    rm /root/algobotmanager
    rm /root/algobotmanager.md5
    cp /tmp/mounts/SD-P1/bin/algobotmanager /root/
    cp /tmp/mounts/SD-P1/bin/algobotmanager.md5 /root/

    # Update the algobot application from SDCARD
    rm /root/algobot/algobot_onionomega.md5
    rm /root/algobot/algobot_onionomega
    cp /tmp/mounts/SD-P1/bin/algobot/algobot_onionomega /root/algobot
    cp /tmp/mounts/SD-P1/bin/algobot/algobot_onionomega.md5 /root/algobot

    echo "- Configuring Web App"   
    # Remove files in web directory
    rm -R /www/*
    # Create link to 
    ln -s /tmp/mounts/SD-P1/www /www/kehops
    # Copy welcome page to web directory
    cp /tmp/mounts/SD-P1/index.html /www/
#    cp /tmp/mounts/SD-P1/www_onion/index.html /www 
 #   rm /www/setup-wizard/index.html
  #  cp /tmp/mounts/SD-P1/www_onion/setup-wizard/index.html /www/setup-wizard
    #ln -s /tmp/mounts/SD-P1/www /www/kehops
        
    # Starting application files
    restart
    
    return $result    
}                                                                                                                                                               



##-------------------------------------------------------------
## BASH FILE ENTRY POINT
## This is the entry point for the bash script.
## It will check the existance of the directories and will create it if not exist.
##
## LIST OF PARAMETERS POSSIBLE:
## ---> checkApp
## ---> checkManager
## ---> updateApp
## ---> updateManager
## ---> install
## ---> restart
##-------------------------------------------------------------       

# Will check the existance of "algobot" directory
# and the MD5 file of the application firmware in the root/algobot directory
# IF NOT EXIST, the algobot directory and and generic MD5 file will be create

if [ -d "/root/algobot" ]; then 
 echo "Algobot directory ok"
 if [ -f /root/algobot/algobot_onionomega.md5 ]; then
  echo "MD5 file is present"
 else 
  touch /root/algobot/algobot_onionomega.md5
  echo "MD5 not existing and was created"
 fi
else 
 echo "Create directory for application" 
 mkdir /root/algobot
 touch /root/algobot/algobot_onionomega.md5
fi

# Will check the existance of "update" directory for the application firmware
# and the MD5 file of the manager application in the root directory
# IF NOT EXIST, the algobot directory and and generic MD5 file will be create

if [ -d "/root/update" ]; then
 echo "Update directory is present"
else                                                                                                               
 echo "Create a directory for update"
 mkdir /root/update                                                                                                     
fi 

 if [ -f /root/algobotmanager.md5 ]; then
  echo "MD5 file is present"
 else 
  touch /root/algobotmanager.md5
 fi
                                                                                                                                                                                                                                                                                
# This part get all the parameters give by the user and will
# call the correspondant functions
#
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
                updateManager        
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

