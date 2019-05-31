# Start logs Dir
mkdir ~/logs
# Start Diode Sender
cd ~/MARTe2Project/GIT/MARTe2Hieratika_V2.1
echo Start Proccess 1
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_1.cfg > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 2
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_2.cfg > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 3
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_3.cfg  > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 4
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_4.cfg  > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 5
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_5.cfg  > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 6
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_6.cfg   > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 7
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_7.cfg   > /dev/null 2>&1 & disown
# sleep 300

echo Start Proccess 8
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneApp.ex  Configurations/DiodeStandaloneSendAll_8.cfg   > /dev/null 2>&1 & disown

