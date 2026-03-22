#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class JoinMeetingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JoinMeetingDialog(QWidget *parent = nullptr);

    QString roomId()   const;
    QString password() const;

    // 展示密码子弹窗（可手动触发）
    bool promptPassword();

private slots:
    void onJoinClicked();

private:
    void setupUi();

    QLineEdit   *m_roomIdEdit   = nullptr;
    QPushButton *m_joinBtn      = nullptr;
    QPushButton *m_cancelBtn    = nullptr;
    QString      m_password;
};
