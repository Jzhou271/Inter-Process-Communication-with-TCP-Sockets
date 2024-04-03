# Practicum_2

### 1. Author
Kaicheng Jia & Jing Zhou

### 2. Command
We have tested all scenarios when sending requests to the server via the commands below:

#### `WRITE`
```
./client WRITE <local> <remote>
./client WRITE <local>
```
- `local` file does not exsit on client => (file not found)
- `local` file exists on client, but not on server => (copy)
- `local` file exists on client, and has duplicates on server => (overwrite)
- `local` file exists on client, but `remote` path is missed. => (use local path)


#### `GET`
```
./client GET <remote> <local>
./client GET <remote>
```
- `remote` file does not exsit on server => (file not found)
- `remote` file exists on server, but not on client => (copy)
- `remote` file exists on server, and has duplicates on client => (overwrite)
- `remote` file exists on server, but `local` path is missed. => (use remote path)


#### `RM`
```
./client RM <remote>
```
- `remote` file does not exsit on server => (file not found)
- `remote` file exists on server => (delete)


<br>

### 3. Multithread
1. We created an unique thread for each client's request to the server.

2. We created an unique file mutex for each file on the server, in order to protect file data when multi-threads tring to access its data.

3. To test it. We send two commands from two clients. One is to write file, another is to delete the same file. We assume that deletion will be pending before the write command is done, since they are modifying the same file on the server.


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


#### `RW (Read-Write)`
- All commands can execute successfully.