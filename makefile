all:
	clear
	gcc *.c -o ntfs -pthread -Wall
	./ntfs ntfs.dat
