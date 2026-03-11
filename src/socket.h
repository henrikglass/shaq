#ifndef SOCKET_H
#define SOCKET_H

/*--- Include files ---------------------------------------------------------------------*/
        
#include "str.h"
#include "hgl_int.h"
#include "shaq_config.h"

#include <sys/types.h>
#include <sys/socket.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    SOCKET_UDP, 
    SOCKET_TCP, 
} SocketKind;

typedef struct
{
    u64 last_lookup_time_ns;
    size_t last_line_length;
    StringView service;
    i32 sockfd;
    SocketKind kind;
    char last_line[SHAQ_SOCKET_LAST_LINE_BUFFER_SIZE];
} Socket;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Socket *socket_create(StringView service, SocketKind kind);
Socket *socket_lookup(StringView service);
void socket_close_unused(void);
void socket_close_all(void);

#endif /* SOCKET_H */

