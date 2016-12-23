#!/usr/bin/perl

$infile = shift;
$linenum = shift;

open(IN, $infile) || die("Failed to open $infile");
@lines = <IN>;
close(IN);
$numout = $lines[$linenum - 1];
print $numout;
