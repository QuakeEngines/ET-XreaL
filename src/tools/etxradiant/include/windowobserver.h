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

#if !defined(INCLUDED_WINDOWOBSERVER_H)
#define INCLUDED_WINDOWOBSERVER_H

#include "math/Vector2.h"

template<typename Element>
class BasicVector2;
typedef BasicVector2<double> Vector2;
typedef Vector2 WindowVector;
typedef struct _GdkEventButton GdkEventButton;

/* greebo: The abstract base class defining a window observer.
 * It has to handle all the mouseDown/Up/Move and keyboard events 
 * as well as window resizing.
 */
class WindowObserver
{
public:
  virtual ~WindowObserver() {}
  virtual void release() = 0;
  virtual void onSizeChanged(int width, int height) = 0;
  virtual void onMouseDown(const WindowVector& position, GdkEventButton* event) = 0;
  virtual void onMouseUp(const WindowVector& position, GdkEventButton* event) = 0;
  virtual void onMouseMotion(const WindowVector& position, const unsigned int& state) = 0;
  virtual void cancelOperation() = 0;
};

#endif
