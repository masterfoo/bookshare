#!/usr/bin/perl

$title = "FSU BookShare";
if($ARGV[0]) { $title.=" - $ARGV[0]"; }
print "<div class=banner><a href=/><p class=title>$title</p></a></div>";
