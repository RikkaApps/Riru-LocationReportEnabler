#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cinttypes>
#include <malloc.h>
#include <dirent.h>
#include <string>
#include <vector>
#include "riru.h"
#include "rirud.h"
#include "logging.h"
#include "socket.h"
#include "misc.h"

#define SOCKET_ADDRESS "rirud"

static const uint32_t ACTION_READ_FILE = 4;
static const uint32_t ACTION_READ_DIR = 5;

int rirud::OpenSocket() {
    if (riru_api_version < 10) return -1;

    int fd;
    struct sockaddr_un addr{};
    socklen_t socklen;

    if ((fd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0) {
        PLOGE("socket");
        return -1;
    }

    socklen = setup_sockaddr(&addr, SOCKET_ADDRESS);

    if (connect(fd, (struct sockaddr *) &addr, socklen) == -1) {
        PLOGE("connect %s", SOCKET_ADDRESS);
        close(fd);
        return -1;
    }

    return fd;
}

bool rirud::ReadFile(int fd, const char *path, std::string **content) {
    uint32_t path_size = strlen(path);
    int32_t reply;
    int32_t file_size;
    uint32_t buffer_size = 1024 * 8;
    bool res = false;

    char *bytes = nullptr;
    ssize_t bytes_size = 0;

    if (write_full(fd, &ACTION_READ_FILE, sizeof(uint32_t)) != 0
        || write_full(fd, &path_size, sizeof(uint32_t)) != 0
        || write_full(fd, path, path_size) != 0) {
        PLOGE("write %s", SOCKET_ADDRESS);
        goto clean;
    }

    if (read_full(fd, &reply, sizeof(int32_t)) != 0) {
        PLOGE("read %s", SOCKET_ADDRESS);
        goto clean;
    }

    if (reply != 0) {
        LOGE("open %s failed with %d from remote: %s", path, reply, strerror(reply));
        errno = reply;
        goto clean;
    }

    if (read_full(fd, &file_size, sizeof(uint32_t)) != 0) {
        PLOGE("read %s", SOCKET_ADDRESS);
        goto clean;
    }

    LOGD("%s size %d", path, file_size);

    if (file_size > 0) {
        bytes = (char *) malloc(file_size);
        while (file_size > 0) {
            LOGD("attempt to read %d bytes", (int) buffer_size);
            auto read_size = TEMP_FAILURE_RETRY(read(fd, bytes + bytes_size, buffer_size));
            if (read_size == -1) {
                PLOGE("read");
                goto clean;
            }

            file_size -= read_size;
            bytes_size += read_size;
            LOGD("read %d bytes (total %d)", (int) read_size, (int) bytes_size);
        }
        res = true;
        *content = new std::string(bytes, bytes_size);
    } else if (file_size == 0) {
        while (true) {
            if (bytes == nullptr) {
                bytes = (char *) malloc(buffer_size);
            } else {
                bytes = (char *) realloc(bytes, bytes_size + buffer_size);
            }

            LOGD("attempt to read %d bytes", (int) buffer_size);
            auto read_size = TEMP_FAILURE_RETRY(read(fd, bytes + bytes_size, buffer_size));
            if (read_size == -1) {
                PLOGE("read");
                goto clean;
            }
            if (read_size == 0) {
                res = true;
                *content = new std::string(bytes, bytes_size);
                goto clean;
            }

            bytes_size += read_size;
            LOGD("read %d bytes (total %d)", (int) read_size, (int) bytes_size);
        }
    }

    clean:
    return res;
}

bool rirud::ReadDir(int fd, const char *path, std::vector<std::string> &dirs) {
    uint32_t path_size = strlen(path);
    int32_t reply;
    bool res = false;
    bool continue_read = true;
    dirent dirent{};

    if (write_full(fd, &ACTION_READ_DIR, sizeof(uint32_t)) != 0
        || write_full(fd, &path_size, sizeof(uint32_t)) != 0
        || write_full(fd, path, path_size) != 0) {
        PLOGE("write %s", SOCKET_ADDRESS);
        goto clean;
    }

    if (read_full(fd, &reply, sizeof(int32_t)) != 0) {
        PLOGE("read %s", SOCKET_ADDRESS);
        goto clean;
    }

    if (reply != 0) {
        LOGE("opendir %s failed with %d from remote: %s", path, reply, strerror(reply));
        errno = reply;
        goto clean;
    }

    while (true) {
        if (write_full(fd, &continue_read, sizeof(uint8_t)) != 0) {
            PLOGE("write %s", SOCKET_ADDRESS);
            goto clean;
        }

        if (read_full(fd, &reply, sizeof(int32_t)) != 0) {
            PLOGE("read %s", SOCKET_ADDRESS);
            goto clean;
        }

        if (reply == -1) {
            res = true;
            goto clean;
        }

        if (reply != 0) {
            LOGE("opendir %s failed with %d from remote: %s", path, reply, strerror(reply));
            continue;
        }

        if (read_full(fd, &dirent.d_type, sizeof(unsigned char)) != 0
            || read_full(fd, dirent.d_name, 256) != 0) {
            PLOGE("read %s", SOCKET_ADDRESS);
            goto clean;
        }

        if (dirent.d_name[0] != '.') {
            dirs.emplace_back(dirent.d_name);
        }
    }

    clean:
    return res;
}