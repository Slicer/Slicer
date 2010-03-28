#!/usr/bin/perl
#
# Simple perl script to generate the expected output of test scripts.
# This should never really be run.
# If you do run it, make sure the output conforms to what you expect.
#

for ( $i = 1; $i <= $ARGV[0]; $i++ )
{
  system "test$i.sh; mv tmp.out test$i.out";
}
