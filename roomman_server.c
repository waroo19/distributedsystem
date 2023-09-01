/**
 * @file
 * @brief	Roommanager Server
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "roommanprotocol.h"
#include "roommanrpc.h"
#include "roomman.h"

#define DEFAULT_PORT    (2323U)
#define DEFAULT_HOST    "127.0.0.1"
#define BUFFER_SIZE 1024
/**
 * @brief Print usage information
 */
static void rtfm(char* argv[])
{
    printf("Usage: %s {<Option>} <param1> \n", argv[0]);
    printf("Function: Room management server\n");
    printf("Options:\n");
    printf("     -p {<port>}                    - port to run the server\n");
    printf("     -h {<IP address>}              - IP address to run the server on (default: 127.0.0.1\n");



}

int rpc_encode_sstring(uint8_t* buffer, char* str, size_t size) {
    // Copy the string into the buffer
    size_t i;
    for (i = 0; i < size; i++) {
        if (str[i] == '\0')
            break;
        buffer[i] = (uint8_t)str[i];
    }
    // Fill the remaining bytes with 0
    for (; i < 64; i++) {
        buffer[i] = 0;
    }
    return 64;
}
int rpc_decode_int32(uint8_t* buffer, int32_t* data) {
    *data = ((int32_t)buffer[0] << 24) | ((int32_t)buffer[1] << 16) | ((int32_t)buffer[2] << 8) | buffer[3];
    return 4;
}
int rpc_decode_sstring(uint8_t* buffer, char* str, size_t size) {
    // Copy the buffer into the string
    size_t i;
    for (i = 0; i < size; i++) {
        if (buffer[i] == 0)
            break;
        str[i] = (char)buffer[i];
    }
    // Null-terminate the string
    str[i] = '\0';
    return i;
}

int rpc_decode_uint16(uint8_t* buffer, uint16_t* data) {
 
    if (buffer[1] == 0x00)
    {
        *data = (uint16_t)buffer[0];

    } else
    {
       *data = ((uint16_t)buffer[0] << 8) | buffer[1];
    }
    
    return 2;
}
int rpc_encode_uint16(uint8_t* buffer, uint16_t data) {
    buffer[0] = (data >> 8) & 0xFF;
    buffer[1] = data & 0xFF;
    return 2;
}
int rpc_encode_octet(uint8_t* buffer, uint8_t data){
    buffer[0] = data;
    return 1; 
}
int rpc_encode_int16(uint8_t* buffer, int16_t data) {
    buffer[0] = (data >> 8) & 0xFF;
    buffer[1] = data & 0xFF;
    return 2;
}
int rpc_encode_int32(uint8_t* buffer, int32_t data) {
    buffer[0] = (data >> 24) & 0xFF;
    buffer[1] = (data >> 16) & 0xFF;
    buffer[2] = (data >> 8) & 0xFF;
    buffer[3] = data & 0xFF;
    return 4;
}
int start_server(char *ipaddr, unsigned long port)
{   
    // setup socket
    int server_Socket1 = socket(AF_INET, SOCK_STREAM, 0);
    int server_Socket2 = socket(AF_INET, SOCK_STREAM, 0);

    if (server_Socket1 < 0)
    {
        perror("Socket 1 creation failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (server_Socket2 < 0)
    {
        perror("Socket 2 creation failed\n");
        exit(EXIT_FAILURE);
    }
    // Add the file descriptors to the respective sets
 /*    FD_SET(sd1, &readfds);
    FD_SET(sd2, &readfds);
    int rc = select(nfds, &readfds, &writefds, &exceptfds, &timeout);

 */
    // configure socket 1
    int opt_val = 1;
    int set_socket1 = setsockopt(server_Socket1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val));

    if (set_socket1 < 0)
    {
        perror("Socket setup failed\n");
        exit(EXIT_FAILURE);
    }
    
    // configure socket 2
    int set_socket2 = setsockopt(server_Socket2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val));

    if (set_socket2 < 0)
    {
        perror("Socket setup failed\n");
        exit(EXIT_FAILURE);
    }
    
    // bind socket 1
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DEFAULT_PORT);
    int bind_socket1 = bind(server_Socket1, (struct sockaddr*)&address, sizeof(address));

    if (bind_socket1 < 0)
    {   
        perror("Socket 1 binding failed\n");
        exit(EXIT_FAILURE);
    }
    // bind socket 2
    struct sockaddr_in address2;
    address2.sin_family = AF_INET;
    address2.sin_addr.s_addr = INADDR_ANY;
    address2.sin_port = htons(DEFAULT_PORT);
    int bind_socket2 = bind(server_Socket2, (struct sockaddr*)&address2, sizeof(address2));

    if (bind_socket2 < 0)
    {   
        perror("Socket 2 binding failed\n");
        exit(EXIT_FAILURE);
    }
    

    // listen for connections 1
    int listen_socket = listen(server_Socket1, 5);
    if (listen_socket < 0)
    {
        perror("Listening failed\n");
        exit(EXIT_FAILURE);
    } else
    {
        printf("Listening succesful\n");
    }
    // listen for connections 2
    int listen_socket2 = listen(server_Socket2, 5);
    if (listen_socket2 < 0)
    {
        perror("Listening failed\n");
        exit(EXIT_FAILURE);
    } else
    {
        printf("Listening succesful\n");
    }

    fd_set fds;  // File descriptor sets
    struct timeval timeout;  // Timeout value
    
    FD_ZERO(&fds);
    timeout.tv_sec = 10;  // Set timeout to 5 seconds
    timeout.tv_usec = 0;
    int client_socket1, client_socket2;

    // Add the file descriptors to the respective sets
    FD_SET(server_Socket1, &fds);
    FD_SET(server_Socket2, &fds);
    int rc = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

    if (rc == -1) {
        perror("Error with selection");
        // Handle error
    } else if (rc == 0) {
        printf("Timeout occurred\n");
    } else {
       for (int i = 0; i < FD_SETSIZE; i++)
       {
        
       if (FD_ISSET(server_Socket1, &fds))
       {
            struct sockaddr_in clientaddr;
            socklen_t clientaddrlen = sizeof(clientaddr);
            client_socket1 = accept(server_Socket1, (struct sockaddr*)&clientaddr, &clientaddrlen);
            if (client_socket1 == -1)
                {
                    //Error Handling
                    perror("Failed to accept client connection\n");
                    exit(EXIT_FAILURE);
                }else
                {
                    printf("Accepting 1 succesful\n");
                }
            while (1)
            { printf("check sd 1\n");
                 // Receive request from client
                uint8_t buffer[BUFFER_SIZE];
                int bytes_received = recv(client_socket1, buffer, BUFFER_SIZE -1, 0);
                if (bytes_received == -1)
                {
                    perror("Failed to receive request from client");
                    exit(EXIT_FAILURE);

                } else if ( bytes_received == 0)
                {
                    printf("Client disconnected\n");
                    exit(EXIT_FAILURE);

                } else
                {
                buffer[bytes_received] = '\0';
                if(bytes_received >= ROOMMAN_RPC_PROT_HEADER_SIZE)
                {   
                    
                    printf("Client sent \n");
                    uint8_t version = buffer[0] >> 4;
                    uint8_t flags = buffer[1];
                    uint16_t messageID = (buffer[2] << 8) | buffer[3];
                    uint16_t payloadLength = (buffer[4] << 8) | buffer[5];
                    uint16_t rpcCallID = (buffer[6] << 8) | buffer[7];
                    
                    // Decode header
                    printf("Version: %u\n", version);
                    printf("Flags: 0x%02X\n", flags);
                    printf("Message ID: %u\n", messageID);
                    printf("Payload Length: %u\n", payloadLength);
                    printf("RPC Call ID: %u\n", rpcCallID);
                    printf("Payload \n");
                    for (size_t i = 0; i < payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE +1 ; i++)
                    {
                        printf("byte %ld 0x%02X \n",i, buffer[i]);
                    }

                    uint16_t optionFlag = buffer[7];
                    //Decode Payload
                    size_t offset = ROOMMAN_RPC_PROT_HEADER_SIZE;
                    char buldingName[ROOMMAN_MAX_NAME+1];
                    char roomName[ROOMMAN_MAX_NAME+1];
                    uint16_t maxPers; 
                    int32_t roomIDDecode;
                    if (optionFlag == ROOMMAN_RPC_FUNCID_CREATE || optionFlag == ROOMMAN_RPC_FUNCID_LOOKUP)
                    {
                        
                        offset += rpc_decode_sstring(buffer + offset, buldingName, sizeof(buldingName));
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        
                        printf("Offset %ld \n",offset);
                        offset += rpc_decode_sstring(buffer + offset, roomName, sizeof(roomName));
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        
                        printf("Offset %ld \n",offset);
                        printf("Bulding: %s\n", buldingName);
                        printf("Room: %s\n", roomName);
                       
                        if (offset < (payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE + 1))
                        {
                            offset += rpc_decode_uint16(buffer+ offset, &maxPers);
                            printf("Max Persons: %u\n",maxPers);
                        }
                        
                      } else if (optionFlag == ROOMMAN_RPC_FUNCID_DELETE || optionFlag == ROOMMAN_RPC_FUNCID_UPDATE || 
                                optionFlag == ROOMMAN_RPC_FUNCID_RESERVE|| optionFlag == ROOMMAN_RPC_FUNCID_CLEAR || optionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                        {
                        offset += rpc_decode_int32(buffer + offset,&roomIDDecode);
                        printf("Room ID : %d \n", roomIDDecode);
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        printf("Offset %ld \n",offset);

                        if (offset < (payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE + 1))
                        {
                            offset += rpc_decode_uint16(buffer+ offset, &maxPers);
                            printf("Update Max Persons: %u\n",maxPers);
                        }
                        
                         } else
                            {
                                /* code */
                            }
                            
                    char *prBuildingName = buldingName;
                    char *prRoomName = roomName;
                    
                    
                    
                    // Determine the corresponding room management function to call
                    // And Call the room management function and obtain the response
                    uint8_t reFlag = 0x01;
                    int16_t returnValue = 0;
                    int32_t returnRoomID = 0;
                    int16_t rePayloadLength = 0;
                    uint16_t reoOptionFlag;
                    char reBuldingName[ROOMMAN_MAX_NAME+1];
                    char reRoomName[ROOMMAN_MAX_NAME+1];
	                uint16_t reCapacity[10];
                    int32_t initial = -1;

                    if (optionFlag == ROOMMAN_RPC_FUNCID_INIT)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_INIT;
                        printf("roomman_init() RPC call\n");
                        int16_t check = roomman_init(true);
                        returnValue = check;
                        rePayloadLength += 2;
                  
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_CREATE)
                    {  reoOptionFlag = ROOMMAN_RPC_FUNCID_CREATE;
                        printf("roomman_create_room() RPC call\n");
                        roomid_t roomID = roomman_create_room(prBuildingName,prRoomName,maxPers);
                        if (roomID > 0)
                        {
                            printf("New created room: %d\n", roomID);
                            returnRoomID = roomID;
                            rePayloadLength += 4;
                        } else
                        {
                            printf("Error Code: %d\n", roomID);
                            returnValue = roomID;
                            reFlag = 0x03;
                            rePayloadLength += 2;
                        }
                        
                    
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_DELETE)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_DELETE;
                        printf("roomman_delete_room() RPC call\n");
                        int8_t check = roomman_delete_room(roomIDDecode);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                        
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_UPDATE)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_UPDATE;
                        printf("roomman_update_capacity() RPC call\n");
                        int16_t check = roomman_update_capacity(roomIDDecode, maxPers);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_RESERVE)
                    {
                        reoOptionFlag = ROOMMAN_RPC_FUNCID_RESERVE;
                        printf("roomman_clear_reservation() RPC call\n");
                        int16_t check = roomman_reserve_room(roomIDDecode);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_CLEAR)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_CLEAR;
                        printf("roomman_clear_reservation() RPC call\n");
                        int16_t check = roomman_clear_reservation(roomIDDecode);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_LOOKUP)
                    {  reoOptionFlag = ROOMMAN_RPC_FUNCID_LOOKUP; 
                        printf("roomman_lookup() RPC call\n");
                        roomid_t roomID = roomman_lookup(prBuildingName,prRoomName);
                        if (roomID > 0)
                        {
                            printf(" Room Found ID: %d\n", roomID);
                            returnRoomID = roomID;
                            rePayloadLength += 6;
                        } else
                        {
                            printf("Error Code: %d\n", roomID);
                            returnValue = roomID;
                            rePayloadLength += 2;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_READENTRY;
                        printf("roomman_readentry() RPC call\n");
                        int16_t check = roomman_readentry(roomIDDecode,reBuldingName,reRoomName,reCapacity);
                        returnValue = check;
                        rePayloadLength += 132;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_DIRECTORY)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_DIRECTORY;
                        printf("roomman_directory() RPC call\n");
                        roomid_t roomID = roomman_directory(&initial, NULL, NULL);
                        if (roomID > 0)
                        {
                            printf(" Room Found ID: %d\n", roomID);
                            returnRoomID = roomID;
                            rePayloadLength += 4;
                        } else
                        {
                            printf("Error Code: %d\n", roomID);
                            returnValue = roomID;
                            rePayloadLength += 2;
                            reFlag = 0x03;
                        }
                    } else
                    {
                    printf("Option Flag Failed\n");
                    }
                    
                    // Encode the response
                    
                    // Clear the buffer
                    memset(buffer, 0, sizeof(buffer));
                    /* printf("Buffer contents after clearing:\n");
                    for (int i = 0; i < sizeof(buffer); i++) {
                        printf("%02X ", buffer[i]);
                    } */
                    offset = 0;
                    //Encode Version
                    uint8_t reVersion = ROOMMAN_RPC_PROT_VERSION;
                    int byteEncoded  = rpc_encode_octet(buffer,reVersion);
                    offset += byteEncoded;
                    printf("Version 0x%02X \n",buffer[0]);

                    //Encode Flag
                    if (reFlag & 0x01) {
                        printf("First bit is set (1)\n");
                    } else {
                        printf("First bit is not set (0)\n");
                    }
                    printf("Offset %ld \n",offset);
                    

                    byteEncoded = rpc_encode_octet(buffer + offset, reFlag);
                    offset += byteEncoded;
                    printf("Flag 0x%02X \n", buffer[1]);

                    //Encode Message ID
                    uint16_t reMessageID = messageID;
                    byteEncoded = rpc_encode_uint16(buffer + offset, reMessageID);
                    offset += byteEncoded;
                    
                    //Encode Payload length
                    byteEncoded = rpc_encode_uint16(buffer + offset, rePayloadLength);
                    offset += byteEncoded;
                    //Encode RPC ID
                    byteEncoded = rpc_encode_uint16(buffer + offset, reoOptionFlag);
                    offset += byteEncoded;
                    
                    //Encode return value
                    if (returnRoomID < 0)
                    {
                        byteEncoded = rpc_encode_int16(buffer + offset, returnValue);
                        offset += byteEncoded;
                    } else
                    {  
                        byteEncoded = rpc_encode_int32(buffer + offset, returnRoomID);
                        offset += byteEncoded;

                    }
                    //Encode out Values
                    uint16_t encodeKapa;
                    if (reoOptionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                    {   printf("Encode out values\n");
                        printf("Building name : %s\n", reBuldingName);
                        printf("Room name : %s\n", reRoomName);
                        printf("Kapa : %d\n", encodeKapa);
                        byteEncoded = rpc_encode_sstring(buffer + offset, reBuldingName, sizeof(reBuldingName));
                        offset += byteEncoded;
                       
                        
                        byteEncoded = rpc_encode_sstring(buffer + offset, reRoomName, sizeof(reRoomName));
                        offset += byteEncoded;

                        encodeKapa = *reCapacity;
                        byteEncoded = rpc_encode_uint16(buffer + offset, encodeKapa);
                        offset +=  byteEncoded;

                    }
                    for (size_t i = 0; i <ROOMMAN_RPC_PROT_HEADER_SIZE + rePayloadLength ; i++)
                        {
                            printf("byte %ld 0x%02X \n",i, buffer[i]);
                        }
                    // Send the response back to the client

                    size_t bufferLength = ROOMMAN_RPC_PROT_HEADER_SIZE + rePayloadLength;
                    send(client_socket1, buffer, bufferLength,0);
                    printf("Sent response back \n");
                    printf("-------------------------------- \n");

                    } 
            
            
                shutdown(server_Socket1,2);
                close(server_Socket1);
            
                }
            }
                
       }
       else if (FD_ISSET(server_Socket2, &fds))
       {    printf("Check sd 2 \n");
            struct sockaddr_in clientaddr2;
            socklen_t clientaddrlen2 = sizeof(clientaddr2);
            client_socket2 = accept(server_Socket2, (struct sockaddr*)&clientaddr2, &clientaddrlen2);
            if (client_socket2 == -1)
                {
                    //Error Handling
                    perror("Failed to accept client connection\n");
                    exit(EXIT_FAILURE);
                }else
                {
                    printf("Accepting 2 succesful\n");
                }
            while (1)
            { printf("check sd 2\n");
                 // Receive request from client
                uint8_t buffer[BUFFER_SIZE];
                int bytes_received = recv(client_socket2, buffer, BUFFER_SIZE -1, 0);
                if (bytes_received == -1)
                {
                    perror("Failed to receive request from client");
                    exit(EXIT_FAILURE);

                } else if ( bytes_received == 0)
                {
                    printf("Client disconnected\n");
                    exit(EXIT_FAILURE);

                } else
                {
                buffer[bytes_received] = '\0';
                if(bytes_received >= ROOMMAN_RPC_PROT_HEADER_SIZE)
                {   
                    /* uint16_t decodeValue;
                    int consumedBytes = rpc_decode_uint16(buffer, &decodeValue);
                    printf("Decoded Value: %u\n", decodeValue);
                    printf("Consumed Bytes: %d\n", consumedBytes); */
                    
                    printf("Client sent \n");
                    uint8_t version = buffer[0] >> 4;
                    uint8_t flags = buffer[1];
                    uint16_t messageID = (buffer[2] << 8) | buffer[3];
                    uint16_t payloadLength = (buffer[4] << 8) | buffer[5];
                    uint16_t rpcCallID = (buffer[6] << 8) | buffer[7];
                    
                    // Decode header
                    printf("Version: %u\n", version);
                    printf("Flags: 0x%02X\n", flags);
                    printf("Message ID: %u\n", messageID);
                    printf("Payload Length: %u\n", payloadLength);
                    printf("RPC Call ID: %u\n", rpcCallID);
                    printf("Payload \n");
                    for (size_t i = 0; i < payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE +1 ; i++)
                    {
                        printf("byte %ld 0x%02X \n",i, buffer[i]);
                    }

                    uint16_t optionFlag = buffer[7];
                    //Decode Payload
                    size_t offset = ROOMMAN_RPC_PROT_HEADER_SIZE;
                    char buldingName[ROOMMAN_MAX_NAME+1];
                    char roomName[ROOMMAN_MAX_NAME+1];
                    uint16_t maxPers; 
                    int32_t roomIDDecode;
                    if (optionFlag == ROOMMAN_RPC_FUNCID_CREATE || optionFlag == ROOMMAN_RPC_FUNCID_LOOKUP)
                    {
                        
                        offset += rpc_decode_sstring(buffer + offset, buldingName, sizeof(buldingName));
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        
                        printf("Offset %ld \n",offset);
                        offset += rpc_decode_sstring(buffer + offset, roomName, sizeof(roomName));
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        
                        printf("Offset %ld \n",offset);
                        printf("Bulding: %s\n", buldingName);
                        printf("Room: %s\n", roomName);
                       
                        if (offset < (payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE + 1))
                        {
                            offset += rpc_decode_uint16(buffer+ offset, &maxPers);
                            printf("Max Persons: %u\n",maxPers);
                        }
                        
                      } else if (optionFlag == ROOMMAN_RPC_FUNCID_DELETE || optionFlag == ROOMMAN_RPC_FUNCID_UPDATE || 
                                optionFlag == ROOMMAN_RPC_FUNCID_RESERVE|| optionFlag == ROOMMAN_RPC_FUNCID_CLEAR || optionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                        {
                        offset += rpc_decode_int32(buffer + offset,&roomIDDecode);
                        printf("Room ID : %d \n", roomIDDecode);
                        while (buffer[offset] == 0x00)
                        {
                            offset += 1;
                        }
                        printf("Offset %ld \n",offset);

                        if (offset < (payloadLength + ROOMMAN_RPC_PROT_HEADER_SIZE + 1))
                        {
                            offset += rpc_decode_uint16(buffer+ offset, &maxPers);
                            printf("Update Max Persons: %u\n",maxPers);
                        }
                        
                         } else
                            {
                                /* code */
                            }
                            
                    char *prBuildingName = buldingName;
                    char *prRoomName = roomName;
                    
                    
                    
                    // Determine the corresponding room management function to call
                    // And Call the room management function and obtain the response
                    uint8_t reFlag = 0x01;
                    int16_t returnValue = 0;
                    int32_t returnRoomID = 0;
                    int16_t rePayloadLength = 0;
                    uint16_t reoOptionFlag;
                    char reBuldingName[ROOMMAN_MAX_NAME+1];
                    char reRoomName[ROOMMAN_MAX_NAME+1];
	                uint16_t reCapacity[10];
                    int32_t initial = -1;

                    if (optionFlag == ROOMMAN_RPC_FUNCID_INIT)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_INIT;
                        printf("roomman_init() RPC call\n");
                        int16_t check = roomman_init(true);
                        returnValue = check;
                        rePayloadLength += 2;
                  
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_CREATE)
                    {  reoOptionFlag = ROOMMAN_RPC_FUNCID_CREATE;
                        printf("roomman_create_room() RPC call\n");
                        roomid_t roomID = roomman_create_room(prBuildingName,prRoomName,maxPers);
                        if (roomID > 0)
                        {
                            printf("New created room: %d\n", roomID);
                            returnRoomID = roomID;
                            rePayloadLength += 4;
                        } else
                        {
                            printf("Error Code: %d\n", roomID);
                            returnValue = roomID;
                            rePayloadLength += 2;
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_DELETE)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_DELETE;
                        printf("roomman_delete_room() RPC call\n");
                        int8_t check = roomman_delete_room(roomIDDecode);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_UPDATE)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_UPDATE;
                        printf("roomman_update_capacity() RPC call\n");
                        int16_t check = roomman_update_capacity(roomIDDecode, maxPers);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_RESERVE)
                    {
                        reoOptionFlag = ROOMMAN_RPC_FUNCID_RESERVE;
                        printf("roomman_clear_reservation() RPC call\n");
                        int16_t check = roomman_reserve_room(roomIDDecode);
                        returnValue = check;
                        rePayloadLength += 2;
                        if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_CLEAR)
                    {reoOptionFlag = ROOMMAN_RPC_FUNCID_CLEAR;
                    printf("roomman_clear_reservation() RPC call\n");
                    int16_t check = roomman_clear_reservation(roomIDDecode);
                    returnValue = check;
                    rePayloadLength += 2;
                    if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_LOOKUP)
                    {  reoOptionFlag = ROOMMAN_RPC_FUNCID_LOOKUP; 
                    printf("roomman_lookup() RPC call\n");
                    roomid_t roomID = roomman_lookup(prBuildingName,prRoomName);
                    if (roomID > 0)
                    {
                        printf(" Room Found ID: %d\n", roomID);
                        returnRoomID = roomID;
                        rePayloadLength += 4;
                    } else
                    {
                        printf("Error Code: %d\n", roomID);
                        returnValue = roomID;
                        rePayloadLength += 2;
                        reFlag = 0x03;
                    }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                    {reoOptionFlag = ROOMMAN_RPC_FUNCID_READENTRY;
                    printf("roomman_readentry() RPC call\n");
                    int16_t check = roomman_readentry(roomIDDecode,reBuldingName,reRoomName,reCapacity);
                    returnValue = check;
                    rePayloadLength += 132;
                    if (check < 0)
                        {
                            reFlag = 0x03;
                        }
                    } else if (optionFlag == ROOMMAN_RPC_FUNCID_DIRECTORY)
                    {   reoOptionFlag = ROOMMAN_RPC_FUNCID_DIRECTORY;
                        printf("roomman_directory() RPC call\n");
                        roomid_t roomID = roomman_directory(&initial, NULL, NULL);
                        if (roomID > 0)
                        {
                            printf(" Room Found ID: %d\n", roomID);
                            returnRoomID = roomID;
                            rePayloadLength += 4;
                        } else
                        {
                            printf("Error Code: %d\n", roomID);
                            returnValue = roomID;
                            rePayloadLength += 2;
                            reFlag = 0x03;
                        }
                    } else
                    {
                         printf("Option Flag Failed\n");
                    }
                    
                    // Encode the response
                    
                    // Clear the buffer
                    memset(buffer, 0, sizeof(buffer));
                    /* printf("Buffer contents after clearing:\n");
                    for (int i = 0; i < sizeof(buffer); i++) {
                        printf("%02X ", buffer[i]);
                    } */
                    offset = 0;
                    //Encode Version
                    uint8_t reVersion = ROOMMAN_RPC_PROT_VERSION;
                    int byteEncoded  = rpc_encode_octet(buffer,reVersion);
                    offset += byteEncoded;
                    printf("Version 0x%02X \n",buffer[0]);

                    //Encode Flag
                    if (reFlag & 0x01) {
                        // The first bit is set (1)
                        printf("First bit is set (1)\n");
                    } else {
                        // The first bit is not set (0)
                        printf("First bit is not set (0)\n");
                    }
                    printf("Offset %ld \n",offset);
                    

                    byteEncoded = rpc_encode_octet(buffer + offset, reFlag);
                    offset += byteEncoded;
                    printf("Flag 0x%02X \n", buffer[1]);

                    //Encode Message ID
                    uint16_t reMessageID = messageID;
                    byteEncoded = rpc_encode_uint16(buffer + offset, reMessageID);
                    offset += byteEncoded;
                    
                    //Encode Payload length
                    byteEncoded = rpc_encode_uint16(buffer + offset, rePayloadLength);
                    offset += byteEncoded;
                    //Encode RPC ID
                    byteEncoded = rpc_encode_uint16(buffer + offset, reoOptionFlag);
                    offset += byteEncoded;
                    
                    //Encode return value
                    if (returnRoomID < 0)
                    {
                        byteEncoded = rpc_encode_int16(buffer + offset, returnValue);
                        offset += byteEncoded;
                    } else
                    {   
                        byteEncoded = rpc_encode_int32(buffer + offset, returnRoomID);
                        offset += byteEncoded;

                    }
                    //Encode out Values
                    uint16_t encodeKapa;
                    if (reoOptionFlag == ROOMMAN_RPC_FUNCID_READENTRY)
                    {   printf("Encode out values\n");
                        printf("Building name : %s\n", reBuldingName);
                        printf("Room name : %s\n", reRoomName);
                        printf("Kapa : %d\n", encodeKapa);
                        byteEncoded = rpc_encode_sstring(buffer + offset, reBuldingName, sizeof(reBuldingName));
                        offset += byteEncoded;
                        
                        byteEncoded = rpc_encode_sstring(buffer + offset, reRoomName, sizeof(reRoomName));
                        offset += byteEncoded;

                        encodeKapa = *reCapacity;
                        byteEncoded = rpc_encode_uint16(buffer + offset, encodeKapa);
                        offset +=  byteEncoded;

                    }
                    
                    for (size_t i = 0; i <ROOMMAN_RPC_PROT_HEADER_SIZE + rePayloadLength ; i++)
                        {
                            printf("byte %ld 0x%02X \n",i, buffer[i]);
                        }
                    // Send the response back to the client

                    size_t bufferLength = ROOMMAN_RPC_PROT_HEADER_SIZE + rePayloadLength;
                    send(client_socket2, buffer, bufferLength,0);
                    printf("Sent response back \n");
                    printf("-------------------------------- \n");

                    } 
            
            
                shutdown(server_Socket2,2);
                close(server_Socket2);
            
                }
            }
            
       }
    

       }
       
       
       
    }
    while (1)
    {
        /* code */
    }
    


    printf("client disconnected");
}

/**
 * @brief Main program
 */
int main(int argc, char* argv[])
{
    // Default hostname and Port
    unsigned long  port = DEFAULT_PORT;
    char *ipaddr = DEFAULT_HOST;
    char option;
    char *end;

    // parse arguments
    while ((option = getopt(argc, argv, "?h:p:v")) != -1)
    {
        switch(option)
        {
            case '?':
                rtfm(argv);
                return EXIT_SUCCESS;
                break;
            case 'p':
                port = strtoul(optarg, &end, 0);
                if (end == optarg)
                {
                    rtfm(argv);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                ipaddr = optarg;
                break;
        }
    }
    if(argc - optind)
    {
        rtfm(argv);
        return 0;
    }

    return start_server(ipaddr, port);
}
