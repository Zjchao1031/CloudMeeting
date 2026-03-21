#pragma once
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateMeetingClicked();
    void onJoinMeetingClicked();
    void onOpenSettingsClicked();
};
