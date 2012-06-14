#!/bin/bash

status=0;

../bin/vcfUtil convert --in testFiles/testTabix.vcf --uncompress --out results/testTabix.vcf 2> results/testConvert.log
let "status |= $?"
diff results/testTabix.vcf testFiles/testTabix.vcf
let "status |= $?"
diff results/testConvert.log expected/testConvert.log
let "status |= $?"

../bin/vcfUtil convert --in testFiles/testInvalidSampleExtra.vcf --uncompress --out results/testInvalidSampleExtra.vcf 2> results/testInvalidSampleExtra.log
let "status |= $?"
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



if [ $status != 0 ]
then
  echo failed testConvert.sh
  exit 1
fi

