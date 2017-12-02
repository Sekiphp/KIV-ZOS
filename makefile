all:
	clear
	gcc *.c -o ntfs -pthread -Wall
	./ntfs soubor.txt
