/*
 * Copyright (c) 2010-2011 Hyun Min Kang, Matthew Flickenger, Matthew Snyder, 
 *          Goncalo Abecasis, and Regents of the University of Michigan
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
#include "VcfCooker.h"

#include "Parameters.h"
#include "BgzfFileType.h"
#include "GenomeSequence.h"
#include "HyunVcfFile.h"
#include "VcfFileReader.h"
#include "VcfFileWriter.h"

#include "Logger.h"
#include <map>
#include <limits.h>
#include <vector>

#include <time.h>
Logger* Logger::gLogger = NULL;

void VcfCooker::vcfCookerDescription()
{
    std::cerr << " vcfCooker - Print the reference string for the specified region" << std::endl;
}


void VcfCooker::description()
{
    vcfCookerDescription();
}


void VcfCooker::usage()
{
    VcfExecutable::usage();
    std::cerr << "\t./vcf vcfCooker --refFile <referenceFilename> --in <input VCF File> --out <output VCF File> [--params]"<< std::endl;
    std::cerr << "\tRequired Parameters:\n"
              << "\t\t--refFile : the reference file\n"
              << "\t\t--in      : VCF file whose reference alleles should be replaced\n"
              << "\t\t--out     : new VCF file with updated reference alleles\n"
              << "\tOptional Parameters:\n"
              << "\t\t--params   : print the parameter settings\n"
              << std::endl;
}



int VcfCooker::execute(int argc, char **argv)
{
   printf("vcfCooker 1.1.1 -- Manipulate VCF files\n"
          "(c) 2010 Hyun Min Kang, Matthew Flickinger, Matthew Snyder, Paul Anderson, Tom Blackwell, Mary Kate Trost, and Goncalo Abecasis\n\n");

   bool bRecipesWriteBed = false;
   bool bRecipesWriteVcf = false;
   bool bRecipesSummarize = false;
   bool bRecipesUpgrade = false; // upgrade from glfMultiples v3.3 to v4.0 format
   bool bRecipesFilter = false;
   bool bRecipesSubset = false;

   bool bFiltOnlySubset = false;

   String sInputVcf, sInputBfile, sInputBed, sInputBim, sInputFam, sInputSubset;
   String sFasta("/data/local/ref/karma.ref/human.g1k.v37.fa");

   String sOut("./vcfCooker");

   int nMinGQ = 0;
   int nMinGD = 0;

   int nMinQUAL = 0;
   int nMinMQ = 0;
   int nMaxDP = INT_MAX;
   int nMinDP = 0;
   int nMaxAB = 100;
   int nWinFFRQ = 0;
   int nMaxFFRQ = 0;
   double fMaxFFRQ = 0;
   int nMinNS = 0;
   int nWinIndel = 0;
   int nMaxSTP = INT_MAX;
   int nMaxTTT = INT_MAX;
   int nMinTTT = INT_MIN;
   int nMaxSTR = 100;
   int nMinSTR = -100;
   int nMaxSTZ = INT_MAX;
   int nMinSTZ = INT_MIN;
   int nMaxLQZ = INT_MAX;
   int nMinLQZ = INT_MIN;
   int nMaxRBZ = INT_MAX;
   int nMinRBZ = INT_MIN;
   int nMaxCBR = 100;
   int nMinCBR = -100;
   int nMaxCBZ = INT_MAX;
   int nMaxCSR = 100;
   int nMinCSR = -100;
   int nMaxIOZ = INT_MAX;
   int nMaxIOR = INT_MAX;
   int nMaxAOZ = INT_MAX;
   int nMaxAOI = INT_MAX;
   int nMaxMQ0 = 100;
   int nMaxMQ20 = 100;

   String sIndelVcf;

   bool bVerbose = true;
   bool bOutPlain = true;
   bool bOutBgzf = false;
   bool bOutGzip = false;
   bool bKeepFilter = false;

   ParameterList pl;

   BEGIN_LONG_PARAMETERS(longParameters)
     LONG_PARAMETER_GROUP("Recipes")
     LONG_PARAMETER("write-bed",&bRecipesWriteBed)
     LONG_PARAMETER("write-vcf",&bRecipesWriteVcf)
     LONG_PARAMETER("upgrade",&bRecipesUpgrade)
     LONG_PARAMETER("summarize",&bRecipesSummarize)
     LONG_PARAMETER("filter",&bRecipesFilter)
     LONG_PARAMETER("subset",&bRecipesSubset)

     LONG_PARAMETER_GROUP("VCF Input options")
     LONG_STRINGPARAMETER("in-vcf",&sInputVcf)

     LONG_PARAMETER_GROUP("BED Input options")
     LONG_STRINGPARAMETER("in-bfile",&sInputBfile)
     LONG_STRINGPARAMETER("in-bed",&sInputBed)
     LONG_STRINGPARAMETER("in-bim",&sInputBim)
     LONG_STRINGPARAMETER("in-fam",&sInputFam)
     LONG_STRINGPARAMETER("ref",&sFasta)

     LONG_PARAMETER_GROUP("Subsetting options")
     LONG_STRINGPARAMETER("in-subset",&sInputSubset)
     LONG_PARAMETER("filt-only-subset",&bFiltOnlySubset)

     LONG_PARAMETER_GROUP("Output Options")
     LONG_STRINGPARAMETER("out",&sOut)

     LONG_PARAMETER_GROUP("Output compression Options")
     EXCLUSIVE_PARAMETER("plain",&bOutPlain)
     EXCLUSIVE_PARAMETER("bgzf",&bOutBgzf)
     EXCLUSIVE_PARAMETER("gzip",&bOutGzip)

     LONG_PARAMETER_GROUP("Genotype-level Filter Options")
     LONG_INTPARAMETER("minGQ",&nMinGQ)
     LONG_INTPARAMETER("minGD",&nMinGD)

     LONG_PARAMETER_GROUP("Filter Options")
     LONG_INTPARAMETER("winIndel",&nWinIndel)
     LONG_STRINGPARAMETER("indelVCF",&sIndelVcf)
     LONG_INTPARAMETER("minQUAL",&nMinQUAL)
     LONG_INTPARAMETER("minMQ",&nMinMQ)
     LONG_INTPARAMETER("maxDP",&nMaxDP)
     LONG_INTPARAMETER("minDP",&nMinDP)
     LONG_INTPARAMETER("maxAB",&nMaxAB)
     LONG_INTPARAMETER("winFFRQ",&nWinFFRQ)
     LONG_INTPARAMETER("maxFFRQ",&nMaxFFRQ)
     LONG_INTPARAMETER("minNS",&nMinNS)
     LONG_INTPARAMETER("maxSTP",&nMaxSTP)
     LONG_INTPARAMETER("maxTTT",&nMaxTTT)
     LONG_INTPARAMETER("minTTT",&nMinTTT)
     LONG_INTPARAMETER("maxSTR",&nMaxSTR)
     LONG_INTPARAMETER("minSTR",&nMinSTR)
     LONG_INTPARAMETER("maxSTZ",&nMaxSTZ)
     LONG_INTPARAMETER("minSTZ",&nMinSTZ)
     LONG_INTPARAMETER("maxCBR",&nMaxCBR)
     LONG_INTPARAMETER("minCBR",&nMinCBR)
     LONG_INTPARAMETER("maxCBZ",&nMaxCBZ)
     LONG_INTPARAMETER("maxCSR",&nMaxCSR)
     LONG_INTPARAMETER("minCSR",&nMinCSR)
     LONG_INTPARAMETER("maxLQZ",&nMaxLQZ)
     LONG_INTPARAMETER("minLQZ",&nMinLQZ)
     LONG_INTPARAMETER("maxRBZ",&nMaxRBZ)
     LONG_INTPARAMETER("minRBZ",&nMinRBZ)
     LONG_INTPARAMETER("maxIOZ",&nMaxIOZ)
     LONG_INTPARAMETER("maxIOR",&nMaxIOR)
     LONG_INTPARAMETER("maxAOZ",&nMaxAOZ)
     LONG_INTPARAMETER("maxAOI",&nMaxAOI)
     LONG_INTPARAMETER("maxMQ0",&nMaxMQ0)
     LONG_INTPARAMETER("maxMQ20",&nMaxMQ20)
     LONG_PARAMETER("keepFilter",&bKeepFilter)
   END_LONG_PARAMETERS();

   pl.Add(new LongParameters("Available Options", longParameters));
   pl.Read(argc-1, &(argv[1]));
   pl.Status();
   
   // create objects for logging
   if ( sOut.IsEmpty() ) {
     fprintf(stderr,"ERROR: output prefix is empty");
     abort();
   }
   Logger::gLogger = new Logger((sOut+".log").c_str(), bVerbose);

   time_t t;
   time(&t);
   Logger::gLogger->writeLog("Analysis started on %s", ctime(&t));

   ////////////////////////////////////////////////////////////
   // check the compatibility of arguments
   ///////////////////////////////////////////////////////////
   // Check the sanity of input file arguments
   ///////////////////////////////////////////////////////////
   bool bVCF = true;
   if ( sInputVcf.IsEmpty() ) {  // No VCF, BED mode should be enabled
     if ( ! sInputBfile.IsEmpty() )  { // --bfile is given
       if ( sInputBim.IsEmpty() && sInputFam.IsEmpty() && sInputBed.IsEmpty() ) {
	 sInputBim = sInputBfile + ".bim";
	 sInputBed = sInputBfile + ".bed";
	 sInputFam = sInputBfile + ".fam";
	 bVCF = false;
       }
       else {
	 Logger::gLogger->error("when --in-bfile is option given, --in-bed, --in-bim, --in-fam cannot be combined together");
       }
     }
     else if ( sInputBim.IsEmpty() || sInputFam.IsEmpty() || sInputBed.IsEmpty() ) {
       // if any of --bim, --fam, --bed is missing report an error
       Logger::gLogger->error("--in-vcf, --in-bfile or all three of --in-bed, --in-bim, --in-fam arguments must be present");
     }
     else {
       // fine. go ahead
       bVCF = false;
     }
   }
   else {  // VCF mode
     if ( sInputBfile.IsEmpty() && sInputBim.IsEmpty() && sInputFam.IsEmpty() && sInputBed.IsEmpty() ) {
	 // if no BED argument was given, fine -- proceed
       bVCF = true;
     }
     else {
       // if BED argument was given, report error
       Logger::gLogger->error("when --in-vcf is option given, other BED format related options cannot be combined");
     }
   }

   if ( bRecipesFilter ) {
     if ( !bVCF ) {
       Logger::gLogger->error("--filter recipes are compatible only with VCF input");
     }
   }

   if ( bRecipesSubset ) {
     if ( sInputSubset.IsEmpty() ) {
       Logger::gLogger->error("--in-subset option is required for --subset");
     }
   }

   try {

     std::map<uint64_t,int> freqLeft;
     std::map<uint64_t,int> freqRight;
     std::vector<uint64_t> leftKeys;
     std::vector<uint64_t> rightKeys;

     if ( bRecipesFilter && ( nWinFFRQ > 0 ) && ( nMaxFFRQ > 0 ) ) {
       Logger::gLogger->writeLog("Reading VCF file and calculating the distribution of flanking %d-mers",nWinFFRQ);

       fMaxFFRQ = VcfHelper::vPhred2Err[nMaxFFRQ];

       // read over the VCF files and calculate the frequency of flanking k-mers
       char* lefts = new char[nWinFFRQ];
       char* rights = new char[nWinFFRQ];

       VcfFileReader inFile;
       VcfHeader header;
       VcfRecord record;
       
       // Open the files.
       inFile.setSiteOnly(true);
       inFile.open(sInputVcf, header);
       // Open and write the header
       //TODO outFile.open(outFileName, header);
       
       GenomeSequence genomeSequence;
       genomeSequence.setReferenceName(sFasta.c_str());

       genomeSequence.useMemoryMap(true);
       
       if ( genomeSequence.open() ) {
	 // write a message that new index file is being created
	 if ( genomeSequence.create(false) ) {
	   throw HyunVcfFileException("Failed creating index file of the reference. Please check the file permission");
	 }
	 if ( genomeSequence.open() ) {
	   throw HyunVcfFileException("Failed opening index file of the reference.");
	 }
       }
       
       while(inFile.readRecord(record))
       {
          
           genomeIndex_t markerIndex = 
               genomeSequence.getGenomePosition(record.getChromStr(), record.get1BasedPosition());
	 for(int i=0; i < nWinFFRQ; ++i) {
	   lefts[nWinFFRQ-i-1] = genomeSequence[markerIndex-i-1];
	   rights[i] = genomeSequence[markerIndex+i+1];
	 }
	 uint32_t leftKey = VcfHelper::str2TwoBits(lefts, nWinFFRQ);
	 uint32_t rightKey = VcfHelper::str2TwoBits(rights, nWinFFRQ);
	 
	 ++(freqLeft[leftKey]);
	 ++(freqRight[rightKey]);
	 leftKeys.push_back(leftKey);
	 rightKeys.push_back(rightKey);
       }

       delete [] lefts;
       delete [] rights;

       Logger::gLogger->writeLog("Finished calculating the distribution of flanking %d-mers",nWinFFRQ);
     }

     StringArray filterKeys;
     std::vector<bool> filterMinMax; // true : min, false : max
     std::vector<double> filterThres;
     std::vector<int> filterIndices;
     StringArray filterNames;
     VcfFile* pIndelVcf = NULL;
     VcfHeader indelHeader;
     // Need pointers so can keep the previous indel record.
     VcfRecord* indelRecord = NULL;
     VcfRecord* prevIndelRecord = NULL;
     VcfRecord* tempIndelRecord = NULL;

     if ( bRecipesFilter ) {
       if ( nMinMQ > 0 ) {
	 filterKeys.Add("MQ");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinMQ));
	 filterNames.Add(String("m")+nMinMQ);
       }
       if ( nMinDP > 0 ) {
	 filterKeys.Add("DP");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinDP));
	 filterNames.Add(String("dp")+nMinDP);
       }
       if ( nMaxDP < INT_MAX ) {
	 filterKeys.Add("DP");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxDP));
	 filterNames.Add(String("DP")+nMaxDP);
       }
       if ( nMinNS > 0 ) {
	 filterKeys.Add("NS");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinNS));
	 filterNames.Add(String("ns")+nMinNS);
       }
       if ( nMaxAB < 100 ) {
	 filterKeys.Add("AB");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxAB)/100.);
	 filterNames.Add(String("AB")+nMaxAB);
       }
       if ( nMaxSTP < INT_MAX ) {
	 filterKeys.Add("STP");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxSTP));
	 filterNames.Add(String("STP")+nMaxSTP);
       }
       if ( nMaxTTT < INT_MAX ) {
	 filterKeys.Add("TTT");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxTTT));
	 filterNames.Add(String("TTT")+nMaxTTT);
       }
       if ( nMinTTT > INT_MIN ) {
	 filterKeys.Add("TTT");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinTTT));
	 filterNames.Add(String("ttt")+nMinTTT);
       }
       if ( nMaxSTR < 100 ) {
	 filterKeys.Add("STR");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxSTR)/100.);
	 filterNames.Add(String("STR")+nMaxSTR);
       }
       if ( nMinSTR > -100 ) {
	 filterKeys.Add("STR");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinSTR)/100.);
	 filterNames.Add(String("str")+nMinSTR);
       }
       if ( nMaxLQZ < INT_MAX ) {
	 filterKeys.Add("LQZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxLQZ));
	 filterNames.Add(String("LQZ")+nMaxLQZ);
       }
       if ( nMinSTZ > INT_MIN ) {
	 filterKeys.Add("STZ");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinSTZ));
	 filterNames.Add(String("stz")+nMinSTZ);
       }
       if ( nMaxSTZ < INT_MAX ) {
	 filterKeys.Add("STZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxSTZ));
	 filterNames.Add(String("STZ")+nMaxSTZ);
       }
       if ( nMinLQZ > INT_MIN ) {
	 filterKeys.Add("LQZ");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinLQZ));
	 filterNames.Add(String("lqz")+nMinLQZ);
       }
       if ( nMaxRBZ < INT_MAX ) {
	 filterKeys.Add("RBZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxRBZ));
	 filterNames.Add(String("RBZ")+nMaxRBZ);
       }
       if ( nMinRBZ > INT_MIN ) {
	 filterKeys.Add("RBZ");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinRBZ));
	 filterNames.Add(String("rbz")+nMinRBZ);
       }
       if ( nMaxCBZ < INT_MAX ) {
	 filterKeys.Add("CBZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxCBZ));
	 filterNames.Add(String("CBZ")+nMaxCBZ);
       }
       if ( nMaxCBR < 100 ) {
	 filterKeys.Add("CBR");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxCBR)/100.);
	 filterNames.Add(String("CBR")+nMaxCBR);
       }
       if ( nMinCBR > -100 ) {
	 filterKeys.Add("CBR");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinCBR)/100.);
	 filterNames.Add(String("cbr")+nMinCBR);
       }
       if ( nMaxCSR < 100 ) {
	 filterKeys.Add("CSR");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxCSR)/100.);
	 filterNames.Add(String("CSR")+nMaxCSR);
       }
       if ( nMinCSR > -100 ) {
	 filterKeys.Add("CSR");
	 filterMinMax.push_back(true);
	 filterThres.push_back(static_cast<double>(nMinCSR)/100.);
	 filterNames.Add(String("csr")+nMinCSR);
       }
       if ( nMaxIOZ < INT_MAX ) {
	 filterKeys.Add("IOZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxIOZ));
	 filterNames.Add(String("IOZ")+nMaxIOZ);
       }
       if ( nMaxIOR < INT_MAX ) {
	 filterKeys.Add("IOR");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxIOR));
	 filterNames.Add(String("IOR")+nMaxIOR);
       }
       if ( nMaxAOZ < INT_MAX ) {
	 filterKeys.Add("AOZ");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxAOZ));
	 filterNames.Add(String("AOZ")+nMaxAOZ);
       }
       if ( nMaxAOI < INT_MAX ) {
	 filterKeys.Add("AOI");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxAOI));
	 filterNames.Add(String("AOI")+nMaxAOI);
       }
       if ( nMaxAOI < INT_MAX ) {
	 filterKeys.Add("MQ0");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxMQ0));
	 filterNames.Add(String("MQ0")+nMaxMQ0);
       }
       if ( nMaxAOI < INT_MAX ) {
	 filterKeys.Add("MQ20");
	 filterMinMax.push_back(false);
	 filterThres.push_back(static_cast<double>(nMaxMQ20));
	 filterNames.Add(String("MQ20")+nMaxMQ20);
       }
       if ( ! sIndelVcf.IsEmpty() ) {
	 pIndelVcf = new VcfFileReader();
	 pIndelVcf->setSiteOnly(true);
	 pIndelVcf->open(sIndelVcf.c_str(), indelHeader);
         indelRecord = new VcfRecord();
         prevIndelRecord = new VcfRecord();
       }

       Logger::gLogger->writeLog("The following filters are in effect:");
       for(int i=0; i < filterKeys.Length(); ++i) {
	 Logger::gLogger->writeLog("%s : %s %s %.2lf",filterNames[i].c_str(), filterKeys[i].c_str(), filterMinMax[i] ? ">=" : "<=", filterThres[i]);
       }
       if ( nMinQUAL > 0 ) {
	 Logger::gLogger->writeLog("q%d : QUAL >= %d",nMinQUAL,nMinQUAL);
       }
       if ( nWinIndel > 0 ) {
	 Logger::gLogger->writeLog("INDEL%d : INDEL >= %d bp with %s",nWinIndel,nWinIndel,sIndelVcf.c_str());
       }
       if ( ( nWinFFRQ > 0 ) && ( nMaxFFRQ > 0 ) ) {
	 Logger::gLogger->writeLog("FFRQ%d : Flanking %d-mer frequency <= 10^{-%.1lf}",nMaxFFRQ,nWinFFRQ,nMaxFFRQ/10.);
       }
       Logger::gLogger->writeLog("");
     }

     if ( ( bRecipesWriteVcf ) || ( bRecipesWriteBed) || ( bRecipesSubset ) ) {

       // Open input VCF/BED file
       VcfFile* pVcf;
       if ( bVCF ) {
	 pVcf = new VcfFile();
         //TODO	 pVcf->setUpgrade(bRecipesUpgrade);
	 pVcf->setSiteOnly(false);
	 pVcf->openForRead(sInputVcf.c_str(), header);
	 pVcf->nMinGD = nMinGD;
	 pVcf->nMinGQ = nMinGQ;
       }
       else {
	 BedFile* pBed = new BedFile();
	 pBed->openForRead(sInputBed.c_str(), sInputBim.c_str(), sInputFam.c_str(), sFasta.c_str());
	 pVcf = (HyunVcfFile*) pBed;
       }
       
       // Open output file
       IFILE oFile = NULL, oFamFile = NULL, oBimFile = NULL;
       if ( bRecipesWriteVcf ) {
	 String sOutVcf = sOut;

	 if ( bOutPlain ) {
	   if ( ( sOutVcf.Right(7).Compare(".vcf.gz") != 0 ) && ( sOutVcf.Right(4).Compare(".vcf") != 0 ) ) {
	     sOutVcf += ".vcf";  // append .vcf extension
	   }
	   oFile = ifopen(sOutVcf.c_str(),"wb");
	 }
	 else if ( bOutBgzf || bOutGzip ) {
	   InputFile::ifileCompression cMode = bOutBgzf ? InputFile::BGZF : InputFile::GZIP;
	   if ( sOutVcf.Right(4).Compare(".vcf") == 0 ) {
	     sOutVcf += ".gz";
	   }
	   else if ( sOutVcf.Right(7).Compare(".vcf.gz") != 0 ) {
	     sOutVcf += ".vcf.gz";
	   }
	   oFile = ifopen(sOutVcf.c_str(),"wb",cMode);
	 }
	 else {
	   throw HyunVcfFileException("Cannot recognize output compression option");
	 }

	 if ( oFile == NULL ) {
	   Logger::gLogger->error("Cannot open %s file",sOutVcf.c_str());
	 }
	 else {
	   Logger::gLogger->writeLog("Writing to VCF file %s",sOutVcf.c_str());
	 }

	 pVcf->printVCFHeader(oFile);
       }
       else if ( bRecipesWriteBed ) {
	 if ( bOutPlain ) {
	   oFile = ifopen((sOut+".bed").c_str(),"wb");
	   oBimFile = ifopen((sOut+".bim").c_str(),"wb");
	   oFamFile = ifopen((sOut+".fam").c_str(),"wb");
	 }
	 else if ( bOutBgzf ) {
	   oFile = ifopen((sOut+".bed.gz").c_str(),"wb",InputFile::BGZF);
	   oBimFile = ifopen((sOut+".bim.gz").c_str(),"wb",InputFile::BGZF);
	   oFamFile = ifopen((sOut+".fam.gz").c_str(),"wb",InputFile::BGZF);
	 }
	 else if ( bOutGzip ) {
	   oFile = ifopen((sOut+".bed.gz").c_str(),"wb",InputFile::GZIP);
	   oBimFile = ifopen((sOut+".bim.gz").c_str(),"wb",InputFile::GZIP);
	   oFamFile = ifopen((sOut+".fam.gz").c_str(),"wb",InputFile::GZIP);
	 }
	 else {
	   throw HyunVcfFileException("Cannot recognize output compression option");
	 }
	 
	 if ( ( oFile == NULL ) || ( oBimFile == NULL ) || ( oFamFile == NULL ) ) {
	   Logger::gLogger->error("Cannot open %s.{bim,bed,fam} file",sOut.c_str());
	 }
	 pVcf->printBEDHeader(oFile,oFamFile);
       }

       // identify the list of individuals to be subsetted
       std::vector< std::string > subsetNames;        // list of subset IDs
       std::vector< std::vector<int> > subsetIndices; // per subset list of individuals
       std::vector< IFILE > subsetOutFiles;           // per subset list of output files
       if ( bRecipesSubset ) {
	 String line;
	 IFILE iSubsetFile = ifopen( sInputSubset.c_str(), "rb" );
	 std::map< std::string, int > name2SampleInd;
	 StringArray tok, tok2;
	 std::string ind, subset;
	 int subsetId;

	 // build map to sample name to index
	 for(int i=0; i < (int)pVcf->vpVcfInds.size(); ++i) {
	   name2SampleInd[pVcf->vpVcfInds[i]->sIndID.c_str()] = i;
	 }

	 if ( iSubsetFile == NULL ) {
	   Logger::gLogger->error("Cannot open %s file",sInputSubset.c_str());
	 }
	 while ( line.ReadLine(iSubsetFile) > 0 ) {
	   tok.ReplaceTokens(line,"\t\r\n ");
	   if ( tok.Length() < 2 ) {
	     Logger::gLogger->error("Cannot recognize subset label for in %s ",sInputSubset.c_str());
	   }
	   tok2.ReplaceColumns(tok[1],',');
	   ind = tok[0].c_str();

	   // check if sample exists in the VCF
	   if ( name2SampleInd.find(ind) == name2SampleInd.end() ) {
	     Logger::gLogger->error("Cannot recognize individual ID %s",ind.c_str());
	   }

	   // iterate thru subset names
	   for(int i=0; i < tok2.Length(); ++i) {
	     subset = tok2[i].c_str();
	     subsetId = -1;
	     for(int j=0; j < (int)subsetNames.size(); ++j) {
	       if ( subset.compare(subsetNames[j]) == 0 ) {
		 subsetId = j;
		 break;
	       }
	     }
	     if ( subsetId < 0 ) {
	       subsetNames.push_back(subset);
	       subsetIndices.push_back(std::vector<int>());
	       subsetId = subsetNames.size()-1;
	     }
	     subsetIndices[subsetId].push_back(name2SampleInd[ind]);
	   }
	 }
	 
	 // and create output files
	 for(int i=0; i < (int)subsetNames.size(); ++i) {
	   String sOutVcf = sOut + "." + subsetNames[i].c_str();
	   IFILE f;

	   if ( bOutPlain ) {
	     if ( ( sOutVcf.Right(7).Compare(".vcf.gz") != 0 ) && ( sOutVcf.Right(4).Compare(".vcf") != 0 ) ) {
	       sOutVcf += ".vcf";  // append .vcf extension
	     }
	     f = ifopen(sOutVcf.c_str(),"wb");
	   }
	   else if ( bOutBgzf || bOutGzip ) {
	     InputFile::ifileCompression cMode = bOutBgzf ? InputFile::BGZF : InputFile::GZIP;
	     if ( sOutVcf.Right(4).Compare(".vcf") == 0 ) {
	       sOutVcf += ".gz";
	     }
	     else if ( sOutVcf.Right(7).Compare(".vcf.gz") != 0 ) {
	       sOutVcf += ".vcf.gz";
	     }
	     f = ifopen(sOutVcf.c_str(),"wb",cMode);
	   }
	   else {
	     throw HyunVcfFileException("Cannot recognize output compression option");
	   }

	   if ( f == NULL ) {
	     Logger::gLogger->error("Error in opening output file");
	   }

	   subsetOutFiles.push_back(f);
	   pVcf->printVCFHeaderSubset(f,subsetIndices[i]);
	 }

	 Logger::gLogger->writeLog("Found %d groups for subsetting",subsetNames.size());
	 for(int i=0; i < (int)subsetNames.size(); ++i) {
	   Logger::gLogger->writeLog("%s - %u individuals",subsetNames[i].c_str(), subsetIndices[i].size());
	 }
       }

       // read input files
       for( int cnt = 0; pVcf->iterateMarker(); ++cnt ) {
	 VcfMarker* pMarker = pVcf->getLastMarker();

	 //Logger::gLogger->writeLog("%s:%d",pMarker->sChrom.c_str(),pMarker->nPos);

	 // Apply filters
	 if ( bRecipesFilter ) {
	   if ( bKeepFilter ) {
	     if ( ( pMarker->asFilters.Length() == 1 ) && ( ( pMarker->asFilters[0].Compare(".") == 0 ) || ( pMarker->asFilters[0].Compare("PASS") == 0 ) || ( pMarker->asFilters[0].Compare("0") == 0 ) ) ) {
	       pMarker->asFilters.Clear();
	     }
	   }
	   else {
	     pMarker->asFilters.Clear();
	   }

	   // QUAL filter
	   if ( pMarker->fQual < nMinQUAL ) {
	     pMarker->asFilters.Add(String("q")+nMinQUAL);
	   }


	   // Indel filter
           // Keep reading the indel vcf until it has gotten to or past the record's position or
           // until the end of the file.
           const char* recChrom = record.getChromStr();;
           int recPos = record.get1BasedPosition();
	   while ( ( pIndelVcf != NULL ) &&
                   ( ! pIndelVcf.readRecord(&indelRecord)) && 
                   (VcfHelper::compareGenomicPos( indelRecord.getChromStr(), 
                                                  indelRecord.get1BasedPosition(),
                                                  recChrom, recPos ) < 0) )
           {
               // just keep looping until we have found or passed our position in the indel file.
               
               // Store this record as the previous one and setup to read hte next one.
               // Temporarily Store the record that had held the previous one.
               tempIndel = prevIndelRecord;
               // Store this record into the pointer to the previous one.
               prevIndelRecord = indelRecord;
               // Set the next one to be read from the temp pointer.
               indelRecord = tempIndel;
               tempIndel = NULL;
	   }
	   
	   if ( ( pIndelVcf != NULL ) && ( !pIndelVcf->bEOF ) && ( nWinIndel > 0 ) ) 
           {
               int d1 = VcfHelper::compareGenomicPos( indelRecord->getChromStr(),
                                                      indelRecord->get1BasedPosition(),
                                                      recChrom, recPos );
               int d2 = ( pIndelVcf->nNumMarkers > 1 ) ? 
                   VcfHelper::compareGenomicPos( recChrom, recPos, 
                                                 prevIndelRecord->getChromStr(), 
                                                 prevIndelRecord->get1BasedPosition() ) : 1000000;
               if ( (d1 < 0) || (d2 < 0) )
               {
                   Logger::gLogger->warning("%s:%d, d1=%d, d2=%d",pMarker->sChrom.c_str(),pMarker->nPos,d1,d2);
               }
               
               if ( ( d1 < nWinIndel ) || ( d2 < nWinIndel ) )
               {
                   pMarker->asFilters.Add(String("INDEL")+nWinIndel);
               }
	   }

	   // Update filter index if order in the INFO field has been changed
	   if ( (pVcf->nBuffers == 1) && (pMarker->bPreserved) )
           {
               // do not update filterIndices
	   }
	   else
           {
               filterIndices.resize(filterKeys.Length());
               for(int i=0; i < filterKeys.Length(); ++i)
               {
                   filterIndices[i] = pMarker->asInfoKeys.Find(filterKeys[i]);
               }
	   }

	   // apply standard filters
	   for(int i=0; i < (int)filterIndices.size(); ++i)
           {
               if ( filterIndices[i] >= 0 )
               {
                   if ( filterMinMax[i] )
                   { // min
                       if ( atof(pMarker->asInfoValues[filterIndices[i]].c_str()) < filterThres[i] )
                       {
                           pMarker->asFilters.Add(filterNames[i]);
                       }
                   }
                   else
                   {
                       if ( atof(pMarker->asInfoValues[filterIndices[i]].c_str()) > filterThres[i] )
                       {
                           pMarker->asFilters.Add(filterNames[i]);
                       }
                   }
               }
	   }

	   // apply flanking frquency filters
	   if ( ( nWinFFRQ > 0 ) && ( nMaxFFRQ > 0 ) )
           {
               int maxFrq = freqLeft[leftKeys[cnt]];
               if ( maxFrq < freqRight[rightKeys[cnt]] )
               {
                   maxFrq = freqRight[rightKeys[cnt]];
               }

               if ( maxFrq > fMaxFFRQ * leftKeys.size() )
               {
                   pMarker->asFilters.Add(String("FFRQ")+nMaxFFRQ);
               }
	   }
	 }

	 if ( bRecipesWriteVcf ) {
	   pMarker->printVCFMarker(oFile,false);
	 }
	 else if ( bRecipesWriteBed ) {
	   pMarker->printBEDMarker(oFile,oBimFile,false);
	 }

	 if ( bRecipesSubset ) {
	   bool filterPass = true;
	   if ( bFiltOnlySubset ) {
	     if ( ( pMarker->asFilters.Length() > 0 ) && ( pMarker->asFilters[0].Compare("PASS") != 0 ) ) {
	       filterPass = false;
	     }
	   }

	   if ( filterPass ) {
	     for(int i=0; i < (int)subsetNames.size(); ++i) {
	       pMarker->printVCFMarkerSubset(subsetOutFiles[i],subsetIndices[i]);
	     }
	   }
	 }
       }
       
       if ( oFile != NULL ) {
	 ifclose(oFile);
       }
       if ( bRecipesWriteBed ) {
	 ifclose(oBimFile);
	 //ifclose(oFamFile);
       }
       if ( bRecipesSubset ) {
	 for(int i=0; i < (int)subsetNames.size(); ++i) {
	   ifclose(subsetOutFiles[i]);
	 }
       }
     }
     else {
       Logger::gLogger->error("One of --write-vcf, --write-bed, --subset or --summarize recipes must be provided to process the input file");
     }
   }
   catch (HyunVcfFileException e) {
     Logger::gLogger->error(e.msg.c_str());
   }

   time(&t);
   Logger::gLogger->writeLog("Analysis finished on %s", ctime(&t));

   return 0;
}
