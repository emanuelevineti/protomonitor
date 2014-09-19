
/*	Modulo di gestione processi	*/

/*
  Funzioni per la gestione dei processi relativi alle chiamate di sistema
  catturate da sysdig.
*/

#include <stdio.h>
#include <stdlib.h>/*system()*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h> /*strncpy(), strlen()*/
#include "uthash.h"
#include "scap.h"
#include "scap-int.h"
#include "global.h"
#include "stat_manager.h"
#include "proto_io.h"
/*
La funzione si occupa di inizializzare le tabelle hash dei tasks e processi
con i processi attivi attualemente sulla macchina
*/
void init_add_active_proc(scap_t* h){
  
  scap_threadinfo* proc = NULL;
  task_data* d = NULL;
  proc_data* p = NULL;	


  for(proc = h->m_proclist; proc != NULL; proc=proc->hh.next){

    d = (task_data*)malloc(sizeof(task_data));

    d->tid = (u_int16_t)proc->tid;
    d->pid = proc->pid;
    d->father_pid = proc->ptid;
    strcpy(d->exe,proc->exe);
    d->uid = proc->uid;
    d->gid = proc->gid;
    d->birth_ts = 0;
    d->death_ts = 0;
    d->valid = 1;
    
    HASH_ADD(hh, g_tasks, tid, sizeof(u_int32_t), d);

    if(d->tid == d->pid){
      p = (proc_data*)malloc(sizeof(proc_data));
      p->pid = d->pid;
      p->valid = 1;
      strcpy(p->state,"R");
      HASH_ADD(hh, g_procs, pid, sizeof(u_int32_t), p);
      updateProcessStats(p);
    }  
    }
    
}
/*
  La funzione si occupa dell settaggio come non valido di un processo/thread
  appena morto
*/
void invalidate_task_proc(u_int16_t tid, time_t death_ts){

  task_data* data = NULL;
  proc_data* proc = NULL;
  
  u_int32_t key = tid;

  HASH_FIND_INT32(g_procs, &key, proc);
  if(proc != NULL && proc->valid != 0){
    finalizeProcess(proc);
    DB2(printf("Eliminazione processo pid : %d\n",proc->pid);)
    proc->valid = 0;
    strcpy(proc->state,"D");
  }

  HASH_FIND_INT32(g_tasks, &key, data);
  if(data != NULL && data->valid != 0){

    DB2(printf("Eliminazione processo tid : %d\n",data->tid);)
    data->death_ts = death_ts;
    
    data->valid = 0;
    if(global_data.log_threads_life)
      if(global_data.log_json_format)
        exportjson_procsdeath_data_onfile(data);
      else  export_procsdeath_data_onfile(data);
  }
}

/*
  La funzione si occupa dell'inserimento dei processi nelle tabelle dei
  processi e dei threads. Questo avviene solo se questi non sono già presenti
  in queste.
*/
void add_task_proc(task_data* data){

  task_data* d = NULL;
  proc_data* p = NULL;
  u_int32_t kpid = data->pid, ktid = data->tid;

  HASH_FIND_INT32(g_tasks, &ktid, d);
  if(d != NULL){
    DB1(printf("errore reinserimento task %d \n",data->tid);)
      }else{
    task_data *p = (task_data*)malloc(sizeof(task_data));
    if(p != NULL) {

      if(global_data.log_threads_life){
        if(global_data.log_json_format)
          exportjson_procsbirth_data_onfile(data);
        else  export_procsbirth_data_onfile(data);
      }
      DB2(printf("Inserimento task %d\n",data->tid);)
	p->tid = ktid, p->pid = data->pid, p->father_pid = data->father_pid,
	p->uid = data->uid, p->gid = data->gid,
	p->valid = data->valid;
        p->birth_ts = data->birth_ts;
        p->death_ts = 0;
      strcpy(p->exe,data->exe);
      HASH_ADD(hh, g_tasks, tid, sizeof(u_int32_t), p);
    }
  }

  HASH_FIND_INT32(g_procs, &kpid, p);
  if(p != NULL){
    DB2(printf("errore reinserimento pid %d \n",data->pid);)
      }else{

    proc_data *p = (proc_data*)malloc(sizeof(proc_data));
    if(p != NULL){
      p->pid = kpid;
      p->valid = data->valid;
      strcpy(p->state, "B");
      HASH_ADD(hh, g_procs, pid,sizeof(u_int32_t), p);
      updateProcessStats(p);
    }DB2(printf("Inserimento processo pid: %d \n", p->pid);)
       }
}
/*
  La funzione si occupa dell'eliminazione delle strutture dei processi/thread
  non più in uso.
*/
void clear_task_proc(scap_t* handle){

  task_data* data = NULL;
  proc_data* d = NULL;
  proc_data* proc = NULL;

  /*	Libera la memoria delle informazioni dei processi e task non più in uso	*/
  for(proc = g_procs; proc != NULL; proc=proc->hh.next){
    if(proc->valid == 0){
      DB1(printf("Eliminazione proc %d \n", proc->pid);)
	HASH_DEL(g_procs, proc);
      if(proc) free(proc);
    }
    if(strstr(proc->state,"B")) strcpy(proc->state,"R");
  }
  /*
    Si attua per ogni thread la ricerca del suo processo principale nella
    tabella dei processi. se questo non è presente(quindi morto e già eliminato)
    si cancella anche il thread. XXX Si è optato per questa soluzione
    in assenza di un lista generale dei processi.
  */
  for(data = g_tasks; data != NULL; data=data->hh.next){

    u_int32_t key = data->pid;
    HASH_FIND_INT32(g_procs, &key, d);
    if(d == NULL || d->valid == 0) data->valid = 0;

    if(data->valid == 0){
      DB1(printf("Eliminazione task %d \n",data->tid);)
	HASH_DEL(g_tasks, data);
      if(data) free(data);
    }
  }


}


/*

  La funzione recupera e gestisce i dati relativi ad un evento di sistema.

  I dati relativi ad i processi sono recuperati atraverso la funzione
  scap_proc_get() che recupera la struttura scap_threadinfo con le info.
  Queste se non già presenti in memoria sono recuperate da sysdig atraverso
  il parsing dei file relativi al tid nella cartella /proc.

  Documentandomi l'unica maniera per il recupero delle informazioni relative
  ad un generico evento di sistema esclusa la creazione sembra essere:

  const struct ppm_event_info *i =  scap_event_getinfo(ev);
  u_int32_t nparams = i->nparams;
  uint16_t *lens = (uint16_t *)((char *)m_pevt + sizeof(struct ppm_evt_hdr));
  char *valptr = (char *)lens + nparams * sizeof(uint16_t);

  for(j = 0; j < nparams; j++)
  {
  //	recupero dati	//
  valptr += lens[j];
  }

  N.B: All'atto dell'apertura della cattura live degli eventi sysdig inizializza
  una lista con i dati dei processi in esecuzione sulla macchina all voce
  h->m_proclist.

*/
void handle_event(struct ppm_evt_hdr* ev, u_int16_t cpuid,scap_t *h) {

  const struct ppm_event_info *i =  scap_event_getinfo(ev);
  char process_name[128];
  task_data data;

  switch(ev->type) {
  case PPME_CLONE_16_X:
    {
      scap_threadinfo* t_info = scap_proc_get(h, ev->tid, 0);
      if(t_info ==NULL){
	DB1(printf("Impossibile recuperare le informazioni: %d\n",
		   (u_int32_t)ev->tid);)
	  return;
      }
      data.tid = (u_int16_t)ev->tid;
      data.pid = t_info->pid;
      data.father_pid = t_info->ptid;
      strcpy(data.exe,t_info->exe);
      data.uid = t_info->uid;
      data.gid = t_info->gid;
      data.birth_ts = time(NULL);
      data.valid = 1;

      /*
		bisogna effettuare la liberazione dell'area di memoria utilizzata
		da sysdig nella lista dei processi per memorizzare le info
		quando non più in uso (vedi morte di un processo) o non più utili
		per il recupero delle informazioni con:
      */scap_proc_free(h, t_info);
    }
    add_task_proc(&data);
    break;

    /*Per catturare sia l'uscita dei thread sia dei programmi principali
      c'è la necessità di gestire diversi tipi di eventi di uscita.
      XXX Senza la cattura di questi 3 eventi diversi non si riesce
      a rilevare la morte di tutti i processi*/
  case PPME_PROCEXIT_E: /* The main process ends */
    invalidate_task_proc(ev->tid, time(NULL));
    break;

  default:
   // printf("Unknown event %u\n", ev->type);
    break;
  }
}

/*
  La funzione gestisce l'update dei dati di un processo, la pulizia dei
  processi e thread morti e infine stampa su stdout i loro dati
*/
void manage_data(scap_t* handle){

  proc_data* proc = NULL;
  global_data.actual_refresh = time(NULL);

  for(proc = g_procs; proc != NULL; proc=proc->hh.next)
    finalizeProcess(proc);

  if(global_data.log_onfile_enabled)
    export_data_onfile();
  if(global_data.export_elk)
    printf("export\n");
    export_data_elk();
  clear_task_proc(handle);
  print_tasks_procs();
  for(proc = g_procs; proc != NULL; proc=proc->hh.next)
    updateProcessStats(proc);
  global_data.last_refresh = global_data.actual_refresh;
}
/*	Fine modulo gestione processi	*/
