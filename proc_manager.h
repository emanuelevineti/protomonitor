
/*	Headerfile proc_manager.c	*/

void manage_data(scap_t* handle);
/*
La funzione si occupa dell settaggio come non valido di un processo/thread
appena morto
*/
void invalidate_task_proc(u_int16_t tid);

/*
La funzione si occupa dell'inserimento dei processi nelle tabelle dei 
processi e dei threads. Questo avviene solo se questi non sono già presenti
in queste.
*/
void add_task_proc(struct task_data* data);
/*
La funzione si occupa dell'eliminazione delle strutture dei processi/thread 
non più in uso.
*/
void clear_task_proc(scap_t* handle);

/*
La funzione stampa su stdout i dati dei processi/threads in memoria
*/
void print_tasks_procs();
/*
La funzione recupera e gestisce i dati relativi ad un evento di sistema
*/
void handle_event(struct ppm_evt_hdr* ev, u_int16_t cpuid, scap_t *h);

/*	End Header	*/
