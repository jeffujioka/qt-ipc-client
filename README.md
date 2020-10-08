# qt-ipc-client
A simple example of how to create an IPC Client on Qt

# To build
Import the project on Qt Creator

# To send data to the server
1. At **"server name"** field set the server name (make sure to use the same name used in the server)
2. At **"Number of bytes to send"** set the number of bytes ("random" chars) to send to the server (it will be stored in a file)
3. Click on **send** button to send the data

# To get data from the server
1. At **"server name"** field set the server name (make sure to use the same name used in the server)
2. At **file name** field set the file name of the file to store the data received from the server
3. Click on **get** button to start receiving the data from the server

**Make sure the server is running before running the client**
