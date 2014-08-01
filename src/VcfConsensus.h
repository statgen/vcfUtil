/*
 *  Copyright (C) 2014  Regents of the University of Michigan
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

#ifndef __VCF_CONSENSUS_H__
#define __VCF_CONSENSUS_H__

#include "VcfExecutable.h"

class VcfConsensus : public VcfExecutable
{
public:
    static void vcfConsensusDescription();
    virtual void description();
    void usage();
    int execute(int argc, char **argv);

private:
};

#endif
