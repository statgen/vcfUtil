#!/bin/bash

status=0;

../bin/vcfUtil split --in testFiles/testTabix.vcf --uncompress --obase results/testSplit 2> results/testSplit.log
let "status |= $?"
diff results/testSplit.chr1.vcf expected/testSplit.chr1.vcf
let "status |= $?"
diff results/testSplit.chr3.vcf expected/testSplit.chr3.vcf
let "status |= $?"
diff results/testSplit.log expected/testSplit.log
let "status |= $?"




if [ $status != 0 ]
then
  echo failed testSplit.sh
  exit 1
fi

