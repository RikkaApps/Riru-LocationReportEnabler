#include <map>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/system_properties.h>
#include "config.h"
#include "misc.h"
#include "logging.h"
#include "rirud.h"

using namespace Config;

#define CONFIG_PATH "/data/adb/riru/modules/" RIRU_MODULE_ID "/config"
#define PROPS_PATH CONFIG_PATH "/properties"
#define PACKAGES_PATH CONFIG_PATH "/packages"

static std::map<std::string, Property *> props;
static std::vector<std::string> packages;

Property *Properties::Find(const char *name) {
    if (!name) return nullptr;

    auto it = props.find(name);
    if (it != props.end()) {
        return it->second;
    }
    return nullptr;
}

void Properties::Put(const char *name, const char *value) {
    if (!name) return;

    auto prop = Find(name);
    delete prop;

    props[name] = new Property(name, value ? value : "");

    LOGD("property: %s %s", name, value);
}

bool Packages::Find(const char *name) {
    if (!name) return false;
    return std::find(packages.begin(), packages.end(), name) != packages.end();
}

void Packages::Add(const char *name) {
    if (!name) return;
    packages.emplace_back(name);

    LOGD("package: %s", name);
}

void Config::Load() {
    auto rirud_fd = rirud::OpenSocket();
    if (rirud_fd != -1) {
        LOGD("try read from rirud");

        std::vector<std::string> prop_dirs, package_dirs;
        if (rirud::ReadDir(rirud_fd, PROPS_PATH, prop_dirs)
            && rirud::ReadDir(rirud_fd, PACKAGES_PATH, package_dirs)) {
            LOGD("read from rirud succeed");

            for (const auto &name : prop_dirs) {
                char path[PATH_MAX];
                char *buf;
                size_t size;

                snprintf(path, PATH_MAX, "%s/%s", PROPS_PATH, name.c_str());

                if (rirud::ReadFile(rirud_fd, path, buf, size)) {
                    Properties::Put(name.c_str(), buf);
                    if (buf) free(buf);
                }
            }

            for (const auto &name : package_dirs) {
                Packages::Add(name.c_str());
            }
        }

        close(rirud_fd);
    } else {
        LOGD("read from rirud failed");

        foreach_dir(PROPS_PATH, [](int dirfd, struct dirent *entry, bool *) {
            auto name = entry->d_name;
            int fd = openat(dirfd, name, O_RDONLY);
            if (fd == -1) return;

            char buf[PROP_VALUE_MAX]{0};
            if (read(fd, buf, PROP_VALUE_MAX) >= 0) {
                Properties::Put(name, buf);
            }

            close(fd);
        });

        foreach_dir(PACKAGES_PATH, [](int, struct dirent *entry, bool *) {
            auto name = entry->d_name;
            Packages::Add(name);
        });
    }
}