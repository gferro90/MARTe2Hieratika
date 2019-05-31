# Start logs Dir
mkdir ~/logs
# Start Diode Receiver
cd ~/MARTe2Project/GIT/MARTe2Hieratika_V2.1
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_1.cfg > ~/logs/DiodeRec_1.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_2.cfg >  ~/logs/DiodeRec_2.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_3.cfg >  ~/logs/DiodeRec_3.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_4.cfg >  ~/logs/DiodeRec_4.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_5.cfg >  ~/logs/DiodeRec_5.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_6.cfg >  ~/logs/DiodeRec_6.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_7.cfg >  ~/logs/DiodeRec_7.log & disown
# sleep 300
stdbuf -oL nohup Build/x86-linux/GTest/DiodeStandaloneRec.ex Configurations/DiodeStandaloneRecDiodePVs_8.cfg >  ~/logs/DiodeRec_8.log & disown

