#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    int getUserId() const { return m_userId; }
    QString getUserRole() const { return m_userRole; }
    QString getUserName() const { return m_userName; }

private slots:
    void onLogin();
    void onCancel();

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_cancelBtn;
    QLabel *m_errorLabel;

    int m_userId;
    QString m_userRole;
    QString m_userName;
};

#endif