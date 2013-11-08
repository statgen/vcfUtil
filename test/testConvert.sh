#!/bin/bash

status=0;

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testTabix.vcf 2> results/testConvert.log
let "status |= $?"
diff results/testTabix.vcf testFiles/testTabix.vcf
let "status |= $?"
diff results/testConvert.log expected/testConvert.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testInvalidSampleExtra.vcf --uncompress --out results/testInvalidSampleExtra.vcf 2> results/testInvalidSampleExtra.log
if [ $? == 0 ]
then
  let "status = 1"
fi
diff results/testInvalidSampleExtra.vcf expected/testInvalidSampleExtra.vcf
let "status |= $?"
diff results/testInvalidSampleExtra.log expected/testInvalidSampleExtra.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testInvalidSampleLess.vcf --uncompress --out results/testInvalidSampleLess.vcf 2> results/testInvalidSampleLess.log
let "status |= $?"
diff results/testInvalidSampleLess.vcf expected/testInvalidSampleLess.vcf
let "status |= $?"
diff results/testInvalidSampleLess.log expected/testInvalidSampleLess.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testTabix1.vcf --refName 1 2> results/testConvert1.log
let "status |= $?"
diff results/testTabix1.vcf expected/testTabix1.vcf
let "status |= $?"
diff results/testConvert1.log expected/testConvert1.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testTabix3.vcf --refName 3 2> results/testConvert3.log
let "status |= $?"
diff results/testTabix3.vcf expected/testTabix3.vcf
let "status |= $?"
diff results/testConvert3.log expected/testConvert3.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testTabix4.vcf --refName 4 2> results/testConvert4.log
let "status |= $?"
diff results/testTabix4.vcf expected/testTabix4.vcf
let "status |= $?"
diff results/testConvert4.log expected/testConvert4.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testExcludeR1ID.vcf --idExclude testFiles/r1Id.txt 2> results/testExcludeR1ID.log
let "status |= $?"
diff results/testExcludeR1ID.vcf expected/testR2.vcf
let "status |= $?"
diff results/testExcludeR1ID.log expected/test2.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testIncludeR2ID.vcf --idInclude testFiles/r2Id.txt 2> results/testIncludeR2ID.log
let "status |= $?"
diff results/testIncludeR2ID.vcf expected/testR2.vcf
let "status |= $?"
diff results/testIncludeR2ID.log expected/test2.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testExcludeR2ID.vcf --idExclude testFiles/r2Id.txt 2> results/testExcludeR2ID.log
let "status |= $?"
diff results/testExcludeR2ID.vcf expected/testR1.vcf
let "status |= $?"
diff results/testExcludeR2ID.log expected/test2.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testIncludeR1ID.vcf --idInclude testFiles/r1Id.txt 2> results/testIncludeR1ID.log
let "status |= $?"
diff results/testIncludeR1ID.vcf expected/testR1.vcf
let "status |= $?"
diff results/testIncludeR1ID.log expected/test2.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testExcludeR1R2ID.vcf --idExclude testFiles/r1r2Id.txt 2> results/testExcludeR1R2ID.log
let "status |= $?"
diff results/testExcludeR1R2ID.vcf expected/testNone.vcf
let "status |= $?"
diff results/testExcludeR1R2ID.log expected/test0.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testIncludeR1R2ID.vcf --idInclude testFiles/r1r2Id.txt 2> results/testIncludeR1R2ID.log
let "status |= $?"
diff results/testIncludeR1R2ID.vcf expected/testR1R2.vcf
let "status |= $?"
diff results/testIncludeR1R2ID.log expected/test4.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testExcludeNoID.vcf --idExclude testFiles/noId.txt 2> results/testExcludeNoID.log
let "status |= $?"
diff results/testExcludeNoID.vcf expected/testR1R2.vcf
let "status |= $?"
diff results/testExcludeNoID.log expected/test4.log
let "status |= $?"

# If nothing is in the include list, it does not get applied.
../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testIncludeNoID.vcf --idInclude testFiles/noId.txt 2> results/testIncludeNoID.log
let "status |= $?"
diff results/testIncludeNoID.vcf expected/testR1R2.vcf
let "status |= $?"
diff results/testIncludeNoID.log expected/test4.log
let "status |= $?"


if [ $status != 0 ]
then
  echo failed testConvert.sh
  exit 1
fi

