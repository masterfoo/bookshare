#!/usr/bin/perl

use strict;
use DBI;

require '/home/ghandi/mods/bookshare/bookshare.pl';

# Chris Handwerker - chandwer@student.fitchburgstate.edu
# Generates user control panel page

# This will display a users book listings
# Get session id and validate
my $username = auth();

if($username)
{
	my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
	my ($sth,$date,$title,$edition,$isbn,$bookid);
	
	$sth = $dbh->prepare("
				SELECT strftime('%m/%d/%Y',listings.date_posted),books.title,books.edition,
						books.isbn,books.book_ref_id
				FROM listings 
					INNER JOIN users ON users.user_id=listings.user_id 
					INNER JOIN books ON listings.book_ref_id=books.book_ref_id 
				WHERE users.username=? ORDER BY listings.date_posted");
	$sth->execute($username);
	$sth->bind_columns(\$date,\$title,\$edition,\$isbn,\$bookid);

print "<table cellpadding=2 border=1>";
print "<tr><td><b>Date Posted</b></td><td><b>Title</b></td><td><b>Edition</b></td><td><b>ISBN</b></td><td><b>Authors</b></td></tr>\n";
	while($sth->fetch())
	{
		print "<tr><td valign=top>$date</td><td valign=top>$title</td><td valign=top>$edition</td><td valign=top>$isbn</td>";
		my $author;
		my $sth = $dbh->prepare("
						SELECT author.author_name 
						FROM author 
							INNER JOIN book_author ON author.author_ref_id=book_author.author_ref_id 
						WHERE book_author.book_ref_id=?");

		$sth->execute($bookid);
		$sth->bind_columns(\$author);
		print "<td>";
		while($sth->fetch())
		{
			print "$author<br>";
		}
		"</td></td>\n";
	}
print "</table>";
}
else
{
	print "<b>You need an account to view this page</b>\n";
}
