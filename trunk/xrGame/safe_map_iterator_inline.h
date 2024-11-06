////////////////////////////////////////////////////////////////////////////
//	Module 		: safe_map_iterator_inline.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Safe map iterator template inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

#define TEMPLATE_SPEZIALIZATION \
	template <\
		typename _key_type,\
		typename _data_type,\
		typename _predicate,\
		bool	 use_time_limit,\
		typename _cycle_type,\
		bool	 use_first_update\
	>

#define CSSafeMapIterator	CSafeMapIterator<_key_type,_data_type,_predicate,use_time_limit,_cycle_type,use_first_update>

TEMPLATE_SPEZIALIZATION
IC	CSSafeMapIterator::CSafeMapIterator			()
{
	m_cycle_count			= 0;
	m_first_update			= use_first_update;
	m_max_process_time		= 0;
//	update_next				();
}

TEMPLATE_SPEZIALIZATION
CSSafeMapIterator::~CSafeMapIterator			()
{
}

TEMPLATE_SPEZIALIZATION
IC	void CSSafeMapIterator::add					(const _key_type &id, _data_type *value, bool no_assert)
{
	_const_iterator			I = m_objects.find(id);
	if (I != m_objects.end()) {
		THROW2				(no_assert,"Specified object has been already found in the registry!");
		return;
	}

//	bool					addition = m_objects.empty();

	m_objects.insert		(std::make_pair(id,value));

//	if (addition)
//		m_next_iterator		= m_objects.begin();
}

TEMPLATE_SPEZIALIZATION
IC	void CSSafeMapIterator::remove				(const _key_type &id, bool no_assert)
{
	_iterator				I = m_objects.find(id);
	if (I == m_objects.end()) {
		THROW2				(no_assert,"Specified object hasn't been found in the registry!");
		return;
	}

	//if (I == m_next_iterator)
	//	update_next			();

	m_objects.erase			(I);

	//if (m_objects.empty())
	//	update_next			();
}
/*
TEMPLATE_SPEZIALIZATION
IC	void CSSafeMapIterator::update_next			()
{
	if (m_objects.empty())
		m_next_iterator		= m_objects.begin();
	else {
		++m_next_iterator;
		if (m_next_iterator == m_objects.end())
			m_next_iterator	= m_objects.begin();
	}
}


TEMPLATE_SPEZIALIZATION
IC	typename CSSafeMapIterator::_iterator	&CSSafeMapIterator::next	()
{
	return				(m_next_iterator);
}
*/

TEMPLATE_SPEZIALIZATION
IC	void CSSafeMapIterator::start_timer			()
{
	m_timer.Start		();
}

TEMPLATE_SPEZIALIZATION
IC	bool CSSafeMapIterator::time_over			()
{
	return use_time_limit && !m_first_update && (m_timer.GetElapsed_ms() >= m_max_process_time);
}

TEMPLATE_SPEZIALIZATION
IC	void CSSafeMapIterator::set_process_time(const int& process_time_millisecond)
{
	m_max_process_time = process_time_millisecond;
}

TEMPLATE_SPEZIALIZATION
IC	const typename CSSafeMapIterator::_REGISTRY	&CSSafeMapIterator::objects	() const
{
	return				(m_objects);
}

TEMPLATE_SPEZIALIZATION
IC void CSSafeMapIterator::clear()
{
	m_objects.clear();
}

TEMPLATE_SPEZIALIZATION
template <typename _update_predicate>
IC void CSSafeMapIterator::update(const _update_predicate &predicate)
{
	if (m_objects.empty())
		return;

	start_timer			();
	++m_cycle_count;

	//if (!time_over())
	{
		//_iterator I = next();
		//VERIFY(I != m_objects.end());

		//for (u32 i = 0; I != m_objects.end() && !time_over() && predicate((*I).second, m_cycle_count); ++i)
		//{
		//	update_next();
		//	I = next();
		//}

		for (auto& i : m_objects)
		{
			if (time_over() || !predicate(i.second, m_cycle_count))
				break;
		}
	}

	m_first_update		= false;
}

#undef TEMPLATE_SPEZIALIZATION
#undef CSSafeMapIterator