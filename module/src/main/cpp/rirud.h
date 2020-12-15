#pragma once

namespace rirud {

    bool ReadFile(int fd, const char *path, std::string **content);

    bool ReadDir(int fd, const char *path, std::vector<std::string> &dirs);

    int OpenSocket();
}