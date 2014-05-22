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
#include "VcfMac.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"
#include "IntervalTree.h"

void VcfMac::vcfMacDescription()
{
    std::cerr << " mac - Print MAC/MAF for each variant" << std::endl;
}


void VcfMac::description()
{
    vcfMacDescription();
}


void VcfMac::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil mac --in1 <input VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in      : VCF file to analyze\n"
              << "\tOptional Parameters:\n"
              << "\t\t--sampleSubset : file with samples IDs to keep.\n"
              << "\t\t--minAC        : min minor allele count to keep\n"
              << "\t\t--filterList   : filename of file containing regions to include,\n"
              << "\t\t                 format: start end\n"
              << "\t\t                 start & end positions should be 1-based inclusive positions.\n"
              << "\t\t--params     : print the parameter settings\n"
              << std::endl;
}



int VcfMac::execute(int argc, char **argv)
{
    String inputVcf = "";
    int minAC = -1;
    String sampleSubset = "";
    String filterList = "";
    bool params = false;

    IntervalTree<int> regions;
    std::vector<int> intersection;
    
    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_PARAMETER_GROUP("Required Parameters")
        LONG_STRINGPARAMETER("in", &inputVcf)
        LONG_PARAMETER_GROUP("Optional Parameters")
        LONG_STRINGPARAMETER("sampleSubset", &sampleSubset)
        LONG_INTPARAMETER("minAC", &minAC)
        LONG_STRINGPARAMETER("filterList", &filterList)
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

    // Open the two input files.
    VcfFileReader inFile;
    VcfHeader header;
    VcfRecord record;

    // Open the file
    if(sampleSubset.IsEmpty())
    {
        inFile.open(inputVcf, header);        
    }
    else
    {
        inFile.open(inputVcf, header, sampleSubset, NULL, NULL);
    }
    
    // Add the discard rule for minor allele count.
    if(minAC >= 0)
    {
        inFile.addDiscardMinMinorAlleleCount(minAC, NULL);
    }
    
    if(!filterList.IsEmpty())
    {
        // Open the filter list.
        IFILE regionFile = ifopen(filterList, "r");
        String regionLine;
        StringArray regionColumn;
        int start;
        int end;
        int intervalVal = 1;
        if(regionFile == NULL)
        {
            std::cerr << "Failed to open " << filterList 
                      << ", so keeping all positions\n";
            filterList.Clear();
        }
        else
        {
            while( regionFile->isOpen() && !regionFile->ifeof())
            {
                // Read the next interval
                regionLine.Clear();
                regionLine.ReadLine(regionFile);
                if(regionLine.IsEmpty())
                {
                    // Nothing on this line, continue to the next.
                    continue;
                }
                regionColumn.ReplaceColumns(regionLine, ' ');
                if(regionColumn.Length() != 2)
                {
                    std::cerr << "Improperly formatted region line: " 
                              << regionLine << "; skipping to the next line.\n";
                    continue;
                }
                // Convert the columns to integers.
                if(!regionColumn[0].AsInteger(start))
                {
                    // The start position (1st column) is not an integer.
                    std::cerr << "Improperly formatted region line, start position "
                              << "(1st column) is not an integer: "
                              << regionColumn[0]
                              << "; Skipping to the next line.\n";
                    continue;
                }
                if(!regionColumn[1].AsInteger(end))
                {
                    // The start position (1st column) is not an integer.
                    std::cerr << "Improperly formatted region line, end position "
                              << "(2nd column) is not an integer: "
                              << regionColumn[1]
                              << "; Skipping to the next line.\n";
                    continue;
                }
                // Add 1-based inclusive intervals.
                regions.add(start,end, intervalVal);
            }
        }
    }


    int numReadRecords = 0;

    while( inFile.readRecord(record))
    {
        if(!filterList.IsEmpty())
        {
            // Check if the region should be kept.
            intersection.clear();
            regions.get_intersecting_intervals(record.get1BasedPosition(), intersection);
            
            if(intersection.empty())
            {
                // not in the interval, so continue to the next record.
                continue;
            }
        }

        ++numReadRecords;

        // Loop through the number of possible alternates.
        unsigned int numAlts = record.getNumAlts();
        int minAlleleCount = -1;
        int curAlleleCount = 0;
        int totalAlleleCount = 0;
        for(unsigned int i = 0; i <= numAlts; i++)
        {
            curAlleleCount = record.getAlleleCount(i);
            if((minAlleleCount == -1) ||
               (curAlleleCount < minAlleleCount))
            {
                minAlleleCount = curAlleleCount;
            }
            totalAlleleCount += curAlleleCount;
        }
        if(totalAlleleCount != 0)
        {
            double maf = (double)minAlleleCount/totalAlleleCount;
            std::cout << record.getIDStr()
                      << "\t" << minAlleleCount
                      << "\t" << maf << "\n";
        }
    }
    
    inFile.close();

    //    std::cerr << "\n\t# Records: " << numReadRecords << "\n";

    // return success.
    return(0);
}

