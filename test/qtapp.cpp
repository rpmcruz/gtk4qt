/***************************************************************************
 *   Copyright (C) 2007 by Ricardo Cruz                                    *
 *   rpmcruz@clix.pt                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/***************************************************************************
 * This style requires a GTK+ patch in order to work as a plugin. You may  *
 * however use it statically on this application.                          *
 ***************************************************************************/
// based on template from KDevelop

#include "qtapp.h"

#include <qimage.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qslider.h>

QtApp::QtApp()
    : QMainWindow( 0, "QtApp", WDestructiveClose )
{
    printer = new QPrinter;
    QPixmap openIcon, saveIcon, printIcon;


    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( tr("&File"), file );


    file->insertItem( tr("&New"), this, SLOT(newDoc()), CTRL+Key_N );

    int id;
    id = file->insertItem( openIcon, tr("&Open..."),
			   this, SLOT(choose()), CTRL+Key_O );

    id = file->insertItem( saveIcon, tr("&Save"),
			   this, SLOT(save()), CTRL+Key_S );

    id = file->insertItem( tr("Save &As..."), this, SLOT(saveAs()) );

    file->insertSeparator();

    id = file->insertItem( printIcon, tr("&Print..."),
			   this, SLOT(print()), CTRL+Key_P );

    file->insertSeparator();

    file->insertItem( tr("&Close"), this, SLOT(close()), CTRL+Key_W );

    file->insertItem( tr("&Quit"), qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    menuBar()->insertSeparator();

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Help"), help );

    help->insertItem( tr("&About"), this, SLOT(about()), Key_F1 );
    help->insertItem( tr("About &Qt"), this, SLOT(aboutQt()) );

	QVBox *box = new QVBox (this);

    e = new QTextEdit (box, "editor");
    e->setFocus();
    e->setHScrollBarMode (QScrollView::AlwaysOn);
    e->setVScrollBarMode (QScrollView::AlwaysOn);

		{  // just read some file
    QFile file( "/etc/passwd" );
    if ( file.open( IO_ReadOnly ) ) {
        QTextStream stream( &file );
        e->setText( stream.read() );
    }
		}


	QPushButton *button = new QPushButton ("Push me!", box);
	connect (button, SIGNAL (clicked()), this, SLOT (print()));

	new QSlider (0, 50, 1, 10, Qt::Horizontal, box);

    setCentralWidget( box );
    statusBar()->message( tr("Ready"), 2000 );

    resize( 450, 600 );
}


QtApp::~QtApp()
{
    delete printer;
}



void QtApp::newDoc()
{
    QtApp *ed = new QtApp;
    ed->setCaption(tr("Qt Example - Application"));
    ed->show();
}

void QtApp::choose()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
					       this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( tr("Loading aborted"), 2000 );
}


void QtApp::load( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QTextStream ts( &f );
    e->setText( ts.read() );
    e->setModified( FALSE );
    setCaption( fileName );
    statusBar()->message( tr("Loaded document %1").arg(fileName), 2000 );
}


void QtApp::save()
{
    if ( filename.isEmpty() ) {
	saveAs();
	return;
    }

    QString text = e->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
	statusBar()->message( tr("Could not write to %1").arg(filename),
			      2000 );
	return;
    }
/*
    QTextStream t( &f );
    t << text;
    f.close();
*/
    e->setModified( FALSE );

    setCaption( filename );

    statusBar()->message( tr( "File %1 saved" ).arg( filename ), 2000 );
}


void QtApp::saveAs()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null,
					       this );
    if ( !fn.isEmpty() ) {
	filename = fn;
	save();
    } else {
	statusBar()->message( tr("Saving aborted"), 2000 );
    }
}


void QtApp::print()
{
    // ###### Rewrite to use QSimpleRichText to print here as well
#if 0
    const int Margin = 10;
    int pageNo = 1;
#endif
    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( tr("Printing...") );
#if 0
	QPainter p;
	if( !p.begin( printer ) )               // paint on printer
	    return;

	p.setFont( e->font() );
	int yPos	= 0;			// y-position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( printer ); // need width/height
						// of printer surface
	for( int i = 0 ; i < e->lines() ; i++ ) {
	    if ( Margin + yPos > metrics.height() - Margin ) {
		QString msg( "Printing (page " );
		msg += QString::number( ++pageNo );
		msg += ")...";
		statusBar()->message( msg );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( Margin, Margin + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->text( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
#endif
	statusBar()->message( tr("Printing completed"), 2000 );
    } else {
	statusBar()->message( tr("Printing aborted"), 2000 );
    }
}

void QtApp::closeEvent( QCloseEvent* ce )
{
    if ( !e->isModified() ) {
	ce->accept();
	return;
    }

    switch( QMessageBox::information( this, tr("QGtkStyle Test Application"),
				      tr("Do you want to save the changes"
				      " to the document?"),
				      tr("Yes"), tr("No"), tr("Cancel"),
				      0, 1 ) ) {
    case 0:
	save();
	ce->accept();
	break;
    case 1:
	ce->accept();
	break;
    case 2:
    default: // just for sanity
	ce->ignore();
	break;
    }
}


void QtApp::about()
{
    QMessageBox::about( this, tr("QGtkStyle Test Application"),
			tr("A few Qt widgets to test QGtkStyle.\n"
			   "Interface based on template from KDevelop."));
}


void QtApp::aboutQt()
{
    QMessageBox::aboutQt( this, tr("QGtkStyle Test Application") );
}

#include <qapplication.h>

#include "../qgtkstyle.h"
#include <gtk/gtkmain.h>
#include <gdk/gdkx.h>

int main (int argc, char *argv[])
{
	// Just a test program
	// TODO: make the style a plugin, so it gets to be used by every Qt app

	QApplication app (argc, argv);
	// uncomment the following line for a comparision
	QApplication::setStyle (new QGtkStyle());

	QtApp *main_widget = new QtApp();
	main_widget->setCaption ("QGtkStyle Test");
	main_widget->show();

	app.connect (&app, SIGNAL (lastWindowClosed()), &app, SLOT (quit()));
	return app.exec();
}
