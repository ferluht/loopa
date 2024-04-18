//
// Created by ferluht on 06/01/2024.
//

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

std::vector<std::string> list_dir(const char *path);