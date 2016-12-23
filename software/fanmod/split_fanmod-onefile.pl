#!/usr/bin/perl

$infile = shift; 
$outdir = shift;
$outbase = shift;

open(IN, $infile) || die "Can't open $infile";

$outlines="";
$count = 1;
while (<IN>) {
    $line = $_;
    chomp($line);

    if ($line eq "") {
	$outfile = "$outdir/$outbase.$count";
	open(OUT, ">$outfile");
	print OUT $outlines;
	close(OUT);
	$outlines = "";
	$count++;
	next;
    }

    ($n1, $n2) = split(/\s+/, $line);
    $newline = "$n1\t$n2\n";
    $outlines .= $newline;
}
close(IN);
