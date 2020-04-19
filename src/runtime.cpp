/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include <feral/VM/VM.hpp>

var_base_t * var_exists( vm_state_t & vm, const fn_data_t & fd )
{
	if( fd.args[ 1 ]->type() != VT_STR ) {
		vm.fail( fd.idx, "expected string argument for variable name, found: %s",
			 vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	return vm.current_source()->vars()->get( STR( fd.args[ 1 ] )->get() ) != nullptr ? vm.tru : vm.fals;
}

INIT_MODULE( runtime )
{
	var_src_t * src = vm.current_source();

	src->add_nativefn( "var_exists", var_exists, 1 );

	return true;
}