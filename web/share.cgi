#!/usr/bin/perl

use warnings;
use strict;

use DBI;
use CGI::Simple;

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
	return 0;
}
my $q = new CGI::Simple;

my $title = $q->cookie('title');
my $edition = $q->cookie('edition');
my $author = $q->cookie('author');
my $isbn = $q->cookie('isbn');

if($title && $edition && $author && $isbn)	# user has cookies
{
	# Validate user session
	my $sid = $q->cookie('sid');

	if(session($sid))	# user has a session
	{
		print $q->header();
		# delete user's cookies
	
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
