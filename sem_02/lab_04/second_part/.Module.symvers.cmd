cmd_/home/a_avortep/prog/os/sem_02/lab_04/second_part/Module.symvers := sed 's/\.ko$$/\.o/' /home/a_avortep/prog/os/sem_02/lab_04/second_part/modules.order | scripts/mod/modpost -m -a  -o /home/a_avortep/prog/os/sem_02/lab_04/second_part/Module.symvers -e -i Module.symvers   -T -
