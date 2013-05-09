#!/usr/bin/perl

use strict;
use warnings;

use CGI::Simple;
use DBI;
use Business::ISBN;

sub isbn_normal
{
	# returns properly formatted ISBN-13 if valid
	# returns 0 if invalid
	my $isbn = Business::ISBN->new($_[0]);
	if($isbn)
	{
		if($isbn->is_valid)
		{
			my $isbn13 = $isbn->as_isbn13;
			return $isbn13->as_string;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}
sub format_result
{
	my ($title,$isbn,$authors_ref,$user,$date,$edition)=@_;
	my $result;

	$result="";
	$result .= "<div class=entry>";
	$result .= "<span class=title>$title</span><br>";
	$result .= "<span class=edition>Edition: $edition</span><br>" unless !defined($edition);
	$result .= "<span class=isbn>ISBN: $isbn</span><br><br>";
	foreach my $author(@$authors_ref)
	{
   		$result .= "<span class=author>$author</span><br>";
	}

	$result .= "<span class=poster>Posted by: $user</span>";
	$result .= "<span class=date>$date</span>";
	$result .= "</div>";
	return $result;
}


# Main
# Treating $q as a new instance of CGI
my $q=new CGI::Simple;

my $type=$q->param('type');
my $keyword=$q->param('keywords');

if($type eq "isbn")
{
	my $isbn= isbn_normal($keyword);
	if($isbn)
	{
		my @authors = ('Chris','Ade','Foo','Meow','Dickbutt');
		print format_result('Foo','1234',\@authors,'testuser','9/11');	

	}	
	else
	{
		print "Invalid ISBN";	
	}

}
elsif($type eq "title")
{

}
elsif($type eq "author")
{

}
