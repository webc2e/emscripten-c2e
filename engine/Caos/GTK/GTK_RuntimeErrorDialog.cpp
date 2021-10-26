#ifndef C2E_USING_GTK
#error "this is the gtk version"
#endif


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../RuntimeErrorDialog.h"

#include "../../Camera/MainCamera.h"



// Only one at a time with this mechanism
RuntimeErrorDialog* currentRuntimeErrorDialog;

RuntimeErrorDialog::RuntimeErrorDialog() :
	myCameraChecked( false ),
	myCameraPossible( true )
{
	currentRuntimeErrorDialog = this;
}

std::string StripAmpersand(const std::string& str)
{
	std::string out;
	for (int i = 0; i < str.size(); ++i)
		if (str[i] != '&')
			out += str[i];
	return out;
}

int RuntimeErrorDialog::DisplayDialog()
{
	// tell the display we're popping up a dialog box (may have to
	// switch from fullscreen to windowed mode)...
	theMainView.PrepareForMessageBox();

	myReturnCode = RED_IGNORE;


	GtkWidget* fenster;
	GtkWidget* text;

	GtkWidget* button_ignore;
	GtkWidget* button_freeze;
	GtkWidget* button_kill;
	GtkWidget* button_pausegame;
	GtkWidget* button_stopscript;
	GtkWidget* cb_movecamera;

	GtkWidget* vbox;
	GtkWidget* hbox_upper;	// pic/ text
	GtkWidget* hbox_lower;	// for buttons


	// create the container widgets
	fenster = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_container_set_border_width(GTK_CONTAINER(fenster), 2);
	gtk_window_set_default_size(GTK_WINDOW(fenster), 420, 250);
	gtk_window_set_title(GTK_WINDOW(fenster),
		theCatalogue.Get("system_title", 0));

	vbox = gtk_vbox_new( 0,0 );	// homogeneous, spacing
	hbox_lower = gtk_hbox_new( TRUE, 10 );
	hbox_upper = gtk_hbox_new( 0,0 );

	// other widgets
	text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(text), FALSE);
	gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, myText.c_str(), -1);

	button_stopscript = gtk_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 8) ).c_str());
	button_kill = gtk_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 2) ).c_str());
	button_freeze = gtk_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 1) ).c_str());
	button_ignore = gtk_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 0) ).c_str());
	button_pausegame = gtk_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 6) ).c_str());

	// move-to-camera tickbox
	cb_movecamera = gtk_check_button_new_with_label(
		StripAmpersand(theCatalogue.Get("runtime_error", 3) ).c_str());

	// TODO: check myCameraPossible flag to see if movecamera checkbox
	// should be disabled.

	// pack child widgets into containers
	gtk_container_add( GTK_CONTAINER(fenster), vbox );

	gtk_box_pack_start( GTK_BOX( vbox),
        hbox_upper, TRUE, TRUE , 0 );
	gtk_box_pack_start( GTK_BOX( hbox_upper ),
        text, TRUE, TRUE , 0 );

	gtk_box_pack_start( GTK_BOX( vbox),
        hbox_lower, FALSE, TRUE , 0 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
       	button_stopscript, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
       	button_kill, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
       	button_freeze, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
       	button_ignore, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
       	button_pausegame, FALSE, FALSE, 0 );

	gtk_box_pack_start( GTK_BOX( vbox ),
       	cb_movecamera, FALSE, FALSE, 0 );

	// wire 'em up
	gtk_signal_connect( GTK_OBJECT(fenster), "delete_event",
		GTK_SIGNAL_FUNC(callback_delete_event), NULL );

	gtk_signal_connect( GTK_OBJECT(fenster), "destroy",
		GTK_SIGNAL_FUNC(callback_destroy), NULL );

	// order is important here - we want to process the click before
	// the destroy gets emitted.
	gtk_signal_connect( GTK_OBJECT( button_ignore ), "clicked",
		GTK_SIGNAL_FUNC(callback_IgnoreClicked), this );
	gtk_signal_connect_object (GTK_OBJECT (button_ignore), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (fenster));

	gtk_signal_connect( GTK_OBJECT( button_freeze ), "clicked",
		GTK_SIGNAL_FUNC(callback_FreezeClicked), this );
	gtk_signal_connect_object (GTK_OBJECT (button_freeze), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (fenster));

	gtk_signal_connect( GTK_OBJECT( button_kill ), "clicked",
		GTK_SIGNAL_FUNC(callback_KillClicked), this );
	gtk_signal_connect_object (GTK_OBJECT (button_kill), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (fenster));

	gtk_signal_connect( GTK_OBJECT( button_pausegame ), "clicked",
		GTK_SIGNAL_FUNC(callback_PauseGameClicked), this );
	gtk_signal_connect_object (GTK_OBJECT (button_pausegame), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (fenster));

	gtk_signal_connect( GTK_OBJECT( button_stopscript ), "clicked",
		GTK_SIGNAL_FUNC(callback_StopScriptClicked), this );
	gtk_signal_connect_object (GTK_OBJECT (button_stopscript), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (fenster));

	gtk_signal_connect( GTK_OBJECT( cb_movecamera ), "toggled",
		GTK_SIGNAL_FUNC(callback_MoveCameraToggled), this );




	// show 'em and run 'em
	gtk_widget_show_all(fenster);

	gtk_main();	
	// widget cleanup already done by the callbacks

	theMainView.EndMessageBox();
	
	return myReturnCode;
}


void RuntimeErrorDialog::SetText(const std::string& text)
{
	myText = text;
}

void RuntimeErrorDialog::SetCameraPossible(bool camera)
{
	myCameraPossible = camera;
}



// static gtk callbacks
int RuntimeErrorDialog::callback_delete_event( GtkWidget* w, GdkEvent* e,
	gpointer dat )
{
	return FALSE;
}

void RuntimeErrorDialog::callback_destroy( GtkWidget* w, gpointer dat )
{
	gtk_main_quit();
}



void RuntimeErrorDialog::callback_IgnoreClicked( GtkWidget* w, gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	that->myReturnCode = RED_IGNORE;
}


void RuntimeErrorDialog::callback_FreezeClicked( GtkWidget* w, gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	that->myReturnCode = RED_FREEZE;
}


void RuntimeErrorDialog::callback_StopScriptClicked( GtkWidget* w, gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	that->myReturnCode = RED_STOPSCRIPT;
}


void RuntimeErrorDialog::callback_KillClicked( GtkWidget* w, gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	that->myReturnCode = RED_KILL;
}


void RuntimeErrorDialog::callback_PauseGameClicked( GtkWidget* w, gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	that->myReturnCode = RED_PAUSEGAME;
}

void RuntimeErrorDialog::callback_MoveCameraToggled( GtkWidget* w,
	gpointer dat )
{
	RuntimeErrorDialog* that = (RuntimeErrorDialog*)dat;
	bool state = GTK_TOGGLE_BUTTON(w)->active ? true:false;
	that->myCameraChecked = state;
}



