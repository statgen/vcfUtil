/*
 *  Copyright (C) 2015  Regents of the University of Michigan
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
#include "VcfSplit.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"

void VcfSplit::vcfSplitDescription()
{
    std::cerr << " split - write 1 VCF file per chromosome" << std::endl;
}


void VcfSplit::description()
{
    vcfSplitDescription();
}


void VcfSplit::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil split --in <input VCF File> --obase <base output of VCF Files> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to read\n"
              << "\t\t--obase   : base of VCF file name to write\n"
              << "\t\t            '.chrN.vcf' or '.chrN.vcf.gz' will be appended\n"
              << "\tOptional Parameters:\n"
              << "\t\t--uncompress : write an uncompressed VCF output file\n"
              << "\t\t--refName    : the reference (chromosome) name to read\n"
              << "\t\t               Defaults to all references.\n"
              << "\t\t--noeof      : do not expect an EOF block on a BGZF file\n"
              << "\t\t--params     : print the parameter settings\n"
              << std::endl;
}



int VcfSplit::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcfBase = "";
    String refName = "";
    bool uncompress = false;
    bool params = false;
    bool noeof = false;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_PARAMETER_GROUP("Required Parameters")
        LONG_STRINGPARAMETER("in", &inputVcf)
        LONG_STRINGPARAMETER("obase", &outputVcfBase)
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
    if(outputVcfBase == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--obase\", a required parameter.\n\n";
        return(-1);
    }
    outputVcfBase += ".";

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
    std::map<std::string, VcfFileWriter*> outFiles;
    VcfHeader header;
    
    // Open the file.
    inFile.open(inputVcf, header);

    if(refName != "")
    {
        inFile.setReadSection(refName.c_str());
    }

    VcfRecord record;
    int numRecords = 0;

    std::string prevChr = "";
    std::string chr = "";
    VcfFileWriter* outFilePtr = 0;
    std::string outName = "";
    while(inFile.readRecord(record))
    {
        ++numRecords;

        chr = record.getChromStr();

        if((outFilePtr == 0) || (chr != prevChr))
        {
            outFilePtr = outFiles[chr];
            if(outFilePtr == 0)
            {
                outFilePtr = new VcfFileWriter();
                outFiles[chr] = outFilePtr;
                outName = outputVcfBase.c_str();
                if(chr.substr(0,3) != "chr")
                {
                    outName += "chr";
                }
                outName += chr + ".vcf";
                // chr not in outFile list.
                if(uncompress)
                {
                    outFilePtr->open(outName.c_str(), header, InputFile::DEFAULT);
                }
                else
                {
                    outName += ".gz";
                    outFilePtr->open(outName.c_str(), header);
                }
            }
        }
        outFilePtr->writeRecord(record);
    }
 
    inFile.close();   

    for (std::map<std::string,VcfFileWriter*>::iterator it = outFiles.begin();
         it != outFiles.end(); ++it)
    {
        if(it->second != 0)
        {
            it->second->close();
            it->second = 0;
        }
    }
  

    std::cerr << "NumRecords: " << numRecords << "\n";
    return(0);
}
