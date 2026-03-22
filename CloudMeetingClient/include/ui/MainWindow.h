#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

class MeetingWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onCreateMeetingClicked();
    void onJoinMeetingClicked();
    void onOpenSettingsClicked();
    void onLeaveRequested();

private:
    void setupUi();
    void updateUserDisplay();

    // Header user area
    QWidget     *m_userContainer  = nullptr;
    QLabel      *m_avatarLabel    = nullptr;
    QLabel      *m_nicknameLabel  = nullptr;

    // Central widgets
    MeetingWindow *m_meetingWindow = nullptr;

    // User data
    QString m_nickname = "用户";
    QPixmap m_avatar;
};
