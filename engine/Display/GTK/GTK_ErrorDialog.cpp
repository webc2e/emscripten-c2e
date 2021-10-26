#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#ifndef C2E_USING_GTK
#error "this is the gtk version"
#endif

#include "GTK_ErrorDialog.h"
#include "../../C2eServices.h"
#include "../DisplayEngine.h"
#include "../../FlightRecorder.h"

#include <gtk/gtk.h>

//#include "foo.xpm"

bool ErrorDialog::ourDisableContinueButton = false;

// Only one at a time with this mechanism (IS THIS NEEDED?)
ErrorDialog* currentErrorDialog;

ErrorDialog::ErrorDialog() :
	myReturnCode( ED_QUIT )
{
	currentErrorDialog = this;
}


int ErrorDialog::DisplayDialog()
{
	theFlightRecorder.Log(FLIGHT_RUNTIME,"Error in: %s",mySource.c_str());
	theFlightRecorder.Log(FLIGHT_RUNTIME,myText.c_str());

	// switch out of fullscreen mode if needed
	DisplayEngine::theRenderer().PrepareForMessageBox();

	GtkWidget* fenster;
	GtkWidget* text;

	GtkWidget* continuebutton;
	GtkWidget* quitbutton;
	GtkWidget* brutalabortbutton;

	GtkWidget* vbox;
	GtkWidget* hbox_upper;
	GtkWidget* hbox_lower;

//	GtkWidget* pic;
//	GdkPixmap* pic_pixmap;

	// create widgets
	fenster = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_container_set_border_width(GTK_CONTAINER(fenster), 2);
	gtk_window_set_default_size(GTK_WINDOW(fenster), 420, 250);
	gtk_window_set_title(GTK_WINDOW(fenster), mySource.c_str());

	vbox = gtk_vbox_new( 0,0 ); // homogeneous, spacing
    hbox_lower = gtk_hbox_new( TRUE,0 );
    hbox_upper = gtk_hbox_new( 0,0 );

	// gtk_window_set_title()...

	text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(text), FALSE);
	gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, myText.c_str(), -1);
	continuebutton = gtk_button_new_with_label( "Continue" );
	quitbutton = gtk_button_new_with_label( "Quit" );
	brutalabortbutton = gtk_button_new_with_label( "Brutal Abort" );

	// create picture widget
//	pic_pixmap = gdk_pixmap_create_from_xpm_d( fenster->window,	NULL,
//		NULL, (gchar**)foo_xpm );
//	pic = gtk_pixmap_new( pic_pixmap, NULL );

	// pack widgets into other widgets
	gtk_container_add( GTK_CONTAINER( fenster ), vbox );

	gtk_box_pack_start( GTK_BOX( vbox),
        hbox_upper, TRUE, TRUE , 0 );
//	gtk_box_pack_start( GTK_BOX( hbox_upper ),
//        pic, FALSE,TRUE ,0  );
	gtk_box_pack_start( GTK_BOX( hbox_upper ),
        text, TRUE, TRUE , 0 );

	gtk_box_pack_start( GTK_BOX( vbox),
        hbox_lower, FALSE, TRUE , 3 );

	gtk_box_pack_start( GTK_BOX( hbox_lower ),
        continuebutton, FALSE, TRUE, 3 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
        quitbutton, FALSE, TRUE, 3 );
	gtk_box_pack_start( GTK_BOX( hbox_lower ),
        brutalabortbutton, FALSE, TRUE, 3 );

	// wire 'em up
	gtk_signal_connect( GTK_OBJECT(fenster), "delete_event",
		GTK_SIGNAL_FUNC(callback_delete_event), NULL );

	gtk_signal_connect( GTK_OBJECT(fenster), "destroy",
		GTK_SIGNAL_FUNC(callback_destroy), NULL );

	// order is important here - want to process the click before
	// emitting a destroy...
	gtk_signal_connect( GTK_OBJECT(continuebutton), "clicked",
		GTK_SIGNAL_FUNC(callback_ContinueClicked), this );
	gtk_signal_connect_object( GTK_OBJECT(continuebutton), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT( fenster ) );

	gtk_signal_connect( GTK_OBJECT(brutalabortbutton), "clicked",
		GTK_SIGNAL_FUNC(callback_BrutalAbortClicked), this );
	gtk_signal_connect_object( GTK_OBJECT(brutalabortbutton), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT( fenster ) );

	gtk_signal_connect( GTK_OBJECT(quitbutton), "clicked",
		GTK_SIGNAL_FUNC(callback_QuitClicked), this );
	gtk_signal_connect_object( GTK_OBJECT(quitbutton), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT( fenster ) );

	// show 'em 
	gtk_widget_show_all(fenster);
	if (ourDisableContinueButton)
	{
		gtk_widget_hide(continuebutton);
		gtk_widget_hide(quitbutton);
		ourDisableContinueButton = false;
	}

	// run 'em
	gtk_main();	

	// widget cleanup is already done by the callbacks

	switch( myReturnCode )
	{
		case ED_CONTINUE:
			theFlightRecorder.Log(FLIGHT_RUNTIME,"Action: Continue");
			break;
		case ED_QUIT:
			theFlightRecorder.Log(FLIGHT_RUNTIME,"Action: Quit");
			break;
		case ED_BRUTAL_ABORT:
			theFlightRecorder.Log(FLIGHT_RUNTIME,"Action: Brutal Abort");
			break;
	}

	// tell display that it can go fullscreen again if it wants to.
	DisplayEngine::theRenderer().EndMessageBox();

	return myReturnCode;
}

void ErrorDialog::SetText(const std::string& source, const std::string& text)
{
	mySource = source;
	myText = text;
}




// static gtk callbacks
int ErrorDialog::callback_delete_event( GtkWidget* w, GdkEvent* e,
	gpointer dat )
{
	return FALSE;
}

void ErrorDialog::callback_destroy( GtkWidget* w, gpointer dat )
{
	gtk_main_quit();
}

void ErrorDialog::callback_BrutalAbortClicked( GtkWidget* w, gpointer dat )
{
	ErrorDialog* that = (ErrorDialog*)dat;
	that->myReturnCode = ED_BRUTAL_ABORT;
}

void ErrorDialog::callback_QuitClicked( GtkWidget* w, gpointer dat )
{
	ErrorDialog* that = (ErrorDialog*)dat;
	that->myReturnCode = ED_QUIT;
}

void ErrorDialog::callback_ContinueClicked( GtkWidget* w, gpointer dat )
{
	ErrorDialog* that = (ErrorDialog*)dat;
	that->myReturnCode = ED_CONTINUE;
}


void ErrorDialog::DisableContinueButtonNextDialog()
{
	ourDisableContinueButton = true;
}




