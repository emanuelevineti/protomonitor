
/*	Headerfile proc_manager.c	*/

/*
La funzione si occupa di inizializzare le tabelle hash dei tasks e processi
con i processi attivi attualemente sulla macchina
*/
void init_add_active_proc(scap_t* h);

void manage_data(scap_t* handle);
/*
La funzione si occupa dell settaggio come non valido di un processo/thread
appena morto
*/
void invalidate_task_proc(u_int16_t tid, u_int64_t death_ts);

/*
La funzione si occupa dell'inserimento dei processi nelle tabelle dei 
processi e dei threads. Questo avviene solo se questi non sono già presenti
in queste.
*/
void add_task_proc(task_data* data);
/*
La funzione si occupa dell'eliminazione delle strutture dei processi/thread 
non più in uso.
*/
void clear_task_proc(scap_t* handle);

/*
La funzione recupera e gestisce i dati relativi ad un evento di sistema
*/
void handle_event(struct ppm_evt_hdr* ev, u_int16_t cpuid, scap_t *h);

/*	End Header	*/
