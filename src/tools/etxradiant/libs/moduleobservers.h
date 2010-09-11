/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_MODULEOBSERVERS_H)
#define INCLUDED_MODULEOBSERVERS_H

#include "debugging/debugging.h"
#include <set>
#include "moduleobserver.h"

#ifdef _DEBUG
#include <iostream>
#endif

#include <stdexcept>

class ModuleObservers
{
	typedef std::set<ModuleObserver*> Observers;
	Observers m_observers;
	
	// greebo: Added this to make debugging easier, so that
	// the warning in the destructor provides more information.
	std::string _ownerName;

public:
	// Default constructor, for backwards compatibility
	ModuleObservers() :
		_ownerName("Owner unknown.")
	{}
	
	ModuleObservers(const std::string& ownerName) :
		_ownerName(ownerName)
	{}

#ifdef _DEBUG
	// Warning if observers still attached in destructor (may not be necessary,
	// replaces previous ASSERT.
	~ModuleObservers() {
		if (!m_observers.empty()) {
			std::cout << "Warning: destroying ModuleObservers with " 
				<< m_observers.size() << " observers attached." 
				<< " (Owner: " << _ownerName << ")"
				<< std::endl;
		}
	}
#endif
	
	
  void attach(ModuleObserver& observer)
  {
    ASSERT_MESSAGE(m_observers.find(&observer) == m_observers.end(), "ModuleObservers::attach: cannot attach observer");
    m_observers.insert(&observer);
  }
  void detach(ModuleObserver& observer)
  {
    ASSERT_MESSAGE(m_observers.find(&observer) != m_observers.end(), "ModuleObservers::detach: cannot detach observer");
    m_observers.erase(&observer);
  }
  void realise()
  {
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->realise();
    }
  }
  void unrealise()
  {
    for(Observers::reverse_iterator i = m_observers.rbegin(); i != m_observers.rend(); ++i)
    {
      (*i)->unrealise();
    }
  }
};

#endif
