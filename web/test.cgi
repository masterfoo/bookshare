#!/usr/bin/perl

use warnings;
use strict;

print <<EOF;
Content-Type: text/html

Auth?
EOF

my $foo = `mod_find accounts:auth`;
print $foo;
