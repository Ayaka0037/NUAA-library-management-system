#include "returnwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <loghelper.h>

ReturnWindow::ReturnWindow(int operatorId, QWidget *parent)
    : QWidget(parent), m_operatorId(operatorId), m_currentReaderId(-1)
{
    setupUI();
}

void ReturnWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("📤");
    QLabel *hTitle = new QLabel("还书");
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

    QHBoxLayout *searchLayout = new QHBoxLayout();
    static const char *kPrimaryStyle =
        "QPushButton {"
        "  background-color: #3498db; color: white; border: none;"
        "  border-radius: 4px; padding: 6px 16px; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1c6ea4; }"
        "QPushButton:disabled { background-color: #bdc3c7; color: #ecf0f1; }";

    QLabel *label = new QLabel("读者证号:");
    m_readerCodeEdit = new QLineEdit();
    m_searchBtn = new QPushButton("查询未还图书");
    m_searchBtn->setStyleSheet(kPrimaryStyle);
    searchLayout->addWidget(label);
    searchLayout->addWidget(m_readerCodeEdit);
    searchLayout->addWidget(m_searchBtn);

    m_borrowTable = new QTableWidget();
    m_borrowTable->setColumnCount(5);
    m_borrowTable->setHorizontalHeaderLabels({"记录ID", "图书ID", "书名", "借书日期", "应还日期"});
    m_borrowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_borrowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_borrowTable->setSelectionMode(QAbstractItemView::SingleSelection);

    m_returnBtn = new QPushButton("归还所选图书");
    m_returnBtn->setStyleSheet(kPrimaryStyle);
    m_returnBtn->setEnabled(false);

    bodyLayout->addLayout(searchLayout);
    bodyLayout->addWidget(m_borrowTable);
    bodyLayout->addWidget(m_returnBtn);

    mainLayout->addWidget(body, 1);

    connect(m_searchBtn, &QPushButton::clicked, this, &ReturnWindow::onSearchReader);
    connect(m_returnBtn, &QPushButton::clicked, this, &ReturnWindow::onReturnBook);
    connect(m_borrowTable, &QTableWidget::itemSelectionChanged, [this](){
        m_returnBtn->setEnabled(m_borrowTable->currentRow() >= 0);
    });
}

void ReturnWindow::onSearchReader()
{
    QString code = m_readerCodeEdit->text().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入读者证号");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT reader_id FROM readers WHERE reader_code = ?");
    query.addBindValue(code);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "提示", "未找到该读者");
        m_currentReaderId = -1;
        m_borrowTable->setRowCount(0);
        m_returnBtn->setEnabled(false);
        return;
    }

    m_currentReaderId = query.value("reader_id").toInt();
    loadBorrowList(m_currentReaderId);
}

void ReturnWindow::loadBorrowList(int readerId)
{
    QSqlQuery query;
    query.prepare("SELECT br.record_id, br.book_id, b.title, br.borrow_date, br.due_date "
                  "FROM borrow_records br "
                  "JOIN books b ON br.book_id = b.book_id "
                  "WHERE br.reader_id = ? AND br.status = 'borrowed'");
    query.addBindValue(readerId);
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询借阅记录失败：" + query.lastError().text());
        return;
    }

    m_borrowTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_borrowTable->insertRow(row);
        m_borrowTable->setItem(row, 0, new QTableWidgetItem(query.value("record_id").toString()));
        m_borrowTable->setItem(row, 1, new QTableWidgetItem(query.value("book_id").toString()));
        m_borrowTable->setItem(row, 2, new QTableWidgetItem(query.value("title").toString()));
        m_borrowTable->setItem(row, 3, new QTableWidgetItem(query.value("borrow_date").toDate().toString("yyyy-MM-dd")));
        m_borrowTable->setItem(row, 4, new QTableWidgetItem(query.value("due_date").toDate().toString("yyyy-MM-dd")));
        row++;
    }

    if (row == 0) {
        QMessageBox::information(this, "提示", "该读者没有未还图书");
    }
}

void ReturnWindow::onReturnBook()
{
    int currentRow = m_borrowTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要归还的图书");
        return;
    }

    int recordId = m_borrowTable->item(currentRow, 0)->text().toInt();
    int bookId = m_borrowTable->item(currentRow, 1)->text().toInt();
    QString title = m_borrowTable->item(currentRow, 2)->text();
    QDate dueDate = QDate::fromString(m_borrowTable->item(currentRow, 4)->text(), "yyyy-MM-dd");
    QDate returnDate = QDate::currentDate();

    double fine = 0;
    if (returnDate > dueDate) {
        int days = dueDate.daysTo(returnDate);
        fine = days * 0.5; // 每天0.5元
    }
    LogHelper::record(m_operatorId, "还书",
                      QString("记录ID:%1 图书:%2 罚款:%3元")
                          .arg(recordId).arg(title).arg(fine));
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    // 更新借阅记录
    query.prepare("UPDATE borrow_records SET return_date = ?, status = 'returned', fine_amount = ? WHERE record_id = ?");
    query.addBindValue(returnDate);
    query.addBindValue(fine);
    query.addBindValue(recordId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "还书失败(1): " + query.lastError().text());
        return;
    }

    // 增加图书可借数量
    query.prepare("UPDATE books SET available_count = available_count + 1 WHERE book_id = ?");
    query.addBindValue(bookId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "还书失败(2): " + query.lastError().text());
        return;
    }

    // 减少读者当前借阅数量
    query.prepare("UPDATE readers SET borrowed_count = borrowed_count - 1 WHERE reader_id = ?");
    query.addBindValue(m_currentReaderId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "还书失败(3): " + query.lastError().text());
        return;
    }

    // 记录操作日志
    query.prepare("INSERT INTO operation_logs (user_id, operation_type, operation_detail) "
                  "VALUES (?, '还书', ?)");
    query.addBindValue(m_operatorId);
    query.addBindValue(QString("记录ID:%1 图书:%2 罚款:%3元").arg(recordId).arg(title).arg(fine));
    query.exec();

    db.commit();

    QMessageBox::information(this, "成功", QString("还书成功！%1\n逾期罚款：%2元").arg(title).arg(fine));
    loadBorrowList(m_currentReaderId);  // 刷新列表
}