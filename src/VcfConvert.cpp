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
    std::cerr << "\t./vcfUtil convert --in <input VCF File> --out <output VCF File> [--sampleInclude|sampleExclude <filename>] [--sampleDelim] [--idInclude|idExclude <filename>] [--uncompress] [--refName <chrom>] [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to read\n"
              << "\t\t--out     : VCF file to write\n"
              << "\tOptional Parameters:\n"
              << "\t\t--sampleInclude : file with samples IDs to keep.\n"
              << "\t\t--sampleExclude : file with samples IDs to remove.\n"
              << "\t\t--sampleDelim   : delimiter for the sample file (default is \\n.\n"
              << "\t\t--idInclude     : file with variant IDs to keep.\n"
              << "\t\t--idExclude     : file with variant IDs to remove.\n"
              << "\t\t--uncompress    : write an uncompressed VCF output file\n"
              << "\t\t--refName       : the reference (chromosome) name to read\n"
              << "\t\t                  Defaults to all references.\n"
              << "\t\t--params        : print the parameter settings\n"
              << std::endl;
}



int VcfConvert::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcf = "";
    String refName = "";
    String includeSampleSubset = "";
    String excludeSampleSubset = "";
    String includeIDs = "";
    String excludeIDs = "";
    //    String includeVariants = "";
    //    String excludeVariants = "";
    String sampleDelim = "\n";
    bool uncompress = false;
    bool params = false;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_PARAMETER_GROUP("Required Parameters")
        LONG_STRINGPARAMETER("in", &inputVcf)
        LONG_STRINGPARAMETER("out", &outputVcf)
        LONG_PARAMETER_GROUP("Optional Parameters")
        LONG_PARAMETER("uncompress", &uncompress)
        LONG_STRINGPARAMETER("sampleInclude", &includeSampleSubset)
        LONG_STRINGPARAMETER("sampleExclude", &excludeSampleSubset)
        LONG_STRINGPARAMETER("sampleDelim", &sampleDelim)
        LONG_STRINGPARAMETER("idInclude", &includeIDs)
        LONG_STRINGPARAMETER("idExclude", &excludeIDs)
        //        LONG_STRINGPARAMETER("variantInclude", &includeVariants)
        //        LONG_STRINGPARAMETER("variantExclude", &excludeVariants)
        LONG_STRINGPARAMETER("refName", &refName)
        LONG_PARAMETER("params", &params)
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
    if((includeSampleSubset != "") && (excludeSampleSubset != ""))
    {
        usage();
        inputParameters.Status();
        std::cerr << "ERROR: cannot specify both \"--includeSamples\" and \"--excludeSamples\".\n\n";
        return(-1);
    }

    if((includeIDs != "") && (excludeIDs != ""))
    {
        usage();
        inputParameters.Status();
        std::cerr << "ERROR: cannot specify both \"--includeIDs\" and \"--excludeIDs\".\n\n";
        return(-1);
    }

    if(params)
    {
        inputParameters.Status();
    }

    VcfFileReader inFile;
    VcfFileWriter outFile;
    VcfHeader header;
    
    // Open the file.
    if(!includeSampleSubset.IsEmpty())
    {
        inFile.open(inputVcf, header, includeSampleSubset, NULL, NULL, sampleDelim);
    }
    else if(!excludeSampleSubset.IsEmpty())
    {
        inFile.open(inputVcf, header, NULL, NULL, excludeSampleSubset, sampleDelim);
    }
    else
    {
        inFile.open(inputVcf, header);
    }

    if(refName != "")
    {
        inFile.setReadSection(refName.c_str());
    }

    // Read the include/exclude IDs into memory.
    if(!includeIDs.IsEmpty())
    {
        inFile.setIncludeIDs(includeIDs);
    }
    else if(!excludeIDs.IsEmpty())
    {
        inFile.setExcludeIDs(excludeIDs);
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
