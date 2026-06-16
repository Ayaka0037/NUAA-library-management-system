#include "borrowwindow.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QList>
#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace {
const char *kPrimaryStyle =
    "QPushButton {"
    "  background-color: #3498db; color: white; border: none;"
    "  border-radius: 4px; padding: 6px 16px; font-size: 13px;"
    "}"
    "QPushButton:hover { background-color: #2980b9; }"
    "QPushButton:pressed { background-color: #1c6ea4; }"
    "QPushButton:disabled { background-color: #bdc3c7; color: #ecf0f1; }";
const char *kCancelStyle =
    "QPushButton {"
    "  background-color: white; color: #7f8c8d;"
    "  border: 1px solid #bdc3c7; border-radius: 4px;"
    "  padding: 6px 16px; font-size: 13px;"
    "}"
    "QPushButton:hover { background-color: #ecf0f1; }"
    "QPushButton:pressed { background-color: #d5dbdb; }";
} // namespace

BorrowWindow::BorrowWindow(int operatorId, QWidget *parent)
    : QWidget(parent), m_operatorId(operatorId), m_currentReaderId(-1), m_currentBookId(-1)
{
    setupUI();
}

void BorrowWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("📥");
    QLabel *hTitle = new QLabel("借书");
    hIcon->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; background: transparent; border: none;");
    hLayout->addWidget(hIcon);
    hLayout->addWidget(hTitle);
    hLayout->addStretch();
    mainLayout->addWidget(header);

    // 内容区
    QWidget *body = new QWidget();
    body->setStyleSheet("background: #f5f6fa;");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(16, 12, 16, 12);

    // === 读者区域 ===
    QGroupBox *readerGroup = new QGroupBox("读者信息");
    QFormLayout *readerForm = new QFormLayout();

    QHBoxLayout *readerCodeLayout = new QHBoxLayout();
    m_readerCodeEdit = new QLineEdit();
    m_searchReaderBtn = new QPushButton("查询");
    m_searchReaderBtn->setStyleSheet(kPrimaryStyle);
    readerCodeLayout->addWidget(m_readerCodeEdit);
    readerCodeLayout->addWidget(m_searchReaderBtn);
    readerForm->addRow("读者证号:", readerCodeLayout);

    m_readerNameLabel = new QLabel();
    m_readerStatusLabel = new QLabel();
    m_borrowedCountLabel = new QLabel();
    readerForm->addRow("姓名:", m_readerNameLabel);
    readerForm->addRow("状态:", m_readerStatusLabel);
    readerForm->addRow("当前借阅:", m_borrowedCountLabel);

    readerGroup->setLayout(readerForm);

    // === 图书区域 ===
    QGroupBox *bookGroup = new QGroupBox("图书信息");
    QFormLayout *bookForm = new QFormLayout();

    QHBoxLayout *bookLayout = new QHBoxLayout();
    m_bookKeywordEdit = new QLineEdit();
    m_searchBookBtn = new QPushButton("查询");
    m_searchBookBtn->setStyleSheet(kPrimaryStyle);
    bookLayout->addWidget(m_bookKeywordEdit);
    bookLayout->addWidget(m_searchBookBtn);
    bookForm->addRow("图书名称/作者/ISBN:", bookLayout);

    m_bookTitleLabel = new QLabel();
    m_bookAvailableLabel = new QLabel();
    bookForm->addRow("书名:", m_bookTitleLabel);
    bookForm->addRow("可借数量:", m_bookAvailableLabel);

    bookGroup->setLayout(bookForm);

    m_borrowBtn = new QPushButton("确认借书");
    m_borrowBtn->setStyleSheet(kPrimaryStyle);
    m_borrowBtn->setEnabled(false);

    bodyLayout->addWidget(readerGroup);
    bodyLayout->addWidget(bookGroup);
    bodyLayout->addWidget(m_borrowBtn);

    mainLayout->addWidget(body, 1);

    connect(m_searchReaderBtn, &QPushButton::clicked, this, &BorrowWindow::onSearchReader);
    connect(m_searchBookBtn, &QPushButton::clicked, this, &BorrowWindow::onSearchBook);
    connect(m_borrowBtn, &QPushButton::clicked, this, &BorrowWindow::onConfirmBorrow);
}

void BorrowWindow::onSearchReader()
{
    QString code = m_readerCodeEdit->text().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入读者证号");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT reader_id, name, status, borrowed_count FROM readers WHERE reader_code = ?");
    query.addBindValue(code);
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }
    if (query.next()) {
        m_currentReaderId = query.value("reader_id").toInt();
        m_readerNameLabel->setText(query.value("name").toString());
        m_readerStatusLabel->setText(query.value("status").toString());
        m_borrowedCountLabel->setText(query.value("borrowed_count").toString());

        if (query.value("status").toString() != "active") {
            QMessageBox::warning(this, "提示", "该读者账户已冻结，不能借书");
            m_borrowBtn->setEnabled(false);
        } else if (query.value("borrowed_count").toInt() >= 5) {
            QMessageBox::warning(this, "提示", "该读者已借满5本书，请先归还");
            m_borrowBtn->setEnabled(false);
        } else {
            if (m_currentBookId != -1 && m_currentAvailable > 0)
                m_borrowBtn->setEnabled(true);
        }
    } else {
        QMessageBox::warning(this, "提示", "未找到该读者");
        clearReaderInfo();
        m_currentReaderId = -1;
        m_borrowBtn->setEnabled(false);
    }
    query.finish();  // 释放结果集，避免后续操作报“函数序列错误”
}

void BorrowWindow::onSearchBook()
{
    QString keyword = m_bookKeywordEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入图书名称或作者或ISBN");
        return;
    }

    QSqlQuery query;
    // 使用 LIKE 模糊匹配更友好，但这里保持等值匹配，可自行修改
    query.prepare("SELECT book_id, title, available_count FROM books WHERE (title LIKE ? OR author LIKE ? OR isbn LIKE ?) AND status != 'deleted'");
    query.addBindValue(keyword);
    query.addBindValue(keyword);
    query.addBindValue(keyword);
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }
    QList<BookInfo> results;
    while (query.next()) {
        BookInfo book;
        book.id = query.value("book_id").toInt();
        book.title = query.value("title").toString();
        book.author = query.value("author").toString();
        book.isbn = query.value("isbn").toString();
        book.available = query.value("available_count").toInt();
        results.append(book);
    }

    if (results.isEmpty()) {
        QMessageBox::warning(this, "提示", "未找到匹配的图书");
        return;
    }

    // 显示选择对话框
    showBookSelection(results);
    query.finish();
}
void BorrowWindow::showBookSelection(const QList<BookInfo> &books){
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("选择图书");
    dlg->resize(600, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ID", "书名", "作者", "ISBN", "可借数量"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for (int i = 0; i < books.size(); ++i) {
        const BookInfo &b = books[i];
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::number(b.id)));
        table->setItem(i, 1, new QTableWidgetItem(b.title));
        table->setItem(i, 2, new QTableWidgetItem(b.author));
        table->setItem(i, 3, new QTableWidgetItem(b.isbn));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(b.available)));
        // 不可借的图书行变色提示
        if (b.available <= 0) {
            for (int col = 0; col < 5; ++col)
                table->item(i, col)->setBackground(Qt::lightGray);
        }
    }

    QPushButton *confirmBtn = new QPushButton("确认借阅");
    confirmBtn->setStyleSheet(kPrimaryStyle);
    QPushButton *cancelBtn = new QPushButton("取消");
    cancelBtn->setStyleSheet(kCancelStyle);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(confirmBtn);
    btnLayout->addWidget(cancelBtn);

    layout->addWidget(table);
    layout->addLayout(btnLayout);

    // 连接信号
    connect(confirmBtn, &QPushButton::clicked, [=]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::warning(dlg, "提示", "请先选择一本图书");
            return;
        }
        BookInfo selected = books[row];
        if (selected.available <= 0) {
            QMessageBox::warning(dlg, "提示", "该图书库存不足，无法借出");
            return;
        }
        onBookSelected(selected);
        dlg->accept();
    });

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(table, &QTableWidget::doubleClicked, [=](const QModelIndex &idx) {
        if (idx.row() >= 0) {
            BookInfo selected = books[idx.row()];
            if (selected.available > 0) {
                onBookSelected(selected);
                dlg->accept();
            } else {
                QMessageBox::warning(dlg, "提示", "该图书库存不足，无法借出");
            }
        }
    });

    dlg->exec();
    delete dlg;
}
void BorrowWindow::onConfirmBorrow()
{
    if (m_currentReaderId == -1 || m_currentBookId == -1) {
        QMessageBox::warning(this, "错误", "请先查询读者和图书");
        return;
    }

    QDate borrowDate = QDate::currentDate();
    QDate dueDate = borrowDate.addDays(30);

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    // 1. 插入借阅记录
    query.prepare("INSERT INTO borrow_records (book_id, reader_id, borrow_date, due_date, status, operator_id) "
                  "VALUES (?, ?, ?, ?, 'borrowed', ?)");
    query.addBindValue(m_currentBookId);
    query.addBindValue(m_currentReaderId);
    query.addBindValue(borrowDate);
    query.addBindValue(dueDate);
    query.addBindValue(m_operatorId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "借书失败(1): " + query.lastError().text());
        return;
    }

    // 2. 更新图书可借数量
    query.prepare("UPDATE books SET available_count = available_count - 1 WHERE book_id = ?");
    query.addBindValue(m_currentBookId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "借书失败(2): " + query.lastError().text());
        return;
    }

    // 3. 更新读者借阅统计
    query.prepare("UPDATE readers SET borrowed_count = borrowed_count + 1, total_borrowed = total_borrowed + 1 WHERE reader_id = ?");
    query.addBindValue(m_currentReaderId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "错误", "借书失败(3): " + query.lastError().text());
        return;
    }

    // 4. 记录操作日志（在事务内，保证与业务操作原子性）
    query.prepare("INSERT INTO operation_logs (user_id, operation_type, operation_detail) "
                  "VALUES (?, '借书', ?)");
    query.addBindValue(m_operatorId);
    query.addBindValue(QString("读者ID:%1 图书ID:%2 书名:%3 应还日期:%4")
                           .arg(QString::number(m_currentReaderId),
                                QString::number(m_currentBookId),
                                m_currentBookTitle,
                                dueDate.toString("yyyy-MM-dd")));
    if (!query.exec()) {
        // 日志插入失败不影响借书，只警告
        qDebug() << "日志记录失败:" << query.lastError().text();
    }

    db.commit();

    QMessageBox::information(this, "成功", "借书成功！\n应还日期：" + dueDate.toString("yyyy-MM-dd"));
    close();
}

void BorrowWindow::clearReaderInfo()
{
    m_readerNameLabel->clear();
    m_readerStatusLabel->clear();
    m_borrowedCountLabel->clear();
}

void BorrowWindow::clearBookInfo()
{
    m_bookTitleLabel->clear();
    m_bookAvailableLabel->clear();
}

void BorrowWindow::onBookSelected(const BookInfo &book)
{
    m_currentBookId = book.id;
    m_currentBookTitle = book.title;
    m_currentAvailable = book.available;
    m_bookTitleLabel->setText(m_currentBookTitle);
    m_bookAvailableLabel->setText(QString::number(m_currentAvailable));

    // 如果读者也已查询，且可借数量 > 0，启用借书按钮
    if (m_currentReaderId != -1 && m_currentAvailable > 0) {
        m_borrowBtn->setEnabled(true);
    }
}