#!/bin/sh
### BEGIN INIT INFO
# Provides:          samba
# Required-Start:  $local_fs $syslog $network
# Required-Stop:   $local_fs $syslog $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start Algobot daemon
### END INIT INFO


check(){
	
        rm update/*

        status=0

## TRY TO DOWNLOAD MD5 FILE
        echo "Start download MD5 from server..."
        CMD=`wget -P update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`

        if [ $? -eq 0 ];
        then
                echo "- Download MD5 file:  OK"
                                               
	   ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE                      
		cmp  update/algobot_onionomega.md5 algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?
                                                                                
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
    rm algobot/*                                                                                                                                                            
    cp update/* algobot/                                                                                                                                                    
    chmod +x algobot/algobot_onionomega                                                                                                                                     
    echo "Restarting application... ";                                                                                                                                        
    ./algobot/algobot_onionomega                                                                                                                                            
} 

restart(){                                                                              
    killall algobot_onionomega;                                                         
    sleep 2;                                                                            
    echo "Restarting application... ";                                                  
    ./algobot/algobot_onionomega                                                        
} 




update(){                                                                                                                                                      
                                                                                                                                            
## CHECK FOR UPDATE
  	check
	checkResult=$?

	updateResult=0

## TRY TO DOWNLOAD BINARY APPLICATION FILE AND UPDATE
	if [ $checkResult -eq 10 ];
	then

        if [ -f "update/*" ];
        then
                rm update/*
        fi

	## TRY TO DOWNLOAD BINARY                                                                                                                                                    
        	echo "Start download binary firmware from server..."                                                                                                                 
	        CMD=`wget -P update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega`
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

		            rm update/*                                                                                    
		else
			updateResult=1
		fi
	fi

	return $updateResult
}         

                                                                                                                                                               
                                                                                                                                                               
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
done

exit $actionResult 

