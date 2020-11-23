# One_Time_Pads

-Contains 5 small programs that encrypt and decrypt information using a one-time pad-like system. The programs are accessible from the command line using standard Unix features like input/output redirection, and job control.
-Uses socket-based inter-process communication and supports up to five concurrent socket connections running at the same time.

enc_client: provides the server a plaintext file and a key file, and requests the server to encrypt the plaintext using the key.

enc_server: once a client establishes connection, enc_server forks a new process and the child process encrypts the information and sends the encrypted text (ciphertext) back to the client. original server daemon process continues listening for new connections.

dec_client: similar to enc_client, but requests the server to decrypt a ciphertext.

dec_server: similar to dec_client, but  decrypts a ciphertext and sends the plaintext back to the client.

keygen: standalone program that generates a random key for the encryption and decryption processes.
