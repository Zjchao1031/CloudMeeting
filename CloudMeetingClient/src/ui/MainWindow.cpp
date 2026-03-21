#include "ui/MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("CloudMeeting");
    resize(960, 640);
}

MainWindow::~MainWindow() {}

void MainWindow::onCreateMeetingClicked() {}

void MainWindow::onJoinMeetingClicked() {}

void MainWindow::onOpenSettingsClicked() {}
