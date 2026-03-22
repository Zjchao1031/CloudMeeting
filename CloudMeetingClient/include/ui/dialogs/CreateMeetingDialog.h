#pragma once
#include <QDialog>
#include <QIntValidator>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

/**
 * @file CreateMeetingDialog.h
 * @brief 声明创建会议对话框。
 */

/**
 * @class CreateMeetingDialog
 * @brief 提供会议创建参数输入界面。
 */
class CreateMeetingDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief 构造创建会议对话框。
     * @param[in] parent 父窗口指针。
     */
    explicit CreateMeetingDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取用户设置的最大参会人数。
     * @return 限制在有效范围内的最大参会人数。
     */
    int maxMembers() const;

    /**
     * @brief 判断是否启用了会议密码。
     * @return 若启用了密码则返回 `true`，否则返回 `false`。
     */
    bool hasPassword() const;

    /**
     * @brief 获取用户输入的会议密码。
     * @return 当前密码输入框中的文本内容。
     */
    QString password() const;

private slots:
    /**
     * @brief 响应密码开关状态变化。
     * @param[in] state 复选框当前状态值。
     */
    void onPasswordToggleChanged(int state);

private:
    /**
     * @brief 初始化创建会议对话框界面。
     */
    void setupUi();

    QLineEdit   *m_maxMembersEdit = nullptr; ///< 最大参会人数输入框。
    QCheckBox   *m_passwordToggle = nullptr; ///< 是否启用会议密码的开关。
    QLineEdit   *m_passwordEdit   = nullptr; ///< 会议密码输入框。
    QLabel      *m_passwordLabel  = nullptr; ///< 会议密码输入框标题。
    QPushButton *m_confirmBtn     = nullptr; ///< 确认创建按钮。
    QPushButton *m_cancelBtn      = nullptr; ///< 取消按钮。
};
