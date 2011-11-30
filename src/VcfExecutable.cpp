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

//////////////////////////////////////////////////////////////////////////
// This file contains the basic class vcf executable.

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "VcfExecutable.h"

VcfExecutable::VcfExecutable()
{
}


VcfExecutable::~VcfExecutable()
{
}


void VcfExecutable::vcfVersion()
{
    std::cerr << "Version: " << VERSION
              << "; Built: " << DATE << " by "<< USER << std::endl;
}

void VcfExecutable::vcfExecutableDescription()
{
    std::cerr << "Set of tools for operating on VCF files." << std::endl;
    vcfVersion();
}


void VcfExecutable::description()
{
    vcfExecutableDescription();
}


void VcfExecutable::usage()
{
    vcfVersion();
    std::cerr << std::endl;
    description();
}
