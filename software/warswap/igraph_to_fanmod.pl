#!/usr/bin/perl

$removeSelfLoops = shift; # 0 for don't remove, 1 for remove
$vertexfilein = shift; # input file
$edgefilein = shift; # input file
$fanmodout = shift; # output file base name

%vnumColorHash = ();

open(VERTS, $vertexfilein) || die("Failed to open $vertexfilein");
$header = <VERTS>;
while (<VERTS>) {
    $line = $_;
    chomp($line);

    ($vnum, $color) = split(/\t/, $line);
    $vnumColorHash{$vnum} = $color;
}
close(VERTS);

%edgehash = ();

open(EDGES, $edgefilein) || die("Failed to open $edgefilein");
while (<EDGES>) {
    $line = $_;
    chomp($line);

    ($v1num, $v2num) = split(/\t/, $line);

    if ($removeSelfLoops) { # Remove self-loops
	if ($v1num eq $v2num) { next; }
    }

    $v1color = $vnumColorHash{$v1num};
    $v2color = $vnumColorHash{$v2num};
    
    $tuple = join("_", $v1num, $v2num, $v1color, $v2color);
    
    if (exists($edgehash{$tuple})) { die("$tuple observed multiple times"); }
    $edgehash{$tuple}{'v1'} = $v1num;
    $edgehash{$tuple}{'v2'} = $v2num;
}
close(EDGES);

open(OUT, ">$fanmodout");
foreach $tuple ( sort { ($edgehash{$a}{'v1'} <=> $edgehash{$b}{'v1'}) ||
			    ($edgehash{$a}{'v2'} <=> $edgehash{$b}{'v2'})
		 } keys %edgehash ) {
    ($v1num, $v2num, $v1color, $v2color) = split(/\_/, $tuple);
    $outtuple = join("\t", $v1num, $v2num, $v1color, $v2color);
    print OUT $outtuple, "\n";
}
close(OUT);
