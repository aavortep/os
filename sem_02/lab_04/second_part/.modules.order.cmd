cmd_/home/a_avortep/prog/os/sem_02/lab_04/second_part/modules.order := {   echo /home/a_avortep/prog/os/sem_02/lab_04/second_part/fortune.ko;   echo /home/a_avortep/prog/os/sem_02/lab_04/second_part/seq.ko; :; } | awk '!x[$$0]++' - > /home/a_avortep/prog/os/sem_02/lab_04/second_part/modules.order