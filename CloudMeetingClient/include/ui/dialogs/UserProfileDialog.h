#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

/**
 * @file UserProfileDialog.h
 * @brief 声明用户资料编辑对话框。
 */

/**
 * @class UserProfileDialog
 * @brief 提供头像与昵称编辑界面。
 */
class UserProfileDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief 构造用户资料对话框。
     * @param[in] parent 父窗口指针。
     */
    explicit UserProfileDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取当前昵称输入值。
     * @return 去除首尾空白后的昵称文本。
     */
    QString nickname() const;

    /**
     * @brief 获取当前头像。
     * @return 当前头像图像。
     */
    QPixmap avatar() const;

    /**
     * @brief 设置昵称输入框内容。
     * @param[in] name 待显示的昵称文本。
     */
    void setNickname(const QString &name);

    /**
     * @brief 设置当前头像并刷新预览。
     * @param[in] pix 待显示的头像图像。
     */
    void setAvatar(const QPixmap &pix);

private slots:
    /**
     * @brief 打开文件选择框并更新头像。
     */
    void onSelectAvatarClicked();

private:
    /**
     * @brief 初始化用户资料对话框界面。
     */
    void setupUi();

    QLabel      *m_avatarLabel  = nullptr; ///< 头像预览标签。
    QPushButton *m_avatarBtn    = nullptr; ///< 选择头像按钮。
    QLineEdit   *m_nicknameEdit = nullptr; ///< 昵称输入框。
    QPushButton *m_saveBtn      = nullptr; ///< 保存按钮。
    QPushButton *m_cancelBtn    = nullptr; ///< 取消按钮。
    QPixmap      m_avatarPixmap;           ///< 当前头像缓存。
};
