#pragma once

#include <QMainWindow>
#include <QSpinBox>
#include <QTimer>
#include <QElapsedTimer>

#include "GameWidget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = nullptr );
    ~MainWindow();

public Q_SLOTS:
    void frame();

private:
    GameWidget *mGame;

    QTimer mFrameTimer;
    bool mIsRunning;
    std::array<double, 10> mFrameTimings;
    size_t mFrameTimingsIndex;
};
