/*
 *  Copyright (C) 2011, 2014  Regents of the University of Michigan
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

#include <string.h>


#include "VcfConsensus.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"

bool isSame(const std::string* gt1, const std::string* gt2);
bool findPos(bool newChrom, const char* chrom1, int pos1, VcfRecord& record2, VcfFileReader& vcf2);

const int UNSET_POS = -1;

void VcfConsensus::vcfConsensusDescription()
{
    std::cerr << " consensus - Write consensus VCF from 3 input VCFs" << std::endl;
}


void VcfConsensus::description()
{
    vcfConsensusDescription();
    std::cerr << "\t* Only processes samples & sites found in --in1.\n";
    std::cerr << "\t* Skips any sites not found in all 3 VCFs.\n";
    std::cerr << "\t* If all 3 VCFs have a different genotype, \"./.\" will be output.\n";
    std::cerr << "\t* If at least 2 VCFs have the same genotype, that genotype will be output.\n";
    std::cerr << "\n";
}


void VcfConsensus::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcfUtil consensus --in1 <input VCF File1>  --in2 <input VCF File2>  --in3 <input VCF File3> --out <output VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--in1      : VCF file1 to read\n"
              << "\t\t--in2      : VCF file2 to read\n"
              << "\t\t--in3      : VCF file3 to read\n"
              << "\t\t--out      : VCF file to write\n"
              << "\tOptional Parameters:\n"
              << "\t\t--uncompress : write an uncompressed VCF output file\n"
              << "\t\t--params     : print the parameter settings\n"
              << std::endl;
}

int VcfConsensus::execute(int argc, char ** argv)
{
    String vcfName1;
    String vcfName2;
    String vcfName3;
    String outputFileName;
    bool uncompress = false;
    bool params = false;

    // Read in the parameters.    
    ParameterList inputParameters;
    BEGIN_LONG_PARAMETERS(longParameterList)
        LONG_PARAMETER_GROUP("Required Parameters")
        LONG_STRINGPARAMETER("in1", &vcfName1)
        LONG_STRINGPARAMETER("in2", &vcfName2)
        LONG_STRINGPARAMETER("in3", &vcfName3)
        LONG_STRINGPARAMETER("out", &outputFileName)
        LONG_PARAMETER_GROUP("Optional Parameters")
        LONG_PARAMETER("uncompress", &uncompress)
        LONG_PARAMETER("params", &params)
        LONG_PHONEHOME(VERSION)
       END_LONG_PARAMETERS();
   
    inputParameters.Add(new LongParameters ("Input Parameters", 
                                            longParameterList));
    
    inputParameters.Read(argc-1, &(argv[1]));
    
    std::string gtField = "GT";

    VcfFileReader vcf1;
    VcfFileReader vcf2;
    VcfFileReader vcf3;
    VcfHeader header1;
    VcfHeader header2;
    VcfHeader header3;
    VcfRecord record1;
    VcfRecord record2;
    VcfRecord record3;
    VcfRecordGenotype* genotypeInfoPtr1 = NULL;
    VcfRecordGenotype* genotypeInfoPtr2 = NULL;
    VcfRecordGenotype* genotypeInfoPtr3 = NULL;
    
    unsigned int numMissing2 = 0;
    unsigned int numMissing3 = 0;
    unsigned int numMismatchRefAlt = 0;
    unsigned int numMissingGT1 = 0;
    const unsigned int myMaxErrors = 4;

    // Check that the required parameters were set.
    if(vcfName1 == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--in1\", a required parameter.\n\n";
        return(-1);
    }
    if(vcfName2 == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--in2\", a required parameter.\n\n";
        return(-1);
    }
    if(vcfName3 == "")
    {
        usage();
        inputParameters.Status();
        std::cerr << "Missing \"--in3\", a required parameter.\n\n";
        return(-1);
    }
    if(outputFileName == "")
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

    
    // Open the files.
    vcf1.open(vcfName1, header1);
    vcf2.open(vcfName2, header2);
    vcf3.open(vcfName3, header3);

    // Setup the sample name maps.
    int numSamples = header1.getNumSamples();
    std::vector<int> sample1ToSample2;
    std::vector<int> sample1ToSample3;
    for(int i = 0; i < numSamples; i++)
    {
        // Look for this sample name in vcf2.
        sample1ToSample2.push_back(header2.getSampleIndex(header1.getSampleName(i)));
        sample1ToSample3.push_back(header3.getSampleIndex(header1.getSampleName(i)));
    }

    VcfFileWriter outputVcf;
    // Open and write the header
    if(uncompress)
    {
        outputVcf.open(outputFileName, header1, InputFile::DEFAULT);
    }
    else
    {
        outputVcf.open(outputFileName, header1);
    }

    const char* chrom1 = NULL;
    int pos1 = UNSET_POS;

    // Read the first record from vcf2 & vcf3.
    vcf2.readRecord(record2);
    vcf3.readRecord(record3);

    bool newChrom = true;
    static std::string prevChrom = "";

    uint64_t numAllMatch = 0;
    uint64_t num1Match2Only = 0;
    uint64_t num1Match3Only = 0;
    uint64_t num2Match3Only = 0;
    uint64_t numNoMatches = 0;

    uint64_t numAllMatch00 = 0;
    uint64_t num1Match2Only00 = 0;
    uint64_t num1Match3Only00 = 0;
    uint64_t num2Match3Only00 = 0;

    uint64_t numAllMatch01 = 0;
    uint64_t num1Match2Only01 = 0;
    uint64_t num1Match3Only01 = 0;
    uint64_t num2Match3Only01 = 0;

    uint64_t numAllMatch11 = 0;
    uint64_t num1Match2Only11 = 0;
    uint64_t num1Match3Only11 = 0;
    uint64_t num2Match3Only11 = 0;

    // Loop through vcf1.
    while(vcf1.readRecord(record1))
    {
        chrom1 = record1.getChromStr();
        pos1 = record1.get1BasedPosition();

        if(strcmp(chrom1, prevChrom.c_str()) == 0)
        {
            newChrom = false;
        }
        else
        {
            prevChrom = chrom1;
            newChrom = true;
        }

        bool found = true;
        if(!findPos(newChrom, chrom1, pos1, record2, vcf2))
        {
            // Failed to find the position, continue to the next position
            if(++numMissing2 <= myMaxErrors)
            {
                std::cerr << "Failed to find " << chrom1 << ":" << pos1 
                          << " in " << vcfName2 << ", so skipping this pos\n";
            }
            found = false;
        }
        
        if(!findPos(newChrom, chrom1, pos1, record3, vcf3))
        {
            // Failed to find the position, continue to the next position
            if(++numMissing3 <= myMaxErrors)
            {
                std::cerr << "Failed to find " << chrom1 << ":" << pos1 
                          << " in " << vcfName3 << ", so skipping this pos\n";
            }
            found = false;
        }

        if(found == false)
        {
            continue;
        }

        // Found the position in all files.
        
        // Validate that the reference & alternate alleles are the same.
        const char* ref1 = record1.getRefStr();
        const char* alt1 = record1.getAltStr();
        if((strcmp(ref1, record2.getRefStr()) != 0) ||
           (strcmp(ref1, record3.getRefStr()) != 0) ||
           (strcmp(alt1, record2.getAltStr()) != 0) ||
           (strcmp(alt1, record3.getAltStr()) != 0))
        {
            if(++numMismatchRefAlt <= myMaxErrors)
            {
                std::cerr << "Mismatching ref/alt found at " << chrom1 << ":" << pos1 << ", so skipping this pos\n";
            }
            continue;
        }

        // Get the genotype information for each.
        genotypeInfoPtr1 = &(record1.getGenotypeInfo());
        genotypeInfoPtr2 = &(record2.getGenotypeInfo());
        genotypeInfoPtr3 = &(record3.getGenotypeInfo());

        // Loop through all the samples in chrom1.
        // Get the Genotype Information.
        for(int i = 0; i < numSamples; i++)
        {
            const std::string* genotypeVal1 = genotypeInfoPtr1->getString(gtField, i);
            const std::string* genotypeVal2 = genotypeInfoPtr2->getString(gtField, sample1ToSample2[i]);
            const std::string* genotypeVal3 = genotypeInfoPtr3->getString(gtField, sample1ToSample3[i]);
            // Need to make sure the field was found.
            if(genotypeVal1 == NULL)
            {
                // GT not found in the first record, so just continue.
                if(++numMissingGT1 <= myMaxErrors)
                {
                    std::cerr << "Missing GT for " << header1.getSampleName(i) << " in " << vcfName1 << "\n";
                }
                continue;
            }


            if(isSame(genotypeVal1, genotypeVal2))
            {
                // genotypeVal1 is majority, so make no change.
                if(isSame(genotypeVal1, genotypeVal3))
                {
                    ++numAllMatch;
                    if(*genotypeVal1 == "0/0")
                    {
                        ++numAllMatch00;
                    }
                    else if((*genotypeVal1 == "0/1") || 
                            (*genotypeVal1 == "1/0"))
                    {
                        ++numAllMatch01;
                    }
                    if(*genotypeVal1 == "1/1")
                    {
                        ++numAllMatch11;
                    }
                }
                else
                {
                    ++num1Match2Only;
                    if(*genotypeVal1 == "0/0")
                    {
                        ++num1Match2Only00;
                    }
                    else if((*genotypeVal1 == "0/1") || 
                            (*genotypeVal1 == "1/0"))
                    {
                        ++num1Match2Only01;
                    }
                    if(*genotypeVal1 == "1/1")
                    {
                        ++num1Match2Only11;
                    }
                }
            }
            else if(isSame(genotypeVal1, genotypeVal3))
            {
                // genotypeVal1 is majority, so make no change.
                    ++num1Match3Only;
                    if(*genotypeVal1 == "0/0")
                    {
                        ++num1Match3Only00;
                    }
                    else if((*genotypeVal1 == "0/1") || 
                            (*genotypeVal1 == "1/0"))
                    {
                        ++num1Match3Only01;
                    }
                    if(*genotypeVal1 == "1/1")
                    {
                        ++num1Match3Only11;
                    }
            }
            else if(isSame(genotypeVal2, genotypeVal3))
            {
                // genotypeVal2 is majority, so change genotypeVal1.
                genotypeInfoPtr1->setString(gtField, i, *genotypeVal2);
                ++num2Match3Only;
                if(*genotypeVal2 == "0/0")
                {
                    ++num2Match3Only00;
                }
                else if((*genotypeVal2 == "0/1") || 
                        (*genotypeVal2 == "1/0"))
                {
                    ++num2Match3Only01;
                }
                if(*genotypeVal2 == "1/1")
                {
                    ++num2Match3Only11;
                }
            }
            else
            {
                // None are the same so set to "./."
                genotypeInfoPtr1->setString(gtField, i, "./.");
                ++numNoMatches;
            }
        } // loop back to vcf1 samples.

        // Write this record.
        outputVcf.writeRecord(record1);
    } // loop back to next vcf1 record.

    std::cerr << "\n";
    if(numMissing2 > myMaxErrors)
    {
        std::cerr << "Suppressed "
                  << numMissing2 - myMaxErrors
                  << " errors about skipped positions because they were not in "
                  << vcfName2
                  << "\n";
    }

    if(numMissing3 > myMaxErrors)
    {
        std::cerr << "Suppressed "
                  << numMissing3 - myMaxErrors
                  << " errors about skipped positions because they were not in "
                  << vcfName3
                  << "\n";
    }

    if(numMismatchRefAlt > myMaxErrors)
    {
        std::cerr << "Suppressed "
                  << numMismatchRefAlt - myMaxErrors
                  << " errors about mismatched ref/alt positions\n";
    }

    if(numMissingGT1 > myMaxErrors)
    {
        std::cerr << "Suppressed "
                  << numMissingGT1 - myMaxErrors
                  << " errors about missing GT for "
                  << vcfName1
                  << "\n";
    }
    std::cerr << "\n";
    // Output the stats.
    std::cerr << "File1 = " << vcfName1 << std::endl;
    std::cerr << "File2 = " << vcfName2 << std::endl;
    std::cerr << "File3 = " << vcfName3 << std::endl;
    std::cerr << "\nType\tTotal\t0/0\t0/1\t1/1\n";
    std::cerr << "AllMatched" 
              << "\t" << numAllMatch
              << "\t" << numAllMatch00 
              << "\t" << numAllMatch01 
              << "\t" << numAllMatch11 << std::endl;
    std::cerr << "1matched2"
              << "\t" << num1Match2Only 
              << "\t" << num1Match2Only00 
              << "\t" << num1Match2Only01 
              << "\t" << num1Match2Only11 << std::endl;
    std::cerr << "1matched3"
              << "\t" << num1Match3Only 
              << "\t" << num1Match3Only00 
              << "\t" << num1Match3Only01 
              << "\t" << num1Match3Only11 << std::endl;
    std::cerr << "2matched3"
              << "\t" << num2Match3Only
              << "\t" << num2Match3Only00 
              << "\t" << num2Match3Only01 
              << "\t" << num2Match3Only11 << std::endl;
    std::cerr << "NoneMatched\t" << numNoMatches << std::endl;

    return(0);
}


bool isSame(const std::string* gt1, const std::string* gt2)
{
    if((gt1 == NULL) || (gt2 == NULL))
    {
        // no gt, so return false.
        return(false);
    }

    if(*gt1 == *gt2)
    {
        return(true);
    }

    // Check for 0/1 && 1/0
    if((*gt1 == "0/1") && (*gt2 == "1/0"))
    {
        return(true);
    }
    if((*gt1 == "1/0") && (*gt2 == "0/1"))
    {
        return(true);
    }
    return(false);
}


// The VcfRecord passed in should already be set with a record.
bool findPos(bool newChrom, const char* chrom1, int pos1, VcfRecord& record2, VcfFileReader& vcf2)
{
    const char* chrom2 = record2.getChromStr();
    int pos2 = record2.get1BasedPosition();

    // Loop until the chrom/pos is found in vcf2.
    bool sameChrom = (strcmp(chrom2, chrom1) == 0);
    while(((pos2 < pos1) && sameChrom) ||
          (newChrom && !sameChrom))
    {
        if(vcf2.readRecord(record2))
        {
            chrom2 = record2.getChromStr();
            pos2 = record2.get1BasedPosition();
            sameChrom = (strcmp(chrom2, chrom1) == 0);
        }
        else
        {
            // no more records.
            chrom2 = NULL;
            pos2 = UNSET_POS;
            return(false);
        }
    }
    // If we wind up here, chrom2 is either at the correct 
    // position, or it is past the correct position.
    if((!sameChrom) || (pos2 != pos1))
    {
        // Position not found.
        return(false);
    }
    return(true);
}    

