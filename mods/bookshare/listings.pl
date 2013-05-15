#!/usr/bin/perl

use strict;

use CGI::Simple;
use DBI;

require '/home/ghandi/mods/bookshare/bookshare.pl';

# Chris Handwerker - chandwer@student.fitchburgstate.edu
# Generates user control panel page

my $q = new CGI::Simple;

# This will display a users book listings
# Get session id and validate
my $sid = $q->cookie('sid');
my $username = auth();

if($username)
{
	my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
	my ($sth,$date,$title,$edition,$isbn,$bookid);
	
	$sth = $dbh->prepare("
				SELECT listings.date_posted,books.title,books.edition,
						books.isbn,books.book_ref_id
				FROM listings 
					INNER JOIN users ON users.user_id=listings.user_id 
					INNER JOIN books ON listings.book_ref_id=books.book_ref_id 
				WHERE users.username=?");
	$sth->execute($username);
	$sth->bind_columns(\$date,\$title,\$edition,\$isbn,\$bookid);

	print "<b>Book listing for:</b> $username<br><br>";
	while($sth->fetch())
	{
		print "$date\t$title\t$edition\t$isbn";
		my $author;
		my $sth = $dbh->prepare("
						SELECT author.author_name 
						FROM author 
							INNER JOIN book_author ON author.author_ref_id=book_author.author_ref_id 
						WHERE book_author.book_ref_id=?");

		$sth->execute($bookid);
		$sth->bind_columns(\$author);
		print "\nAuthors: ";
		while($sth->fetch())
		{
			print "$author, ";
		}
		print "\n\n\n\n";
	}
}
