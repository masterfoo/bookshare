#!/usr/bin/perl

use warnings;
use strict;

use CGI::Simple;
use CGI::Carp 'fatalsToBrowser';
use Business::ISBN;
use DBI;

# ISBN Checker
# Chris Handwerker - chandwer@student.fitchburgstate.edu
#
# Takes an ISBN number and normalizes it
# Checks the database for that ISBN number to see if book
# details already exist for that ISBN
#
# Returns a normalized ISBN at least but also returns
# book title, edition, and author(s) if available in XML format
#
# If ISBN invalid returns 400 Bad Request 

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
sub db_check
{
	my ($isbn,$dbh) = @_;
	my $sth;
	my ($title,$edition,$author);
	my (@authors,@ret);

	$sth = $dbh->prepare("
				SELECT books.title,books.edition,author.author_name FROM books 
					INNER JOIN book_author ON books.book_ref_id=book_author.book_ref_id 
					INNER JOIN author ON book_author.author_ref_id=author.author_ref_id 
				WHERE books.isbn=? 
				GROUP BY author.author_name");
	$sth->execute($isbn);
	$sth->bind_columns(\$title,\$edition,\$author);
	while($sth->fetch())
	{
		push(@authors,$author);
	}
	return($title,$edition,@authors);
}

my $q = new CGI::Simple;
my $isbn = isbn_normal($q->param('isbn'));
if($isbn)
{
	# isbn is valid, check the database
	my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
	my ($title,$edition,@authors) = db_check($isbn,$dbh);
	$title = $q->escapeHTML($title);
	$edition = $q->escapeHTML($edition);

	print $q->header(-status=> '200 OK',-type=> 'text/xml');
	print "<book>\n";
	print "<isbn>$isbn</isbn>\n";
	print "<title>$title</title>\n" unless !defined($title);
	print "<edition>$edition</edition>\n" unless !defined($edition);

	foreach my $author(@authors)
	{
		print "<author>$author</author>\n";
	}

	print "</book>";
}
else
{
	print $q->header(-status => '400 Bad Request');
}
