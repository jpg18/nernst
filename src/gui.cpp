/* gui.cpp
 *
 * The main application GUI.
 */


#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <math.h>
#include <fstream>

#include "gui.h"
#include "status.h"
#include "xsim.h"
#include "ctrl.h"
#include "paint.h"
#include "options.h"
#include "atom.h"
#include "world.h"


double x_iters[ MAX_ITERS ], y_volts[ MAX_ITERS ], y_nernst[ MAX_ITERS ];


NernstGUI::NernstGUI( struct options *options, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   o = options;

   // Simulation
   sim = new XNernstSim( o, this );

   // Control panel
   ctrl = new NernstCtrl( o, this );

   ctrlFrame = new QFrame();
   ctrlFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   ctrlLayout = new QVBoxLayout();
   ctrlLayout->addWidget( ctrl );
   ctrlFrame->setLayout( ctrlLayout );

   // World visualization
   canvas = new NernstPainter( o, this );

   canvasFrame = new QFrame();
   canvasFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   canvasLayout = new QVBoxLayout();
   canvasLayout->addWidget( canvas );
   canvasFrame->setLayout( canvasLayout );

   // Potential plot   
   voltsPlot = new QwtPlot();
   voltsPlot->setTitle( "Membrane Potential" );
   voltsPlot->setAxisTitle( 0, "Potential (mV)" );
   voltsPlot->setAxisTitle( 2, "Time (iters)" );

   numCurves = 4;
   curves = new QwtPlotCurve *[ numCurves ];
   currentNernstCurve = 1;

   curves[ 0 ] = new QwtPlotCurve( "Membrane Potential" );
   curves[ 0 ]->setData( x_iters, y_volts, 0 );
   curves[ 0 ]->attach( voltsPlot );

   curves[ currentNernstCurve ] = new QwtPlotCurve( "Nernst Potential" );
   curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
   curves[ currentNernstCurve ]->setData( x_iters, y_nernst, 0 );
   curves[ currentNernstCurve ]->attach( voltsPlot );

   // Main window
   mainLayout = new QGridLayout();
   mainLayout->addWidget( ctrlFrame, 0, 0 );
   mainLayout->addWidget( canvasFrame, 0, 1 );
   mainLayout->addWidget( voltsPlot, 0, 2 );
   mainLayout->setColumnMinimumWidth( 2, 350 );
   mainLayout->setColumnStretch( 2, 1 );

   mainWidget = new QWidget();
   mainWidget->setLayout( mainLayout );
   setCentralWidget( mainWidget );
   setWindowTitle( "Nernst Potential Simulator | v0.8.3" );
   setWindowIcon( QIcon( ":/img/nernst.png" ) );
   statusBar = new NernstStatusBar( o, this );
   setStatusBar( statusBar );

   // Actions
   saveInitAct = new QAction( "&Save Initial Conditions", this );
   saveInitAct->setStatusTip( "Save the control panel settings for later use" );
   connect( saveInitAct, SIGNAL( triggered() ), this, SLOT( saveInit() ) );

   loadInitAct = new QAction( "&Load Initial Conditions", this );
   loadInitAct->setStatusTip( "Load control panel settings from a file" );
   connect( loadInitAct, SIGNAL( triggered() ), this, SLOT( loadInit() ) );

   quitAct = new QAction( "&Quit", this );
   quitAct->setStatusTip( "Quit the simulator" );
   connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );

   aboutAct = new QAction( "&About", this );
   aboutAct->setStatusTip( "" );
   connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );

   aboutQtAct = new QAction( "About &Qt", this );
   aboutQtAct->setStatusTip( "" );
   connect( aboutQtAct, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );

   // Menus
   fileMenu = menuBar()->addMenu( "&File" );
   fileMenu->addAction( saveInitAct );
   fileMenu->addAction( loadInitAct );
   fileMenu->addSeparator();
   fileMenu->addAction( quitAct );

   helpMenu = menuBar()->addMenu( "&Help" );
   helpMenu->addAction( aboutAct );
   helpMenu->addAction( aboutQtAct );

   // Signals
   connect( this, SIGNAL( settingsLoaded() ), ctrl, SLOT( reloadSettings() ) );

   connect( sim, SIGNAL( moveCompleted( int ) ), ctrl, SLOT( updateIter( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), canvas, SLOT( update() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), this, SLOT( updatePlots( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), statusBar, SLOT( updateProgressBar( int ) ) );
   connect( sim, SIGNAL( updateStatus( QString ) ), statusBar, SLOT( setStatusLbl( QString ) ) ); 
   connect( sim, SIGNAL( updateVoltsStatus( int, int ) ), statusBar, SLOT( setVoltsLbl( int, int ) ) );
   connect( sim, SIGNAL( finished() ), ctrl, SLOT( reenableCtrl() ) );

   connect( ctrl, SIGNAL( startBtnClicked() ), canvas, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), this, SLOT( disableSaveLoad() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), sim, SLOT( runSim() ) );
   connect( ctrl, SIGNAL( pauseBtnClicked() ), sim, SLOT( pauseSim() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SLOT( recalcNernst() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), sim, SLOT( unpauseSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( enableSaveLoad() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), statusBar, SLOT( resetProgress() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );
   connect( ctrl, SIGNAL( worldShrunk() ), this, SLOT( shrinkWindow() ) );
#ifdef BLR_USELINUX
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( update() ) );
#else
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( cleanUpdate() ) );
#endif

   connect( canvas, SIGNAL( previewRedrawn() ), this, SLOT( fixRedraw() ) );
}


void
NernstGUI::about()
{
   QMessageBox::about( this, "About Nernst Potential Simulator",
      "<h3>About Nernst Potential Simulator</h3><br>"
      "<br>"
      "Version 0.8.3<br>"
      "Copyright " + QString( 0x00A9 ) + " 2008  "
      "Jeff Gill, Barry Rountree, Kendrick Shaw, "
      "Catherine Kehl, Jocelyn Eckert, "
      "and Dr. Hillel J. Chiel.<br>"
      "<br>"
      "Released under the GPL version 3 or any later version. "
      "This is free software; see the source for copying "
      "conditions. There is NO warranty; not even for "
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.<br>"
      "<br>"
      "Have a suggestion? Find a bug? Send us your comments "
      "at <a href='mailto:autopoiesis@case.edu'>"
      "autopoiesis@case.edu</a>." );
}


void
NernstGUI::saveInit()
{
   QString fileName;
   fileName = QFileDialog::getSaveFileName( this, "Save Initial Conditions", "settings.init", "Initial Conditions (*.init)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "Initial Conditions", QString("Cannot write file %1:\n%2.")
            .arg( file.fileName() )
            .arg( file.errorString() ) );
      return;
   }

   QTextStream out( &file );
   QApplication::setOverrideCursor( Qt::WaitCursor );
   
   // Begin writing to file
   out << o->iters;           endl( out );
   out << o->x;               endl( out );
   out << o->y;               endl( out );
   out << o->pores;           endl( out );
   out << o->randseed;        endl( out );
   out << o->lconc;           endl( out );
   out << o->rconc;           endl( out );
   out << o->selectivity;     endl( out );
   out << o->electrostatics;  endl( out );

   QApplication::restoreOverrideCursor();
   return;
}


void
NernstGUI::loadInit()
{
   QString fileName;
   fileName = QFileDialog::getOpenFileName( this, "Load Initial Conditions", "", "Initial Conditions (*.init)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this, "Initial Conditions", QString("Cannot read file %1:\n%2.")
            .arg( file.fileName() )
            .arg( file.errorString() ) );
      return;
   }

   QTextStream in( &file );
   int iters, x, y, pores, randseed, lconc, rconc, selectivity, electrostatics;

   // Begin reading file
   in >> iters;
   in >> x;
   in >> y;
   in >> pores;
   in >> randseed;
   in >> lconc;
   in >> rconc;
   in >> selectivity;
   in >> electrostatics;

   o->iters = iters;
   o->x = x;
   o->y = y;
   o->pores = pores;
   o->randseed = randseed;
   o->lconc = lconc;
   o->rconc = rconc;
   o->selectivity = selectivity;
   o->electrostatics = electrostatics;

   emit settingsLoaded();
}


void
NernstGUI::disableSaveLoad()
{
   saveInitAct->setEnabled( 0 );
   loadInitAct->setEnabled( 0 );
}


void
NernstGUI::enableSaveLoad()
{
   saveInitAct->setEnabled( 1 );
   loadInitAct->setEnabled( 1 );
}


void
NernstGUI::recalcNernst()
{
   int x, y;
   initLHS_K = 0;
   initRHS_K = 0;
   initLHS_Cl = 0;
   initRHS_Cl = 0;

   // Count up the ions on the LHS
   for( x = 0; x < o->x / 2; x++ )
   {
      for( y = 0; y < o->y; y++ )
      {
         switch( world[ idx( x, y ) ].color )
         {
            case ATOM_K:
               initLHS_K++;
               break;
            case ATOM_K_TRACK:
               initLHS_K++;
               break;
            case ATOM_Cl:
               initLHS_Cl++;
               break;
            case ATOM_Cl_TRACK:
               initLHS_Cl++;
               break;
            default:
               break;
         }
      }
   }

   // Count up the ions on the RHS
   for( x = o->x / 2 + 1; x < o->x; x++ )
   {
      for( y = 0; y < o->y; y++ )
      {
         switch( world[ idx( x, y ) ].color )
         {
            case ATOM_K:
               initRHS_K++;
               break;
            case ATOM_K_TRACK:
               initRHS_K++;
               break;
            case ATOM_Cl:
               initRHS_Cl++;
               break;
            case ATOM_Cl_TRACK:
               initRHS_Cl++;
               break;
            default:
               break;
         }
      }
   }
}


void
NernstGUI::updatePlots( int currentIter )
{
   static int nernstHasSomeData = 0;
   static int beginThisNernstCurve = 0;

   x_iters[ currentIter ] = currentIter;
   y_volts[ currentIter ] = LRcharge * e / ( c * a * o->y ) * 1000;                                      // Membrane potential (mV)

   curves[ 0 ]->setData( x_iters, y_volts, currentIter );

   if( o->electrostatics == 1 )
   {
      if( !nernstHasSomeData )
      {
         beginThisNernstCurve = currentIter;
         nernstHasSomeData = 1;
      }
      if( o->pores == 0 )
      {
         y_nernst[ currentIter ] = y_volts[ currentIter ];
      } else {
         if( o->selectivity == 0 )
         {
            y_nernst[ currentIter ] = 0;
         } else {
            y_nernst[ currentIter ] = R * t / F * log( (double)initRHS_K / (double)initLHS_K ) * 1000;   // Equilibrium potential (mV)
         }
      }
      curves[ currentNernstCurve ]->setData( x_iters + beginThisNernstCurve,
                                             y_nernst + beginThisNernstCurve,
                                             currentIter - beginThisNernstCurve + 1 );

   } else {

      if( nernstHasSomeData )
      {
         currentNernstCurve++;
         if( currentNernstCurve >= numCurves )
         {
            // Grow the curves array.
            QwtPlotCurve *temp[ numCurves ];
            for( int i = 0; i < numCurves; i++ )
            {
               temp[ i ] = curves[ i ];
            }
            numCurves *= 2;
            curves = new QwtPlotCurve *[ numCurves ];
            for( int i = 0; i < numCurves / 2; i++ )
            {
               curves[ i ] = temp[ i ];
            }
         }
         curves[ currentNernstCurve ] = new QwtPlotCurve( "Nernst Potential" );
         curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
         curves[ currentNernstCurve ]->attach( voltsPlot );
         nernstHasSomeData = 0;
      }

   }

   voltsPlot->replot();
}


void
NernstGUI::resetPlots()
{
   curves[ 0 ]->setData( x_iters, y_volts, 0 );

   currentNernstCurve = 1;
   curves[ currentNernstCurve ]->setData( x_iters, y_nernst, 0 );

   voltsPlot->replot();
}


void
NernstGUI::fixRedraw()
{
   // Fixes a redraw issue whenever the world size is changed in Windows.
   canvasFrame->hide();
   canvasFrame->show();
}


void
NernstGUI::shrinkWindow()
{
   // Shrinks the window to a managable size if it was enlarged too much.
   if( !isMaximized() )
   {
      resize( sizeHint() );
   }
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   sim->quitSim();
   QWidget::closeEvent( event );
}

