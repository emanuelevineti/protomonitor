

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h> /*strstr()*/
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include "scap.h"
#include "scap-int.h"
#include "global.h"
#include <errno.h>
/*
La funzione stampa su stdout le informazioni riguardanti l'uso del programma
*/
void print_help(){
  printf("\n\n	protomonitor: processes life live moninitor \n\n");
  printf("	-p  | --proc_only		show only information about main processes\n");
  printf("	-h  | --help			show information about the inline command\n");
  printf("	-a  | --all			track the activity of all active processes\n");
  printf("	-t  | --time           <1-60>	set the refresh time (1 sec to 60)\n");
  printf("	-mc | --min_cpu	       <min>	show processes with the cpu usage value greater than <min>\n");
  printf("	-mf | --min_pagefaults <min>	show processes with the page faults value greater than <min>\n");
  printf("	-mi | --min_iowait     <min>	show processes with the io_wait value greater than <min>\n");
  printf("	-l  | --log 			export proc data into a log file\n");
  printf("	-lp | --log_path       <path>   export proc data into a log file specifying the destination path\n");
  printf("	-lj | --log_json               	export proc data into a log file using a JSON format\n");
  printf("	-ljp| --logjson_path   <path>	export proc data into a log file using a JSON format specifying the destination path\n\n");
  printf("	-lt | --log_threads           	export threads life data on a log file\n");
  printf("	-elk| --elk_export 		export proc_data on elk\n");

}

bool path_is_valid(char *argv[]){

  FILE* f;
  char path[128];
  strcpy(path,argv[0]);
  strcat(path,"proto_stat.log");

  if(f = fopen(path,"a")){
    fclose(f);
    return true;
  }else return false;
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
  global_data.log_onfile_enabled = false;
  global_data.show_help_enabled = false;
  global_data.show_only_procs = false;
  global_data.log_json_format = false;
  global_data.last_refresh = 0;
  global_data.log_threads_life = false;
  strcpy(global_data.stat_path, "./proto_stat.log");
  strcpy(global_data.proclife_path, "./proto_proclife.log");
  global_data.export_elk = false;

  for(i = 1; i<argc; i++){
    inv_arg = true;		//invalid argument check
    if( strstr(argv[i], "-lp") || strstr(argv[i], "--log_path")){
      if((i+1) < argc && path_is_valid(&argv[i+1])){
        global_data.log_onfile_enabled = true;
        strcpy(global_data.stat_path,argv[i+1]);
        strcpy(global_data.proclife_path,argv[i+1]);
        strcat(global_data.stat_path,"proto_stat.log");
        strcat(global_data.proclife_path,"proto_proclife.log");
        inv_arg = false;
        i++;
      }
    }
    if( strstr(argv[i], "-lj") || strstr(argv[i], "--log_json")){
      global_data.log_onfile_enabled = true;
      global_data.log_json_format = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-ljp") || strstr(argv[i], "--logjson_path")){
      if((i+1) < argc && path_is_valid(&argv[i+1])){
        global_data.log_onfile_enabled = true;
        global_data.log_json_format = true;
        strcpy(global_data.stat_path,argv[i+1]);
        strcpy(global_data.proclife_path,argv[i+1]);
        strcat(global_data.stat_path,"proto_stat.log");
        strcat(global_data.proclife_path,"proto_proclife.log");
        inv_arg = false;
        i++;
      }
    }
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
    if( strstr(argv[i], "-elk") || strstr(argv[i],"elk_export")){
      global_data.export_elk = true;
      inv_arg = false;
    }
    if( strstr(argv[i], "-t") || strstr(argv[i], "--time")){
      if((i+1) < argc && atoi(argv[i+1])>=1 && atoi(argv[i+1])<=60 ){
	global_data.refresh_t = atoi(argv[i+1]);
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
    if(strstr(argv[i],"-lt") || strstr(argv[i], "--log_threads")){
    global_data.log_threads_life = true;
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
  task_data* f_task;
  task_data* task;
  u_int32_t pkey = proc->pid,ppkey;
  time_t t = time(NULL);
  HASH_FIND_INT32( g_tasks, &pkey, task );
  if(global_data.get_all_proc){
    ppkey = task->father_pid;
    HASH_FIND_INT32(g_tasks, &ppkey,f_task);
  }
  fprintf(f,"%s",proc->state);
  if( task->birth_ts > global_data.last_refresh )
    fprintf(f,"%ju",(uintmax_t)task->birth_ts);
  else 
    fprintf(f,"%ju",(uintmax_t)global_data.last_refresh);
  if( task->death_ts > 0)
    fprintf(f,"%ju",(uintmax_t)task->death_ts);
  else fprintf(f,"%ju",global_data.actual_refresh);
  
  fprintf(f, "%s %d %s %d ", ctime(&t), proc->pid, 
    task->exe, task->father_pid);

  if(global_data.get_all_proc)
    fprintf(f,"%s ",f_task->exe);
  else fprintf(f,"no name ");
  fprintf(f,"%u %u %.2f %.2f %u \n",proc->actual_memory, proc->peak_memory,
  proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
}

void writejson_onfile_proc_info(FILE* f, proc_data* proc){
  task_data* f_task;
  task_data* task;
  u_int32_t pkey = proc->pid,ppkey;
  time_t t = time(NULL);
  char *pname;
  HASH_FIND_INT32( g_tasks, &pkey, task );
  if(global_data.get_all_proc){
    ppkey = task->father_pid;
    HASH_FIND_INT32(g_tasks, &ppkey,f_task);
  }
  pname = strrchr(task->exe,'/'); 
  fprintf(f,"{ \"state\" : \"%s\", ",proc->state);
  fprintf(f,"\"start_t\" : ");
  if( task->birth_ts > global_data.last_refresh )
    fprintf(f,"%ju, ",(uintmax_t)task->birth_ts);
  else 
    fprintf(f,"%ju, ",(uintmax_t)global_data.last_refresh);
  fprintf(f,"\"end_t\" : ");
  if( task->death_ts > 0)
    fprintf(f,"%ju, ",(uintmax_t)task->death_ts);
  else fprintf(f,"%ju, ",global_data.actual_refresh);

  fprintf(f, "\"pid\" : %d, \"ppath\" : \"%s\", \"pname\" : \"%s\", \"ppid\""
    ": %d, ", proc->pid, 
    task->exe, &pname[1], task->father_pid);

  if(global_data.get_all_proc)
    fprintf(f,"\"ppname\" : \"%s\", ",f_task->exe);
  else fprintf(f,"\"ppname\" : \"no name\", ");
  fprintf(f,"\"actul_memory\" : %u, \"peak_memory\" : %u, \"avg_load\" : "
    "%.2f, \"iowait\" : %.2f, \"page_fault\" : %u }\n",proc->actual_memory, 
    proc->peak_memory, proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
}
/*
La funzione esporta su file le informazioni riguardanti tutti i main 
procceses in memoria 
*/
void export_data_onfile(){

  proc_data* proc = NULL;
  FILE* f;

  f = fopen (global_data.stat_path, "a");
  if(f != NULL){
   
    for(proc = g_procs; proc != NULL; proc=proc->hh.next){
      if( global_data.log_json_format )
        writejson_onfile_proc_info(f, proc);
      else write_onfile_proc_info(f, proc);
    }
  }else printf("stat_log file open error\n");
  fclose(f);   
}
/*
La funzione esporta i dati di un thread appena nato su un file
*/
void export_procsbirth_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (global_data.proclife_path, "a");

  if(f != NULL){
    fprintf(f, "%s %d  %d %d %d %d %s %s \n",
      "BIRTH:",data->tid, data->pid, data->father_pid, 
      data->gid, data->uid, data->exe, ctime(&data->birth_ts));
  }else printf("proclife_log file open error\n");
  fclose(f);   
}
void send_data_to_es(proc_data* proc){
  char message[3025];
  char tmp[2050];
  
  task_data* f_task;
  task_data* task;
  u_int32_t pkey = proc->pid,ppkey;
  time_t t = time(NULL);
  char *pname;
  
  HASH_FIND_INT32( g_tasks, &pkey, task );
  if(global_data.get_all_proc){
    ppkey = task->father_pid;
    HASH_FIND_INT32(g_tasks, &ppkey,f_task);
  }
  pname = strrchr(task->exe,'/'); 
  sprintf(message,"{ \"state\" : \"%s\", \"start_t\" : ",proc->state);
  if( task->birth_ts > global_data.last_refresh )
    sprintf(tmp, "%ju, ",(uintmax_t)task->birth_ts);
  else 
    sprintf(tmp, "%ju, ",(uintmax_t)global_data.last_refresh);
  strcat(message,tmp);
  sprintf(tmp, "\"end_t\" : ");
  strcat(message,tmp);
  if( task->death_ts > 0)
    sprintf(tmp, "%ju, ",(uintmax_t)task->death_ts);
  else sprintf(tmp, "%ju, ",global_data.actual_refresh);
  strcat(message,tmp);
  sprintf(tmp,"\"pid\" : %d, \"ppath\" : \"%s\", \"pname\" : \"%s\", \"ppid\""
    ": %d, ", proc->pid, 
    task->exe, &pname[1], task->father_pid);
  strcat(message,tmp);
  if(global_data.get_all_proc)
    sprintf(tmp, "\"ppname\" : \"%s\", ",f_task->exe);
  else sprintf(tmp, "\"ppname\" : \"no name\", ");
  strcat(message,tmp);
  sprintf(tmp, "\"actul_memory\" : %u, \"peak_memory\" : %u, \"avg_load\" : "
    "%.2f, \"iowait\" : %.2f, \"page_fault\" : %u }\n",proc->actual_memory, 
    proc->peak_memory, proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
  strcat(message,tmp);

  send(global_data.socket_desc,message,strlen(message),0);
}
/*
La funzione esporta i dati del programma atraverso il socket di rete
*/
void export_data_elk(){

  proc_data* proc = NULL;
  
  printf("4\n");
  for(proc = g_procs; proc != NULL; proc=proc->hh.next)
    send_data_to_es(proc);
}
/*
La funzione inizializza una connessione aprendo un nuovo socket di rete
*/
void init_connection_socket(){
  struct sockaddr_in address;
  int addrlen;
  /* type of socket created in socket() */
  global_data.socket_desc = socket(AF_INET,SOCK_STREAM,0);
  if(global_data.socket_desc < 0){
    perror("Create socket Error: ");
    global_data.show_help_enabled = true;
    return;
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(5652);
  if(connect(global_data.socket_desc,(struct sockaddr *)&address,sizeof(address)) == -1){
    perror("Connect Error: ");
    global_data.show_help_enabled = true;
    return;
  }
}
/*
La funzione esporta i dati di un thread appena morto su un file
*/
void export_procsdeath_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (global_data.proclife_path, "a");

  if(f != NULL){
    fprintf(f, "%s %d %d %s \n",
      "DEATH:", data->tid, data->pid, ctime(&data->death_ts));
  }else printf("proclife_log file open error\n");
  fclose(f);   
}
/*
La funzione esporta i dati di un thread appena morto su un file
In formato JSON
*/
void exportjson_procsdeath_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (global_data.proclife_path, "a");

  if(f != NULL){
    fprintf(f, "{\n\"state\" : \"%s\",\n\"tid\" : %d,\n\"pid\" : %d,\n\"death_ts\" : \"%s\",\n}",
      "DEATH", data->tid, data->pid, ctime(&data->death_ts));
  }else printf("proclife_log file open error\n");
  fclose(f);   
}
/*
La funzione esporta i dati di un thread appena nato su un file
In formato JSON
*/
void exportjson_procsbirth_data_onfile(task_data* data){
  
  FILE* f;

  f = fopen (global_data.proclife_path, "a");

  if(f != NULL){
    fprintf(f, "{\n\"state\" : \"%s\",\n\"tid\" : %d,\n\"pid\" : %d,\n\"ppid\" : %d,\n\"gid\" : %d,\n\"uid\" %d,\n\"exe\" : \"%s\",\n\"birth_ts\" : \"%s\",\n}",
      "BIRTH",data->tid, data->pid, data->father_pid, 
      data->gid, data->uid, data->exe, ctime(&data->birth_ts));
  }else printf("proclife_log file open error\n");
  fclose(f);   
}
