
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
#include "global.h"
#include "stat_manager.h"

/*
La funzione si occupa dell settaggio come non valido di un processo/thread
appena morto
*/
void invalidate_task_proc(u_int16_t tid){

	struct task_data* data = NULL;
	struct proc_data* proc = NULL;

	u_int32_t key = tid;

	HASH_FIND_INT32(g_procs, &key, proc);
	if(proc != NULL && proc->valid != 0){
		
		DB2(printf("Eliminazione processo pid : %d\n",proc->pid);)
		proc->valid = 0;
	}
	HASH_FIND_INT32(g_tasks, &key, data);
	if(proc != NULL && data->valid != 0){
		
		DB2(printf("Eliminazione processo pid : %d\n",data->tid);)
		data->valid = 0;
	}
}

/*
La funzione si occupa dell'inserimento dei processi nelle tabelle dei 
processi e dei threads. Questo avviene solo se questi non sono già presenti
in queste.
*/
void add_task_proc(struct task_data* data){

	struct task_data* d = NULL;
	struct proc_data* p = NULL;
	u_int32_t kpid = data->pid, ktid = data->tid;

	HASH_FIND_INT32(g_tasks, &ktid, d);	
	if(d != NULL){
		DB1(printf("errore reinserimento task %d \n",data->tid);)
	}else{	
		struct task_data *p = (struct task_data*)malloc(sizeof(struct task_data));
		if(p != NULL) {
			DB2(printf("Inserimento task %d\n",data->tid);)
			p->tid = ktid, p->pid = data->pid, p->father_pid = data->father_pid, 
			p->uid = data->uid, p->gid = data->gid,
			p->valid = data->valid;
			strncpy(p->exe,data->exe,strlen(data->exe));
			HASH_ADD(hh, g_tasks, tid, sizeof(u_int32_t), p);
		}
	}
	
	HASH_FIND_INT32(g_procs, &kpid, p);
	if(p != NULL){
		DB2(printf("errore reinserimento pid %d \n",data->pid);)
	}else{
		
		struct proc_data *p = (struct proc_data*)malloc(sizeof(struct proc_data));
		if(p != NULL){
			p->pid = kpid;
			p->valid = data->valid;
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

	struct task_data* data = NULL; 
	struct proc_data* d = NULL;
	struct proc_data* proc = NULL;
	
/*	Libera la memoria delle informazioni dei processi e task non più in uso	*/
	for(proc = g_procs; proc != NULL; proc=proc->hh.next){
		if(proc->valid == 0){
			DB1(printf("Eliminazione proc %d \n", proc->pid);)
			HASH_DEL(g_procs, proc);	
			if(proc) free(proc);
		}
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
La funzione stampa su stdout i dati dei processi/threads in memoria
*/
void print_tasks_procs(){
	struct task_data* data = NULL;
	struct proc_data* proc = NULL;
	
	system("clear");	
	
	printf("\t\t\tTASKS\n\n");
	for(data = g_tasks; data != NULL; data=data->hh.next) {
		if(data->valid != 0){		
		printf("tid = %d : pid = %d : ppid = %d gid = %d : uid = %d : exe = %s \n",
			data->tid, data->pid, data->father_pid, data->gid, data->uid, data->exe);
		}
	}
	printf("\n\n\t\t\tPROCS\n\n");
	for(proc = g_procs; proc != NULL; proc=proc->hh.next) {
		if(proc->valid != 0){		
		printf("pid = %d actual_memory = %lu : peak_memory = %lu : avg_load = %.2f %% : iowait: %.2f"
		 " : process_page_faults = %u \n",
		 proc->pid, proc->actual_memory, proc->peak_memory,
		 proc->avg_load, proc->iowait_time_pctg, proc->process_page_faults);
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
	struct task_data data;

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
	invalidate_task_proc(ev->tid);
	break;

	case PPM_SC_EXIT_GROUP:	
	invalidate_task_proc(ev->tid);
	break;

	case PPM_SC_EXIT:
	invalidate_task_proc(ev->tid);
	break;

	default:
	printf("Unknown event %u\n", ev->type);
	break;
	}
}

/*
	La funzione gestisce l'update dei dati di un processo, la pulizia dei 
	processi e thread morti e infine stampa su stdout i loro dati 
*/
void manage_data(scap_t* handle){

	struct proc_data* proc = NULL;

	for(proc = g_procs; proc != NULL; proc=proc->hh.next)
		finalizeProcess(proc);

	clear_task_proc(handle);	
	print_tasks_procs();
		

	for(proc = g_procs; proc != NULL; proc=proc->hh.next)
		updateProcessStats(proc);
			
}
/*	Fine modulo gestione processi	*/
