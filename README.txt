This program encrypts and decrypts plaintext into ciphertext using a one-time pad-like system.
Valid characters are uppercase letters and

It accessible from the command line using standard Unix features like input/output redirection, and job control.

in exactly the same fashion as above, except it will be using modulo 27 operations: your 27 characters are the 26 capital letters, and the space character. All 27 characters will be encrypted and decrypted as above.

To do this, you will be creating five small programs in C. Two of these will function as servers, and will be accessed using network sockets. Two will be clients, each one of these will use one of the servers to perform work, and the last program is a standalone utility.

Your programs must use the API for network IPC that we have discussed in the class (socket, connect, bind, listen, & accept to establish connections; send, recv to send and receive sequences of bytes) for the purposes of encryption and decryption by the appropriate servers.