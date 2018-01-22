all:
	clear
	gcc *.c -o pseudontfs -pthread -Wall
	./pseudontfs ntfs.dat
