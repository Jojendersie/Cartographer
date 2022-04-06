#pragma once

#include <vector>
#include <ca/pa/containers/hashmap.hpp>
#include "widget.hpp"

namespace ca { namespace gui {

	/// The group is the simplest form of an element container.
	///
	/// The group only forwards everything without thinking. It does not render itself,
	/// makes no clipping and is never resized.
	/// TODO: positioning might make sense.
	/// You can show/hide or enable/disable all contained elements by
	///	show/hide/enable/disable all the group.
	class Group : public Widget
	{
		template<typename GroupT, typename DataT>
		class IteratorT
		{
			GroupT* m_group;
			uint m_idx;
			IteratorT(GroupT* _group, uint _idx) : m_group(_group), m_idx(_idx) {}
			friend GroupT;
		public:
			IteratorT& operator ++ ()
			{
				++m_idx;
				return *this;
			}

			bool operator == (const IteratorT& _other) const { return m_group == _other.m_group && m_idx == _other.m_idx; }
			bool operator != (const IteratorT& _other) const { return m_group != _other.m_group || m_idx != _other.m_idx; }

			DataT& operator * () const { return *m_group->m_children[m_idx].widget; }

			operator bool () const { return m_group != nullptr && m_idx < m_group->m_children.size(); }
		};
	public:
		Group();

		/// Implement the draw method
		void draw() const override;

		/// Add a new child element.
		/// \param [in] _innerLayer Stir rendering order within the component.
		///		The larger the layer the later is the element drawn (more in foreground).
		void add(WidgetPtr _widget, unsigned _innerLayer = 0);
		/// The named add allows to assign a unique value to each element to find it later.
		/// \param [in] _name Some unique value which should identify this item. This could
		///		be a compile-time constant like an enum.
		void add(uint _name, WidgetPtr _widget, unsigned _innerLayer = 0);

		/// Find and remove a widget (O(n) with n = number of elements)
		void remove(WidgetPtr _widget);

		/// HashMap O(1) search for an element with this unique identifier.
		/// \details Elements which where not added with a name cannot be found.
		/// \return An element or nullptr.
		WidgetPtr find(uint _name);

		/// Remove all widgets from this group.
		void clear();

		/// Forward input to subelements and to properties
		virtual bool processInput(const struct MouseState& _mouseState) override;

		typedef IteratorT<Group, Widget> Iterator;
		typedef IteratorT<const Group, const Widget> ConstIterator;
		/// Returns the first element or an invalid handle, if there is no children.
		Iterator begin() { return Iterator(this, 0); }
		ConstIterator begin() const { return ConstIterator(this, 0); }

		/// Return the invalid handle for range based for loops
		Iterator end() { return Iterator(this, (uint)m_children.size()); }
		ConstIterator end() const { return ConstIterator(this, (uint)m_children.size()); }
	protected:
		struct WEntry {
			WidgetPtr widget;
			unsigned innerLayer;
		};
		std::vector<WEntry> m_children;			///< List of subelements. The list is sorted after layers and last focus time (last element inside a layer has the focus).
		pa::HashMap<uint, WidgetPtr> m_nameMap;	///< A mapping to find elements with a constant integer or enum.

	private:
		/// Resort the children list to bring focused element to the front.
		void moveToFront(size_t _index);
	};

	typedef pa::RefPtr<Group> GroupPtr;

}} // namespace ca::gui