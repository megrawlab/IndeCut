#!/usr/bin/perl

$infile = shift; # input file
$outbase = shift; # output file base name

%vnumColorHash = ();

print($infile . "\n");
open(EDGES, ">$outbase.edges.txt");
open(IN, $infile) || die("Failed to open $infile");
while (<IN>) {
    $line = $_;
    chomp($line);

    ($v1num, $v2num, $v1color, $v2color) = split(/\t/, $line);
    print EDGES "$v1num\t$v2num\n";
    if (exists($vnumColorHash{$v1num})) {
	unless ($vnumColorHash{$v1num} eq $v1color) {
	    die("ERROR: Conflicting colors for vertex $v1num");
	}
    }
    else {
	$vnumColorHash{$v1num} = $v1color;
    }
    if (exists($vnumColorHash{$v2num})) {
	unless ($vnumColorHash{$v2num} eq $v2color) {
	    die("ERROR: Conflicting colors for vertex $v2num");
	}
    }
    else {
	$vnumColorHash{$v2num} = $v2color;
    }
}
close(IN);
close(EDGES);

open(VERTS, ">$outbase.vertices.txt");
print VERTS "name\tcolor\n";
foreach $v (sort numerically keys %vnumColorHash) {
    $color = $vnumColorHash{$v};
    print VERTS "$v\t$color\n";
}
close(VERTS);

sub numerically {$a <=> $b; }
