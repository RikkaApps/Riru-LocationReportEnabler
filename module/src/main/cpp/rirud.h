#pragma once

namespace rirud {

    bool ReadFile(const char *path, char *&bytes, size_t &bytes_size);
    bool ForeachDir(const char *path, void(*callback)(struct dirent *, bool *continue_read));
}