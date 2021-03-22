#include <map>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/system_properties.h>
#include <riru.h>
#include "config.h"
#include "misc.h"
#include "logging.h"

using namespace Config;

namespace Config {

    namespace Properties {

        void Put(const char *name, const char *value);
    }

    namespace Packages {

        void Add(const char *name);
    }
}


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
    char buf[PATH_MAX];

    snprintf(buf, PATH_MAX, "%s/config/properties", riru_get_magisk_module_path());
    foreach_dir(buf, [](int dirfd, struct dirent *entry, bool *) {
        auto name = entry->d_name;
        int fd = openat(dirfd, name, O_RDONLY);
        if (fd == -1) return;

        char buf[PROP_VALUE_MAX]{0};
        if (read(fd, buf, PROP_VALUE_MAX) >= 0) {
            Properties::Put(name, buf);
        }

        close(fd);
    });

    snprintf(buf, PATH_MAX, "%s/config/packages", riru_get_magisk_module_path());
    foreach_dir(buf, [](int, struct dirent *entry, bool *) {
        auto name = entry->d_name;
        Packages::Add(name);
    });
}