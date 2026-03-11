/*--- Include files ---------------------------------------------------------------------*/

#include "socket.h"

#include "alloc.h"

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct
{
    Socket arr[SHAQ_MAX_N_SOCKETS];
    u32 count;
} sockets = {0};

/*--- Public functions ------------------------------------------------------------------*/

Socket *socket_create(StringView service, SocketKind kind)
{
    i32 err;
    i32 sockfd;
    struct addrinfo hints;
    struct addrinfo *addrs;
    struct addrinfo *ai;
    i32 socktype = 0;
    i32 prot = 0;

    assert(kind == SOCKET_UDP);

    if (sockets.count >= SHAQ_MAX_N_SOCKETS) {
        fprintf(stderr, "Max # of open sockets reached\n");
        return NULL;
    }

    socktype = SOCK_DGRAM;
    prot     = IPPROTO_UDP;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; /* use IPv4 */
    hints.ai_socktype = socktype;
    hints.ai_protocol = prot;
    hints.ai_flags    = AI_PASSIVE; /* use my ip */

    /* lookup address information */
    char *service_cstr = sv_make_cstr_copy(service, tmp_alloc);
    err = getaddrinfo(NULL, service_cstr, &hints, &addrs);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
        return NULL;
    }

    /* loop through the results and connect to the first possible option */
    for (ai = addrs; ai != NULL; ai = ai->ai_next) {
        /* attempt to open socket */
        sockfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        /* attempt to configure socket options */
        i32 yes = 1;
        err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }
        err = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }

        /* attempt to bind */
        err = bind(sockfd, ai->ai_addr, ai->ai_addrlen);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }

        /* done */
        break;
    }

    freeaddrinfo(addrs);
    if (ai == NULL || sockfd == -1 || err != 0) {
        if (sockfd != -1) {
            close(sockfd);
        }
        return NULL;
    }

    Socket s = {
        .service   = sv_make_copy(service, r2r_arena_alloc),
        .kind      = SOCKET_UDP,
        .socket_fd = sockfd,
    };
    sockets.arr[sockets.count] = s;
    sockets.count++;

    return &sockets.arr[sockets.count - 1];
}

Socket *socket_lookup(StringView service)
{
    for (u32 i = 0; i < sockets.count; i++) {
        Socket *s = &sockets.arr[i];
        if (sv_equals(s->service, service)) {
            return s;
        }
    }
    return NULL;
}

void socket_close_all()
{
    for (u32 i = 0; i < sockets.count; i++) {
        Socket *s = &sockets.arr[i];
        s->last_line_length = 0;
        close(s->socket_fd);
    }
    sockets.count = 0;
}

/*--- Private functions -----------------------------------------------------------------*/


