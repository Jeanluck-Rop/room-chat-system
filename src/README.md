# Room chat protocol

## **Type messages the server sends and receives**

## IDENTIFY
Identifies a user in the server:
```
{ "type": "IDENTIFY",
  "username": "<username>" }
```

In case of success the server responds:
```
{ "type": "RESPONSE",
  "operation": "IDENTIFY",
  "result": "SUCCESS",
  "extra": "<username>" }
```
also it sends the message NEW_USER to the rest of connected clients:
```
{ "type": "NEW_USER",
  "username": "<username>" }
```

If the username is already in use the server responds:
```
{ "type": "RESPONSE",
  "operation": "IDENTIFY",
  "result": "USER_ALREADY_EXISTS",
  "extra": "<username>" }
```


## STATUS
Changes the user status:
```
{ "type": "STATUS",
  "status": "AWAY" }
```

If the status was successfully changed, the server sends the message NEW_STATUS to the rest of connected users:
```
{ "type": "NEW_STATUS",
  "username": "<username>",
  "status": <new_user_status> }
```


## USERS
Returns the list of users in the chat:
```
{ "type": "USERS" }
```

The server responds with a dictionary with usernames and their statuses:
```
{ "type": "USER_LIST",
  "users": { "<user_1>": "<status>",
             "<user_2>": "<status>",
             "<user_3>": "<status>",
             "<user_4>": "<status>" } }
```


## TEXT
Sends a private text to the user:
```
{ "type": "TEXT",
  "username": "<recipient_username>",
  "text": "<text_content>" }
```

If the recipient user exists, the server doesn't respond and sends the message TEXT_FROM to the user:
```
{ "type": "TEXT_FROM",
  "username": "<username>",
  "text": "<text_content>" }
```

If the recipient user doesn't exists, the server respond:
```
{ "type": "RESPONSE",
  "operation": "TEXT",
  "result": "NO_SUCH_USER",
  "extra": "<recipient_username" }
```


## PUBLIC_TEXT
Sends a public text to all connected users:
```
{ "type": "PUBLIC_TEXT",
  "text": "<text_content>" }
```

The server doesn't respond and sends the message PUBLIC_TEXT_FROM to the rest of connected users:
```
{ "type": "PUBLIC_TEXT_FROM",
  "username": "<username>",
  "text": "<text_content>" }
```


## NEW_ROOM
Creates a chat room:
```
{ "type": "NEW_ROOM",
  "roomname": "<roomname_to_create>" }
```

If the room is created successfully, the server responds with:
```
{ "type": "RESPONSE",
  "operation": "NEW_ROOM",
  "result": "SUCCESS",
  "extra": "<roomname_created>" }
```
Additionally, the user who created the room becomes the first and only user in it immediately after creation.

If the room name already exists, the server responds with:
```
{ "type": "RESPONSE",
  "operation": "NEW_ROOM",
  "result": "ROOM_ALREADY_EXISTS",
  "extra": "<roomname>" }
```


## INVITE
Invites one or multiple users to a room; only users who are already in a room can invite others to it:
```
{ "type": "INVITE",
  "roomname": "<invitation_room",
  "usernames": [ "<username_1>", "<username_2>", "<username_3>" ] }
```

The room and all the users must exist. In that case, the server sends no response but sends the message INVITATION to each user in the list:
```
{ "type": "INVITATION",
  "username" "<invited_user>",
  "roomname": "<invited_room>" }
```

If the room does not exist, the server responds:
```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NO_SUCH_ROOM",
  "extra": "<roomname>" }
```

If the room exists but the user has not been invited or has not joined it yet, the server responds:
```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NOT_JOINED",
  "extra": "<roomname>" }
```

If one or more of the users do not exist, the server responds as soon as it detects the first invalid user:
```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NO_SUCH_USER",
  "extra": "<invalid_username>" }
```

If the user tries to invite himself, the server responds:
```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "SELF_INVITE",
  "extra": "<invalid_username>" }
```

If one or more of the users are already invited or member of the room, the server responds as soon as it detects the user:
```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NO_SUCH_USER",
  "extra": "<invalid_username>" }
```

If a user is already in the room or has already been invited, that user is ignored and does not receive the message INVITATION.



## JOIN_ROOM
Joins a room; the user must have been previously invited to it:
```
{ "type": "JOIN_ROOM",
  "roomname": "<room_to_join>" }
```

If the room exists and the user was previously invited, the server responds:
```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "SUCCESS",
  "extra": "<roomname>" }
```

The user then joins the room, and the server sends the message JOINED_ROOM to all users in the room:
```
{ "type": "JOINED_ROOM",
  "roomname": "<user_joined_room>",
  "username": "<user_joined>" }
```

If the room does not exist, the server responds:
```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "NO_SUCH_ROOM",
  "extra": "<roomname>" }
```

If the user was not previously invited to the room, the server responds:
```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "NOT_INVITED",
  "extra": "<roomname>" }
```


## ROOM_USERS
Returns the list of users in the room:
```
{ "type": "ROOM_USERS",
  "roomname": "<roomname_list>" }
```

If the room exists and the user has joined it, the server responds with a dictionary of users and their statuses:
```
{ "type": "ROOM_USER_LIST",
  "roomname": "<roomname_list>",
  "users": { "<user_1>": "<status>",
             "<user_2>": "<status>",
             "<user_3>": "<status>",
             "<user_4>": "<status>" } }
```

If the room does not exist, the server responds:
```
{ "type": "RESPONSE",
  "operation": "ROOM_USERS",
  "result": "NO_SUCH_ROOM",
  "extra": "<roomname>" }
```

If the room exists but the user has not been invited or has not joined yet, the server responds:
```
{ "type": "RESPONSE",
  "operation": "ROOM_USERS",
  "result": "NOT_JOINED",
  "extra": "<roomname>" }
```


## ROOM_TEXT
Sends a message to a room:
```
{ "type": "ROOM_TEXT",
  "roomname": "<room_to_text>",
  "text": "<text_content>" }
```

If the room exists and the user has joined it, the server sends no response but broadcasts the message ROOM_TEXT_FROM to the other users in the room:
```
{ "type": "ROOM_TEXT_FROM",
  "roomname": "<room_texted>",
  "username": "<recipient_username>",
  "text": "<text_content>" }
```

If the room does not exist, the server responds:
```
{ "type": "RESPONSE",
  "operation": "ROOM_TEXT",
  "result": "NO_SUCH_ROOM",
  "extra": "<roomname>" }
```

If the room exists but the user has not been invited or has not joined it yet, the server responds:
```
{ "type": "RESPONSE",
  "operation": "ROOM_TEXT",
  "result": "NOT_JOINED",
  "extra": "<roomname>" }
```


## LEAVE_ROOM
The user leaves a room:
```
{ "type": "LEAVE_ROOM",
  "roomname": "<room_to_leave>" }
```

If the room exist and the user has joined it, the server does not respond and sends the messge LEFT_ROOM to all other users in the room:
```
{ "type": "LEFT_ROOM",
  "roomname": "<leaved_room>",
  "username": "<leaver>" }
```

If the room does not exist the server responds:
```
{ "type": "RESPONSE",
  "operation": "LEAVE_ROOM",
  "result": "NO_SUCH_ROOM",
  "extra": "<roomname>" }
```

If the room exists but the user has not been invited, or has been invited but has not joined, the server responds:
```
{ "type": "RESPONSE",
  "operation": "LEAVE_ROOM",
  "result": "NOT_JOINED",
  "extra": "<roomname>" }
```


## DISCONNECT
Disconnect the user from the chat, including leaving all rooms where they have joined:
```
{ "type": "DISCONNECT" }
```

The server does not respond at all and sends the DISCONNECTED message to all connected users:
```
{ "type": "DISCONNECTED",
  "username": "<user_disconnected>" }
```

Moreover, if the user had joined rooms, the message LEFT_ROOM is sent to each room:
```
{ "type": "LEFT_ROOM",
  "roomname": "<leaved_roomname>",
  "username": "<leaver>" }
```



## **Type messages the client sends and receives**

## NEW_USER
A new user has entered the chat and identified:
```
{ "type": "NEW_USER",
  "username": "<user_connected>" }
```


## NEW_STATUS
A user changed their status:
```
{ "type": "NEW_STATUS",
  "username": "<username>",
  "status": "AWAY" }
```


## USERS_LIST
In response to USERS:
```
{ "type": "USER_LIST",
  "users": { "<user>": "<status>",
             "<user>": "<status>",
             "<user>": "<status>",
             "<user>": "<status>" } }
```


## TEXT_FROM
Receive a private text:
```
{ "type": "TEXT_FROM",
  "username": "<recipient_username>",
  "text": "<text_content>" }
```


## PUBLIC_TEXT_FROM
Receive a public text:
```
{ "type": "PUBLIC_TEXT_FROM",
  "username": "<recipient_username>",
  "text": "<text_content>" }
```


## JOINED_ ROOM
A new user joined the room:
```
{ "type": "JOINED_ROOM",
  "roomname": "<room_joined>",
  "username": "<user_joined>" }
```


## ROOM_USERS_LIST
In response to ROOM_USERS:
```
{ "type": "ROOM_USER_LIST",
  "roomname": "Sala 1",
  "users": { "<user_1>": "<status>",
             "<user_2>": "<status>",
             "<user_3>": "<status>",
             "<user_4>": "<status>" } }
```


## ROOM_TEXT_FROM
Receive a room text:
```
{ "type": "ROOM_TEXT_FROM",
  "roomname": "<text_from_roomname>",
  "username": "<recipient_username>",
  "text": "<text_content>" }
```


## LEFT_ROOM
A user left the room:
```
{ "type": "LEFT_ROOM",
  "roomname": "<leaved_roomname>",
  "username": "user_leaver" }
```


## DISCONNECTED
A user has disconnected from the chat:
```
{ "type": "DISCONNECTED",
  "username": "<user_disconnected>" }
```


### Notes:
Usernames must be limited to 8 characters, and room names to 16 characters.

When all users have left a room, the room disappears.

A user always connects with the status ACTIVE.

If a user has not identified themselves, they cannot perform any actions until they do; any message other than IDENTIFY will be responded to with the following:
```
{ "type": "RESPONSE",
  "operation": "INVALID",
  "result": "NOT_IDENTIFIED" }
```
After responding, the server will proceed to disconnect the client.

If a message is incomplete (for example, a TEXT message missing the "username" key); or fails to meet expected values (such as a status other than ACTIVE, AWAY, or BUSY); or is unrecognizable (in particular, if it is not a JSON dictionary with the key "type"); the server will respond with the following:
```
{ "type": "RESPONSE",
  "operation": "INVALID",
  "result": "INVALID" }
```
and the client will be disconnected.
