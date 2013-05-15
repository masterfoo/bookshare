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
sub auth
{
	`mod_find accounts:auth` =~ /^(OK)\s(.+)/;
	return $1?$2:0;
}
1;
