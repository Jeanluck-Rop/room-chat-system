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
- **Graphical User Interface**: Built using **GTK-4**.


## **How to Run the Project**

1. Clone and cd the repository:  
   ```
   git clone https://github.com/Jeanluck-Rop/room-chat-system.git
   ```
   ```
   cd room-chat-system
   ```

2. Check Gtk-4 version, you must have 4.10.X or greater:
	```
	pkg-config --modversion gtk4
	```
	
	If you don't have gtk 4.10 you can do:
	```
	git clone https://gitlab.gnome.org/GNOME/gtk.git
	```
	```
	cd gtk
	```
	```
	meson setup builddir --prefix=/usr/local
	```
	```
	meson compile -C builddir
	```
	```
	sudo meson install -C builddir
	```

    And check again the Gtk-4 version:
	```
	pkg-config --modversion gtk4
	```
	   
3. Create a compilation directory and enter it to compile and run the program:  
   ```
   mkdir build && cd build
   ```
   
4. Compile the program:  
   ```
   cmake ..
   ```
   ```
   $ make -j
   [  6%] Building CXX object src/client/CMakeFiles/client_library.dir/src/main.cpp.o
   ...
   [ 46%] Built target client
   [ 53%] Building C object src/server/CMakeFiles/server_library.dir/src/main.c.o
   ...
   [100%] Built target server
   ```

5. To run the server:
   ```
   ./src/server/server <port>
   ```
   
   Example: 
   ```
   ./src/server/server 8080
   ```
   
6. To run the client, open another terminal, tab or window:
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
