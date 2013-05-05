#!/usr/bin/perl

use warnings;
use strict;

use DBI;
use CGI::Simple;
use CGI::Carp 'fatalsToBrowser';	# send errors to browser for debugging

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
	my $username = "chandwer";		# just pretending
	return $username;
}
my $q = new CGI::Simple;

my $title = $q->cookie('title');
my $edition = $q->cookie('edition');
my $isbn = $q->cookie('isbn');
my @authors = split('\n',$q->cookie('author')) unless !$q->cookie('author');

if($title && $edition && $isbn && @authors)	# user has cookies
{
	# Validate user session
	my $username = session($q->cookie('sid'));
	if($username)	# user has a session
	{
		# make a db entry for das book
		my $dbh = DBI->connect("dbi:SQLite:/home/ghandi/db/bookshare.db") or die $DBI::errstr;
		my $sth;

		# insert authors into 'author' if it isn't already in the db
		foreach my $author(@authors)
		{
			# some SQL magic
			$sth = $dbh->prepare("
						INSERT INTO author(author_name) 
						SELECT * FROM (SELECT ?) AS tmp 
						WHERE NOT EXISTS
						(
							SELECT author_name FROM author WHERE author_name = ?
						) 
						LIMIT 1");
			$sth->execute($author,$author) or die $DBI::errstr;
		}

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
