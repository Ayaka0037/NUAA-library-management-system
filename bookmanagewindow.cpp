#include "bookmanagewindow.h"
#include "addbookwindow.h"
#include "editbookdialog.h"
#include "loghelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

#include "categorymanagedialog.h"

BookManageWindow::BookManageWindow(int userId, QWidget *parent)
    : QWidget(parent), m_userId(userId)
{
    setupUI();
    onSearch();  // 打开即加载全部图书
}

void BookManageWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("📖");
    QLabel *hTitle = new QLabel("图书管理");
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

    static const char *kPrimaryStyle =
        "QPushButton {"
        "  background-color: #3498db; color: white; border: none;"
        "  border-radius: 4px; padding: 6px 16px; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1c6ea4; }"
        "QPushButton:disabled { background-color: #bdc3c7; color: #ecf0f1; }";
    static const char *kSecondaryStyle =
        "QPushButton {"
        "  background-color: white; color: #3498db;"
        "  border: 1px solid #3498db; border-radius: 4px;"
        "  padding: 6px 16px; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #3498db; color: white; }"
        "QPushButton:pressed { background-color: #2980b9; color: white; }";

    // ---------- 搜索行 ----------
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入书名、作者、ISBN 或出版社搜索");
    m_searchBtn = new QPushButton("搜索");
    m_searchBtn->setStyleSheet(kPrimaryStyle);
    m_categoryFilter = new QComboBox();
    searchLayout->addWidget(m_categoryFilter);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);

    // ---------- 操作按钮行 ----------
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_addBtn       = new QPushButton("添加图书");
    m_editBtn      = new QPushButton("编辑所选");
    m_deleteBtn    = new QPushButton("删除所选");
    m_categoryBtn  = new QPushButton("分类管理");
    m_exportBtn    = new QPushButton("导出");
    m_addBtn->setStyleSheet(kSecondaryStyle);
    m_editBtn->setStyleSheet(kSecondaryStyle);
    m_deleteBtn->setStyleSheet(kSecondaryStyle);
    m_categoryBtn->setStyleSheet(kSecondaryStyle);
    m_exportBtn->setStyleSheet(kSecondaryStyle);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_categoryBtn);
    btnLayout->addWidget(m_exportBtn);
    btnLayout->addStretch();

    // ---------- 表格 ----------
    m_bookTable = new QTableWidget();
    m_bookTable->setColumnCount(9);
    m_bookTable->setHorizontalHeaderLabels({
        "ID", "ISBN", "书名", "作者", "出版社",
        "分类", "总数量", "可借数量", "状态"
    });
    m_bookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookTable->setSelectionMode(QAbstractItemView::SingleSelection);

    bodyLayout->addLayout(searchLayout);
    bodyLayout->addLayout(btnLayout);
    bodyLayout->addWidget(m_bookTable);

    mainLayout->addWidget(body, 1);

    // ---------- 初始化分类下拉 ----------
    loadCategoryFilter();

    // ---------- 信号连接 ----------
    connect(m_searchBtn,       &QPushButton::clicked, this, &BookManageWindow::onSearch);
    connect(m_addBtn,          &QPushButton::clicked, this, &BookManageWindow::onAddBook);
    connect(m_editBtn,         &QPushButton::clicked, this, &BookManageWindow::onEditBook);
    connect(m_deleteBtn,       &QPushButton::clicked, this, &BookManageWindow::onDeleteBook);
    connect(m_categoryBtn,     &QPushButton::clicked, this, &BookManageWindow::onManageCategories);
    connect(m_exportBtn,       &QPushButton::clicked, this, &BookManageWindow::onExport);
    connect(m_categoryFilter,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookManageWindow::onFilterCategory);
}

// ==================== 查询 ====================
void BookManageWindow::onSearch()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    QString keyword = m_searchEdit->text().trimmed();
    int catId = m_categoryFilter->currentData().toInt();

    QString sql = "SELECT b.book_id, b.isbn, b.title, b.author, b.publisher, "
                  "c.category_name, b.total_count, b.available_count, b.status "
                  "FROM books b "
                  "LEFT JOIN categories c ON b.category_id = c.category_id "
                  "WHERE b.status != 'deleted'";

    if (!keyword.isEmpty()) {
        QString safe = keyword;
        safe.replace("'", "''");
        sql += QString(
            " AND (b.title LIKE '%%1%' OR b.author LIKE '%%1%' OR "
            "b.isbn LIKE '%%1%' OR b.publisher LIKE '%%1%')"
        ).arg(safe);
    }

    if (catId > 0) {
        sql += QString(" AND b.category_id = %1").arg(catId);
    }

    QSqlQuery query;
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    m_bookTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_bookTable->insertRow(row);
        m_bookTable->setItem(row, 0, new QTableWidgetItem(query.value("book_id").toString()));
        m_bookTable->setItem(row, 1, new QTableWidgetItem(query.value("isbn").toString()));
        m_bookTable->setItem(row, 2, new QTableWidgetItem(query.value("title").toString()));
        m_bookTable->setItem(row, 3, new QTableWidgetItem(query.value("author").toString()));
        m_bookTable->setItem(row, 4, new QTableWidgetItem(query.value("publisher").toString()));
        m_bookTable->setItem(row, 5, new QTableWidgetItem(query.value("category_name").toString()));
        m_bookTable->setItem(row, 6, new QTableWidgetItem(query.value("total_count").toString()));
        m_bookTable->setItem(row, 7, new QTableWidgetItem(query.value("available_count").toString()));
        m_bookTable->setItem(row, 8, new QTableWidgetItem(query.value("status").toString()));
        row++;
    }
}

// ==================== 添加 ====================
void BookManageWindow::onAddBook()
{
    AddBookWindow *win = new AddBookWindow(m_userId);
    win->setAttribute(Qt::WA_DeleteOnClose);
    // 添加窗口关闭后自动刷新列表
    connect(win, &QWidget::destroyed, this, &BookManageWindow::onSearch);
    win->show();
}

// ==================== 编辑 ====================
void BookManageWindow::onEditBook()
{
    int row = m_bookTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一本图书");
        return;
    }

    int bookId = m_bookTable->item(row, 0)->text().toInt();
    EditBookDialog dlg(bookId, this);
    if (dlg.exec() == QDialog::Accepted) {
        onSearch();  // 刷新列表
    }
}

// ==================== 删除 ====================
bool BookManageWindow::isBookBorrowed(int bookId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM borrow_records "
                  "WHERE book_id = ? AND status = 'borrowed'");
    query.addBindValue(bookId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void BookManageWindow::onDeleteBook()
{
    int row = m_bookTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一本图书");
        return;
    }

    int bookId = m_bookTable->item(row, 0)->text().toInt();
    QString title = m_bookTable->item(row, 2)->text();

    if (isBookBorrowed(bookId)) {
        QMessageBox::warning(this, "无法删除",
            QString("《%1》尚有未归还的借阅记录，不能删除！").arg(title));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除《%1》吗？\n此操作为软删除，不可恢复！").arg(title),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("UPDATE books SET status = 'deleted' WHERE book_id = ?");
    query.addBindValue(bookId);
    if (query.exec()) {
        LogHelper::record(m_userId, "删除图书",
                          QString("图书ID:%1 书名:%2").arg(bookId).arg(title));
        QMessageBox::information(this, "成功", "图书已删除！");
        onSearch();
    } else {
        QMessageBox::critical(this, "错误", "删除失败：" + query.lastError().text());
    }
}

// ==================== 分类筛选 ====================
void BookManageWindow::loadCategoryFilter()
{
    m_categoryFilter->clear();
    m_categoryFilter->addItem("全部类别", 0);

    QSqlQuery query;
    if (query.exec("SELECT category_id, category_name FROM categories ORDER BY category_id")) {
        while (query.next()) {
            m_categoryFilter->addItem(query.value("category_name").toString(),
                                      query.value("category_id").toInt());
        }
    }
}

void BookManageWindow::onFilterCategory()
{
    onSearch();
}

void BookManageWindow::onManageCategories()
{
    CategoryManageDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        // 分类变更后刷新下拉和列表
        loadCategoryFilter();
        onSearch();
    }
}

// ==================== 导出 CSV ====================
void BookManageWindow::onExport()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出图书列表", "books.csv", "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件：" + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << QChar(0xFEFF);  // UTF-8 BOM

    // 写表头
    QStringList headers;
    for (int col = 0; col < m_bookTable->columnCount(); ++col)
        headers << "\"" + m_bookTable->horizontalHeaderItem(col)->text() + "\"";
    out << headers.join(",") << "\n";

    // 写数据行
    for (int row = 0; row < m_bookTable->rowCount(); ++row) {
        QStringList cols;
        for (int col = 0; col < m_bookTable->columnCount(); ++col) {
            QTableWidgetItem *item = m_bookTable->item(row, col);
            QString val = item ? item->text() : "";
            val.replace("\"", "\"\"");
            cols << "\"" + val + "\"";
        }
        out << cols.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "成功", "数据已导出到 " + filePath);
}
