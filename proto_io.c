

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h> /*strstr()*/
#include "scap.h"
#include "global.h"

/*
La funzione stampa su stdout le informazioni riguardanti l'uso del programma
*/
void print_help(){
  printf("protomonitor: processes life live moninitor \n\n");
  printf("	-l  | --log 			export	proc data into a log file\n");
  printf("	-p  | --proc_only		show only information about main processes\n");
  printf("	-h  | --help			show information about the inline command\n");
  printf("	-a  | --all			track the activity of all active processes\n");
  printf("	-t  | --time           <1-10>	set the refresh time (1 sec to 10)\n");
  printf("	-mc | --min_cpu	       <min>	show processes with the cpu usage value greater than <min>\n");
  printf("	-mf | --min_pagefaults <min>	show processes with the page faults value greater than <min>\n");
  printf("	-mi | --min_iowait     <min>	show processes with the io_wait value greater than <min>\n");
}

/*
La funzione si occupa di leggere i parametri passati al programma all'atto
della sua invocazione e setta le variabili globali corrispondenti alle
funzionalità richieste
*/
void read_argv(int argc, char *argv[]){

  int i;
  bool inv_arg = true;
  global_data.refresh_t = 5;
  global_data.refresh_man = 5;
  global_data.log_onfile_enabled = false;
  global_data.show_help_enabled = false;
  global_data.show_only_procs =false;

  for(i = 1; i<argc; i++){
    inv_arg = true;		//invalid argument check
    if( strstr(argv[i], "-l") || strstr(argv[i], "--log")){
      global_data.log_onfile_enabled = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-p") || strstr(argv[i], "--proc_only")){
      global_data.show_only_procs = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-h") || strstr(argv[i], "--help")){
      global_data.show_help_enabled = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-a") || strstr(argv[i], "--all")){
      global_data.get_all_proc = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-t") || strstr(argv[i], "--time")){
      if((i+1) < argc && atoi(argv[i+1])>=1 && atoi(argv[i+1])<=10 ){
	global_data.refresh_t = atoi(argv[i+1]);
        global_data.refresh_man = atoi(argv[i+1]);
	i++;
        inv_arg = false;
      }
    }
    if( strstr(argv[i], "-mc") || strstr(argv[i], "--min_cpu")){
       if((i+1) < argc && atof(argv[i+1])>=1){
        global_data.min_cpu_val = atof(argv[i+1]);
        inv_arg = false;
	i++;
      }
    }
    if( strstr(argv[i], "-mf") || strstr(argv[i], "--min_pagefaults")){
      if((i+1) < argc && atof(argv[i+1])>=1){
        global_data.min_iowait = atof(argv[i+1]);
        inv_arg = false;
	i++;
      }
    }
    if( strstr(argv[i], "-mi") || strstr(argv[i], "--min_iowait")){
      if((i+1) < argc && atoi(argv[i+1])>=1){
        global_data.min_page_faults = atoi(argv[i+1]);
        inv_arg = false;
        i++;
      }
    }
    if( inv_arg == true){
      printf("%s : invalid argument\n",argv[i]);
      global_data.show_help_enabled = true;
    }
  }
}

/*
  La funzione stampa su stdout i dati dei processi/threads in memoria
*/
void print_tasks_procs(){
  task_data* data = NULL;
  proc_data* proc = NULL;

  system("clear");
  if(!global_data.show_only_procs){

    printf("\t\t\tTASKS\n\n");
    for(data = g_tasks; data != NULL; data=data->hh.next) {
      if(data->valid != 0){
        printf("tid = %d : pid = %d : ppid = %d : gid = %d : uid = %d : exe = %s \n",
  	     data->tid, data->pid, data->father_pid, data->gid, data->uid, data->exe);
      }
    }
  }

  printf("\n\n\t\t\tPROCS\n\n");
  for(proc = g_procs; proc != NULL; proc=proc->hh.next) {
    if(proc->valid != 0 && proc->avg_load >= global_data.min_cpu_val &&
        proc->iowait_time_pctg >= global_data.min_iowait && 
        proc->process_page_faults >= global_data.min_page_faults){
      printf("pid = %d actual_memory = %u : peak_memory = %u : avg_load = %.2f %% : iowait: %.2f"
	     " : process_page_faults = %u \n",
	     proc->pid, proc->actual_memory, proc->peak_memory,
	     proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
    }
  }
}

/*
La funzione scrive su file le informazioni riguardanti un singolo processo
*/
void write_onfile_proc_info(FILE* f, proc_data* proc){

  fprintf(f, "%d %u %u %.2f %.2f %u \n",
  proc->pid, proc->actual_memory, proc->peak_memory,
  proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
}

/*
La funzione esporta su file le informazioni riguardanti tutti i main 
procceses in memoria 
*/
void export_data_onfile(){

  proc_data* proc = NULL;
  FILE* f;

  f = fopen (LOG_STAT_PATH, "a");
  if(f != NULL){
    fprintf(f,"%lu \n",time(NULL));
    for(proc = g_procs; proc != NULL; proc=proc->hh.next){
      write_onfile_proc_info(f, proc);
    }
  }else printf("stat_log file open error\n");
  fclose(f);   
}
/*
La funzione esporta i dati di un thread appena nato su un file
*/
void export_procsbirth_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (LOG_PROCLIFE_PATH, "a");

  if(f != NULL){
    fprintf(f, "%s %d  %d %d %d %d %s %lu \n",
      "BIRTH:",data->tid, data->pid, data->father_pid, 
      data->gid, data->uid, data->exe, data->birth_ts);
  }else printf("proclife_log file open error\n");
  fclose(f);   
}

/*
La funzione esporta i dati di un thread appena morto su un file
*/
void export_procsdeath_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (LOG_PROCLIFE_PATH, "a");

  if(f != NULL){
    fprintf(f, "%s %d %d %lu \n",
      "DEATH:", data->tid, data->pid, data->death_ts);
  }else printf("proclife_log file open error\n");
  fclose(f);   
}
