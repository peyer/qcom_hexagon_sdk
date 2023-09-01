#!/usr/bin/perl
# convert newer assembly with allocframe, dealloc_return containing :raw
# to older format.
#
# Usage: asm_to_older_compat.pl < infile >outfile
#
while (<>) {
  my $line = $_;
  $line =~ s/r31:30=dealloc_return\(r30\):nt:raw/dealloc_return:nt/g;
  $line =~ s/r31:30=dealloc_return\(r30\):t:raw/dealloc_return:t/g;
  $line =~ s/r31:30=dealloc_return\(r30\):raw/dealloc_return/g;
  $line =~ s/r31:30=deallocframe\(r30\):raw/deallocframe/g;
  $line =~ s/allocframe\(r29,/allocframe\(/g;
  if ($line =~ m/allocframe/) {
    $line =~s/:raw//g;
  }
  print $line;
}

