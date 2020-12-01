#ifndef SOCKET_H
#define SOCKET_H

socklen_t setup_sockaddr(struct sockaddr_un *sun, const char *name);

#endif // SOCKET_H