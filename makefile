default: all
keygen : keygen.c
	gcc -std=gnu99 -o keygen keygen.c
dec_client : dec_client.c
	gcc -std=gnu99 -o dec_client dec_client.c
dec_server : dec_server.c
	gcc -std=gnu99 -o dec_server dec_server.c
enc_client : enc_client.c
	gcc -std=gnu99 -o enc_client enc_client.c
enc_server : end_server.c
	gcc -std=gnu99 -o enc_server enc_server.c

all:
	gcc -std=gnu99 -o keygen keygen.c
	gcc -std=gnu99 -o dec_client dec_client.c
	gcc -std=gnu99 -o dec_server dec_server.c
	gcc -std=gnu99 -o enc_server enc_server.c
	gcc -std=gnu99 -o enc_client enc_client.c

