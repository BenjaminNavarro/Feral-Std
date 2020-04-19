/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include <feral/VM/VM.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// Classes //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// initialize this in the init_ptr function
static int ptr_typeid;

class var_ptr_t : public var_base_t
{
	var_base_t * m_val;
public:
	var_ptr_t( var_base_t * val, const size_t & src_id, const size_t & idx );
	~var_ptr_t();

	void * get_data( const size_t & idx );

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	void update( var_base_t * with );

	var_base_t * get();
};
#define PTR( x ) static_cast< var_ptr_t * >( x )

var_ptr_t::var_ptr_t( var_base_t * val, const size_t & src_id, const size_t & idx )
	: var_base_t( ptr_typeid, src_id, idx ), m_val( val )
{ var_iref( m_val ); }
var_ptr_t::~var_ptr_t() { var_dref( m_val ); }

void * var_ptr_t::get_data( const size_t & idx ) { return m_val; }

var_base_t * var_ptr_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_ptr_t( m_val, src_id, idx );
}
void var_ptr_t::set( var_base_t * from )
{
	var_dref( m_val );
	m_val = PTR( from )->m_val;
	var_iref( m_val );
}

void var_ptr_t::update( var_base_t * with )
{
	var_dref( m_val );
	m_val = with;
	var_iref( m_val );
}

var_base_t * var_ptr_t::get() { return m_val; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// Functions /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * ptr_new_native( vm_state_t & vm, const fn_data_t & fd )
{
	return make< var_ptr_t >( fd.args[ 1 ] );
}

var_base_t * ptr_set( vm_state_t & vm, const fn_data_t & fd )
{
	var_ptr_t * self = PTR( fd.args[ 0 ] );
	self->update( fd.args[ 1 ] );
	return fd.args[ 0 ];
}

var_base_t * ptr_get( vm_state_t & vm, const fn_data_t & fd )
{
	return PTR( fd.args[ 0 ] )->get();
}

INIT_MODULE( ptr )
{
	var_src_t * src = vm.current_source();
	src->add_nativefn( "new_native", ptr_new_native, 1 );

	// get the type id for file_iterable type (register_type)
	ptr_typeid = vm.register_new_type( "ptr_t", src_id, idx );

	vm.add_typefn_native( ptr_typeid, "set", ptr_set, 1, src_id, idx );
	vm.add_typefn_native( ptr_typeid, "get", ptr_get, 0, src_id, idx );
	return true;
}