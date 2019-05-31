# Start logs Dir
mkdir ~/logs
# Start Diode Receiver
cd ~/MARTe2Project/GIT/MARTe2Hieratika_V2.1
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_1.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_2.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_3.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_4.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_5.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_6.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_7.cfg  & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_8.cfg  & disown

