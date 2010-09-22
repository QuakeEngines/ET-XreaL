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

#if !defined(INCLUDED_MAPFILE_H)
#define INCLUDED_MAPFILE_H

#include <limits>

#include "iscenegraph.h"
#include <boost/function/function_fwd.hpp>

const std::size_t MAPFILE_MAX_CHANGES = std::numeric_limits<std::size_t>::max();

class MapFile
{
public:
  virtual ~MapFile() {}
  virtual void save() = 0;
  virtual bool saved() const = 0;
  virtual void changed() = 0;
  virtual void setChangedCallback(const boost::function<void()>& changed) = 0;
  virtual std::size_t changes() const = 0;
};
typedef boost::shared_ptr<MapFile> MapFilePtr;

inline MapFilePtr Node_getMapFile(const scene::INodePtr& node)
{
	return boost::dynamic_pointer_cast<MapFile>(node);
}

namespace scene
{

inline MapFile* findMapFile(INodePtr node)
{
	while (node != NULL)
	{
		 MapFilePtr map = Node_getMapFile(node);

		 if (map != NULL)
		 {
			 return map.get();
		 }

		 node = node->getParent();
	}

	return NULL;
}

} // namespace scene

#endif
