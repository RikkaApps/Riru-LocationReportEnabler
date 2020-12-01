#include <dirent.h>
#include <unistd.h>
#include <cerrno>

bool foreach_dir(const char *path, void(*callback)(int, struct dirent *, bool *)) {
    DIR *dir;
    struct dirent *entry;
    int fd;
    bool continue_read = true;

    if ((dir = opendir(path)) == nullptr)
        return false;

    fd = dirfd(dir);

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        callback(fd, entry, &continue_read);
        if (!continue_read) break;
    }

    closedir(dir);
    return true;
}

static ssize_t read_eintr(int fd, void *out, size_t len) {
    ssize_t ret;
    do {
        ret = read(fd, out, len);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

int read_full(int fd, void *out, size_t len) {
    while (len > 0) {
        ssize_t ret = read_eintr(fd, out, len);
        if (ret <= 0) {
            return -1;
        }
        out = (void *) ((uintptr_t) out + ret);
        len -= ret;
    }
    return 0;
}

int write_full(int fd, const void *buf, size_t count) {
    while (count > 0) {
        ssize_t size = write(fd, buf, count < SSIZE_MAX ? count : SSIZE_MAX);
        if (size == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        buf = (const void *) ((uintptr_t) buf + size);
        count -= size;
    }
    return 0;
}