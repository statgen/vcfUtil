/*
 *  Copyright (C) 2012  Regents of the University of Michigan
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
#include "VcfConvert.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"

void VcfConvert::vcfConvertDescription()
{
    std::cerr << " convert - rewrite the vcf file" << std::endl;
}


void VcfConvert::description()
{
    vcfConvertDescription();
}


void VcfConvert::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil convert --in <input VCF File> --out <output VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to read\n"
              << "\t\t--out     : VCF file to write\n"
              << "\tOptional Parameters:\n"
              << "\t\t--uncompress : write an uncompressed VCF output file\n"
              << "\t\t--refName    : the reference (chromosome) name to read\n"
              << "\t\t               Defaults to all references.\n"
              << "\t\t--noeof      : do not expect an EOF block on a BGZF file\n"
              << "\t\t--params     : print the parameter settings\n"
              << std::endl;
}



int VcfConvert::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcf = "";
    String refName = "";
    bool uncompress = false;
    bool params = false;
    bool noeof = false;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_PARAMETER_GROUP("Required Parameters")
        LONG_STRINGPARAMETER("in", &inputVcf)
        LONG_STRINGPARAMETER("out", &outputVcf)
        LONG_PARAMETER_GROUP("Optional Parameters")
        LONG_PARAMETER("uncompress", &uncompress)
        LONG_STRINGPARAMETER("refName", &refName)
        LONG_PARAMETER("noeof", &noeof)
        LONG_PARAMETER("params", &params)
        LONG_PHONEHOME(VERSION)
        END_LONG_PARAMETERS();
   
    inputParameters.Add(new LongParameters ("Input Parameters", 
                                            longParameterList));
    
    inputParameters.Read(argc-1, &(argv[1]));
    
    // Check that all files were specified.
    if(inputVcf == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--in\", a required parameter.\n\n";
        return(-1);
    }
    if(outputVcf == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--out\", a required parameter.\n\n";
        return(-1);
    }

    if(params)
    {
        inputParameters.Status();
    }

    // If no eof block is required for a bgzf file, set the bgzf file type to 
    // not look for it.
    if(noeof)
    {
        // Set that the eof block is not required.
        BgzfFileType::setRequireEofBlock(false);
    }

    VcfFileReader inFile;
    VcfFileWriter outFile;
    VcfHeader header;
    
    // Open the file.
    inFile.open(inputVcf, header);

    if(refName != "")
    {
        inFile.setReadSection(refName.c_str());
    }

    if(uncompress)
    {
        outFile.open(outputVcf, header, InputFile::DEFAULT);
    }
    else
    {
        outFile.open(outputVcf, header);
    }

    VcfRecord record;
    int numRecords = 0;

    while(inFile.readRecord(record))
    {
        ++numRecords;

        outFile.writeRecord(record);
    }
 
    inFile.close();   

    std::cerr << "NumRecords: " << numRecords << "\n";
    return(0);
}
