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



if [ $status != 0 ]
then
  echo failed testConvert.sh
  exit 1
fi

