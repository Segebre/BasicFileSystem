proyectofinal: main.o console.o device.o file.o hash.o
	gcc -o $@ $^

main.o: main.c
	gcc -c -o $@ $^
	
console.o: console.c
	gcc -c -o $@ $^

device.o: device.c
	gcc -c -o $@ $^

file.o: file.c
	gcc -c -o $@ $^

hash.o: hash.c
	gcc -c -o $@ $^