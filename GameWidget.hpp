#pragma once

#include <QWidget>

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget( QWidget *parent = nullptr );

Q_SIGNALS:

public Q_SLOTS:
    void setGridWidth( int width );
    void setGridHeight( int height );
    void generate();
    void iterate();

private:
    void paintEvent( QPaintEvent *event ) override;
    void showEvent( QShowEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;

    void recalculateGridRect();

    void allocateGrid();

    void calculateNextState( size_t cellIndex );

private:
    // Dimensions that will be used when creating a new grid
    size_t mNewWidth;
    size_t mNewHeight;

    // Dimensions used while creating the grid.
    // Seperate from above in case those are updated when simulation is running.
    size_t mGridWidth;
    size_t mGridHeight;

    // Current state
    // 0 for dead
    // 1 for alive
    // Having integers instead of booleans makes calculation faster
    // To remove boundary check and branching cost, we allocate 1 more row/column in every direction
    // such that every cell has exactly 8 neighbors
    // Array size is (width+2) * (height+2)
    int *mCellStates;

    // Next state
    int *mNextStates;

    size_t mStateArraySize;
    size_t mStride;

    // Region to draw
    QRect mGridRect;

    // Image to draw on. This will be drawn on to screen at once
    QImage mFramebuffer;

    // Raw pixel value access to frame buffer
    uint *mFramebufferBits;

    // Colors for cell states
    // rgbs[0] for dead
    // rgbs[1] for alive
    // Lookup makes it faster
    QRgb mRgbs[2];

    // Cell at (0,0) would normally be on index (0)
    // But as we allocate extra rows/columns, it is actually on index (w+3)
    // Cell on the next row is in index (w+3 + w+2) for the same reason
    // we keep the indices in a seperate array just because it enables us to use parallel algorithm
    // Each element in the array encodes (x,y) coordinate
    size_t *mIndices;
    size_t mIndicesSize;
};
