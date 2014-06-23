/*
 * JitUtil.hpp -- Jit-related utility functions
 *
 * Copyright (C) 2011, Computing Systems Laboratory (CSLab), NTUA.
 * Copyright (C) 2011, Vasileios Karakasis
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */

#ifndef SPARSEX_INTERNALS_JIT_UTIL_HPP
#define SPARSEX_INTERNALS_JIT_UTIL_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

string SourceFromFile(const char *filename);

string SourceFromFile(const string &filename)
{
    return SourceFromFile(filename.c_str());
}

void SourceToFile(const char *filename, const string &source);

void SourceToFile(const string &filename, const string &source)
{
    SourceToFile(filename.c_str(), source);
}

const char *UniqueFilename(string &tmpl);

void RemoveFile(const char *path)
{
    if (remove(path) < 0) {
        perror("RemoveFile()");
        exit(1);
    }
}

template<typename T>
string Stringify(const T& t)
{
    stringstream ss;
    ss << t;
    return ss.str();
}

string Tabify(int num)
{
    stringstream ss;
    
    for (int i = 0; i < num; i++)
        ss << "\t";
    
    return ss.str();
}
#endif  // SPARSEX_INTERNALS_JIT_UTIL_HPP

// vim:expandtab:tabstop=8:shiftwidth=4:softtabstop=4
