#include "mainmenu.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QPixmap>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QSqlQuery>

#include "borrowwindow.h"
#include "returnwindow.h"
#include "bookmanagewindow.h"
#include "readermanagewindow.h"
#include "reminderwindow.h"
#include "statswindow.h"
#include "logwindow.h"

// 在多个目录下查找图片文件
static QString findImage(const QString &filename)
{
    QStringList searchDirs;
    searchDirs << QApplication::applicationDirPath()           // exe 同目录 (release/)
               << QDir::currentPath()                          // 工作目录
               << QApplication::applicationDirPath() + "/..";  // 项目根目录
    for (const QString &dir : searchDirs) {
        QString path = QDir(dir).filePath(filename);
        if (QFileInfo::exists(path)) return QDir::cleanPath(path);
    }
    return QString();
}

MainMenu::MainMenu(int userId, const QString &userRole, QWidget *parent)
    : QWidget(parent), m_userId(userId), m_userRole(userRole)
{
    setupUI();
    switchPage(0);  // 默认显示欢迎页
}

// ==================== 创建导航按钮 ====================
QPushButton *MainMenu::createNavButton(const QString &text)
{
    QPushButton *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(42);
    return btn;
}

// ==================== 构建左侧导航栏 ====================
void MainMenu::setupSidebar(QWidget *sidebar)
{
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(200);

    QVBoxLayout *navLayout = new QVBoxLayout(sidebar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);

    // --- Logo 区域（预留图片位置） ---
    QWidget *logoArea = new QWidget();
    logoArea->setObjectName("logoArea");
    logoArea->setMinimumHeight(160);
    QVBoxLayout *logoLayout = new QVBoxLayout(logoArea);
    logoLayout->setAlignment(Qt::AlignCenter);

    // Logo 图标：在多个目录下查找
    QString logoPath = findImage("logo.png");
    if (logoPath.isEmpty()) logoPath = findImage("logo.jpg");
    QLabel *iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setMinimumSize(60, 60);
    if (!logoPath.isEmpty()) {
        QPixmap pix(logoPath);
        iconLabel->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText("📚");
        iconLabel->setStyleSheet("font-size: 42px; color: #ecf0f1; background: transparent;");
    }

    // 标题
    QLabel *titleLabel = new QLabel("图书管理系统");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #ecf0f1; "
        "background: transparent; padding-top: 6px;");

    // 副标题（预留）
    QLabel *subtitleLabel = new QLabel("Library Manager");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "font-size: 10px; color: #95a5a6; background: transparent;");

    logoLayout->addWidget(iconLabel);
    logoLayout->addWidget(titleLabel);
    logoLayout->addWidget(subtitleLabel);
    navLayout->addWidget(logoArea);

    // --- 分割线 ---
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #34495e; max-height: 1px; margin: 0 12px;");
    navLayout->addWidget(line);

    // --- 导航按钮 ---
    const char *labels[] = {
        "🏠  首    页", "📥  借    书", "📤  还    书",
        "📖  图书管理", "👤  读者管理",
        "⏰  到期提醒", "📊  统计看板", "📋  操作日志"
    };

    for (int i = 0; i < 8; ++i) {
        m_navButtons[i] = createNavButton(labels[i]);
        navLayout->addWidget(m_navButtons[i]);
        connect(m_navButtons[i], &QPushButton::clicked, this, [this, i]() {
            switchPage(i);
        });
    }

    // 非管理员隐藏图书管理和读者管理
    if (m_userRole != "admin") {
        m_navButtons[3]->setVisible(false);  // 图书管理
        m_navButtons[4]->setVisible(false);  // 读者管理
    }

    navLayout->addStretch();

    // --- 退出按钮 ---
    QPushButton *exitBtn = new QPushButton("🚪  退    出");
    exitBtn->setCursor(Qt::PointingHandCursor);
    exitBtn->setMinimumHeight(44);
    exitBtn->setObjectName("exitBtn");
    navLayout->addWidget(exitBtn);
    connect(exitBtn, &QPushButton::clicked, this, &MainMenu::onExit);
}

// ==================== 构建右侧页面 ====================
void MainMenu::setupPages()
{
    m_stack->setObjectName("contentArea");

    // 0: 欢迎首页
    setupWelcomePage();
    m_stack->addWidget(m_welcomePage);

    // 1-7: 功能页（创建时 parent 设为 m_stack，嵌入而非弹窗）
    m_borrowPage   = new BorrowWindow(m_userId, m_stack);
    m_returnPage   = new ReturnWindow(m_userId, m_stack);
    m_bookPage     = new BookManageWindow(m_userId, m_stack);
    m_readerPage   = new ReaderManageWindow(m_userId, m_stack);
    m_reminderPage = new ReminderWindow(m_stack);
    m_statsPage    = new StatsWindow(m_stack);
    m_logPage      = new LogWindow(m_stack);

    m_stack->addWidget(m_borrowPage);    // 1
    m_stack->addWidget(m_returnPage);    // 2
    m_stack->addWidget(m_bookPage);      // 3
    m_stack->addWidget(m_readerPage);    // 4
    m_stack->addWidget(m_reminderPage);  // 5
    m_stack->addWidget(m_statsPage);     // 6
    m_stack->addWidget(m_logPage);       // 7
}

// ==================== 欢迎首页 ====================
void MainMenu::setupWelcomePage()
{
    m_welcomePage = new QWidget();
    m_welcomePage->setStyleSheet("background: #f5f6fa;");

    QVBoxLayout *outerLayout = new QVBoxLayout(m_welcomePage);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // --- 顶部 Banner 区域 ---
    QWidget *banner = new QWidget();
    banner->setMinimumHeight(200);
    banner->setMaximumHeight(280);
    banner->setAutoFillBackground(true);

    // 用 QPalette 设置背景图（比 QSS url 可靠）
    QString bgPath = findImage("banner.jpg");
    if (bgPath.isEmpty()) bgPath = findImage("banner.png");
    if (!bgPath.isEmpty()) {
        QPixmap bg(bgPath);
        bg = bg.scaled(1200, 280, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPalette pal = banner->palette();
        pal.setBrush(QPalette::Window, QBrush(bg));
        banner->setPalette(pal);
    } else {
        banner->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #2c3e50, stop:1 #3498db);");
    }

    QVBoxLayout *bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setAlignment(Qt::AlignCenter);

    QLabel *bigIcon = new QLabel("📚");
    bigIcon->setAlignment(Qt::AlignCenter);
    bigIcon->setStyleSheet("font-size: 52px; background: transparent;");

    QLabel *welcomeLabel = new QLabel("欢迎使用图书管理系统");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #ffffff; background: transparent;");

    QLabel *hintLabel = new QLabel("请从左侧导航选择功能模块");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet(
        "font-size: 13px; color: rgba(255,255,255,0.7); background: transparent;");

    bannerLayout->addWidget(bigIcon);
    bannerLayout->addWidget(welcomeLabel);
    bannerLayout->addWidget(hintLabel);
    outerLayout->addWidget(banner);

    // --- 统计卡片区（居中） ---
    QVBoxLayout *bottomLayout = new QVBoxLayout();
    bottomLayout->setAlignment(Qt::AlignCenter);
    bottomLayout->setContentsMargins(20, 20, 20, 20);

    QWidget *centerWidget = new QWidget();
    centerWidget->setMaximumWidth(700);
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);
    layout->setSpacing(20);
    QGridLayout *cardGrid = new QGridLayout();
    cardGrid->setSpacing(12);

    // 查询统计数据的 lambda
    auto queryCount = [](const QString &sql) -> QString {
        QSqlQuery q;
        if (q.exec(sql) && q.next()) return q.value(0).toString();
        return "0";
    };

    struct CardInfo {
        QString emoji, title, sql;
    };
    CardInfo cards[] = {
        {"📖", "馆藏总量", "SELECT COUNT(*) FROM books WHERE status != 'deleted'"},
        {"📥", "在借数量", "SELECT COUNT(*) FROM borrow_records WHERE status = 'borrowed'"},
        {"⏰", "逾期数量", "SELECT COUNT(*) FROM borrow_records WHERE status = 'borrowed' AND due_date < CURDATE()"},
        {"👤", "活跃读者", "SELECT COUNT(*) FROM readers WHERE status = 'active'"},
    };

    for (int i = 0; i < 4; ++i) {
        QGroupBox *box = new QGroupBox();
        box->setMinimumSize(130, 90);
        box->setStyleSheet(
            "QGroupBox { background: white; border: 1px solid #e0e0e0; border-radius: 8px; }");
        QVBoxLayout *boxLayout = new QVBoxLayout(box);
        boxLayout->setAlignment(Qt::AlignCenter);
        boxLayout->setSpacing(4);

        QLabel *icon = new QLabel(cards[i].emoji);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("font-size: 24px; background: transparent;");

        QLabel *num = new QLabel(queryCount(cards[i].sql));
        num->setAlignment(Qt::AlignCenter);
        num->setStyleSheet(
            "font-size: 22px; font-weight: bold; color: #2c3e50; background: transparent;");

        QLabel *title = new QLabel(cards[i].title);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet(
            "font-size: 11px; color: #7f8c8d; background: transparent;");

        boxLayout->addWidget(icon);
        boxLayout->addWidget(num);
        boxLayout->addWidget(title);
        cardGrid->addWidget(box, 0, i);
    }

    layout->addLayout(cardGrid);

    bottomLayout->addWidget(centerWidget);
    outerLayout->addLayout(bottomLayout);
    outerLayout->addStretch();
}

// ==================== 组装主界面 ====================
void MainMenu::setupUI()
{
    setWindowTitle("图书管理系统");
    resize(1150, 700);
    setMinimumSize(960, 620);

    // 全局 QSS 样式
    setStyleSheet(R"(
        #sidebar {
            background-color: #2c3e50;
        }
        #logoArea {
            background-color: #1a252f;
        }
        #contentArea {
            background-color: #f5f6fa;
        }
        QPushButton {
            background: transparent;
            color: #bdc3c7;
            border: none;
            text-align: left;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #34495e;
            color: #ecf0f1;
        }
        QPushButton:checked {
            background-color: #3498db;
            color: #ffffff;
            font-weight: bold;
        }
        #exitBtn {
            color: #e74c3c;
        }
        #exitBtn:hover {
            background-color: #c0392b;
            color: #ffffff;
        }
    )");

    // 根布局
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 左侧导航栏
    QWidget *sidebar = new QWidget();
    setupSidebar(sidebar);
    rootLayout->addWidget(sidebar);

    // 右侧页面容器
    m_stack = new QStackedWidget();
    setupPages();
    rootLayout->addWidget(m_stack, 1);
}

// ==================== 页面切换 ====================
void MainMenu::switchPage(int index)
{
    m_stack->setCurrentIndex(index);

    // 更新导航按钮选中状态
    for (int i = 0; i < 8; ++i) {
        m_navButtons[i]->setChecked(i == index);
    }

    // 各功能页面在构造时已初始化数据，无需额外刷新
    Q_UNUSED(index);
}

// ==================== 退出 ====================
void MainMenu::onExit()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认退出", "确定要退出系统吗？",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}
