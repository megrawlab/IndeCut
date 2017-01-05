#!/usr/bin/perl

$infile = shift; # input file
$outbase = shift; # output file base name

print($infile . "\n");
open(EDGES, ">$outbase");
open(IN, $infile) || die("Failed to open $infile");
while (<IN>) {
    $line = $_;
    chomp($line);

    ($v1num, $v2num) = split(/\t/, $line);
    $dv1 = $v1num + 1;
    $dv2 = $v2num + 1;
    print EDGES "$dv1\t$dv2\n";
}
close(IN);
close(EDGES);

