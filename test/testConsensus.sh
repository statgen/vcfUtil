#!/bin/bash

status=0;

../bin/vcfUtil consensus --in1 testFiles/testTabix.vcf --in2 testFiles/testTabix.vcf --in3 testFiles/testTabix.vcf --noph --uncompress --out results/testConsensusSame.vcf 2> results/testConsensusSame.log
let "status |= $?"
diff results/testConsensusSame.vcf testFiles/testTabix.vcf
let "status |= $?"
diff results/testConsensusSame.log expected/testConsensusSame.log
let "status |= $?"

../bin/vcfUtil consensus --in1 testFiles/testTabix2.vcf --in2 testFiles/testTabix2.vcf --in3 testFiles/testTabix2.vcf --noph --uncompress --out results/testConsensusSame2.vcf 2> results/testConsensusSame2.log
let "status |= $?"
diff results/testConsensusSame2.vcf testFiles/testTabix2.vcf
let "status |= $?"
diff results/testConsensusSame2.log expected/testConsensusSame2.log
let "status |= $?"

../bin/vcfUtil consensus --in1 testFiles/testTabix2.vcf --in2 testFiles/testTabix.vcf --in3 testFiles/testTabix2.vcf --noph --uncompress --out results/testConsensusMismatchRefAlt.vcf 2> results/testConsensusMismatchRefAlt.log
let "status |= $?"
diff results/testConsensusMismatchRefAlt.vcf expected/testConsensusMismatchRefAlt.vcf
let "status |= $?"
diff results/testConsensusMismatchRefAlt.log expected/testConsensusMismatchRefAlt.log
let "status |= $?"


../bin/vcfUtil consensus --in1 testFiles/testTabix2.vcf --in2 testFiles/testTabix3.vcf --in3 testFiles/testTabix2.vcf --noph --uncompress --out results/testConsensus3.vcf 2> results/testConsensus3.log
let "status |= $?"
diff results/testConsensus3.vcf testFiles/testTabix2.vcf
let "status |= $?"
diff results/testConsensus3.log expected/testConsensus3.log
let "status |= $?"


../bin/vcfUtil consensus --in1 testFiles/testTabix2.vcf --in2 testFiles/testTabix3.vcf --in3 testFiles/testTabix4.vcf --noph --uncompress --out results/testConsensus4.vcf 2> results/testConsensus4.log
let "status |= $?"
diff results/testConsensus4.vcf expected/testConsensus4.vcf
let "status |= $?"
diff results/testConsensus4.log expected/testConsensus4.log
let "status |= $?"

../bin/vcfUtil consensus --in1 testFiles/testTabix3.vcf --in2 testFiles/testTabix4.vcf --in3 testFiles/testTabix2.vcf --noph --uncompress --out results/testConsensus5.vcf 2> results/testConsensus5.log
let "status |= $?"
diff results/testConsensus5.vcf expected/testConsensus4.vcf
let "status |= $?"
diff results/testConsensus5.log expected/testConsensus5.log
let "status |= $?"

../bin/vcfUtil consensus --in1 testFiles/testTabix4.vcf --in2 testFiles/testTabix2.vcf --in3 testFiles/testTabix3.vcf --noph --uncompress --out results/testConsensus6.vcf 2> results/testConsensus6.log
let "status |= $?"
diff results/testConsensus6.vcf expected/testConsensus4.vcf
let "status |= $?"
diff results/testConsensus6.log expected/testConsensus6.log
let "status |= $?"

if [ $status != 0 ]
then
  echo failed testConsensus.sh
  exit 1
fi

