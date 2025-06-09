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
{ "type": "USER_LIST",
  "users": { "<user_1>": "ACTIVE",
             "<user_2>": "BUSY",
             "<user_3>": "AWAY",
             "<user_4>": "ACTIVE" } }
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


## INVITE


## JOIN_ROOM


## ROOM_USERS


## ROOM_TEXT


## LEAVE_ROOM


## DISCONNECT
Disconnect the user from the chat, including leaving all rooms where they have joined.
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
{ "type": "",
  "roomname": <>,
  "username": <> }
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


## ROOM_USERS_LIST


## ROOM_TEXT_FROM


## LEFT_ROOM


## DISCONNECTED
A user has disconnected from the chat:
```
{ "type": "DISCONNECTED",
  "username": "<user_disconnected>" }
```


### Notas:
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
