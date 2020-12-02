#pragma once

namespace rirud {

    bool ReadFile(int fd, const char *path, char *&bytes, size_t &bytes_size);
    bool ReadDir(int fd, const char *path, std::vector<std::string> &dirs);
    int OpenSocket();
}