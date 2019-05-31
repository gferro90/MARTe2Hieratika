#bin/bash
cd ~/MARTe2Project/GIT/MARTe2Hieratika_V2/Startup/Tests/TestAll/expandedDBs_Test
tmux  new-session -d -s softIoc_session 'softIoc -d Diode.db' &



