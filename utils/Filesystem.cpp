//
// Created by ferluht on 06/01/2024.
//

#include "Filesystem.h"

std::vector<std::string> list_dir(const char *path) {
    std::vector<std::string> ret;
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == nullptr) return ret;

    while ((entry = readdir(dir)) != nullptr)
        ret.emplace_back(entry->d_name);

    closedir(dir);
    return ret;
}
