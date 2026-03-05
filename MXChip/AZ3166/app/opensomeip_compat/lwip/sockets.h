#ifndef OPENSOMEIP_NETX_LWIP_SOCKETS_H
#define OPENSOMEIP_NETX_LWIP_SOCKETS_H

#ifndef NX_BSD_ENABLE_NATIVE_API
#define NX_BSD_ENABLE_NATIVE_API
#endif

#include "nxd_bsd.h"
#include <stdint.h>
#include <stddef.h>

#ifndef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 1
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 0xFFFFFFFFUL
#endif

#ifdef ERROR
#undef ERROR
#endif

#ifndef sockaddr
#define sockaddr nx_bsd_sockaddr
#endif
#ifndef sockaddr_in
#define sockaddr_in nx_bsd_sockaddr_in
#endif
#ifndef in_addr
#define in_addr nx_bsd_in_addr
#endif
#ifndef ip_mreq
#define ip_mreq nx_bsd_ip_mreq
#endif
#ifndef in_addr_t
#define in_addr_t nx_bsd_in_addr_t
#endif
#ifndef socklen_t
#define socklen_t nx_bsd_socklen_t
#endif

static inline uint16_t opensomeip_bswap16(uint16_t v)
{
    return (uint16_t)((v << 8) | (v >> 8));
}

static inline uint32_t opensomeip_bswap32(uint32_t v)
{
    return ((v & 0x000000FFUL) << 24) |
           ((v & 0x0000FF00UL) << 8) |
           ((v & 0x00FF0000UL) >> 8) |
           ((v & 0xFF000000UL) >> 24);
}

#ifdef htons
#undef htons
#endif
#ifdef ntohs
#undef ntohs
#endif
#ifdef htonl
#undef htonl
#endif
#ifdef ntohl
#undef ntohl
#endif

#define htons(v) opensomeip_bswap16((uint16_t)(v))
#define ntohs(v) opensomeip_bswap16((uint16_t)(v))
#define htonl(v) opensomeip_bswap32((uint32_t)(v))
#define ntohl(v) opensomeip_bswap32((uint32_t)(v))

static inline int lwip_socket(int domain, int type, int protocol)
{
    return nx_bsd_socket(domain, type, protocol);
}

static inline int lwip_close(int fd)
{
    return nx_bsd_soc_close(fd);
}

static inline int lwip_bind(int fd, const sockaddr* addr, socklen_t addrlen)
{
    return nx_bsd_bind(fd, (const struct nx_bsd_sockaddr*)addr, (INT)addrlen);
}

static inline int lwip_listen(int fd, int backlog)
{
    return nx_bsd_listen(fd, backlog);
}

static inline int lwip_accept(int fd, sockaddr* addr, socklen_t* addrlen)
{
    INT len = (addrlen != NULL) ? (INT)(*addrlen) : 0;
    INT rc = nx_bsd_accept(fd, (struct nx_bsd_sockaddr*)addr, &len);
    if (addrlen != NULL)
    {
        *addrlen = (socklen_t)len;
    }
    return rc;
}

static inline int lwip_connect(int fd, const sockaddr* addr, socklen_t addrlen)
{
    return nx_bsd_connect(fd, (struct nx_bsd_sockaddr*)addr, (INT)addrlen);
}

static inline int lwip_send(int fd, const void* data, size_t len, int flags)
{
    return nx_bsd_send(fd, (const CHAR*)data, (INT)len, flags);
}

static inline int lwip_sendto(int fd, const void* data, size_t len, int flags,
                              const sockaddr* to, socklen_t tolen)
{
    return nx_bsd_sendto(fd, (CHAR*)(uintptr_t)data, (INT)len, flags,
                         (struct nx_bsd_sockaddr*)to, (INT)tolen);
}

static inline int lwip_recv(int fd, void* data, size_t len, int flags)
{
    return nx_bsd_recv(fd, data, (INT)len, flags);
}

static inline int lwip_recvfrom(int fd, void* data, size_t len, int flags,
                                sockaddr* from, socklen_t* fromlen)
{
    INT len_local = (fromlen != NULL) ? (INT)(*fromlen) : 0;
    INT rc = nx_bsd_recvfrom(fd, (CHAR*)data, (INT)len, flags,
                             (struct nx_bsd_sockaddr*)from, &len_local);
    if (fromlen != NULL)
    {
        *fromlen = (socklen_t)len_local;
    }
    return rc;
}

static inline int lwip_setsockopt(int fd, int level, int optname,
                                  const void* optval, socklen_t optlen)
{
    return nx_bsd_setsockopt(fd, level, optname, optval, (INT)optlen);
}

static inline int lwip_getsockopt(int fd, int level, int optname,
                                  void* optval, socklen_t* optlen)
{
    INT len_local = (optlen != NULL) ? (INT)(*optlen) : 0;
    INT rc = nx_bsd_getsockopt(fd, level, optname, optval, &len_local);
    if (optlen != NULL)
    {
        *optlen = (socklen_t)len_local;
    }
    return rc;
}

static inline int lwip_getsockname(int fd, sockaddr* addr, socklen_t* addrlen)
{
    INT len_local = (addrlen != NULL) ? (INT)(*addrlen) : 0;
    INT rc = nx_bsd_getsockname(fd, (struct nx_bsd_sockaddr*)addr, &len_local);
    if (addrlen != NULL)
    {
        *addrlen = (socklen_t)len_local;
    }
    return rc;
}

static inline int lwip_shutdown(int fd, int how)
{
    NX_PARAMETER_NOT_USED(how);
    return nx_bsd_soc_close(fd);
}

static inline int lwip_fcntl(int fd, int cmd, int arg)
{
    return nx_bsd_fcntl(fd, (UINT)cmd, (UINT)arg);
}

static inline int lwip_select(int nfds, nx_bsd_fd_set* rfds, nx_bsd_fd_set* wfds,
                              nx_bsd_fd_set* efds, struct nx_bsd_timeval* tv)
{
    return nx_bsd_select(nfds, rfds, wfds, efds, tv);
}

static inline int lwip_poll(struct nx_bsd_pollfd* fds, ULONG nfds, int timeout)
{
    return nx_bsd_poll(fds, nfds, timeout);
}

static inline const char* lwip_inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
    return nx_bsd_inet_ntop(af, src, (CHAR*)dst, (nx_bsd_socklen_t)size);
}

static inline int lwip_inet_pton(int af, const char* src, void* dst)
{
    return nx_bsd_inet_pton(af, src, dst);
}

static inline in_addr_t lwip_inet_addr(const char* src)
{
    return nx_bsd_inet_addr(src);
}

#ifndef socket
#define socket(...) lwip_socket(__VA_ARGS__)
#endif
#ifndef close
#define close(...) lwip_close(__VA_ARGS__)
#endif
#ifndef bind
#define bind(...) lwip_bind(__VA_ARGS__)
#endif
#ifndef listen
#define listen(...) lwip_listen(__VA_ARGS__)
#endif
#ifndef accept
#define accept(...) lwip_accept(__VA_ARGS__)
#endif
#ifndef send
#define send(...) lwip_send(__VA_ARGS__)
#endif
#ifndef sendto
#define sendto(...) lwip_sendto(__VA_ARGS__)
#endif
#ifndef recv
#define recv(...) lwip_recv(__VA_ARGS__)
#endif
#ifndef recvfrom
#define recvfrom(...) lwip_recvfrom(__VA_ARGS__)
#endif
#ifndef setsockopt
#define setsockopt(...) lwip_setsockopt(__VA_ARGS__)
#endif
#ifndef getsockopt
#define getsockopt(...) lwip_getsockopt(__VA_ARGS__)
#endif
#ifndef getsockname
#define getsockname(...) lwip_getsockname(__VA_ARGS__)
#endif
#ifndef shutdown
#define shutdown(...) lwip_shutdown(__VA_ARGS__)
#endif
#ifndef fcntl
#define fcntl(...) lwip_fcntl(__VA_ARGS__)
#endif
#ifndef select
#define select(...) lwip_select(__VA_ARGS__)
#endif
#ifndef poll
#define poll(...) lwip_poll(__VA_ARGS__)
#endif
#ifndef inet_ntop
#define inet_ntop(...) lwip_inet_ntop(__VA_ARGS__)
#endif
#ifndef inet_pton
#define inet_pton(...) lwip_inet_pton(__VA_ARGS__)
#endif
#ifndef inet_addr
#define inet_addr(...) lwip_inet_addr(__VA_ARGS__)
#endif

#endif // OPENSOMEIP_NETX_LWIP_SOCKETS_H
