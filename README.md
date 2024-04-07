# Practicum_2

### 1. Author
Kaicheng Jia & Jing Zhou

### 2. Command
We have tested all scenarios when sending requests to the server via the commands below. Initially, we create two files **`data/sales.txt`** and **`data/book.txt`** on the client side for testing.

#### `WRITE`
```
./client WRITE <local> <remote>
./client WRITE <local>
```
- `local` file does not exsit on client => (file not found)
- `local` file exists on client, but not on server => (copy)
- `local` file exists on client, and has duplicates on server => (overwrite)
- `local` file exists on client, but `remote` path is missed. => (use local path)

***Test WRITE*** <br>
To simplify, we assign each file's permission as Read-Write.
```
(1) ./client WRITE data/buy.txt data/buy.txt RW
(2) ./client WRITE data/sales.txt data/copy.txt RW
(3) ./client WRITE data/sales.txt data/copy.txt RW
(4) ./client WRITE data/sales.txt RW
```


#### `GET`
```
./client GET <remote> <local>
./client GET <remote>
```
- `remote` file does not exsit on server => (file not found)
- `remote` file exists on server, but not on client => (copy)
- `remote` file exists on server, and has duplicates on client => (overwrite)
- `remote` file exists on server, but `local` path is missed. => (use remote path)

***Test GET*** 
```
(1) ./client GET data/buy.txt data/buy.txt
(2) ./client GET data/copy.txt data/copy.txt
(3) ./client GET data/copy.txt data/sales.txt
(4) ./client GET data/sales.txt
```

#### `RM`
```
./client RM <remote>
```
- `remote` file does not exsit on server => (file not found)
- `remote` file exists on server => (delete)

***Test RM*** 
```
(1) ./client RM data/buy.txt
(2) ./client RM data/copy.txt
```

#### `LS`
```
./client LS <remote>
```
- `remote` file does not exsit on server => (file not found)
- `remote` file exists on server => (display permission)

***Test LS*** <br>
Write another file with permission Read-Only to the server.
```
    ./client WRITE data/book.txt RO
(1) ./client LS data/buy.txt
(2) ./client LS data/sales.txt
(2) ./client LS data/book.txt
```

<br>


#### `Encryption and Decryption`

***Test WRITE Operation with Encryption*** <br>
```
Files ecp1.txt, ecp2.txt, ecp3.txt, ecp4.txt are located in the local data folder. We now WRITE these four files into remote folders using the encryption model. Execute the following commands:
    ./client WRITE data/ecp1.txt remote_doc/1.txt RW
    ./client WRITE data/ecp2.txt remote_doc/2.txt RW
    ./client WRITE data/ecp3.txt remote_doc/3.txt RW
    ./client WRITE data/ecp4.txt remote_doc/4.txt RW
After executing these commands, check the remote_doc directory. The files 1.txt, 2.txt, 3.txt, 4.txt will contain encrypted numerical data.
```


***Test GET Operation with Decryption*** <br>
```
In the remote_doc directory on the server, the files 1.txt, 2.txt, 3.txt, 4.txt contain encrypted numerical data. The following steps describe how to GET these files into local directories while maintaining encryption.
Execute the following commands to retrieve the files:
    ./client GET remote_doc/1.txt local/a1.txt
    ./client GET remote_doc/2.txt local/a2.txt
    ./client GET remote_doc/3.txt local/a3.txt
    ./client GET remote_doc/4.txt local/a4.txt
After successfully executing these commands, the local folder will contain a1.txt, a2.txt, a3.txt, a4.txt will will decrypted the original text file contents.
```


### 3. Multithread
1. We created an unique thread for each client's request to the server.

2. We created an unique file mutex for each file on the server, in order to protect file data when multi-threads tring to access its data.

3. To test it. We send two commands from two clients. One is to write file, another is to delete the same file. We assume that deletion will be pending before the write command is done, since they are modifying the same file on the server.

***Test Multi-thread*** <br>
Step 0: Uncomment the ```sleep(10)``` code in the at `write_file` function at file `server_func.c`. <br> Since we deliberately want to let one thread occupy a file. Remember to re-compile using the `Makefile`.

Step 1: Create two clients A & B <br>
Step 2: Client A send WRITE command to the server file `data/sales.txt` <br>
Step 3: Client B send RM command to the same server file <br>

We should see client B is waiting when client A is writing to the file and the lock status of mutex on server.
```
(client A) ./client WRITE data/sales.txt RW
(client B) ./client RM data/sales.txt
```

<br>

### 4. Permission Management
File permission is set when a file is first created on the server.
```
./client WRITE <local> <remote> <permission>
./client WRITE <local> <permission>
``` 

#### `RO (Read-Only)`
- `Write` to a new file on server => SUCCESS
- `Write` to an exsiting file on server => FAIL
- `Get` the file from server => SUCCESS
- `Remove` the file from server => FAIL

***Test Permission - Read-Only*** <br>
Before test, please comment the ```sleep(10)``` code in the at `write_file` function at file `server_func.c`. <br> Remember to re-compile using the `Makefile`.

Let's first clean and delete all files on server, and then write a Read-Only file to the server using the commands below. Please notice the info printed on the server.
```
(1) ./client WRITE data/sales.txt RO
(2) ./client WRITE data/sales.txt RO
(3) ./client GET data/sales.txt
(4) ./client RM data/sales.txt
```

#### `RW (Read-Write)`
- All commands can execute successfully.

***Test Permission - Read-Write*** <br>
Let's write a Read-Write file to the server using the commands below. Please notice the info printed on the server.
```
(1) ./client WRITE data/book.txt RW
(2) ./client WRITE data/book.txt RW
(3) ./client GET data/book.txt
(4) ./client RM data/book.txt
```