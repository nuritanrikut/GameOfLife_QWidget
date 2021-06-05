#include "GameWidget.hpp"

#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionFocusRect>
#include <QRandomGenerator>

#include <execution>

GameWidget::GameWidget( QWidget *parent )
    : QWidget( parent ),
      mNewWidth( 10 ),
      mNewHeight( 10 ),
      mGridWidth( 0 ),
      mGridHeight( 0 ),
      mCellStates( nullptr ),
      mNextStates( nullptr ),
      mStateArraySize( 0 ),
      mStride( 0 ),
      mFramebufferBits( nullptr ),
      mIndices( nullptr ),
      mIndicesSize( 0 )
{
    allocateGrid();
    mGridRect.setSize( QSize( width(), height() ) );
    mGridRect.moveCenter( QPoint( width() / 2, height() / 2 ) );

    const QColor colorDead = palette().color( QPalette::Window );
    const QColor colorAlive = palette().color( QPalette::WindowText );
    mRgbs[0] = colorDead.rgb();
    mRgbs[1] = colorAlive.rgb();
}

void GameWidget::setGridWidth( int width )
{
    mNewWidth = width;
    allocateGrid();
    generate();
}

void GameWidget::setGridHeight( int height )
{
    mNewHeight = height;
    allocateGrid();
    generate();
}

void GameWidget::generate()
{
    int seed = QRandomGenerator::global()->generate();

    std::fill_n( mCellStates, mStateArraySize, 0 );
    std::fill_n( mNextStates, mStateArraySize, 0 );

    int *ptr = mCellStates + 1;

    for( size_t y = 0; y < mGridHeight; y++ )
    {
        ptr += mStride;

        // Normally you don't want to capture writable reference to a variable with par_unseq
        // However we are trying to generate random data
        // Garbage values are random as well :)
        std::generate_n( std::execution::par_unseq, ptr, mGridWidth, [&seed]() {
            seed = ( 214013 * seed + 2531011 );
            return ( seed >> 16 ) & 0x1;
        } );
    }

    iterate();
}

void GameWidget::calculateNextState( size_t index )
{
    const size_t y = index >> 16;
    const size_t x = index & 0xFFFF;
    const size_t cellIndex = ( y + 1 ) * mStride + ( x + 1 );
    const size_t framebufferIndex = y * mGridWidth + x;

    int aliveNeighbors = 0;
    aliveNeighbors += mCellStates[cellIndex - mStride - 1];
    aliveNeighbors += mCellStates[cellIndex - mStride + 0];
    aliveNeighbors += mCellStates[cellIndex - mStride + 1];
    aliveNeighbors += mCellStates[cellIndex - 1];
    aliveNeighbors += mCellStates[cellIndex + 1];
    aliveNeighbors += mCellStates[cellIndex + mStride - 1];
    aliveNeighbors += mCellStates[cellIndex + mStride + 0];
    aliveNeighbors += mCellStates[cellIndex + mStride + 1];

    const size_t lookupKey = ( aliveNeighbors << 1 ) | mCellStates[cellIndex];
    constexpr std::array<size_t, 18> lookup{ 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    const size_t next = mNextStates[cellIndex] = lookup[lookupKey];

    mFramebufferBits[framebufferIndex] = mRgbs[next];
}

void GameWidget::iterate()
{
    std::for_each_n(
        std::execution::par_unseq, mIndices, mIndicesSize, [this]( size_t index ) { calculateNextState( index ); } );

    std::swap( mCellStates, mNextStates );

    update();
}

void GameWidget::paintEvent( QPaintEvent * /*event*/ )
{
    QPainter painter( this );

    QStyleOptionFocusRect option;
    option.initFrom( this );
    option.backgroundColor = palette().color( QPalette::Window );

    style()->drawPrimitive( QStyle::PE_Frame, &option, &painter, this );

    painter.drawImage( mGridRect, mFramebuffer );
}

void GameWidget::showEvent( QShowEvent * /*event*/ )
{
    recalculateGridRect();
}

void GameWidget::resizeEvent( QResizeEvent * /*event*/ )
{
    recalculateGridRect();
}

void GameWidget::recalculateGridRect()
{
    const double gridAspectRatio = double( mGridWidth ) / mGridHeight;
    const double widgetAspectRatio = double( width() ) / height();

    if( gridAspectRatio < widgetAspectRatio )
    {
        // grid is narrower than widget
        // use full height and calculate width from aspect ratio
        mGridRect.setSize( QSize( qCeil( height() * gridAspectRatio ), height() ) );
    }
    else
    {
        // grid is wider than widget
        // use full width and calculate height from aspect ratio
        mGridRect.setSize( QSize( width(), qCeil( width() / gridAspectRatio ) ) );
    }

    mGridRect.moveCenter( QPoint( width() / 2, height() / 2 ) );
}

void GameWidget::allocateGrid()
{
    mGridWidth = mNewWidth;
    mGridHeight = mNewHeight;

    recalculateGridRect();

    if( mCellStates )
    {
        delete[] mCellStates;
        delete[] mNextStates;
    }

    mStateArraySize = ( mGridWidth + 2 ) * ( mGridHeight + 2 );
    mStride = mGridWidth + 2;

    mCellStates = new int[mStateArraySize];
    mNextStates = new int[mStateArraySize];

    mFramebuffer = QImage( mGridWidth, mGridHeight, QImage::Format_ARGB32_Premultiplied );
    mFramebufferBits = reinterpret_cast<uint32_t *>( mFramebuffer.bits() );

    if( mIndices )
        delete[] mIndices;

    mIndicesSize = mGridWidth * mGridHeight;
    mIndices = new size_t[mIndicesSize];

    for( size_t y = 0; y < mGridHeight; y++ )
    {
        for( size_t x = 0; x < mGridWidth; x++ )
        {
            mIndices[y * mGridWidth + x] = y << 16 | x;
        }
    }
}
