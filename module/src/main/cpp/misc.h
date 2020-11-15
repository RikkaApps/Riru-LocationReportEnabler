#pragma once

#include <dirent.h>

int foreach_dir(const char *path, void(*callback)(int dirfd, struct dirent * entry));