# Room Chat System

A chat system app, designed for real-time communication between users.

## **Features**

+ **Public Chat**:  
   - All users connected to the server can participate in a shared chat-room, where everyone can read and write messages.  

+ **Private Rooms**:  
   - Users can create private chat-rooms.  
   - Only invited users can read and send messages within the respective room.  

+ **Direct Messages (DMs)**:  
   - Send one-on-one private messages to other users on the server.


## **Technical Details**  

- **Server**: Developed in **C**.  
- **Client**: Developed in **C++**.  
- **Graphical User Interface**: Built using **GTK**.
- **Portability**: Enhanced with **Flatpak** for cross-platform compatibility. (Not yet)


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
   $ cmake ..
   ...
   $ make 
   [  6%] Building CXX object src/client/CMakeFiles/client_library.dir/src/main.cpp.o
   ...
   [ 46%] Built target client
   [ 53%] Building C object src/server/CMakeFiles/server_library.dir/src/main.c.o
   ...
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
   
   
## Contributors

- [Ianluck Rojo](https://github.com/Jeanluck-Rop)
- [Fernanda Ramírez](https://github.com/Mariafernandarj)


### Contact: rpmanianck@gmail.com
