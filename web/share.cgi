#!/usr/bin/perl

use warnings;
use strict;

use DBI;
use CGI::Simple;

# "Share a Book" form handler
# Chris Handwerker - chandwer@student.fitchburgstate.edu
#
# This form handler will take input from the "share" form on the
# main page and store the title, edition, author, and ISBN in the
# users cookies and redirect to a details page.
#
# The details page will check to see if the user is logged in and
# has the previous cookies set. If they are not logged in
# they will get redirected to a login page. If they don't have the
# previous form cookies set they will get redirected to the main page.
# 
# If they have the previous share a book cookie data set and they are logged
# in they will be asked for a few more details about the book and then it will
# be put into the database

