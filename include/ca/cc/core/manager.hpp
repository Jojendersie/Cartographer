#pragma once

#include <ca/pa/containers/hashmap.hpp>
#include <ca/pa/log.hpp>
#include <string>

namespace ca { namespace cc {
		
	/// Mixin which creates a singleton resource manager for a specific type.
	/// \details To use this manager declare a new type in some header e.g.
	///		typedef Manager<Texture> TexMan;.
	///		Then you can access resources TexMan::get("bla.png"). You don't need to unload
	///		resources, but if you want to, there is a clear.
	///		TODO: if a resource is changed during runtime it is reloaded automatically.
	/// \tparam TLoader a type which must have a static load(string...), a static
	///		unload(handle) method and an inner type definition for the handle type:
	///		TLoader::Handle.
	template<typename TLoader>
	class Manager
	{
	public:
		/// Find a resource and load only if necessary.
		/// \param [inout] _args Additional arguments which might be required by
		///		the resource's load() funtion
		template<typename... Args>
		static typename TLoader::Handle get(const char* _name, const Args&... _args);
		
		/// Call to unload all resources. Should always be done on shut-down!
		static void clear();
		
		~Manager();
	private:
		/// Private construction
		Manager();
		
		/// Singleton access
		static Manager& inst();
		
		/// Compute a hash for a string
		struct FastStringHash
		{
			uint32 operator () (const std::string& _string);
		};

		pa::HashMap<std::string, typename TLoader::Handle, FastStringHash> m_resourceMap;
	};



	// ********************************************************************************************* //
	// IMPLEMENTATION																				 //
	// ********************************************************************************************* //
	template<typename TLoader>
	Manager<TLoader>::Manager()
	{
	}

	template<typename TLoader>
	Manager<TLoader>::~Manager()
	{
		m_resourceMap.clear();
	}

	template<typename TLoader>
	Manager<TLoader>& Manager<TLoader>::inst()
	{
		static Manager theOnlyInstance;
		return theOnlyInstance;
	}

	template<typename TLoader>
	template<typename... Args>
	typename TLoader::Handle Manager<TLoader>::get(const char* _name, const Args&... _args)
	{
		std::string name(_name);
		// Search in hash map
		auto handle = inst().m_resourceMap.find(name);
		if(handle) {
			pa::logPedantic("Reusing resource '", _name, "'.");
			return handle.data();
		}

		// Add/Load new element
		handle = inst().m_resourceMap.add(move(name), TLoader::load(_name, _args...));
		return handle.data();
	}

	template<typename TLoader>
	void Manager<TLoader>::clear()
	{
		for(auto it : inst().m_resourceMap)
			TLoader::unload(it.data());
		inst().m_resourceMap.clear();
	}

	template<typename TLoader>
	uint32 Manager<TLoader>::FastStringHash::operator () (const std::string& _string)
	{
		uint32 hashvalue = 208357;

		const char* string = _string.c_str();
		while(int c = *string++)
			hashvalue = ((hashvalue << 5) + (hashvalue << 1) + hashvalue) ^ c; 

		return hashvalue;
	}

}} // namespace ca::cc
