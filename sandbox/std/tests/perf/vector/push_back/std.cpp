// Copyright (c) 2008-2010 Kent State University
// Copyright (c) 2011-2012 Texas A&M University
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#include <iostream>
#include <vector>

#include "perf.hpp"
#include "test.hpp"

int main(int argc, char *argv[])
{
  origin::performance_tester env(argc, argv);
  exec<std::vector<int>>(env);
}
