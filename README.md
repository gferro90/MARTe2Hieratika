# MARTe2Hieratika

#Firewall Local Configuration
iptables -F
iptables -A OUTPUT -p udp -s 192.168.129.249 --dport 5064 -j ACCEPT
iptables -A INPUT -p udp -s 192.168.129.249 --dport 5064 -j ACCEPT
iptables -A OUTPUT -p udp --dport 5064 -j DROP
iptables -A INPUT -p udp --dport 5064 -j DROP

do the same for port 5065

sudo iptables-save > /etc/sysconfig/iptables


#Git repos
https://vcis-gitlab.f4e.europa.eu/aneto/MARTe2 
https://github.com/gferro90/MARTe2Hieratika

#Local Configuration
Change prepare.sh file
source prepare.sh

#Compilation
cd MARTe2
make -f Makefile.gcc
cd MARTe2Hieratika
make -f Makefile.gcc

#Run SoftIoc
cd MARTe2Hieratika/Startup/Tests/TestAll/expandedDBs_Test/
nohup sleep infinity | softIoc -d Diode.db &

#Run DiodeReceiver
Configuration at MARTe2Hieratika/Configurations/DiodeReceiver.cfg
cd MARTe2Hieratika
Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeReceiver.cfg

#Run DiodeSender
Configuration at MARTe2Hieratika/Configurations/DiodeSender.cfg
cd MARTe2Hieratika
Build/x86-linux/GTest/DiodeStandaloneApp.ex Configurations/DiodeSender.cfg

#Run DiodeSender simulator
cd MARTe2Hieratika
Build/x86-linux/GTest/TestEpicsWriter.ex Startup/Tests/TestReal/PV_List_sorted.xml ACCELERATORVIEW:TS:DUTY

#Configuration folders
DiodeSender contains configuration of the diode sender 
DiodeReceiver contains configuration of the diode receiver 