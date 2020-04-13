/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include <feral/VM/VM.hpp>

std::vector< var_base_t * > _str_split( const std::string & data, const char delim,
					const size_t & src_id, const size_t & idx );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// Functions /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * str_size( vm_state_t & vm, const fn_data_t & fd )
{
	return make< var_int_t >( STR( fd.args[ 0 ] )->get().size() );
}

var_base_t * str_empty( vm_state_t & vm, const fn_data_t & fd )
{
	return STR( fd.args[ 0 ] )->get().size() == 0 ? vm.tru : vm.fals;
}

var_base_t * str_front( vm_state_t & vm, const fn_data_t & fd )
{
	std::string & str = STR( fd.args[ 0 ] )->get();
	return str.size() == 0 ? vm.nil : make< var_str_t >( std::string( 1, str.front() ) );
}

var_base_t * str_back( vm_state_t & vm, const fn_data_t & fd )
{
	std::string & str = STR( fd.args[ 0 ] )->get();
	return str.size() == 0 ? vm.nil : make< var_str_t >( std::string( 1, str.back() ) );
}

var_base_t * str_push( vm_state_t & vm, const fn_data_t & fd )
{
	if( fd.args[ 1 ]->type() != VT_STR ) {
		vm.src_stack.back()->src()->fail( fd.idx, "expected string argument for string.push(), found: %s",
						  vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	std::string & src = STR( fd.args[ 1 ] )->get();
	std::string & dest = STR( fd.args[ 0 ] )->get();
	if( src.size() > 0 ) dest += src;
	return fd.args[ 0 ];
}

var_base_t * str_pop( vm_state_t & vm, const fn_data_t & fd )
{
	std::string & str = STR( fd.args[ 0 ] )->get();
	if( str.size() > 0 ) str.pop_back();
	return fd.args[ 0 ];
}

var_base_t * str_setat( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src_file = vm.src_stack.back()->src();
	if( fd.args[ 1 ]->type() != VT_INT ) {
		src_file->fail( fd.idx, "expected first argument to be of type integer for string.set(), found: %s",
				vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	if( fd.args[ 2 ]->type() != VT_STR ) {
		src_file->fail( fd.idx, "expected second argument to be of type string for string.set(), found: %s",
				vm.type_name( fd.args[ 2 ]->type() ).c_str() );
		return nullptr;
	}
	size_t pos = INT( fd.args[ 1 ] )->get().get_ui();
	std::string & dest = STR( fd.args[ 0 ] )->get();
	if( pos >= dest.size() ) {
		src_file->fail( fd.idx, "position %zu is not within string of length %zu",
				pos, dest.size() );
		return nullptr;
	}
	std::string & src = STR( fd.args[ 2 ] )->get();
	if( src.size() == 0 ) return fd.args[ 0 ];
	dest[ pos ] = src[ 0 ];
	return fd.args[ 0 ];
}

var_base_t * str_insert( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src_file = vm.src_stack.back()->src();
	if( fd.args[ 1 ]->type() != VT_INT ) {
		src_file->fail( fd.idx, "expected first argument to be of type integer for string.insert(), found: %s",
				vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	if( fd.args[ 2 ]->type() != VT_STR ) {
		src_file->fail( fd.idx, "expected second argument to be of type string for string.insert(), found: %s",
				vm.type_name( fd.args[ 2 ]->type() ).c_str() );
		return nullptr;
	}
	size_t pos = INT( fd.args[ 1 ] )->get().get_ui();
	std::string & dest = STR( fd.args[ 0 ] )->get();
	if( pos > dest.size() ) {
		src_file->fail( fd.idx, "position %zu is greater than string length %zu",
				pos, dest.size() );
		return nullptr;
	}
	std::string & src = STR( fd.args[ 2 ] )->get();
	dest.insert( dest.begin() + pos, src.begin(), src.end() );
	return fd.args[ 0 ];
}

var_base_t * str_erase( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src_file = vm.src_stack.back()->src();
	if( fd.args[ 1 ]->type() != VT_INT ) {
		src_file->fail( fd.idx, "expected argument to be of type integer for string.erase(), found: %s",
				vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	size_t pos = INT( fd.args[ 1 ] )->get().get_ui();
	std::string & str = STR( fd.args[ 0 ] )->get();
	if( pos < str.size() ) str.erase( str.begin() + pos );
	return fd.args[ 0 ];
}

var_base_t * str_last( vm_state_t & vm, const fn_data_t & fd )
{
	return make< var_int_t >( STR( fd.args[ 0 ] )->get().size() - 1 );
}

var_base_t * str_trim( vm_state_t & vm, const fn_data_t & fd )
{
	std::string & str = STR( fd.args[ 0 ] )->get();
	while( str.size() > 0 && isspace( str.back() ) ) { str.pop_back(); }
	while( str.size() > 0 && isspace( str.front() ) ) { str.erase( str.begin() ); }
	return fd.args[ 0 ];
}

var_base_t * str_split( vm_state_t & vm, const fn_data_t & fd )
{
	var_str_t * str = STR( fd.args[ 0 ] );
	if( fd.args[ 1 ]->type() != VT_STR ) {
		vm.src_stack.back()->src()->fail( fd.idx, "expected string argument for delimiter, found: %s",
						  vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	if( STR( fd.args[ 1 ] )->get().size() == 0 ) {
		vm.src_stack.back()->src()->fail( fd.idx, "found empty delimiter for string split" );
		return nullptr;
	}
	char delim = STR( fd.args[ 1 ] )->get()[ 0 ];
	std::vector< var_base_t * > res_vec = _str_split( str->get(), delim, fd.src_id, fd.src_id );
	return make< var_vec_t >( res_vec );
}

// character (str[0]) to its ASCII (int)
var_base_t * c_to_i( vm_state_t & vm, const fn_data_t & fd )
{
	const std::string & str = STR( fd.args[ 0 ] )->get();
	if( str.empty() ) return make< var_int_t >( 0 );
	return make< var_int_t >( str[ 0 ] );
}

// ASCII (int) to character (str)
var_base_t * i_to_c( vm_state_t & vm, const fn_data_t & fd )
{
	const mpz_class & num = INT( fd.args[ 0 ] )->get();
	return make< var_str_t >( std::string( 1, ( char )num.get_si() ) );
}

INIT_MODULE( str )
{
	var_src_t * src = vm.src_stack.back();

	vm.add_typefn_native( VT_STR,     "len", str_size,   0, src_id, idx );
	vm.add_typefn_native( VT_STR,   "empty", str_empty,  0, src_id, idx );
	vm.add_typefn_native( VT_STR,   "front", str_front,  0, src_id, idx );
	vm.add_typefn_native( VT_STR,    "back", str_back,   0, src_id, idx );
	vm.add_typefn_native( VT_STR,    "push", str_push,   1, src_id, idx );
	vm.add_typefn_native( VT_STR,     "pop", str_pop,    0, src_id, idx );
	vm.add_typefn_native( VT_STR,  "insert", str_insert, 2, src_id, idx );
	vm.add_typefn_native( VT_STR,   "erase", str_erase,  1, src_id, idx );
	vm.add_typefn_native( VT_STR, "lastidx", str_last,   0, src_id, idx );
	vm.add_typefn_native( VT_STR,     "set", str_setat,  2, src_id, idx );

	vm.add_typefn_native( VT_STR,  "trim", str_trim, 0, src_id, idx );
	vm.add_typefn_native( VT_STR, "split_native", str_split, 1, src_id, idx );

	vm.add_typefn_native( VT_STR, "c_to_i", c_to_i, 0, src_id, idx );
	vm.add_typefn_native( VT_INT, "i_to_c", i_to_c, 0, src_id, idx );

	return true;
}


std::vector< var_base_t * > _str_split( const std::string & data, const char delim,
					const size_t & src_id, const size_t & idx )
{
	std::string temp;
	std::vector< var_base_t * > vec;

	for( auto c : data ) {
		if( c == delim ) {
			if( temp.empty() ) continue;
			vec.push_back( new var_str_t( temp, src_id, idx ) );
			temp.clear();
			continue;
		}

		temp += c;
	}

	if( !temp.empty() ) vec.push_back( new var_str_t( temp, src_id, idx ) );
	return vec;
}