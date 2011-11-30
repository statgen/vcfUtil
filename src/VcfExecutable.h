/*
 *  Copyright (C) 2011  Regents of the University of Michigan
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VCF_EXECUTABLE_H__
#define __VCF_EXECUTABLE_H__

#include "StringBasics.h"
#include "Parameters.h"

/// Base Class VCF Executable.
class VcfExecutable
{
public: 
    static void vcfVersion();
    static void vcfExecutableDescription();
    VcfExecutable();
    virtual ~VcfExecutable();

    /// Print the 
    virtual void description();
    virtual void usage();
    virtual int execute(int argc, char**argv) = 0;

protected:

private:
};

#endif
