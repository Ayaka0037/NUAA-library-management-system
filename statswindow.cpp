#include "statswindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

StatsWindow::StatsWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    onRefresh();
}

QLabel *StatsWindow::createCard(const QString &title, QWidget *parent)
{
    QGroupBox *box = new QGroupBox(parent);
    QVBoxLayout *layout = new QVBoxLayout(box);

    QLabel *titleLabel = new QLabel(title);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(10);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #7f8c8d;");

    QLabel *valueLabel = new QLabel("0");
    QFont valueFont = valueLabel->font();
    valueFont.setPointSize(28);
    valueFont.setBold(true);
    valueLabel->setFont(valueFont);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("color: #2c3e50;");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    // 设置固定大小确保卡片风格一致
    box->setMinimumSize(150, 120);
    box->setStyleSheet("QGroupBox { border: 1px solid #ddd; border-radius: 6px; "
                       "background: #fafafa; margin-top: 10px; }");

    // 返回 valueLabel 以便后续更新
    return valueLabel;
}

void StatsWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("📊");
    QLabel *hTitle = new QLabel("统计看板");
    hIcon->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; background: transparent; border: none;");
    hLayout->addWidget(hIcon);
    hLayout->addWidget(hTitle);
    hLayout->addStretch();
    mainLayout->addWidget(header);

    QWidget *body = new QWidget();
    body->setStyleSheet("background: #f5f6fa;");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(20, 16, 20, 16);

    // 顶部刷新
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    m_refreshBtn = new QPushButton("刷新");
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db; color: white; border: none;"
        "  border-radius: 4px; padding: 6px 16px; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1c6ea4; }");
    topLayout->addWidget(m_refreshBtn);

    // 卡片网格：2行 x 4列
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(15);

    // Row 1
    QLabel *label;
    label = createCard("馆藏总量", this);
    m_totalBooksValue = label;
    grid->addWidget(label->parentWidget(), 0, 0);

    label = createCard("在借数量", this);
    m_borrowedValue = label;
    grid->addWidget(label->parentWidget(), 0, 1);

    label = createCard("可借数量", this);
    m_availableValue = label;
    grid->addWidget(label->parentWidget(), 0, 2);

    label = createCard("逾期数量", this);
    m_overdueValue = label;
    grid->addWidget(label->parentWidget(), 0, 3);

    // Row 2
    label = createCard("活跃读者", this);
    m_activeReadersValue = label;
    grid->addWidget(label->parentWidget(), 1, 0);

    label = createCard("今日借阅", this);
    m_todayBorrowValue = label;
    grid->addWidget(label->parentWidget(), 1, 1);

    label = createCard("今日归还", this);
    m_todayReturnValue = label;
    grid->addWidget(label->parentWidget(), 1, 2);

    // 第4个留空或放一个空的 QGroupBox
    QGroupBox *placeholder = new QGroupBox(this);
    placeholder->setMinimumSize(150, 120);
    placeholder->setStyleSheet("QGroupBox { border: none; background: transparent; }");
    grid->addWidget(placeholder, 1, 3);

    bodyLayout->addLayout(topLayout);
    bodyLayout->addLayout(grid);
    bodyLayout->addStretch();

    mainLayout->addWidget(body, 1);

    connect(m_refreshBtn, &QPushButton::clicked, this, &StatsWindow::onRefresh);
}

int StatsWindow::querySingleInt(const QString &sql)
{
    QSqlQuery query;
    if (query.exec(sql) && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

void StatsWindow::onRefresh()
{
    m_totalBooksValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM books WHERE status != 'deleted'")));
    m_borrowedValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM borrow_records WHERE status = 'borrowed'")));
    m_availableValue->setText(
        QString::number(querySingleInt("SELECT COALESCE(SUM(available_count),0) FROM books WHERE status != 'deleted'")));
    m_overdueValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM borrow_records WHERE status = 'borrowed' AND due_date < CURDATE()")));
    m_activeReadersValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM readers WHERE status = 'active'")));
    m_todayBorrowValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM borrow_records WHERE DATE(borrow_date) = CURDATE()")));
    m_todayReturnValue->setText(
        QString::number(querySingleInt("SELECT COUNT(*) FROM borrow_records WHERE DATE(return_date) = CURDATE()")));
}
