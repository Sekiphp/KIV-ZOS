all:
	clear
	rm ntfs.dat
	gcc *.c -o pseudontfs -pthread -Wall
	./pseudontfs ntfs.dat
