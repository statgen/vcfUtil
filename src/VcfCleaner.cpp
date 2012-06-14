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
#include "VcfCleaner.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"

void VcfCleaner::vcfCleanerDescription()
{
    std::cerr << " cleaner - Clean/Reduce VCF files" << std::endl;
}


void VcfCleaner::description()
{
    vcfCleanerDescription();
}


void VcfCleaner::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil cleaner --in <input VCF File> --out <output VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to read\n"
              << "\t\t--out     : VCF file to write\n"
              << "\tOptional Parameters:\n"
              << "\t\t--uncompress : write an uncompressed VCF output file\n"
              << "\t\t--params     : print the parameter settings\n"
              << std::endl;
}



int VcfCleaner::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcf = "";
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

    if(params)
    {
        inputParameters.Status();
    }

    VcfFileReader inFile;
    VcfFileWriter outFile;
    VcfHeader header;
    VcfRecord record;

    // Open the file.
    inFile.open(inputVcf, header);
    if(uncompress)
    {
        outFile.open(outputVcf, header, InputFile::DEFAULT);
    }
    else
    {
        outFile.open(outputVcf, header);
    }

    int numReadRecords = 0;
    int numWrittenRecords = 0;
    int returnVal = 0;

    // Set to only store/write the GT field.
    VcfRecordGenotype::addStoreField("GT");
    while(inFile.readRecord(record))
    {
        ++numReadRecords;
        // Check if any samples are missing GT or if any are not phased.
        if(!record.hasAllGenotypeAlleles() || !record.allPhased())
        {
            // Missing a GT or not phased, so continue without writing.
            continue;
        }
        
        // Clear the INFO field.
        record.getInfo().clear();
        // Write the record.
        if(!outFile.writeRecord(record))
        {
            // Write error.
            std::cerr << "Failed writing a vcf record.\n";
            returnVal = -1;
        }
        ++numWrittenRecords;
    }
 
    inFile.close();   
    outFile.close();   

    std::cerr << "NumReadRecords: " << numReadRecords
              << "; NumWrittenRecords: " << numWrittenRecords << "\n";
    return(returnVal);
}

