#include "MainWindow.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent ),
      mGame( new GameWidget() ),
      mIsRunning( false ),
      mFrameTimingsIndex( 0 )
{
    mFrameTimings.fill( 0 );

    setWindowTitle( "Game of Life" );
    setMinimumSize( 800, 600 );

    auto topLayout = new QHBoxLayout();

    {
        topLayout->addWidget( new QLabel( "Width" ) );

        auto spinbox = new QSpinBox();
        spinbox->setMinimum( 3 );
        spinbox->setMaximum( 10000 );
        spinbox->setValue( 8 );
        connect( spinbox, &QSpinBox::valueChanged, mGame, &GameWidget::setGridWidth );
        mGame->setGridWidth( spinbox->value() );
        topLayout->addWidget( spinbox );
    }

    {
        topLayout->addWidget( new QLabel( "Height" ) );

        auto spinbox = new QSpinBox();
        spinbox->setMinimum( 3 );
        spinbox->setMaximum( 10000 );
        spinbox->setValue( 6 );
        connect( spinbox, &QSpinBox::valueChanged, mGame, &GameWidget::setGridHeight );
        mGame->setGridHeight( spinbox->value() );
        topLayout->addWidget( spinbox );
    }

    {
        // Once connections are done and added to layout, we don't need to keep the pointer to button
        // Hence it is defined locally, not as a member variable
        auto button = new QPushButton( "Generate" );
        connect( button, &QPushButton::clicked, mGame, &GameWidget::generate );
        topLayout->addWidget( button );
    }

    {
        topLayout->addWidget( new QLabel( "Update Interval (ms)" ) );

        auto spinbox = new QSpinBox();
        spinbox->setMinimum( 1 );
        spinbox->setMaximum( 10000 );

        // We can connect signals to lambda functions
        connect( spinbox, &QSpinBox::valueChanged, [this]( int value ) {
            mFrameTimer.setInterval( std::chrono::milliseconds( value ) );
        } );

        // Connect before setting the value, so that slot would be called
        spinbox->setValue( 33 );
        mFrameTimer.setInterval( std::chrono::milliseconds( spinbox->value() ) );

        topLayout->addWidget( spinbox );
    }

    {
        auto button = new QPushButton( "Single Iteration" );
        connect( button, &QPushButton::clicked, mGame, &GameWidget::iterate );
        topLayout->addWidget( button );
    }

    // SpacerItem will expand horizontally and align everthing else to the left
    topLayout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred ) );

    {
        auto mainLayout = new QVBoxLayout();
        mainLayout->addLayout( topLayout );
        mainLayout->addWidget( mGame );

        auto mainWidget = new QWidget();
        mainWidget->setLayout( mainLayout );
        setCentralWidget( mainWidget );
    }

    // MenuBar is enabled by default
    {
        // Add one top level menu
        // and some sub menus
        auto menuFile = menuBar()->addMenu( "File" );

        auto actionGenerate = menuFile->addAction( "Generate", this, [this]() {
            // This message will be shown for 1sec and disappear
            statusBar()->showMessage( "Generating", 1000 );
            mGame->generate();
            statusBar()->showMessage( "Generated", 1000 );
        } );
        auto actionStart = menuFile->addAction( "Start", this, [this]() {
            mIsRunning = true;
            mFrameTimer.start();
        } );
        auto actionStop = menuFile->addAction( "Stop", this, [this]() {
            mIsRunning = false;
            mFrameTimer.stop();
            statusBar()->showMessage( "Stopped", 1000 );
        } );
        auto actionExit = menuFile->addAction( "Exit", this, [this]() {
            // There are five levels of logging functions
            // qDebug()
            // qInfo()
            // qWarning()
            // qCritical()
            // qFatal()

            qDebug() << "Exit";

            // We will close the main window, thus exit the application
            close();
        } );

        // We can add a ToolBar
        auto toolbar = addToolBar( "File" );

        // We can use the same actions above, or create new ones
        toolbar->addAction( actionGenerate );
        toolbar->addAction( actionStart );
        toolbar->addAction( actionStop );
        toolbar->addAction( actionExit );
    }

    // StatusBar is not created by default
    setStatusBar( new QStatusBar() );

    // If set to singleShot, timer will fire only once
    mFrameTimer.setSingleShot( false );
    mFrameTimer.stop();
    connect( &mFrameTimer, &QTimer::timeout, this, &MainWindow::frame );

    mGame->generate();

    setFocus();
}

MainWindow::~MainWindow() { }

void MainWindow::frame()
{
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    mGame->iterate();

    auto elapsedNs = elapsedTimer.nsecsElapsed();
    double elapsedMs = elapsedNs / 1000000.0;
    mFrameTimings[mFrameTimingsIndex++] = elapsedMs;
    if( mFrameTimingsIndex == mFrameTimings.size() )
        mFrameTimingsIndex = 0;

    double averageFrameTime =
        std::accumulate( std::begin( mFrameTimings ), std::end( mFrameTimings ), 0.0 ) / mFrameTimings.size();

    statusBar()->showMessage( QString( "Average time per frame: %1 ms" ).arg( averageFrameTime, 0, 'f', 3 ) );
}
