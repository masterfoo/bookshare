function lock_selected_user(){
	document.forms.users.code.value="-1";
	document.forms.users.op.value="sudo";
	document.forms.users.submit();
}
function set_perm_level(){
	document.forms.users.code.value=document.forms.users.code_s.value;
	document.forms.users.op.value="sudo";
	document.forms.users.submit();
}
