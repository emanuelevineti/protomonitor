/*	proto_i\o headerfile	*/
/*
La funzione stampa su stdout le informazioni riguardanti l'uso del programma
*/
void print_help();
/*
La funzione si occupa di leggere i parametri passati al programma all'atto
della sua invocazione e setta le variabili globali corrispondenti alle
funzionalità richieste
*/
void read_argv(int argc, char *argv[]);

/*
La funzione stampa su stdout i dati dei processi/threads in memoria
*/
void print_tasks_procs();

/*
La funzione esporta su file le informazioni riguardanti tutti i main 
procceses in memoria 
*/
void export_data_onfile();
/*
La funzione esporta i dati di un thread appena nato su un file
*/
void export_procsdeath_data_onfile(task_data* data);
/*
La funzione esporta i dati di un thread appena morto su un file
*/
void export_procsbirth_data_onfile(task_data* data);

void exportjson_procsdeath_data_onfile(task_data* data);
void exportjson_procsbirth_data_onfile(task_data* data);
/*	end header		*/

