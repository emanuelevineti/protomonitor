protomonitor
============

Il programma permette di monitorare la vita dei processi all'interno della macchina, tracciando la nascita e la morte dei processi e calcolando statistiche su CPU, memoria ed IO nel loro periodo di esecuzione.

Come usare protomonitor:
========================

		HELP comandi 

	-p  | --proc_only		show only information about main processes
	-h  | --help			show information about the inline command
	-a  | --all			track the activity of all active processes
	-t  | --time           <1-60>	set the refresh time (1 sec to 60)
	-mc | --min_cpu	       <min>	show processes with the cpu usage value greater than <min>
	-mf | --min_pagefaults <min>	show processes with the page faults value greater than <min>
	-mi | --min_iowait    <min>	show processes with the io_wait value greater than <min>
	-l  | --log 			export proc data into a log file
	-lp | --log_path       <path>   export proc data into a log file specifying the destination path
	-lj | --log_json               	export proc data into a log file using a JSON format
	-ljp| --logjson_path   <path>	export proc data into a log file using a JSON format specifying the destination path

	-lt | --log_threads           	export threads life data on a log file
	-elk| --elk_export  <IP> <port>	export proc_data on elk


esempi:

Monitorare tutti i processi attivi sulla macchina e esportare su di un file di log le informazioni raccolte:
./protomonitor -a -l

Monitorare i processi attivi sulla macchina ed esportare i dati ogni 30 secondi verso logstash al localhost:
./protomonitor -a -elk 127.0.0.1 5652 -t 30

Come installare protomonitor:
=============================

Per l'installazione è necessario procurarsi la libreria libscap di sysdig e ELK stack(elasticsearch, logstash, kibana) per un analisi dei dati.

-libscap -> seguire le instruzioni per il download e la compilazione del codice sorgente a questo link : http://www.sysdig.org/wiki/how-to-install-sysdig-from-the-source-code/

-elasticsearch, logstash, kibana -> scaricare ed installare i tre programmi seguendo la guida a questo link: http://www.elasticsearch.org/overview/elkdownloads/ .

-Clonare la repository del protomonitor : # git clone https://github.com/emanuelevineti/protomonitor.git
è necessario modificare il file di configurazione di logstash sostituendolo con quello per la acquisizione dati del protomonitor.

Se si installato ELK come servizi sulla macchina è necessario eseguire i seguenti passi:
-copiare il file di configurazione nella repository del programma nella directory /etc/logstash/config.d/ sostituendo l'attuale configurazione in uso.
-resettare il servizio di logstash : service logstash reset .

Se si è scaricato l'archivio di logstash dal sito, basterà copiare il file di conf all'interno della directory di logstash ed eseguirlo nella seguente maniera: ./bin/logstash agent -f protomonitor.conf .

Ora siamo in grado di mandare i dati dal protomonitor a Logstash che li invierà a sua volta a elasticsearch.

Per caricare le Dashboard per l'analisi e visualizzazione dei dati da parte di kibana basterà eseguire kibana atraverso il proprio browser e caricare le dashboard atraverso i seguenti passi: 
-in alto a destra premere sul tasto load
-selezionare l'opzione advanced
-caricare i file delle dashboard all'interno della repository del protomonitor.

Dopo averli installati assicurarsi che il percorso nel Makefile nella directory protomonitor per recuperare le librerie della libscap sia giusto $HOME/sysdig/build/zlib-prefix/src/zlib/libz.a  $HOME/sysdig/build/userspace/libscap/libscap.a e $HOME/sysdig/userspace/libscap/ . 
Ora andare nella directory del protomonitor ed eseguire: # make protomonior

Prima di eseguire l'eseguibile è necessario inserire il driver di sysdig nel kernel : # sudo insmod /sysdig/build/driver/sysdig-probe.ko .





