#!/usr/bin/perl

use warnings;
use strict;

use CGI::Simple;
use CGI::Carp 'fatalsToBrowser';	# send errors to browser for debugging
use Business::ISBN;
use DBI;

# "Share a Book" form handler
# Chris Handwerker - chandwer@student.fitchburgstate.edu
#
# This script looks for cookies, set by the "Share a book" form
# on the index page via javascript, containing Title,Edition.Author,
# and ISBN.
#
# If the user is logged in it stores the book entry, destroys the cookies,
# and redirects user to their management page
#
# If user it not logged in they get redirected to a login/register page
sub session
{
	# some function to validate user's session
	# returns username if session id is valid
	my $username = "testuser";		# just pretending
	return $username;
}
sub get_isbn
{
	# returns properly formatted ISBN-13 if valid
	# returns false if invalid
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

my $q = new CGI::Simple;

# cookies
my $title = $q->cookie('title');
my $edition = $q->cookie('edition');
my $isbn = $q->cookie('isbn');
my @authors = split('\n',$q->cookie('author')) unless !$q->cookie('author');

$isbn = get_isbn($isbn);
 
if($title && $isbn && @authors)	# user has cookies
{
	# Validate user session
	my $username = session($q->cookie('sid'));
	if($username)	# user has a session
	{
		# make a db entry for das book
		my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
		my $sth;

		my @author_ref_ids;
		my $book_ref_id;
		my $user_id;

		# insert authors into 'author' if it isn't already in the db
		foreach my $author(@authors)
		{
			#insert author unless a duplicate
			$sth = $dbh->prepare("
						INSERT INTO author(author_name) 
						SELECT * FROM (SELECT ?) AS tmp 
						WHERE NOT EXISTS
						(
							SELECT author_name FROM author WHERE author_name = ?
						) 
						LIMIT 1");
			$sth->execute($author,$author) or die $DBI::errstr;

			# get author_ref_id(s)
			$sth = $dbh->prepare("
						SELECT author_ref_id FROM author
						WHERE author_name=?");
			$sth->execute($author);
			push(@author_ref_ids,$sth->fetchrow_array);
		}
		
		# insert book into 'books' if it isn't already in the db	
		$sth = $dbh->prepare("
						INSERT INTO books(title,edition,isbn)
						SELECT * FROM (SELECT ?,?,?) AS tmp
						WHERE NOT EXISTS
						(
							SELECT title,edition,isbn FROM books
							WHERE title=? AND (edition=? OR edition IS NULL) AND isbn=?
						)
						LIMIT 1");
		$sth->execute($title,$edition,$isbn,$title,$edition,$isbn) or die $DBI::errstr;

		# get book ref id	
		$sth = $dbh->prepare("
						SELECT book_ref_id FROM books
						WHERE title=? AND (edition=? OR edition IS NULL) AND isbn=?");

		$sth->execute($title,$edition,$isbn) or die $DBI::errstr;
		
		$book_ref_id = $sth->fetchrow_array;

		# generate book_author listings
		foreach my $author_id(@author_ref_ids)
		{
			$sth = $dbh->prepare("
						INSERT INTO book_author(book_ref_id,author_ref_id)
						SELECT * FROM (SELECT ?,?) AS tmp
						WHERE NOT EXISTS
						(
							SELECT book_ref_id,author_ref_id FROM book_author
							WHERE book_ref_id=? AND author_ref_id=?
						)
						LIMIT 1");
			$sth->execute($book_ref_id,$author_id,$book_ref_id,$author_id) or die $DBI::errstr;
		}

		# get user id
		$sth = $dbh->prepare("SELECT user_id FROM users WHERE username=?");
		$sth->execute($username) or die $DBI::errstr;
		$user_id = $sth->fetchrow_array;

		# make a book listing
		$sth = $dbh->prepare("INSERT INTO listings (user_id,book_ref_id) VALUES (?,?)");
		$sth->execute($user_id,$book_ref_id) or die $DBI::errstr;

		# delete book entry cookies & redirect to user management page
		my $title_c = $q->cookie(-name => 'title', -value => "deleted", -expires => "Thursday, 01-Jan-1970 00:00:01 GMT");
		my $edition_c = $q->cookie(-name => 'edition', -value => "deleted", -expires => "Thursday, 01-Jan-1970 00:00:01 GMT");
		my $author_c = $q->cookie(-name => 'author', -value => "deleted", -expires => "Thursday, 01-Jan-1970 00:00:01 GMT");
		my $isbn_c = $q->cookie(-name => 'isbn', -value => "deleted", -expires => "Thursday, 01-Jan-1970 00:00:01 GMT");
		print $q->redirect(-uri=>'user.html',-cookie=>[$title_c,$edition_c,$author_c,$isbn_c]);

		# TODO add some input validation like so
		# if() # data valid
   		# {	
		# 	# create db entry
	
		# 	#redirect to user management page
		# }
		# else
		# {
		# 	# invalid request
		# }
	}
	else	# user doesn't have a session
	{
		# redirect to login page
		print $q->redirect('register.html');
	}
}
else
{
	# invalid request
	print $q->redirect('index.html');
}
