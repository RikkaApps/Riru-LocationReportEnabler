#pragma once

#include <dirent.h>

bool foreach_dir(const char *path, void(*callback)(int, struct dirent *, bool *));
int read_full(int fd, void *buf, size_t count);
int write_full(int fd, const void *buf, size_t count);