#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exécuter ce fichier
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit comme ici


md5sum /home/raph-pnp/Documents/dev/embedded_soft/algobot_onion/algobot_onionOmega/dist/Debug/GNU_Omega-Linux/algobot_onionomega | cut -c -32 > dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5
echo "Fichier MD5 cr�e: "`cat dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5`

echo "Transfere des fichiers binaire..."

#rsync -a /home/raph/NetBeansProjects/algobot_onionOmega/dist/Debug/GNU_Omega-Linux/algobot_onionomega  root@192.168.3.1:~/algobot
rsync -a /home/raph-pnp/Documents/dev/embedded_soft/algobot_onion/algobot_onionOmega/dist/Debug/GNU_Omega-Linux/algobot_onionomega  root@192.168.3.1:~/algobot

echo "Algobot Manager: Transfere du fichier MD5..."
rsync -a /home/raph-pnp/Documents/dev/embedded_soft/algobot_onion/algobot_onionOmega/dist/Debug/GNU_Omega-Linux/algobot_onionomega.md5  root@192.168.3.1:~/algobot

echo "Lancement de l'application via SSH...\n"
#sshpass -p onioneer ssh root@192.168.3.1 "~/algobot/./algobot_onionomega"
 
echo "Fermeture du tunnel SSH"

exit
