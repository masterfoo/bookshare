// Handles and validaites the "share" form
// Written by Chris Handwerker - chandwer@student.fitchburgstate.edu

function shareBook()
{
	if(validateForm())
	{
		// set cookies
		setCookie('title',document.share.title.value);
		setCookie('edition',document.share.edition.value);
		setCookie('author',document.share.author.value);
		setCookie('isbn',document.share.isbn.value);

		// redirect to form handler
		window.location = "share.cgi";
	}
}
function setCookie(name,value)
{
	var expire = new Date();
	expire.setHours(expire.getHours()+1);
	var c_value = escape(value)+"; explire="+expire.toUTCString();
	document.cookie=name+"="+c_value;
}
function validateISBN()
{
	if(document.share.isbn.value != "")
	{
		var request = new XMLHttpRequest();
		request.onreadystatechange = function()
		{
			if(request.readyState==4)
			{
				if(request.status == 200)	// valid isbn
				{
					// parse XML
					xml = request.responseXML;
					// fix isbn formatting
					document.share.isbn.value = xml.getElementsByTagName("isbn")[0].childNodes[0].nodeValue;
					// get title
					var title = xml.getElementsByTagName("title");
					document.share.title.value = title.length > 0 ? title[0].childNodes[0].nodeValue : "";
					// get edition
					var edition = xml.getElementsByTagName("edition");
					document.share.edition.value = edition.length > 0 ? edition[0].childNodes[0].nodeValue : "";
					// get authors
					var author = xml.getElementsByTagName("author");
					if(author.length > 0)
					{
						document.share.author.value = "";
						for(var i=0;i<author.length;i++)
						{
							document.share.author.value += author[i].childNodes[0].nodeValue + "\n";
						}
					}
					else
					{
						document.share.author.value = "";
					}

					// reset label
					document.getElementById("isbn").style.color = 'black';
					document.getElementById("isbn").style.fontWeight = 'normal';
					document.getElementById("isbn").innerHTML = "ISBN";
				}
				else	// invalid isbn
				{
					// clear form
					document.share.isbn.value = "";
					// set label
					document.getElementById("isbn").style.color = 'red';
					document.getElementById("isbn").style.fontWeight = 'normal';
					document.getElementById("isbn").innerHTML = "ISBN - Invalid Format";
				}
			}
		}
		request.open("GET","isbncheck.cgi?isbn="+document.share.isbn.value,true);
		request.send();
	}
}
function validateForm()
{
	if(document.share.title.value == "")
	{
		document.getElementById("title").style.fontWeight = 'bold';
		document.getElementById("title").style.color = 'red';
	}
	else
	{
		document.getElementById("title").style.fontWeight = 'normal';
		document.getElementById("title").style.color = 'black';
	}
	
	if(document.share.author.value == "")
	{
		document.getElementById("author").style.fontWeight = 'bold';
		document.getElementById("author").style.color = 'red';
	}
	else
	{
		document.getElementById("author").style.fontWeight = 'normal';
		document.getElementById("author").style.color = 'black';
	}

	if(document.share.isbn.value == "")
	{
		document.getElementById("isbn").style.fontWeight = 'bold';
		document.getElementById("isbn").style.color = 'red';
	}
	else
	{
		document.getElementById("isbn").style.fontWeight = 'normal';
		document.getElementById("isbn").style.color = 'black';
	}

	if
	(
		document.share.title.value == "" 	||	
		document.share.author.value == "" 	||
		document.share.isbn.value == ""
	)
	{
		return false;
	}
	else
	{
		return true;
	}
}
