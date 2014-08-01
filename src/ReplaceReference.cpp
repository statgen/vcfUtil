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
#include "ReplaceReference.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "GenomeSequence.h"
#include "HyunVcfFile.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"
#include <time.h>

void ReplaceReference::replaceReferenceDescription()
{
    std::cerr << " replaceReference - Print the reference string for the specified region" << std::endl;
}


void ReplaceReference::description()
{
    replaceReferenceDescription();
}


void ReplaceReference::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcf replaceReference --refFile <referenceFilename> --in <input VCF File> --out <output VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--refFile : the reference file\n"
              << "\t\t--in      : VCF file whose reference alleles should be replaced\n"
              << "\t\t--out     : new VCF file with updated reference alleles\n"
              << "\tOptional Parameters:\n"
              << "\t\t--params   : print the parameter settings\n"
              << std::endl;
}



int ReplaceReference::execute(int argc, char **argv)
{
    String refFile = "";
    String inputVcf = "";
    String outputVcf = "";
    bool params = false;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_STRINGPARAMETER("refFile", &refFile)
        LONG_STRINGPARAMETER("in", &inputVcf)
        LONG_STRINGPARAMETER("out", &outputVcf)
        LONG_PARAMETER("params", &params)
        LONG_PHONEHOME(VERSION)
        END_LONG_PARAMETERS();
   
    inputParameters.Add(new LongParameters ("Input Parameters", 
                                            longParameterList));
    
    inputParameters.Read(argc-1, &(argv[1]));
    
    // Check that all files were specified.
    if(refFile == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--refFile\", a required parameter.\n\n";
        return(-1);
    }
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

    // Open the reference.
    GenomeSequence reference(refFile);

    time_t start;
    time_t end;
    

    ////////////////////////////////////////////////////////////////////////////
    // Using IFILE.
    String outFileName;
    start = time(NULL);
    outFileName = "ifile_" + outputVcf;

    // Open the vcf files.
    IFILE vcfIn = ifopen(inputVcf, "r");
    IFILE vcfOut = ifopen(outFileName, "w");

    String line = "";
    StringArray tokens;
    while(!ifeof(vcfIn))
    {
        if(line.ReadLine(vcfIn) < 0)
        {
            // Error/end of file, exit loop.
            break;
        }

        line += '\n';
        // Check if it is a header line.
        if(line[0] == '#')
        {
            // Meta/Header line, so just write it.
            ifwrite(vcfOut, line.c_str(), line.Length());
        }
        else
        {
            // Data line.
            // Parse out the tabs.
            tokens.ReplaceColumns(line,'\t');

            if(tokens.Length() < 4)
            {
                // Not enough tokens for having a reference, so just write it
                ifwrite(vcfOut, line.c_str(), line.Length());
            }
            else
            {
                // Get the chromosome start position.
                uint32_t chrStart = 
                    reference.getGenomePosition(tokens[0].c_str());

                int pos = 0;
                if(tokens[1].AsInteger(pos))
                {
                    // Successfully converted to integer, so make 0 based.
                    --pos;
                    char newRef = reference[chrStart + pos];

                    tokens[3] = newRef;
                }

                // Write out the line.
                for(int i = 0; i < tokens.Length(); i++)
                {
                    if(i != 0)
                    {
                        ifprintf(vcfOut, "\t%s", tokens[i].c_str());
                    }
                    else
                    {
                        ifprintf(vcfOut, "%s", tokens[i].c_str());
                    }
                }
            }
        }
    }
 
    ifclose(vcfOut);
    ifclose(vcfIn);

    end = time(NULL);

    std::cerr << "Took: " << end-start << " to run IFILE verison\n";

    ////////////////////////////////////////////////////////////////////////////
    // Using Hyun's library.
    start = time(NULL);
    outFileName = "hyun_" + outputVcf;

    HyunVcfFile inFileH;
    IFILE outFileH;

    // Open the files.
    inFileH.openForRead(inputVcf);
    outFileH = ifopen(outFileName.c_str(), "wb");

    // Write the header.
    inFileH.printVCFHeader(outFileH);

    while(inFileH.iterateMarker())
    {
        VcfMarker* pMarker = inFileH.getLastMarker();

        // Get the ref position.
        genomeIndex_t markerIndex = 
            reference.getGenomePosition(pMarker->sChrom.c_str(), pMarker->nPos);
        
        char newRef = reference[markerIndex];
            
        pMarker->sRef = newRef;
        pMarker->printVCFMarker(outFileH,false);
    }

     ifclose(outFileH);
     end = time(NULL);

     std::cerr << "Took: " << end-start << " to run Hyun verison\n";

    ////////////////////////////////////////////////////////////////////////////
    // Using my library.
    start = time(NULL);
    outFileName = "lib_" + outputVcf;
    
    VcfFileReader inFile;
    VcfHeader header;
    VcfFileWriter outFile;
    
    // Open the files.
    inFile.open(inputVcf, header);
    // Open and write the header
    outFile.open(outFileName, header);

    VcfRecord record;

    std::string newRef = "";
    while(inFile.readRecord(record))
    {
        // Get the ref position.
        genomeIndex_t markerIndex = 
            reference.getGenomePosition(record.getChromStr(), record.get1BasedPosition());
        
        newRef = reference[markerIndex];
        
        record.setRef(newRef.c_str());
        outFile.writeRecord(record);
    }
 
    inFile.close();   
    outFile.close();   
    end = time(NULL);
    
    std::cerr << "Took: " << end-start << " to run my verison\n";
    return(0);
}

