/* Copyright (C) 2018-2020 by Arm Limited. All rights reserved. */

#ifndef INCLUDE_LIB_UTILS_H
#define INCLUDE_LIB_UTILS_H

#include <cstdint>
#include <set>
#include <sys/utsname.h>

// From include/generated/uapi/linux/version.h
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

namespace lib {

    int parseLinuxVersion(struct utsname & utsname);

    int readIntFromFile(const char * fullpath, int & value);
    int readInt64FromFile(const char * fullpath, int64_t & value);

    int writeCStringToFile(const char * fullpath, const char * data);
    int writeIntToFile(const char * path, int value);
    int writeInt64ToFile(const char * path, int64_t value);

    int writeReadIntInFile(const char * path, int & value);
    int writeReadInt64InFile(const char * path, int64_t & value);

    std::set<int> readCpuMaskFromFile(const char * path);
}

#endif // INCLUDE_LIB_UTILS_H
