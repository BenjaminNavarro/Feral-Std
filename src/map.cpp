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

// initialize this in the init_utils function
static int map_iterable_typeid;
static int map_iterable_element_struct_id;

class var_map_iterable_t : public var_base_t
{
	var_map_t * m_map;
	std::unordered_map< std::string, var_base_t * >::iterator m_curr;
public:
	var_map_iterable_t( var_map_t * map, const size_t & src_id, const size_t & idx );
	~var_map_iterable_t();

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	bool next( var_base_t * & val, const size_t & src_id, const size_t & idx );
};
#define MAP_ITERABLE( x ) static_cast< var_map_iterable_t * >( x )

var_map_iterable_t::var_map_iterable_t( var_map_t * map, const size_t & src_id, const size_t & idx )
	: var_base_t( map_iterable_typeid, src_id, idx ), m_map( map ), m_curr( map->get().begin() )
{
	var_iref( m_map );
}
var_map_iterable_t::~var_map_iterable_t() { var_dref( m_map ); }

var_base_t * var_map_iterable_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_map_iterable_t( m_map, src_id, idx );
}
void var_map_iterable_t::set( var_base_t * from )
{
	var_dref( m_map );
	m_map = MAP_ITERABLE( from )->m_map;
	var_iref( m_map );
	m_curr = MAP_ITERABLE( from )->m_curr;
}

bool var_map_iterable_t::next( var_base_t * & val, const size_t & src_id, const size_t & idx )
{
	if( m_curr == m_map->get().end() ) return false;
	std::unordered_map< std::string, var_base_t * > attrs;
	var_iref( m_curr->second );
	attrs[ "0" ] = new var_str_t( m_curr->first, src_id, idx );
	attrs[ "1" ] = m_curr->second;
	val = make< var_struct_t >( map_iterable_element_struct_id, attrs );
	++m_curr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// Functions /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t * map_new( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	if( ( fd.args.size() - 1 ) % 2 != 0 ) {
		src->fail( fd.idx, "argument count must be even to create a map" );
		return nullptr;
	}
	std::unordered_map< std::string, var_base_t * > map_val;
	for( size_t i = 1; i < fd.args.size(); ++i ) {
		std::string key;
		if( !fd.args[ i ]->to_str( vm, key, fd.src_id, fd.idx ) ) {
			return nullptr;
		}
		if( map_val.find( key ) != map_val.end() ) var_dref( map_val[ key ] );
		map_val[ key ] = fd.args[ ++i ]->copy( fd.src_id, fd.idx );
	}
	return make< var_map_t >( map_val );
}

var_base_t * map_size( vm_state_t & vm, const fn_data_t & fd )
{
	return make< var_int_t >( MAP( fd.args[ 0 ] )->get().size() );
}

var_base_t * map_insert( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	std::unordered_map< std::string, var_base_t * > & map = MAP( fd.args[ 0 ] )->get();
	std::string key;
	if( !fd.args[ 1 ]->to_str( vm, key, fd.src_id, fd.idx ) ) {
		return nullptr;
	}
	if( map.find( key ) != map.end() ) {
		var_dref( map[ key ] );
	}
	var_iref( fd.args[ 2 ] );
	map[ key ] = fd.args[ 2 ];
	return fd.args[ 0 ];
}

var_base_t * map_erase( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	std::unordered_map< std::string, var_base_t * > & map = MAP( fd.args[ 0 ] )->get();
	std::string key;
	if( !fd.args[ 1 ]->to_str( vm, key, fd.src_id, fd.idx ) ) {
		return nullptr;
	}
	if( map.find( key ) != map.end() ) {
		var_dref( map[ key ] );
	}
	return fd.args[ 0 ];
}

var_base_t * map_get( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	std::unordered_map< std::string, var_base_t * > & map = MAP( fd.args[ 0 ] )->get();
	std::string key;
	if( !fd.args[ 1 ]->to_str( vm, key, fd.src_id, fd.idx ) ) {
		return nullptr;
	}
	if( map.find( key ) == map.end() ) {
		return vm.nil;
	}
	return map[ key ];
}

var_base_t * map_find( vm_state_t & vm, const fn_data_t & fd )
{
	srcfile_t * src = vm.src_stack.back()->src();
	std::unordered_map< std::string, var_base_t * > & map = MAP( fd.args[ 0 ] )->get();
	std::string key;
	if( !fd.args[ 1 ]->to_str( vm, key, fd.src_id, fd.idx ) ) {
		return nullptr;
	}
	return map.find( key ) != map.end() ? vm.tru : vm.fals;
}

var_base_t * map_each( vm_state_t & vm, const fn_data_t & fd )
{
	return make< var_map_iterable_t >( MAP( fd.args[ 0 ] ) );
}

var_base_t * map_iterable_next( vm_state_t & vm, const fn_data_t & fd )
{
	var_map_iterable_t * it = MAP_ITERABLE( fd.args[ 0 ] );
	var_base_t * res = nullptr;
	if( !it->next( res, fd.src_id, fd.idx ) ) return vm.nil;
	return res;
}

INIT_MODULE( map )
{
	var_src_t * src = vm.src_stack.back();

	src->add_nativefn( "new", map_new, 0, true );

	vm.add_typefn_native( VT_MAP, "len", map_size, 0, src_id, idx );
	vm.add_typefn_native( VT_MAP, "insert", map_insert, 2, src_id, idx );
	vm.add_typefn_native( VT_MAP,  "erase", map_erase,  1, src_id, idx );
	vm.add_typefn_native( VT_MAP,    "get", map_get,    1, src_id, idx );
	vm.add_typefn_native( VT_MAP,     "[]", map_get,    1, src_id, idx );
	vm.add_typefn_native( VT_MAP,   "find", map_find,   1, src_id, idx );
	vm.add_typefn_native( VT_MAP,   "each", map_each,   0, src_id, idx );

	// get the type id for map iterable and map iterator element (register_type)
	map_iterable_typeid = vm.register_new_type( "map_iterable_t", src_id, idx );
	map_iterable_element_struct_id = vm.register_struct_enum_id();
	vm.set_typename( map_iterable_element_struct_id, "map_iterable_element_t" );

	vm.add_typefn_native( map_iterable_typeid, "next", map_iterable_next, 0, src_id, idx );

	return true;
}