#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class UserProfileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserProfileDialog(QWidget *parent = nullptr);

    QString nickname() const;
    QPixmap avatar() const;

    void setNickname(const QString &name);
    void setAvatar(const QPixmap &pix);

private slots:
    void onSelectAvatarClicked();

private:
    void setupUi();

    QLabel      *m_avatarLabel  = nullptr;
    QPushButton *m_avatarBtn    = nullptr;
    QLineEdit   *m_nicknameEdit = nullptr;
    QPushButton *m_saveBtn      = nullptr;
    QPushButton *m_cancelBtn    = nullptr;
    QPixmap      m_avatarPixmap;
};
