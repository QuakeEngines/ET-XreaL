#include "GtkLogRedirector.h"

#include "itextstream.h"
#include <iostream>

namespace applog {

GtkLogRedirector::GtkLogRedirector()
{
	// Define the flags
	GLogLevelFlags flags = (GLogLevelFlags) (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | 
		G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | 
		G_LOG_LEVEL_DEBUG | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION);

	_handles["Gdk"] = g_log_set_handler("Gdk", flags, handleLogMessage, this);
	_handles["GTK"] = g_log_set_handler("Gtk", flags,	handleLogMessage, this);
	_handles["GtkGLExt"] = g_log_set_handler("GtkGLExt", flags, handleLogMessage, this);
	_handles["GLib"] = g_log_set_handler("GLib", flags, handleLogMessage, this);
	_handles[""] = g_log_set_handler(0, flags,	handleLogMessage, this);
}

GtkLogRedirector::~GtkLogRedirector()
{
	for (std::map<std::string, guint>::iterator i = _handles.begin(); i != _handles.end(); ++i)
	{
		g_log_remove_handler(i->first.empty() ? NULL : i->first.c_str(), i->second);
	}

	_handles.clear();
}

void GtkLogRedirector::init()
{
	if (InstancePtr() == NULL)
	{
		InstancePtr() = GtkLogRedirectorPtr(new GtkLogRedirector);
	}
}

void GtkLogRedirector::destroy()
{
	// Clear the shared_ptr, this will disconnect the handlers
	InstancePtr() = GtkLogRedirectorPtr();
}

void GtkLogRedirector::handleLogMessage(const gchar* log_domain, 
										GLogLevelFlags log_level, 
										const gchar* message, 
										gpointer user_data)
{
	if (log_level & (G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL))
	{
		globalErrorStream() << message << std::endl;
	}
	else if (log_level & G_LOG_LEVEL_WARNING)
	{
		globalWarningStream() << message << std::endl;
	}
	else 
	{
		globalOutputStream() << message << std::endl;
	}
}

GtkLogRedirectorPtr& GtkLogRedirector::InstancePtr()
{
	static GtkLogRedirectorPtr _instancePtr;
	return _instancePtr;
}

} // namespace applog
