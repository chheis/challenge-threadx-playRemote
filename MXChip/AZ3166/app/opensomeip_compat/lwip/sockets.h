#ifndef OPENSOMEIP_NETX_LWIP_SOCKETS_H
#define OPENSOMEIP_NETX_LWIP_SOCKETS_H

#include "nxd_bsd.h"

#ifndef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 1
#endif

#ifndef lwip_socket
#define lwip_socket socket
#endif

#ifndef lwip_close
#define lwip_close soc_close
#endif

#ifndef lwip_bind
#define lwip_bind bind
#endif

#ifndef lwip_listen
#define lwip_listen listen
#endif

#ifndef lwip_accept
#define lwip_accept accept
#endif

#ifndef lwip_connect
#define lwip_connect connect
#endif

#ifndef lwip_send
#define lwip_send send
#endif

#ifndef lwip_sendto
#define lwip_sendto sendto
#endif

#ifndef lwip_recv
#define lwip_recv recv
#endif

#ifndef lwip_recvfrom
#define lwip_recvfrom recvfrom
#endif

#ifndef lwip_setsockopt
#define lwip_setsockopt setsockopt
#endif

#ifndef lwip_getsockopt
#define lwip_getsockopt getsockopt
#endif

#ifndef lwip_getsockname
#define lwip_getsockname getsockname
#endif

#ifndef lwip_shutdown
#define lwip_shutdown shutdown
#endif

#ifndef lwip_fcntl
#define lwip_fcntl fcntl
#endif

#ifndef lwip_select
#define lwip_select select
#endif

#ifndef lwip_poll
#define lwip_poll poll
#endif

#ifndef lwip_inet_ntop
#define lwip_inet_ntop inet_ntop
#endif

#ifndef lwip_inet_pton
#define lwip_inet_pton inet_pton
#endif

#endif // OPENSOMEIP_NETX_LWIP_SOCKETS_H
