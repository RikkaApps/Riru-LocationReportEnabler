#include <dirent.h>

int foreach_dir(const char *path, void(*callback)(int, struct dirent *)) {
    DIR *dir;
    struct dirent *entry;
    int fd;

    if ((dir = opendir(path)) == nullptr)
        return -1;

    fd = dirfd(dir);

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        callback(fd, entry);
    }

    closedir(dir);

    return 0;
}