# Start logs Dir
mkdir ~/logs
# Start Diode Sender
cd ~/MARTe2Project/GIT/MARTe2Hieratika_V2.1
echo Start Proccess 1
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_1.cfg > ~/logs/DiodeSend_1.log & disown
# sleep 300

echo Start Proccess 2
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_2.cfg > ~/logs/DiodeSend_2.log & disown
# sleep 300

echo Start Proccess 3
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_3.cfg > ~/logs/DiodeSend_3.log & disown
# sleep 300

echo Start Proccess 4
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_4.cfg > ~/logs/DiodeSend_4.log & disown
# sleep 300

echo Start Proccess 5
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_5.cfg > ~/logs/DiodeSend_5.log & disown
# sleep 300

echo Start Proccess 6
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_6.cfg > ~/logs/DiodeSend_6.log & disown
# sleep 300

echo Start Proccess 7
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_7.cfg > ~/logs/DiodeSend_7.log & disown
# sleep 300

echo Start Proccess 8
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_8.cfg > ~/logs/DiodeSend_8.log & disown

