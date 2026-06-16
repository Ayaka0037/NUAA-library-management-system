#include "reminderwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ReminderWindow::ReminderWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadOverdue();
    loadDueSoon();
}

void ReminderWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("⏰");
    QLabel *hTitle = new QLabel("到期提醒");
    hIcon->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; background: transparent; border: none;");
    hLayout->addWidget(hIcon);
    hLayout->addWidget(hTitle);
    hLayout->addStretch();
    mainLayout->addWidget(header);

    QWidget *body = new QWidget();
    body->setStyleSheet("background: #f5f6fa;");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(16, 12, 16, 12);

    // 顶部刷新行
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    m_refreshBtn = new QPushButton("刷新");
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1c6ea4;"
        "}");
    topLayout->addWidget(m_refreshBtn);

    // 标签页
    m_tabWidget = new QTabWidget();

    // --- 已逾期表格 ---
    m_overdueTable = new QTableWidget();
    m_overdueTable->setColumnCount(7);
    m_overdueTable->setHorizontalHeaderLabels({
        "记录ID", "读者姓名", "书名", "借阅日期", "应还日期", "逾期天数", "预估罚款(元)"
    });
    m_overdueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_overdueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_overdueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_overdueTable->setSelectionMode(QAbstractItemView::SingleSelection);

    m_exportOverdueBtn = new QPushButton("导出逾期列表");
    m_exportOverdueBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: white;"
        "  color: #3498db;"
        "  border: 1px solid #3498db;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3498db;"
        "  color: white;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #2980b9;"
        "  color: white;"
        "}");

    // --- 即将到期表格 ---
    m_dueSoonTable = new QTableWidget();
    m_dueSoonTable->setColumnCount(6);
    m_dueSoonTable->setHorizontalHeaderLabels({
        "记录ID", "读者姓名", "书名", "借阅日期", "应还日期", "剩余天数"
    });
    m_dueSoonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dueSoonTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dueSoonTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dueSoonTable->setSelectionMode(QAbstractItemView::SingleSelection);

    m_exportDueSoonBtn = new QPushButton("导出即将到期列表");
    m_exportDueSoonBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: white;"
        "  color: #3498db;"
        "  border: 1px solid #3498db;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3498db;"
        "  color: white;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #2980b9;"
        "  color: white;"
        "}");

    // 将表格包装在布局中
    QWidget *overdueTab = new QWidget();
    QVBoxLayout *overdueLayout = new QVBoxLayout(overdueTab);
    overdueLayout->addWidget(m_overdueTable);
    QHBoxLayout *overdueBtnRow = new QHBoxLayout();
    overdueBtnRow->addStretch();
    overdueBtnRow->addWidget(m_exportOverdueBtn);
    overdueLayout->addLayout(overdueBtnRow);

    QWidget *dueSoonTab = new QWidget();
    QVBoxLayout *dueSoonLayout = new QVBoxLayout(dueSoonTab);
    dueSoonLayout->addWidget(m_dueSoonTable);
    QHBoxLayout *dueSoonBtnRow = new QHBoxLayout();
    dueSoonBtnRow->addStretch();
    dueSoonBtnRow->addWidget(m_exportDueSoonBtn);
    dueSoonLayout->addLayout(dueSoonBtnRow);

    m_tabWidget->addTab(overdueTab, "已逾期");
    m_tabWidget->addTab(dueSoonTab, "即将到期 (7天内)");

    bodyLayout->addLayout(topLayout);
    bodyLayout->addWidget(m_tabWidget);

    mainLayout->addWidget(body, 1);

    // 信号连接
    connect(m_refreshBtn,        &QPushButton::clicked, this, &ReminderWindow::onRefresh);
    connect(m_exportOverdueBtn,  &QPushButton::clicked, this, &ReminderWindow::onExportOverdue);
    connect(m_exportDueSoonBtn,  &QPushButton::clicked, this, &ReminderWindow::onExportDueSoon);
}

void ReminderWindow::loadOverdue()
{
    QSqlQuery query;
    if (!query.exec(
        "SELECT br.record_id, r.name AS reader_name, b.title AS book_title, "
        "br.borrow_date, br.due_date, "
        "DATEDIFF(CURDATE(), br.due_date) AS overdue_days "
        "FROM borrow_records br "
        "JOIN readers r ON br.reader_id = r.reader_id "
        "JOIN books b ON br.book_id = b.book_id "
        "WHERE br.status = 'borrowed' AND br.due_date < CURDATE() "
        "ORDER BY overdue_days DESC")) {
        QMessageBox::critical(this, "错误", "加载逾期列表失败：" + query.lastError().text());
        return;
    }

    m_overdueTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_overdueTable->insertRow(row);
        m_overdueTable->setItem(row, 0, new QTableWidgetItem(query.value("record_id").toString()));
        m_overdueTable->setItem(row, 1, new QTableWidgetItem(query.value("reader_name").toString()));
        m_overdueTable->setItem(row, 2, new QTableWidgetItem(query.value("book_title").toString()));
        m_overdueTable->setItem(row, 3, new QTableWidgetItem(query.value("borrow_date").toString()));
        m_overdueTable->setItem(row, 4, new QTableWidgetItem(query.value("due_date").toString()));
        int overdueDays = query.value("overdue_days").toInt();
        m_overdueTable->setItem(row, 5, new QTableWidgetItem(QString::number(overdueDays)));
        double fine = overdueDays * 0.5;
        m_overdueTable->setItem(row, 6, new QTableWidgetItem(QString::number(fine, 'f', 1)));

        // 红色背景标记
        for (int col = 0; col < 7; ++col)
            m_overdueTable->item(row, col)->setBackground(QColor(255, 230, 230));
        row++;
    }
}

void ReminderWindow::loadDueSoon()
{
    QSqlQuery query;
    if (!query.exec(
        "SELECT br.record_id, r.name AS reader_name, b.title AS book_title, "
        "br.borrow_date, br.due_date, "
        "DATEDIFF(br.due_date, CURDATE()) AS days_remaining "
        "FROM borrow_records br "
        "JOIN readers r ON br.reader_id = r.reader_id "
        "JOIN books b ON br.book_id = b.book_id "
        "WHERE br.status = 'borrowed' "
        "AND br.due_date >= CURDATE() "
        "AND br.due_date <= DATE_ADD(CURDATE(), INTERVAL 7 DAY) "
        "ORDER BY days_remaining ASC")) {
        QMessageBox::critical(this, "错误", "加载即将到期列表失败：" + query.lastError().text());
        return;
    }

    m_dueSoonTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_dueSoonTable->insertRow(row);
        m_dueSoonTable->setItem(row, 0, new QTableWidgetItem(query.value("record_id").toString()));
        m_dueSoonTable->setItem(row, 1, new QTableWidgetItem(query.value("reader_name").toString()));
        m_dueSoonTable->setItem(row, 2, new QTableWidgetItem(query.value("book_title").toString()));
        m_dueSoonTable->setItem(row, 3, new QTableWidgetItem(query.value("borrow_date").toString()));
        m_dueSoonTable->setItem(row, 4, new QTableWidgetItem(query.value("due_date").toString()));
        int daysRemaining = query.value("days_remaining").toInt();
        m_dueSoonTable->setItem(row, 5, new QTableWidgetItem(QString::number(daysRemaining)));

        // ≤3天黄色背景
        if (daysRemaining <= 3) {
            for (int col = 0; col < 6; ++col)
                m_dueSoonTable->item(row, col)->setBackground(QColor(255, 255, 200));
        }
        row++;
    }
}

void ReminderWindow::onRefresh()
{
    loadOverdue();
    loadDueSoon();
}

void ReminderWindow::exportTable(QTableWidget *table, const QString &defaultName)
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出列表", defaultName, "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件：" + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << QChar(0xFEFF);  // UTF-8 BOM

    QStringList headers;
    for (int col = 0; col < table->columnCount(); ++col)
        headers << "\"" + table->horizontalHeaderItem(col)->text() + "\"";
    out << headers.join(",") << "\n";

    for (int row = 0; row < table->rowCount(); ++row) {
        QStringList cols;
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            QString val = item ? item->text() : "";
            val.replace("\"", "\"\"");
            cols << "\"" + val + "\"";
        }
        out << cols.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "成功", "数据已导出到 " + filePath);
}

void ReminderWindow::onExportOverdue()
{
    exportTable(m_overdueTable, "overdue_books.csv");
}

void ReminderWindow::onExportDueSoon()
{
    exportTable(m_dueSoonTable, "due_soon_books.csv");
}
