/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/


#include <unistd.h>

#include <feral/VM/VM.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// Classes //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// initialize this in the init_utils function
static int file_typeid;
static int file_iterable_typeid;

class var_file_t : public var_base_t
{
	FILE * m_file;
	bool m_owner;
public:
	var_file_t( FILE * const file, const size_t & src_id, const size_t & idx, const bool owner = true );
	~var_file_t();

	void * get_data();

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	FILE * const get();
};
#define FILE( x ) static_cast< var_file_t * >( x )

var_file_t::var_file_t( FILE * const file, const size_t & src_id, const size_t & idx, const bool owner )
	: var_base_t( file_typeid, src_id, idx ), m_file( file ), m_owner( owner )
{}
var_file_t::~var_file_t()
{
	if( m_owner ) fclose( m_file );
}

void * var_file_t::get_data() { return m_file; }

var_base_t * var_file_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_file_t( m_file, src_id, idx, false );
}

void var_file_t::set( var_base_t * from )
{
	if( m_owner ) fclose( m_file );
	m_owner = false;
	m_file = FILE( from )->get();
}

FILE * const var_file_t::get() { return m_file; }

class var_file_iterable_t : public var_base_t
{
	var_file_t * m_file;
public:
	var_file_iterable_t( var_file_t * file, const size_t & src_id, const size_t & idx );
	~var_file_iterable_t();

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	bool next( var_base_t * & val );
};
#define FILE_ITERABLE( x ) static_cast< var_file_iterable_t * >( x )

var_file_iterable_t::var_file_iterable_t( var_file_t * file, const size_t & src_id, const size_t & idx )
	: var_base_t( file_iterable_typeid, src_id, idx ), m_file( file )
{
	var_iref( m_file );
}
var_file_iterable_t::~var_file_iterable_t() { var_dref( m_file ); }

var_base_t * var_file_iterable_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_file_iterable_t( m_file, src_id, idx );
}
void var_file_iterable_t::set( var_base_t * from )
{
	var_dref( m_file );
	m_file = FILE_ITERABLE( from )->m_file;
	var_iref( m_file );
}

bool var_file_iterable_t::next( var_base_t * & val )
{
	char * line_ptr = NULL;
	size_t len = 0;
	ssize_t read = 0;

	if( ( read = getline( & line_ptr, & len, m_file->get() ) ) != -1 ) {
		std::string line = line_ptr;
		free( line_ptr );
		while( line.back() == '\n' ) line.pop_back();
		while( line.back() == '\r' ) line.pop_back();
		val = make< var_str_t >( line );
		return true;
	}
	if( line_ptr ) free( line_ptr );
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// Functions /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * fs_exists( vm_state_t & vm, const fn_data_t & fd )
{
	if( fd.args[ 1 ]->type() != VT_STR ) {
		vm.src_stack.back()->src()->fail( fd.idx, "expected string argument for path, found: %s",
						  vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	return access( STR( fd.args[ 1 ] )->get().c_str(), F_OK ) != -1 ? vm.tru : vm.fals;
}

var_base_t * fs_open( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	if( fd.args[ 1 ]->type() != VT_STR ) {
		src->fail( fd.idx, "expected string argument for file name, found: %s",
			   vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	if( fd.args[ 2 ]->type() != VT_STR ) {
		src->fail( fd.idx, "expected string argument for file open mode, found: %s",
			   vm.type_name( fd.args[ 2 ]->type() ).c_str() );
		return nullptr;
	}
	const std::string & file_name = STR( fd.args[ 1 ] )->get();
	const std::string & mode = STR( fd.args[ 2 ] )->get();
	FILE * file = fopen( file_name.c_str(), mode.c_str() );
	if( !file ) {
		src->fail( fd.idx, "failed to open file '%s' in mode: %s",
			   file_name.c_str(), mode.c_str() );
		return nullptr;
	}
	return make< var_file_t >( file );
}

var_base_t * fs_file_all_lines( vm_state_t & vm, const fn_data_t & fd )
{
	FILE * const file = FILE( fd.args[ 0 ] )->get();
	char * line_ptr = NULL;
	size_t len = 0;
	ssize_t read = 0;

	std::vector< var_base_t * > lines;
	while( ( read = getline( & line_ptr, & len, file ) ) != -1 ) {
		std::string line = line_ptr;
		while( line.back() == '\n' ) line.pop_back();
		while( line.back() == '\r' ) line.pop_back();
		lines.push_back( new var_str_t( line, fd.src_id, fd.idx ) );
	}
	if( line_ptr ) free( line_ptr );
	fseek( file, 0, SEEK_SET );

	return make< var_vec_t >( lines );
}

var_base_t * fs_file_seek( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	FILE * const file = FILE( fd.args[ 0 ] )->get();
	if( fd.args[ 1 ]->type() != VT_INT ) {
		src->fail( fd.idx, "expected int argument for file seek position, found: %s",
			   vm.type_name( fd.args[ 1 ]->type() ).c_str() );
		return nullptr;
	}
	if( fd.args[ 2 ]->type() != VT_INT ) {
		src->fail( fd.idx, "expected int argument for file seek origin, found: %s",
			   vm.type_name( fd.args[ 2 ]->type() ).c_str() );
		return nullptr;
	}
	long pos = INT( fd.args[ 1 ] )->get().get_si();
	int origin = INT( fd.args[ 2 ] )->get().get_si();
	return make< var_int_t >( fseek( file, pos, origin ) );
}

var_base_t * fs_file_readlines( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	return make< var_file_iterable_t >( FILE( fd.args[ 0 ] ) );
}

var_base_t * fs_file_iterable_next( vm_state_t & vm, const fn_data_t & fd )
{
	var_file_iterable_t * it = FILE_ITERABLE( fd.args[ 0 ] );
	var_base_t * res = nullptr;
	if( !it->next( res ) ) return vm.nil;
	return res;
}

INIT_MODULE( fs )
{
	var_src_t * src = vm.src_stack.back();
	const std::string & src_name = src->src()->path();

	// get the type id for file and file_iterable type (register_type)
	file_typeid = vm.register_new_type( "var_file_t", "file_t" );
	file_iterable_typeid = vm.register_new_type( "var_file_iterable_t", "file_iterable_t" );

	src->add_nativefn( "exists", fs_exists, { "" } );
	src->add_nativefn( "open_native", fs_open, { "", "" }, {} );

	vm.add_typefn( file_typeid, "alllines", new var_fn_t( src_name, {}, {}, { .native = fs_file_all_lines }, 0, 0 ), false );
	vm.add_typefn( file_typeid, "readlines", new var_fn_t( src_name, {}, {}, { .native = fs_file_readlines }, 0, 0 ), false );

	vm.add_typefn( file_typeid, "seek", new var_fn_t( src_name, { "", "" }, {}, { .native = fs_file_seek }, 0, 0 ), false );

	vm.add_typefn( file_iterable_typeid, "next", new var_fn_t( src_name, {}, {}, { .native = fs_file_iterable_next }, 0, 0 ), false );

	// constants
	src->add_nativevar( "SEEK_SET", make< var_int_t >( SEEK_SET ) );
	src->add_nativevar( "SEEK_CUR", make< var_int_t >( SEEK_CUR ) );
	src->add_nativevar( "SEEK_END", make< var_int_t >( SEEK_END ) );

	return true;
}
