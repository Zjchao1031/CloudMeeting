#include "ui/MeetingWindow.h"

MeetingWindow::MeetingWindow(QWidget *parent)
    : QWidget(parent)
{
    resize(1280, 720);
}

MeetingWindow::~MeetingWindow() {}

void MeetingWindow::bindSignals() {}

void MeetingWindow::refreshLayout() {}

void MeetingWindow::showRoomClosedDialog() {}
