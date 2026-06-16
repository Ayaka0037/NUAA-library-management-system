#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <loghelper.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), m_userId(-1)
{
    setWindowTitle("图书管理系统 - 登录");
    setModal(true);
    setFixedSize(420, 520);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 全局样式
    setStyleSheet(R"(
        QDialog {
            background: #f0f2f5;
        }
        #cardWidget {
            background: #ffffff;
            border-radius: 12px;
            border: 1px solid #e8eaed;
        }
        #logoIcon {
            background: transparent;
            font-size: 48px;
        }
        #titleLabel {
            background: transparent;
            font-size: 22px;
            font-weight: bold;
            color: #1a1a2e;
        }
        #subtitleLabel {
            background: transparent;
            font-size: 13px;
            color: #95a5a6;
        }
        QLineEdit {
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 14px;
            background: #f8f9fa;
            color: #333;
        }
        QLineEdit:focus {
            border: 1px solid #3498db;
            background: #ffffff;
        }
        QLineEdit:hover {
            border: 1px solid #b0bec5;
        }
        #loginBtn {
            background: #3498db;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 11px;
            font-size: 15px;
            font-weight: bold;
        }
        #loginBtn:hover {
            background: #2980b9;
        }
        #loginBtn:pressed {
            background: #2471a3;
        }
        #cancelBtn {
            background: transparent;
            color: #7f8c8d;
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            padding: 10px;
            font-size: 14px;
        }
        #cancelBtn:hover {
            background: #f5f6fa;
            color: #555;
        }
        #errorLabel {
            background: #fef0f0;
            color: #e74c3c;
            border: 1px solid #fde2e2;
            border-radius: 4px;
            padding: 8px 12px;
            font-size: 13px;
        }
        #fieldLabel {
            background: transparent;
            font-size: 13px;
            color: #606266;
            padding-bottom: 2px;
        }
    )");

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);

    // --- 顶部 Logo + 标题 ---
    QLabel *logoIcon = new QLabel("📚");
    logoIcon->setObjectName("logoIcon");
    logoIcon->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("图书管理系统");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("Library Management System");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addSpacing(10);
    mainLayout->addWidget(logoIcon);
    mainLayout->addSpacing(4);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(24);

    // --- 白色卡片包含表单 ---
    QFrame *card = new QFrame();
    card->setObjectName("cardWidget");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 20);
    cardLayout->setSpacing(0);

    // 加轻微阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 30));
    card->setGraphicsEffect(shadow);

    // 用户名标签
    QLabel *userLabel = new QLabel("用户名");
    userLabel->setObjectName("fieldLabel");
    cardLayout->addWidget(userLabel);
    cardLayout->addSpacing(4);
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("请输入用户名");
    cardLayout->addWidget(m_usernameEdit);

    cardLayout->addSpacing(16);

    // 密码标签
    QLabel *passLabel = new QLabel("密码");
    passLabel->setObjectName("fieldLabel");
    cardLayout->addWidget(passLabel);
    cardLayout->addSpacing(4);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");
    cardLayout->addWidget(m_passwordEdit);

    // 错误提示
    cardLayout->addSpacing(12);
    m_errorLabel = new QLabel();
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    cardLayout->addWidget(m_errorLabel);

    // 登录按钮
    cardLayout->addSpacing(16);
    m_loginBtn = new QPushButton("登  录");
    m_loginBtn->setObjectName("loginBtn");
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(m_loginBtn);

    // 取消按钮
    cardLayout->addSpacing(10);
    m_cancelBtn = new QPushButton("取  消");
    m_cancelBtn->setObjectName("cancelBtn");
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(m_cancelBtn);

    mainLayout->addWidget(card);
    mainLayout->addStretch();

    // 信号
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LoginDialog::onCancel);

    // 回车直接登录
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, [this]() {
        m_passwordEdit->setFocus();
    });
}

void LoginDialog::onLogin()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();
    if (username.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText("用户名和密码不能为空");
        m_errorLabel->setVisible(true);
        return;
    }
    LogHelper::record(m_userId, "登录", "用户登录系统");
    QSqlQuery query;
    query.prepare("SELECT user_id, role, real_name FROM users WHERE username = ? AND password = ? AND status = 'active'");
    query.addBindValue(username);
    query.addBindValue(password);
    if (!query.exec()) {
        m_errorLabel->setText("查询失败：" + query.lastError().text());
        m_errorLabel->setVisible(true);
        return;
    }
    if (query.next()) {
        m_userId = query.value("user_id").toInt();
        m_userRole = query.value("role").toString();
        m_userName = query.value("real_name").toString();
        accept();
    } else {
        m_errorLabel->setText("用户名或密码错误，或账户已禁用");
        m_errorLabel->setVisible(true);
    }
}

void LoginDialog::onCancel()
{
    reject();
}
