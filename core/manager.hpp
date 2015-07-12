#pragma once

namespace MiR {
		
	/// Mixin which creates a singleton resource manager for a specific type.
	/// \details To use this manager declare a new type in some header e.g.
	///		typedef Manager<Texture> TexMan; and INSTANCE_OF(TexMan) in a single cpp file.
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
		static typename TLoader::Handle get(const char* _name, Args... _args);
		
		/// Call to unload all resources. Should always be done on shut-down!
		static void clear();
		
		~Manager();
	private:
		/// Private construction
		Manager();
		
		/// Singleton access
		static Manager& inst();
		
		struct Entry
		{
			char* name;
			uint32 hash;
			typename TLoader::Handle resource;
			
			Entry() : name(nullptr)	{}
			
			~Entry()
			{
				free(name);
			}
			
			Entry(const Entry&) = delete;
			Entry& operator = (const Entry&) = delete;
			Entry(Entry&& _other) :
				name(_other.name),
				hash(_other.hash),
				resource(_other.resource)
			{
				_other.name = nullptr;
			}
			
			Entry& operator = (Entry&& _other)
			{
				if(name)
				{
					free(name);
					TLoader::unload(resource);
				}
				name = _other.name;
				hash = _other.hash;
				resource = _other.resource;
				_other.name = nullptr;
				return *this;
			}
		};
		
		struct Bucket
		{
			Entry elem[4];
			int num;
			
			Bucket() : num(0) {}
		};

		/// Resize the hashmap
		/// Reinserts all elements with new positions.
		void resize(uint32 _size);
		
		/// Compute a hash for an string
		uint32 hash(const char* _string);
		
		Bucket* m_hashMap;
		uint32 m_size;
	};



	// ********************************************************************************************* //
	// IMPLEMENTATION																				 //
	// ********************************************************************************************* //
	template<typename TLoader>
	Manager<TLoader>::Manager() :
		m_hashMap(new Bucket[7]),
		m_size(7)
	{
	}

	template<typename TLoader>
	Manager<TLoader>::~Manager()
	{
		clear();
		delete[] m_hashMap;
	}

	template<typename TLoader>
	Manager<TLoader>& Manager<TLoader>::inst()
	{
		static Manager theOnlyInstance;
		return theOnlyInstance;
	}

	template<typename TLoader>
	template<typename... Args>
	typename TLoader::Handle Manager<TLoader>::get(const char* _name, Args... _args)
	{
		// Search in hash map
		uint32 h = inst().hash(_name);
		Bucket* bucket = &inst().m_hashMap[h % inst().m_size];
		// Check if it is in the bucket
		for(int i=0; i<bucket->num; ++i)
		{
			if(strcmp(bucket->elem[i].name, _name) == 0)
				return bucket->elem[i].resource;
		}
		// Not in bucket (otherwise the return had left the loop)
		if(bucket->num == 4) { // Bad it is full -> resize
			inst().resize(inst().m_size * 2 - 1);
			bucket = &inst().m_hashMap[h % inst().m_size];
			if(bucket->num == 4) throw "Too many hash collisions, resize had no effect!";
		}
		size_t l = strlen(_name);
		bucket->elem[bucket->num].name = (char*)malloc(l+1);
		memcpy(bucket->elem[bucket->num].name, _name, l+1);
		bucket->elem[bucket->num].resource = TLoader::load(_name, _args...);
		bucket->num++;
		return bucket->elem[bucket->num-1].resource;
	}

	template<typename TLoader>
	void Manager<TLoader>::clear()
	{
		// Keep array but clear all buckets
		for(uint32 b=0; b<inst().m_size; ++b)
		{
			Bucket& bucket = inst().m_hashMap[b];
			for(int i=0; i<bucket.num; ++i)
			{
				TLoader::unload(bucket.elem[i].resource);
				free(bucket.elem[i].name);
				bucket.elem[i].name = nullptr;
			}
		}
	}

	template<typename TLoader>
	void Manager<TLoader>::resize(uint32 _size)
	{
		Bucket* newMap = new Bucket[_size];
		
		// Reinsert all items
		for(uint32 b=0; b<m_size; ++b)
		{
			for(int i=0; i<m_hashMap[b].num; ++i)
			{
				Bucket& target = newMap[m_hashMap[b].elem[i].hash % _size];
				if(target.num == 4) throw "Error while resizing hashmap: too many collisions inside bucket!";
				target.elem[target.num++] = std::move(m_hashMap[b].elem[i]);
			}
		}
		
		// Swap arrays
		delete[] m_hashMap;
		m_hashMap = newMap;
		m_size = _size;
	}
		
	template<typename TLoader>
	uint32 Manager<TLoader>::hash(const char* _string)
	{
		uint32 hashvalue = 208357;

		while(int c = *_string++)
			hashvalue = ((hashvalue << 5) + (hashvalue << 1) + hashvalue) ^ c; 

		return hashvalue;
	}

} // namespace MiR