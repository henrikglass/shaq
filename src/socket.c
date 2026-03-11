/*--- Include files ---------------------------------------------------------------------*/

#include "socket.h"

#include "shaq_core.h"
#include "alloc.h"
#include "log.h"

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
        log_error("Max # of open sockets reached");
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
        log_error("Failed to create socket. `getaddrinfo()` error: %s\n", gai_strerror(err));
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
            log_error("Failed to set socket options. errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }
        err = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
        if (err != 0) {
            log_error("Failed to set socket file descriptor options. errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }

        /* attempt to bind */
        err = bind(sockfd, ai->ai_addr, ai->ai_addrlen);
        if (err != 0) {
            log_error("Failed to bind socket. errno = %s.\n", strerror(errno));
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
        .service             = sv_make_copy(service, p2p_fs_alloc),
        .kind                = SOCKET_UDP,
        .sockfd              = sockfd,
        .last_line_length    = 0,
        .last_lookup_time_ns = shaq_timestamp_ns(),
    };
    sockets.arr[sockets.count] = s;
    sockets.count++;

    log_info("Created socket on port `" SV_FMT "`.", SV_ARG(service));

    return &sockets.arr[sockets.count - 1];
}

Socket *socket_lookup(StringView service)
{
    for (u32 i = 0; i < sockets.count; i++) {
        Socket *s = &sockets.arr[i];
        if (sv_equals(s->service, service)) {
            s->last_lookup_time_ns = shaq_timestamp_ns();
            return s;
        }
    }
    return NULL;
}

void socket_close_unused()
{
    u64 now = shaq_timestamp_ns();
    for (u32 i = 0; i < sockets.count; i++) {
        Socket *s = &sockets.arr[i];
        
        /* delta is smaller than threshold -> leave open & continue */
        u64 dt_ms = (now - s->last_lookup_time_ns) / 1000000;
        if (dt_ms < SHAQ_SOCKET_AUTOCLOSE_THRESHOLD_MS) {
            continue;
        }

        /* delta is larget than threshold -> close & remove w. backswap strategy */
        close(s->sockfd);
        log_info("Closed inactive socket on port `" SV_FMT "`. Socket was not used for %d seconds.",
                SV_ARG(s->service), SHAQ_SOCKET_AUTOCLOSE_THRESHOLD_MS/1000);
        Socket *back = &sockets.arr[sockets.count - 1];
        if (back != s) {
            memcpy(s, back, sizeof(Socket));
        }
        sockets.count--;
    }
}

void socket_close_all()
{
    for (u32 i = 0; i < sockets.count; i++) {
        Socket *s = &sockets.arr[i];
        close(s->sockfd);
    }
    sockets.count = 0;
}

/*--- Private functions -----------------------------------------------------------------*/


