/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include <feral/VM/VM.hpp>

const size_t MAX_C_STR_LEN = 1025;
extern std::unordered_map< std::string, const char * > COL;
int apply_colors( std::string & str );

var_base_t * print( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	for( size_t i = 1; i < fd.args.size(); ++i ) {
		std::string str;
		if( !fd.args[ i ]->to_str( vm, str, fd.src_id, fd.idx ) ) {
			return nullptr;
		}
		fprintf( stdout, "%s", str.c_str() );
	}
	return vm.nil;
}

var_base_t * println( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	for( size_t i = 1; i < fd.args.size(); ++i ) {
		std::string str;
		if( !fd.args[ i ]->to_str( vm, str, fd.src_id, fd.idx ) ) {
			return nullptr;
		}
		fprintf( stdout, "%s", str.c_str() );
	}
	fprintf( stdout, "\n" );
	return vm.nil;
}

var_base_t * scan( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	if( fd.args.size() > 1 ) {
		if( fd.args[ 1 ]->type() != VT_STR ) {
			src->fail( fd.args[ 1 ]->idx(), "expected string data for input prompt" );
			return nullptr;
		}
		fprintf( stdout, "%s", STR( fd.args[ 1 ] )->get().c_str() );
	}

	char str[ MAX_C_STR_LEN ];
	fgets( str, MAX_C_STR_LEN, stdin );
	std::string res( str );
	while( res.back() == '\n' ) res.pop_back();
	while( res.back() == '\r' ) res.pop_back();

	return make< var_str_t >( res );
}

var_base_t * col_println( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	for( size_t i = 1; i < fd.args.size(); ++i ) {
		std::string str;
		if( !fd.args[ i ]->to_str( vm, str, fd.src_id, fd.idx ) ) {
			return nullptr;
		}
		apply_colors( str );
		fprintf( stdout, "%s", str.c_str() );
	}
	fprintf( stdout, "\n" );
	return vm.nil;
}

var_base_t * flushout( vm_state_t & vm, const fn_data_t & fd )
{
	fflush( stdout );
	return vm.nil;
}

INIT_MODULE( io )
{
	var_src_t * src = vm.src_stack.back();
	const std::string & src_name = src->src()->path();
	src->add_nativefn( "print", print, {}, {}, true );
	src->add_nativefn( "println", println, {}, {}, true );
	src->add_nativefn( "scan", scan, { "" }, {}, true );
	src->add_nativefn( "cprintln", col_println, {}, {}, true );
	src->add_nativefn( "flushout", flushout );
	return true;
}

std::unordered_map< std::string, const char * > COL = {
	{ "0", "\033[0m" },

	{ "r", "\033[0;31m" },
	{ "g", "\033[0;32m" },
	{ "y", "\033[0;33m" },
	{ "b", "\033[0;34m" },
	{ "m", "\033[0;35m" },
	{ "c", "\033[0;36m" },
	{ "w", "\033[0;37m" },

	{ "br", "\033[1;31m" },
	{ "bg", "\033[1;32m" },
	{ "by", "\033[1;33m" },
	{ "bb", "\033[1;34m" },
	{ "bm", "\033[1;35m" },
	{ "bc", "\033[1;36m" },
	{ "bw", "\033[1;37m" },
};

int apply_colors( std::string & str )
{
	int chars = 0;
	for( size_t i = 0; i < str.size(); ) {
		if( str[ i ] == '{' && ( i == 0 || ( str[ i - 1 ] != '$' && str[ i - 1 ] != '%' && str[ i - 1 ] != '#' && str[ i - 1 ] != '\\' ) ) ) {
			str.erase( str.begin() + i );
			if( i < str.size() && str[ i ] == '{' ) {
				++i;
				++chars;
				continue;
			}

			std::string var;

			while( i < str.size() && str[ i ] != '}' ) {
				var += str[ i ];
				str.erase( str.begin() + i );
			}

			// Remove the ending brace
			if( i < str.size() ) str.erase( str.begin() + i );

			if( var.empty() ) continue;

			if( COL.find( var ) != COL.end() ) {
				std::string val = COL[ var ];
				str.insert( str.begin() + i, val.begin(), val.end() );
				i += val.size();
			}
		}
		else {
			++i;
			++chars;
		}
	}
	return chars;
}