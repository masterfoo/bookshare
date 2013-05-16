#!/usr/bin/perl

use strict;
use warnings;

use CGI::Simple;
use DBI;
use Business::ISBN;
use CGI::Carp 'fatalsToBrowser';	# send errors to browser for debugging

require '/home/ghandi/mods/bookshare/bookshare.pl';

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

my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
my ($sth,$date,$title,$edition,$isbn,$bookid,$username);
my $search_term;

if($type eq "isbn")
{
	my $isbn= get_isbn($keyword);
	if($isbn)
	{
		print "<b>Results for:</b> $isbn<br><br>";
		#get book match & author ref id
		$sth = $dbh->prepare("
			SELECT listings.date_posted,books.title,books.edition,
					books.isbn,books.book_ref_id,users.username
			FROM listings 
				INNER JOIN users ON users.user_id=listings.user_id 
				INNER JOIN books ON listings.book_ref_id=books.book_ref_id 
			WHERE books.isbn=? ORDER BY listings.date_posted");
		$sth->execute($isbn);
		$sth->bind_columns(\$date,\$title,\$edition,\$isbn,\$bookid,\$username);
		
		while($sth->fetch())
		{
			#Get author associated with the book
			my $author;
			my $sth = $dbh->prepare("
							SELECT author.author_name
							FROM author
								INNER JOIN book_author ON author.author_ref_id=book_author.author_ref_id
							WHERE book_author.book_ref_id=? ORDER BY author.author_name");
			$sth->execute($bookid);
			$sth->bind_columns(\$author);
			#Add authors to list
			my @authors;
			while($sth->fetch())
			{
				push(@authors, $author);
			}
			print format_result($title,$isbn,\@authors,$username,$date,$edition);	
		}
	}	
	else
	{
		print "Invalid ISBN";	
	}

}
elsif($type eq "title")
{
	if($keyword)
	{   
		$search_term=$keyword;
		my @keywords = split(/\s+/,$keyword);
		$keyword = "";
		foreach(@keywords) { $keyword .= "\\b$_\\b|"; } 
		chop($keyword);
		$keyword = lc($keyword);
		print "<b>Results for:</b> $search_term<br><br>";
		#get book match & author ref id
		$sth = $dbh->prepare("
			SELECT listings.date_posted,books.title,books.edition,
					books.isbn,books.book_ref_id,users.username
			FROM listings 
				INNER JOIN users ON users.user_id=listings.user_id 
				INNER JOIN books ON listings.book_ref_id=books.book_ref_id 
			WHERE LOWER(books.title) REGEXP '$keyword' ORDER BY listings.date_posted");
		$sth->execute();
		$sth->bind_columns(\$date,\$title,\$edition,\$isbn,\$bookid,\$username);
		
		while($sth->fetch())
		{
			#Get author associated with the book
			my $author;
			my $sth = $dbh->prepare("
							SELECT author.author_name
							FROM author
								INNER JOIN book_author ON author.author_ref_id=book_author.author_ref_id
							WHERE book_author.book_ref_id=? ORDER BY author.author_name");
			$sth->execute($bookid);
			$sth->bind_columns(\$author);
			#Add authors to list
			my @authors;
			while($sth->fetch())
			{
				push(@authors, $author);
			}
			print format_result($title,$isbn,\@authors,$username,$date,$edition);	
		}
	}	
	else
	{
		print "Invalid Title";	
	}

}
elsif($type eq "author")
{
	if($keyword)
	{
		$search_term=$keyword;
		my @keywords = split(/\s+/,$keyword);
		$keyword = "";
		foreach(@keywords) { $keyword .= "\\b$_\\b|"; } 
		chop($keyword);
		$keyword = lc($keyword);
		print "<b>Results for:</b> $search_term<br><br>";
		#get book match & author ref id
		$sth = $dbh->prepare("
			SELECT listings.date_posted,books.title,books.edition,
					books.isbn,books.book_ref_id,users.username
			FROM listings 
				INNER JOIN users ON users.user_id=listings.user_id 
				INNER JOIN books ON listings.book_ref_id=books.book_ref_id
				INNER JOIN book_author ON books.book_ref_id=book_author.book_ref_id
			    INNER JOIN author ON author.author_ref_id=book_author.author_ref_id	
			WHERE LOWER(author.author_name) REGEXP '$keyword' ORDER BY listings.date_posted");
		$sth->execute();
		$sth->bind_columns(\$date,\$title,\$edition,\$isbn,\$bookid,\$username);
		
		while($sth->fetch())
		{
			#Get author associated with the book
			my $author;
			my $sth = $dbh->prepare("
							SELECT author.author_name
							FROM author
								INNER JOIN book_author ON author.author_ref_id=book_author.author_ref_id
							WHERE book_author.book_ref_id=? ORDER BY author.author_name");
			$sth->execute($bookid);
			$sth->bind_columns(\$author);
			#Add authors to list
			my @authors;
			while($sth->fetch())
			{
				push(@authors, $author);
			}
			print format_result($title,$isbn,\@authors,$username,$date,$edition);	
		}
	}	
	else
	{
		print "Invalid Author";	
	}



}
else
{
	print "<b>Invalid request</b>";
}
