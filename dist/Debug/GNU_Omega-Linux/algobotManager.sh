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
                echo "- Download MD5 :  OK";
        else
                echo "- Download MD5 : ERROR";
                status=status+1
        fi

        if [ $status -eq 0 ];
        then

                ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE
                cmp update/algobot_onionomega.md5 algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?

                if [ $resultat -eq 0 ];
                then
                        echo "Firmware version is last, no update !"
                elif [ $resultat -eq 1 ];
                then
                        echo "New firmware found !"
                else
                        echo "Unexisting MD5 file, please update application"
                fi
        else
                echo "Error, impossible to download file";
		
        fi

}

update(){                                                                                                                                                      
        rm update/*                                                                                                                                            
                                                                                                                                                               
        status=0                                                                                                                                               
                                                                                                                                                               
                                                                                                                                                               
## TRY TO DOWNLOAD BINARY                                                                                                                                      
        echo "Start download binary firmware from server..."                                                                                                   
        CMD=`wget -P update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega`    
                                                                                                                                                               
        if [ $? -eq 0 ];                                                                                                                                       
        then                                                                                                                                                   
                echo "- Download binary :  OK";                                                                                                                
        else                                                                                                                                                   
                echo "- Download binary : ERROR";                                                                                                              
                status=status+1                                                                                                                                
        fi                                                                                                                                                     
                                                                                                                                                               
## TRY TO DOWNLOAD MD5 FILE                                                                                                                                    
        echo "Start download MD5 from server..."                                                                                                               
        CMD=`wget -P update/ -q https://raw.githubusercontent.com/raphaelthurnherr/algobot_onionOmega/master/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`
                                                                                                                                                               
        if [ $? -eq 0 ];                                                                                                                                       
        then                                                                                                                                                   
                echo "- Download MD5 :  OK";                                                                                                                   
        else                                                                                                                                                   
                echo "- Download MD5 : ERROR";                                                                                                                 
                status=status+1                                                                                                                                
        fi                                                                                                                                                     
                                                                                                                                                               
        if [ $status -eq 0 ];                                                                                                                                  
        then                                                                                                                                                   
                echo "Files downloading with success";                                                                                                         
                                                                                                                                                               
                ## COMPARE THE 2 MD5 FILE FOR CHECK UPDATE                                                                                                     
                cmp  update/algobot_onionomega.md5 algobot/algobot_onionomega.md5 1>/dev/null 2>&1; resultat=$?                                                
                                                                                                                                                               
                if [ $resultat -eq 0 ];                                                                                                                        
                then                                                                                                                                           
                        echo "Firmware version is last, no install necessary !"                                                                                
                elif [ $resultat -eq 1 ];                                                                                                                      
                then                                                                                                                                           
                        echo "Firmware update downloaded, please install !"                                                                                    
                else                                                                                                                                           
                echo "No MD5 found, please add them or install manually"                                                                                       
                fi                                                                                                                                             
        else                                                                                                                                                   
                echo "Update aborted, error during downloading files";                                                                                         
        fi                                                                                                                                                     
}         
install(){                                                                                                                                                     
                                                                                                                                                               
    echo "Starting firmware upgrade... ";                                                                                                                      
    killall algobot_onionomega;                                                                                                                                
    sleep 2;                                                                                                                                                   
    rm algobot/*                                                                                                                                               
    cp update/* algobot/                                                                                                                                       
    chmod +x algobot/algobot_onionomega                                                                                                                        
    echo "Starting application... ";                                                                                                                           
    ./algobot/algobot_onionomega                                                                                                                               
}                                                                                                                                                              
                                                                                                                                                               
# Execution des parametres recus                                                                                                                               
                                                                                                                                                               
clear;                                                                                                                                                         
                                                                                                                                                               
for param in "$@"                                                                                                                                              
do                                                                                                                                                             
        if [ "$param" = "check" ];                                                                                                                             
        then                                                                                                                                                   
                check                                                                                                                                          
        fi                                                                                                                                                     
                                                                                                                                                               
        if [ "$param" = "update" ];                                                                                                                            
        then                                                                                                                                                   
                update                                                                                                                                         
        fi                                                                                                                                                     
                                                                                                                                                               
        if [ "$param" = "install" ];                                                                                                                           
        then                                                                                                                                                   
                install                                                                                                                                        
        fi                                                                                                                                                     
done
