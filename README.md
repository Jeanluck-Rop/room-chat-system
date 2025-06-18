# Room Chat System

A robust chat system designed for real-time communication between users.

## **Features**

+ **Public Chat**:  
   - All users connected to the server can participate in a shared chatroom, where everyone can read and write messages.  

+ **Private Rooms**:  
   - Users can create private chatrooms.  
   - Only invited users can read and send messages within the respective room.  

+ **Direct Messages (DMs)**:  
   - Send one-on-one private messages to other users on the server.


## **Technical Details**  

- **Server**: Developed in **C**.  
- **Client**: Developed in **C++**.  
- **Graphical User Interface**: Built using **GTK**.  (Not implemented yet)
- **Portability**: Enhanced with **Flatpak** for cross-platform compatibility. (Not implemented yet)


## Contributors

- [Ianluck Rojo](https://github.com/Jeanluck-Rop)
- [Fernanda Ramírez](https://github.com/Mariafernandarj)


## **How to Run the Project**

1. Clone and cd the repository:  
   ```
   git clone https://github.com/Jeanluck-Rop/room-chat-system.git
   ```
   ```
   cd room-chat-system
   ```

2. Create a compilation directory and enter it to compile and run the program:  
   ```
   mkdir build && cd build
   ```
   
3. Compile the program:  
   ```
   cmake ..
   ...
   make
   [  6%] Building CXX object src/client/CMakeFiles/client_library.dir/src/main.cpp.o
   [ 13%] Building CXX object src/client/CMakeFiles/client_library.dir/src/client.cpp.o
   [ 20%] Building CXX object src/client/CMakeFiles/client_library.dir/src/message.cpp.o
   [ 26%] Building CXX object src/client/CMakeFiles/client_library.dir/ui/view.cpp.o
   [ 33%] Linking CXX static library libclient_library.a
   [ 33%] Built target client_library
   [ 40%] Building CXX object src/client/CMakeFiles/client.dir/src/main.cpp.o
   [ 46%] Linking CXX executable client
   [ 46%] Built target client
   [ 53%] Building C object src/server/CMakeFiles/server_library.dir/src/main.c.o
   [ 60%] Building C object src/server/CMakeFiles/server_library.dir/src/message.c.o
   [ 66%] Building C object src/server/CMakeFiles/server_library.dir/src/server.c.o
   [ 73%] Building C object src/server/CMakeFiles/server_library.dir/src/room.c.o
   [ 80%] Building C object src/server/CMakeFiles/server_library.dir/src/cJSON.c.o
   [ 86%] Linking C static library libserver_library.a
   [ 86%] Built target server_library
   [ 93%] Building C object src/server/CMakeFiles/server.dir/src/main.c.o
   [100%] Linking C executable server
   [100%] Built target server
   ```

4. To run the server:
   ```
   ./src/server/server <port>
   ```
   
   Example: 
   ```
   ./src/server/server 8080
   ```
   
5. To run the client, open another terminal, tab or window:
   ```
   ./src/client/client <IPv4> <port>
   ```
   Example:
   ```
   ./src/client/client 127.0.0.1 8080
   ```
   
### Contact: rpmanianck@gmail.com
