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
#include "VcfExample.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "VcfFileReader.h"

void VcfExample::vcfExampleDescription()
{
    std::cerr << " vcfExample - Print the number of samples" << std::endl;
}


void VcfExample::description()
{
    vcfExampleDescription();
}


void VcfExample::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil vcfExample --in <input VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to read\n"
              << "\tOptional Parameters:\n"
              << "\t\t--params   : print the parameter settings\n"
              << std::endl;
}



int VcfExample::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcf = "";
    bool params = false;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_STRINGPARAMETER("in", &inputVcf)
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

    if(params)
    {
        inputParameters.Status();
    }

    VcfFileReader inFile;
    VcfHeader header;
    
    // Open the file.
    inFile.open(inputVcf, header);

    VcfRecord record;
    VcfRecordGenotype* genotypeInfoPtr = NULL;

    int numRecords = 0;
    int maxSamples = 0;
    int numSamples = 0;
    const std::string* genotypeVal = NULL;
    std::string gtField = "GT";
    int numPhased = 0;
    int numUnphased = 0;

    while(inFile.readRecord(record))
    {
        ++numRecords;

        // Get the genotype inforamtion.
        genotypeInfoPtr = &(record.getGenotypeInfo());

        // Get the number of samples.
        numSamples = genotypeInfoPtr->getNumSamples();

        if(maxSamples < numSamples)
        {
            maxSamples = numSamples;
        }

        // Get the Genotype Information.
        for(int i = 0; i < numSamples; i++)
        {
            genotypeVal = genotypeInfoPtr->getString(gtField, i);
            // Need to make sure the field was found.
            if(genotypeVal != NULL)
            {
                // Check if it is phased or unphased.
                // This isn't pretty and there should be a method added for it.
                for(unsigned int j = 0; j < genotypeVal->length(); j++)
                {
                    if((*genotypeVal)[j] == '|')
                    {
                        ++numPhased;
                        break;
                    }
                    else if((*genotypeVal)[j] == '/')
                    {
                        ++numUnphased;
                        break;
                    }
                }
            }
        }
    }
 
    inFile.close();   

    std::cerr << "NumRecords: " << numRecords << "; Max Samples: " << maxSamples
              << "; Num Phased: " << numPhased << "; Num Unphased: " << numUnphased << "\n";
    return(0);
}

