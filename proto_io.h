/*	proto_i\o headerfile	*/

void print_help();

void read_argv(int argc, char *argv[]);

/*
La funzione stampa su stdout i dati dei processi/threads in memoria
*/
void print_tasks_procs();

void write_onfile_proc_info(FILE* f,proc_data* proc);

void export_data_onfile();

void export_procsdeath_data_onfile(task_data* data);

void export_procsbirth_data_onfile(task_data* data);

/*	end header		*/

