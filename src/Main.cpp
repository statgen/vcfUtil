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

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

#include "ReplaceReference.h"
//#include "VcfCooker.h"
#include "VcfCleaner.h"
#include "VcfExample.h"
#include "VcfConvert.h"
#include "VcfMac.h"
#include "PhoneHome.h"

void Usage()
{
    VcfExecutable::vcfExecutableDescription();
    std::cerr << std::endl;
    std::cerr << "Tools: " << std::endl;
    ReplaceReference::replaceReferenceDescription();
    VcfCleaner::vcfCleanerDescription();
    VcfConvert:: vcfConvertDescription();
    VcfMac:: vcfMacDescription();

    std::cerr << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << "\tvcfUtil <tool> [<tool arguments>]" << std::endl;
    std::cerr << "The usage for each tool is described by specifying the tool with no arguments." << std::endl;
}


int main(int argc, char ** argv)
{
    VcfExecutable* vcfExe = NULL;

    // Verify at least one arg.
    if(argc < 2)
    {
        // Not enough args...
        Usage();
        exit(-1);
    }

    String cmd = argv[1];

    if(cmd.SlowCompare("replaceReference") == 0)
    {
        vcfExe = new ReplaceReference();
    }
    else if(cmd.SlowCompare("cleaner") == 0)
    {
        vcfExe = new VcfCleaner();
    }
//     if(cmd.SlowCompare("vcfCooker") == 0)
//     {
//         vcfExe = new VcfCooker();
//     }
    else if(cmd.SlowCompare("vcfExample") == 0)
    {
        vcfExe = new VcfExample();
    }
    else if(cmd.SlowCompare("convert") == 0)
    {
        vcfExe = new VcfConvert();
    }
    else if(cmd.SlowCompare("mac") == 0)
    {
        vcfExe = new VcfMac();
    }
    else
    {
        std::cerr << "No tool specified\n";
        Usage();
        exit(-1);
    }
  
    if(vcfExe != NULL)
    {
        int returnVal = 0;
        String compStatus;
        try
        {
            returnVal = vcfExe->execute(argc, argv);
        }
        catch (std::runtime_error e)
        {
            compStatus = "Exception";
            PhoneHome::completionStatus(compStatus.c_str());
            std::string errorMsg = "Exiting due to ERROR:\n\t";
            errorMsg += e.what();
            std::cerr << errorMsg << std::endl;
            returnVal = -1;
        }

        compStatus = returnVal;
        PhoneHome::completionStatus(compStatus.c_str());
        delete vcfExe;
        vcfExe = NULL;
        return(returnVal);
    }
    return(-1);
}



