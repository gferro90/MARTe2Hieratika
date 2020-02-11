cp DiodeSender.conf /opt/rh/httpd24/root/etc/httpd/conf.d/
cp httpd.conf /opt/rh/httpd24/root/etc/httpd/conf.d/ 
cp PV_List_sorted.xml MARTe2Hieratika/Startup/Tests/TestReal/
cp DiodeSender.cfg MARTe2Hieratika/Configuration
cp prepare.sh MARTe2Hieratika

#change the paths in the prepare.sh script and execute it
cd MARTe2Hieratika
source prepare.sh

#launch httpd service (su)
service httpd24-httpd start

#launch the MARTe application
Build/x86-linux/GTest/DiodeStandaloneApp.ex Configurations/DiodeSender.cfg

