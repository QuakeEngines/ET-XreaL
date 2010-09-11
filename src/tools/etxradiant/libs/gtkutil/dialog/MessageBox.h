#ifndef _UI_MESSAGEBOX_H_
#define _UI_MESSAGEBOX_H_

#include "Dialog.h"

namespace gtkutil
{

/**
 * A MessageBox is a specialised Dialog used for popup messages of various purpose.
 * Supported are things like Notifications, Warnings, Errors and simple Yes/No questions.
 *
 * Each messagebox is equipped with a special GTK stock icon, corresponding to its type.
 */
class MessageBox :
	public Dialog
{
protected:
	// The message text
	std::string _text;

	// The message type
	ui::IDialog::MessageType _type;

public:
	// Constructs a new messageBox using the given title and text
	MessageBox(const std::string& title, 
			   const std::string& text, 
			   ui::IDialog::MessageType type,
			   GtkWindow* parent = NULL);

protected:
	// Constructs the dialog (adds buttons, text and icons)
	virtual void construct();

	// Override Dialog::createButtons() to add the custom ones
	virtual GtkWidget* createButtons();

	// Creates an icon from stock (notification, warning, error)
	GtkWidget* createIcon();

	// GTK Callbacks, additional to the ones defined in the base class
	static void onYes(GtkWidget* widget, MessageBox* self);
	static void onNo(GtkWidget* widget, MessageBox* self);
};
typedef boost::shared_ptr<MessageBox> MessageBoxPtr;

} // namespace gtkutil

#endif /* _UI_MESSAGEBOX_H_ */
