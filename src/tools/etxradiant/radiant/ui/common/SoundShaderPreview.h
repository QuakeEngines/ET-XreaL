#ifndef SOUNDSHADERPREVIEW_H_
#define SOUNDSHADERPREVIEW_H_

#include <string>

// Forward declaration
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkListStore GtkListStore;
typedef struct _GtkTreeSelection GtkTreeSelection;
typedef struct _GtkButton GtkButton;

namespace ui {

/** greebo: This class provides the UI elements to inspect a given
 * 			sound shader with playback option.
 *
 * 			Use the GtkWidget* cast operator to pack this into a
 * 			parent container.
 */
class SoundShaderPreview
{
	// The main container widget of this preview
	GtkWidget* _widget;

	GtkWidget* _playButton;
	GtkWidget* _stopButton;
	GtkWidget* _statusLabel;

	// The currently "previewed" sound file
	std::string _soundFile;

public:
	SoundShaderPreview();

	/** greebo: Sets the soundshader to preview.
	 * 			This updates the preview liststore and treeview.
	 */
	void setSoundFile(const std::string& fileName);

	/** greebo: Operator cast to GtkWidget to pack this into a
	 * 			parent container widget.
	 */
	operator GtkWidget*();

private:
	/** greebo: Returns the currently selected sound file (file list)
	 *
	 * @returns: the filename as defined in the shader or "" if nothing selected.
	 */
	std::string getSelectedSoundFile();

	/** greebo: Creates the control widgets (play button) and such.
	 */
	GtkWidget* createControlPanel();

	/** greebo: Updates the list according to the active soundshader
	 */
	void update();

	// GTK Callbacks
	static void onPlay(GtkButton* button, SoundShaderPreview* self);
	static void onStop(GtkButton* button, SoundShaderPreview* self);
	static void onSelectionChange(GtkTreeSelection* ts, SoundShaderPreview* self);
};

} // namespace ui

#endif /*SOUNDSHADERPREVIEW_H_*/
