#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exécuter ce fichier
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit comme ici

echo Transfere du fichier binaire...
rsync -a /home/raph/NetBeansProjects/Algo_HW_lib_test/dist/Debug/GNU_Omega-Linux/algo_hw_lib_test  root@192.168.3.1:~/app

echo "Lancement de l'application via SSH...\n"

sshpass -p 8051f320 ssh root@192.168.3.1 "~/app/./algo_hw_lib_test"
 
echo "Fermeture du tunnel SSH"

exit
