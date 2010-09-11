#include "WindowPosition.h"

#include "iregistry.h"
#include "string/string.h"
#include "MultiMonitor.h"

namespace {
	const int DEFAULT_POSITION_X = 50;
	const int DEFAULT_POSITION_Y = 25;
	const int DEFAULT_SIZE_X = 400;
	const int DEFAULT_SIZE_Y = 300;
}

namespace gtkutil
{

WindowPosition::WindowPosition() : 
	_position(DEFAULT_POSITION_X, DEFAULT_POSITION_Y),
	_size(DEFAULT_SIZE_X, DEFAULT_SIZE_Y),
	_window(NULL)
{}

// Connect the passed window to this object
void WindowPosition::connect(GtkWindow* window) {
	_window = window;
	applyPosition();
	g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(onConfigure), this);
}

const PositionVector& WindowPosition::getPosition() const {
	return _position;
}

const SizeVector& WindowPosition::getSize() const {
	return _size;
}

void WindowPosition::setPosition(int x, int y) {
	_position[0] = x;
	_position[1] = y;
}

void WindowPosition::setSize(int width, int height) {
	_size[0] = width;
	_size[1] = height;
}

void WindowPosition::saveToPath(const std::string& path)
{
	GlobalRegistry().setAttribute(path, "xPosition", intToStr(_position[0]));
	GlobalRegistry().setAttribute(path, "yPosition", intToStr(_position[1]));
	GlobalRegistry().setAttribute(path, "width", intToStr(_size[0]));
	GlobalRegistry().setAttribute(path, "height", intToStr(_size[1]));
}

void WindowPosition::loadFromPath(const std::string& path)
{
	_position[0] = strToInt(GlobalRegistry().getAttribute(path, "xPosition"));
	_position[1] = strToInt(GlobalRegistry().getAttribute(path, "yPosition"));

	_size[0] = strToInt(GlobalRegistry().getAttribute(path, "width"));
	_size[1] = strToInt(GlobalRegistry().getAttribute(path, "height"));
}

// Applies the internally stored size/position info to the GtkWindow
// The algorithm was adapted from original GtkRadiant code (window.h) 
void WindowPosition::applyPosition() {
	if (_window != NULL) {
		gtk_window_set_gravity(_window, GDK_GRAVITY_STATIC);

		GdkScreen* screen = gdk_screen_get_default();
		
		// Sanity check of the window position
		if (_position[0] < 0 || _position[1] < 0 || 
			_position[0] > gdk_screen_get_width(screen) || 
			_position[1] > gdk_screen_get_height(screen))
		{
			gtk_window_set_position(_window, GTK_WIN_POS_CENTER_ON_PARENT);
		}
		else {
			gtk_window_move(_window, _position[0], _position[1]);
		}

		gtk_window_set_default_size(_window, _size[0], _size[1]);
		gtk_window_resize(_window, _size[0], _size[1]);
	}
}

// Reads the position from the GtkWindow
void WindowPosition::readPosition() {
	//gtk_window_set_gravity(_window, GDK_GRAVITY_STATIC);
	gtk_window_get_position(_window, &_position[0], &_position[1]);
	gtk_window_get_size(_window, &_size[0], &_size[1]);
}

void WindowPosition::fitToScreen(float xfraction, float yfraction) {
	if (_window == NULL) return;

	GdkRectangle geom = MultiMonitor::getMonitorForWindow(_window);
	
	// Pass the call
	fitToScreen(geom, xfraction, yfraction);
}

void WindowPosition::fitToScreen(GdkRectangle screen, float xfraction, float yfraction) {
	_size[0] = static_cast<int>(screen.width * xfraction) - 12;
	_size[1] = static_cast<int>(screen.height * yfraction) - 48;

	_position[0] = screen.x + static_cast<int>((screen.width - _size[0] - 12)/2);
	_position[1] = screen.y + static_cast<int>((screen.height - _size[1] - 48)/2);
}

// The static GTK callback that gets invoked on window size/position changes
gboolean WindowPosition::onConfigure(GtkWidget* widget, GdkEventConfigure *event, WindowPosition* self) {
	
	self->setPosition(event->x, event->y);
	self->setSize(event->width, event->height);

	return FALSE;
}

} // namespace gtkutil
