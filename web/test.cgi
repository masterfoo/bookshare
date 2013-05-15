#!/usr/bin/perl

sub auth
{
	`mod_find accounts:auth` =~ /^(OK)\s(.+)/;
	return $1?$2:0;
}

$username=auth();

print "text/html\n\n$username";
