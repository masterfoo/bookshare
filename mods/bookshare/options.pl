#!/usr/bin/perl

use strict;

use CGI::Simple;

# Search Type Select Box Options
# Chris Handwerker - chandwer@student.fitchburgstate.edu
#
# Looks at "type" param in query string and generates
# options in the correct order for a select box.
# Defaults to "Title".

sub option
{
	my $value = shift;
	print "<option value=".lc($value).">$value</option>";
}

my $q = new CGI::Simple;
my $type = $q->param('type');

if($type eq "isbn")
{
	option("ISBN");
	option("Title");
	option("Author");
}
elsif($type eq "author")
{
	option("Author");
	option("Title");
	option("ISBN");
}
else
{
	option("Title");
	option("Author");
	option("ISBN");
}
