/*

Copyright (c) 2003-2016, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "libtorrent/config.hpp"

#if TORRENT_USE_IOSTREAM
#include <iostream>
#endif

#ifndef TORRENT_NO_DEPRECATE
#include "libtorrent/lazy_entry.hpp"
#endif
#include "libtorrent/bdecode.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/hex.hpp"

namespace
{
}

namespace libtorrent
{
	namespace detail
	{
		TORRENT_EXPORT char const* integer_to_str(char* buf, int size
			, entry::integer_type val)
		{
			int sign = 0;
			if (val < 0)
			{
				sign = 1;
				val = -val;
			}
			buf[--size] = '\0';
			if (val == 0) buf[--size] = '0';
			for (; size > sign && val != 0;)
			{
				buf[--size] = '0' + char(val % 10);
				val /= 10;
			}
			if (sign) buf[--size] = '-';
			return buf + size;
		}
	}

	namespace
	{
		TORRENT_NO_RETURN inline void throw_error()
		{
#ifndef BOOST_NO_EXCEPTIONS
			throw system_error(error_code(errors::invalid_entry_type
					, get_libtorrent_category()));
#else
			std::terminate();
#endif
		}

		template <class T>
		void call_destructor(T* o)
		{
			TORRENT_ASSERT(o);
			o->~T();
		}
	}

	entry& entry::operator[](char const* key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i != dict().end()) return i->second;
		dictionary_type::iterator ret = dict().insert(
			std::pair<const std::string, entry>(key, entry())).first;
		return ret->second;
	}

	entry& entry::operator[](std::string const& key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i != dict().end()) return i->second;
		dictionary_type::iterator ret = dict().insert(
			std::make_pair(key, entry())).first;
		return ret->second;
	}

	entry* entry::find_key(char const* key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i == dict().end()) return nullptr;
		return &i->second;
	}

	entry const* entry::find_key(char const* key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) return nullptr;
		return &i->second;
	}

	entry* entry::find_key(std::string const& key)
	{
		dictionary_type::iterator i = dict().find(key);
		if (i == dict().end()) return nullptr;
		return &i->second;
	}

	entry const* entry::find_key(std::string const& key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) return nullptr;
		return &i->second;
	}

	const entry& entry::operator[](char const* key) const
	{
		return (*this)[std::string(key)];
	}

	const entry& entry::operator[](std::string const& key) const
	{
		dictionary_type::const_iterator i = dict().find(key);
		if (i == dict().end()) throw_error();
		return i->second;
	}

	entry::data_type entry::type() const
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return entry::data_type(m_type);
	}

	entry::~entry() { destruct(); }

	entry& entry::operator=(const entry& e)
	{
		if (&e == this) return *this;
		destruct();
		copy(e);
		return *this;
	}

	entry& entry::operator=(entry&& e)
	{
		swap(e);
		return *this;
	}

	entry::integer_type& entry::integer()
	{
		if (m_type == undefined_t) construct(int_t);
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		if (m_type != int_t) throw_error();
		TORRENT_ASSERT(m_type == int_t);
		return *reinterpret_cast<integer_type*>(&data);
	}

	entry::integer_type const& entry::integer() const
	{
		if (m_type != int_t) throw_error();
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		TORRENT_ASSERT(m_type == int_t);
		return *reinterpret_cast<const integer_type*>(&data);
	}

	entry::string_type& entry::string()
	{
		if (m_type == undefined_t) construct(string_t);
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		if (m_type != string_t) throw_error();
		TORRENT_ASSERT(m_type == string_t);
		return *reinterpret_cast<string_type*>(&data);
	}

	entry::string_type const& entry::string() const
	{
		if (m_type != string_t) throw_error();
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		TORRENT_ASSERT(m_type == string_t);
		return *reinterpret_cast<const string_type*>(&data);
	}

	entry::list_type& entry::list()
	{
		if (m_type == undefined_t) construct(list_t);
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		if (m_type != list_t) throw_error();
		TORRENT_ASSERT(m_type == list_t);
		return *reinterpret_cast<list_type*>(&data);
	}

	entry::list_type const& entry::list() const
	{
		if (m_type != list_t) throw_error();
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		TORRENT_ASSERT(m_type == list_t);
		return *reinterpret_cast<const list_type*>(&data);
	}

	entry::dictionary_type& entry::dict()
	{
		if (m_type == undefined_t) construct(dictionary_t);
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		if (m_type != dictionary_t) throw_error();
		TORRENT_ASSERT(m_type == dictionary_t);
		return *reinterpret_cast<dictionary_type*>(&data);
	}

	entry::dictionary_type const& entry::dict() const
	{
		if (m_type != dictionary_t) throw_error();
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		TORRENT_ASSERT(m_type == dictionary_t);
		return *reinterpret_cast<const dictionary_type*>(&data);
	}

	entry::preformatted_type& entry::preformatted()
	{
		if (m_type == undefined_t) construct(preformatted_t);
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		if (m_type != preformatted_t) throw_error();
		TORRENT_ASSERT(m_type == preformatted_t);
		return *reinterpret_cast<preformatted_type*>(&data);
	}

	entry::preformatted_type const& entry::preformatted() const
	{
		if (m_type != preformatted_t) throw_error();
#ifdef BOOST_NO_EXCEPTIONS
		TORRENT_ASSERT(m_type_queried);
#endif
		TORRENT_ASSERT(m_type == preformatted_t);
		return *reinterpret_cast<const preformatted_type*>(&data);
	}

	entry::entry()
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
	}

	entry::entry(data_type t)
		: m_type(undefined_t)
	{
		construct(t);
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
	}

	entry::entry(const entry& e)
		: m_type(undefined_t)
	{
		copy(e);
#if TORRENT_USE_ASSERTS
		m_type_queried = e.m_type_queried;
#endif
	}

	entry::entry(entry&& e)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		uint8_t type_queried = e.m_type_queried;
#endif
		swap(e);
#if TORRENT_USE_ASSERTS
		m_type_queried = type_queried;
#endif
	}

	entry::entry(dictionary_type v)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		new(&data) dictionary_type(std::move(v));
		m_type = dictionary_t;
	}

	entry::entry(span<char const> v)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		new(&data) string_type(v.data(), v.size());
		m_type = string_t;
	}

	entry::entry(list_type v)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		new(&data) list_type(std::move(v));
		m_type = list_t;
	}

	entry::entry(integer_type v)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		new(&data) integer_type(std::move(v));
		m_type = int_t;
	}

	entry::entry(preformatted_type v)
		: m_type(undefined_t)
	{
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		new(&data) preformatted_type(std::move(v));
		m_type = preformatted_t;
	}

	// convert a bdecode_node into an old skool entry
	entry& entry::operator=(bdecode_node const& e)
	{
		switch (e.type())
		{
			case bdecode_node::string_t:
				this->string() = e.string_value();
				break;
			case bdecode_node::int_t:
				this->integer() = e.int_value();
				break;
			case bdecode_node::dict_t:
			{
				dictionary_type& d = this->dict();
				for (int i = 0; i < e.dict_size(); ++i)
				{
					std::pair<std::string, bdecode_node> elem = e.dict_at(i);
					d[elem.first] = elem.second;
				}
				break;
			}
			case bdecode_node::list_t:
			{
				list_type& l = this->list();
				for (int i = 0; i < e.list_size(); ++i)
				{
					l.push_back(entry());
					l.back() = e.list_at(i);
				}
				break;
			}
			case bdecode_node::none_t:
				destruct();
				break;
		}
		return *this;
	}

#ifndef TORRENT_NO_DEPRECATE
	// convert a lazy_entry into an old skool entry
	entry& entry::operator=(lazy_entry const& e)
	{
		switch (e.type())
		{
			case lazy_entry::string_t:
				this->string() = e.string_value();
				break;
			case lazy_entry::int_t:
				this->integer() = e.int_value();
				break;
			case lazy_entry::dict_t:
			{
				dictionary_type& d = this->dict();
				for (int i = 0; i < e.dict_size(); ++i)
				{
					std::pair<std::string, lazy_entry const*> elem = e.dict_at(i);
					d[elem.first] = *elem.second;
				}
				break;
			}
			case lazy_entry::list_t:
			{
				list_type& l = this->list();
				for (int i = 0; i < e.list_size(); ++i)
				{
					l.push_back(entry());
					l.back() = *e.list_at(i);
				}
				break;
			}
			case lazy_entry::none_t:
				destruct();
				break;
		}
		return *this;
	}
#endif

	entry& entry::operator=(preformatted_type v)
	{
		destruct();
		new(&data) preformatted_type(std::move(v));
		m_type = preformatted_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return *this;
	}

	entry& entry::operator=(dictionary_type v)
	{
		destruct();
		new(&data) dictionary_type(std::move(v));
		m_type = dictionary_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return *this;
	}

	entry& entry::operator=(span<char const> v)
	{
		destruct();
		new(&data) string_type(v.data(), v.size());
		m_type = string_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return *this;
	}

	entry& entry::operator=(list_type v)
	{
		destruct();
		new(&data) list_type(std::move(v));
		m_type = list_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return *this;
	}

	entry& entry::operator=(integer_type v)
	{
		destruct();
		new(&data) integer_type(std::move(v));
		m_type = int_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
		return *this;
	}

	bool entry::operator==(entry const& e) const
	{
		if (type() != e.type()) return false;

		switch (m_type)
		{
		case int_t:
			return integer() == e.integer();
		case string_t:
			return string() == e.string();
		case list_t:
			return list() == e.list();
		case dictionary_t:
			return dict() == e.dict();
		case preformatted_t:
			return preformatted() == e.preformatted();
		default:
			TORRENT_ASSERT(m_type == undefined_t);
			return true;
		}
	}

	void entry::construct(data_type t)
	{
		switch (t)
		{
		case int_t:
			new (&data) integer_type;
			break;
		case string_t:
			new (&data) string_type;
			break;
		case list_t:
			new (&data) list_type;
			break;
		case dictionary_t:
			new (&data) dictionary_type;
			break;
		case undefined_t:
			break;
		case preformatted_t:
			new (&data) preformatted_type;
			break;
		}
		m_type = t;
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
	}

	void entry::copy(entry const& e)
	{
		switch (e.type())
		{
		case int_t:
			new (&data) integer_type(e.integer());
			break;
		case string_t:
			new (&data) string_type(e.string());
			break;
		case list_t:
			new (&data) list_type(e.list());
			break;
		case dictionary_t:
			new (&data) dictionary_type(e.dict());
			break;
		case undefined_t:
			TORRENT_ASSERT(e.type() == undefined_t);
			break;
		case preformatted_t:
			new (&data) preformatted_type(e.preformatted());
			break;
		}
		m_type = e.type();
#if TORRENT_USE_ASSERTS
		m_type_queried = true;
#endif
	}

	void entry::destruct()
	{
		switch(m_type)
		{
		case int_t:
			call_destructor(reinterpret_cast<integer_type*>(&data));
			break;
		case string_t:
			call_destructor(reinterpret_cast<string_type*>(&data));
			break;
		case list_t:
			call_destructor(reinterpret_cast<list_type*>(&data));
			break;
		case dictionary_t:
			call_destructor(reinterpret_cast<dictionary_type*>(&data));
			break;
		case preformatted_t:
			call_destructor(reinterpret_cast<preformatted_type*>(&data));
			break;
		default:
			TORRENT_ASSERT(m_type == undefined_t);
			break;
		}
		m_type = undefined_t;
#if TORRENT_USE_ASSERTS
		m_type_queried = false;
#endif
	}

	void entry::swap(entry& e)
	{
		bool clear_this = false;
		bool clear_that = false;

		if (m_type == undefined_t && e.m_type == undefined_t)
			return;

		if (m_type == undefined_t)
		{
			construct(data_type(e.m_type));
			clear_that = true;
		}

		if (e.m_type == undefined_t)
		{
			e.construct(data_type(m_type));
			clear_this = true;
		}

		if (m_type == e.m_type)
		{
			switch (m_type)
			{
			case int_t:
				std::swap(*reinterpret_cast<integer_type*>(&data)
					, *reinterpret_cast<integer_type*>(&e.data));
				break;
			case string_t:
				std::swap(*reinterpret_cast<string_type*>(&data)
					, *reinterpret_cast<string_type*>(&e.data));
				break;
			case list_t:
				std::swap(*reinterpret_cast<list_type*>(&data)
					, *reinterpret_cast<list_type*>(&e.data));
				break;
			case dictionary_t:
				std::swap(*reinterpret_cast<dictionary_type*>(&data)
					, *reinterpret_cast<dictionary_type*>(&e.data));
				break;
			case preformatted_t:
				std::swap(*reinterpret_cast<preformatted_type*>(&data)
					, *reinterpret_cast<preformatted_type*>(&e.data));
				break;
			default:
				break;
			}

			if (clear_this)
				destruct();

			if (clear_that)
				e.destruct();
		}
		else
		{
			// currently, only swapping entries of the same type or where one
			// of the entries is uninitialized is supported.
			TORRENT_ASSERT_FAIL();
		}
	}

	std::string entry::to_string() const
	{
		std::string ret;
		to_string_impl(ret, 0);
		return ret;
	}

	void entry::to_string_impl(std::string& out, int indent) const
	{
		TORRENT_ASSERT(indent >= 0);
		for (int i = 0; i < indent; ++i) out += " ";
		switch (type())
		{
		case int_t:
			out += libtorrent::to_string(integer()).data();
			out += "\n";
			break;
		case string_t:
			{
				bool binary_string = false;
				for (std::string::const_iterator i = string().begin(); i != string().end(); ++i)
				{
					if (!is_print(static_cast<unsigned char>(*i)))
					{
						binary_string = true;
						break;
					}
				}
				if (binary_string)
				{
					out += aux::to_hex(string());
					out += "\n";
				}
				else
				{
					out += string();
					out += "\n";
				}
			} break;
		case list_t:
			{
				out += "list\n";
				for (list_type::const_iterator i = list().begin(); i != list().end(); ++i)
				{
					i->to_string_impl(out, indent+1);
				}
			} break;
		case dictionary_t:
			{
				out += "dictionary\n";
				for (dictionary_type::const_iterator i = dict().begin(); i != dict().end(); ++i)
				{
					bool binary_string = false;
					for (std::string::const_iterator k = i->first.begin(); k != i->first.end(); ++k)
					{
						if (!is_print(static_cast<unsigned char>(*k)))
						{
							binary_string = true;
							break;
						}
					}
					for (int j = 0; j < indent+1; ++j) out += " ";
					out += "[";
					if (binary_string) out += aux::to_hex(i->first);
					else out += i->first;
					out += "]";

					if (i->second.type() != entry::string_t
						&& i->second.type() != entry::int_t)
						out += "\n";
					else out += " ";
					i->second.to_string_impl(out, indent+2);
				}
			} break;
		case preformatted_t:
			out += "<preformatted>\n";
			break;
		case undefined_t:
			out += "<uninitialized>\n";
		}
	}
}
