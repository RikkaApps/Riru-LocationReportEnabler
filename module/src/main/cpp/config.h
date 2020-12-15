#pragma once

#include <string>

namespace Config {

    struct Property {

        std::string name;
        std::string value;

        Property(const char *name, const char *value) : name(name), value(value) {}
    };

    void Load();

    namespace Properties {

        Property *Find(const char *name);
    }

    namespace Packages {

        bool Find(const char *name);
    }
}
