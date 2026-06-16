#include "readermanagewindow.h"
#include "readereditdialog.h"
#include "loghelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

ReaderManageWindow::ReaderManageWindow(int userId, QWidget *parent)
    : QWidget(parent), m_userId(userId)
{
    setupUI();
    onSearch();
}

void ReaderManageWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("👤");
    QLabel *hTitle = new QLabel("读者管理");
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
    m_searchEdit->setPlaceholderText("输入读者证号或姓名搜索");
    m_searchBtn = new QPushButton("搜索");
    m_searchBtn->setStyleSheet(kPrimaryStyle);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);

    // ---------- 操作按钮行 ----------
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_addBtn    = new QPushButton("添加读者");
    m_editBtn   = new QPushButton("编辑所选");
    m_freezeBtn = new QPushButton("冻结/解冻");
    m_deleteBtn = new QPushButton("删除所选");
    m_exportBtn = new QPushButton("导出");
    m_addBtn->setStyleSheet(kSecondaryStyle);
    m_editBtn->setStyleSheet(kSecondaryStyle);
    m_freezeBtn->setStyleSheet(kSecondaryStyle);
    m_deleteBtn->setStyleSheet(kSecondaryStyle);
    m_exportBtn->setStyleSheet(kSecondaryStyle);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_freezeBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_exportBtn);
    btnLayout->addStretch();

    // ---------- 表格 ----------
    m_table = new QTableWidget();
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "ID", "读者证号", "姓名", "状态", "当前借阅数", "累计借阅数"
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    bodyLayout->addLayout(searchLayout);
    bodyLayout->addLayout(btnLayout);
    bodyLayout->addWidget(m_table);

    mainLayout->addWidget(body, 1);

    // ---------- 信号连接 ----------
    connect(m_searchBtn,  &QPushButton::clicked, this, &ReaderManageWindow::onSearch);
    connect(m_addBtn,     &QPushButton::clicked, this, &ReaderManageWindow::onAddReader);
    connect(m_editBtn,    &QPushButton::clicked, this, &ReaderManageWindow::onEditReader);
    connect(m_freezeBtn,  &QPushButton::clicked, this, &ReaderManageWindow::onToggleFreeze);
    connect(m_deleteBtn,  &QPushButton::clicked, this, &ReaderManageWindow::onDeleteReader);
    connect(m_exportBtn,  &QPushButton::clicked, this, &ReaderManageWindow::onExport);
}

// ==================== 查询 ====================
void ReaderManageWindow::onSearch()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    QString keyword = m_searchEdit->text().trimmed();
    QString sql = "SELECT reader_id, reader_code, name, status, "
                  "borrowed_count, total_borrowed "
                  "FROM readers WHERE status != 'deleted'";

    if (!keyword.isEmpty()) {
        QString safe = keyword;
        safe.replace("'", "''");
        sql += QString(
            " AND (reader_code LIKE '%%1%' OR name LIKE '%%1%')"
        ).arg(safe);
    }

    sql += " ORDER BY reader_id";

    QSqlQuery query;
    if (!query.exec(sql)) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    m_table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(query.value("reader_id").toString()));
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("reader_code").toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(query.value("name").toString()));
        m_table->setItem(row, 3, new QTableWidgetItem(query.value("status").toString()));
        m_table->setItem(row, 4, new QTableWidgetItem(query.value("borrowed_count").toString()));
        m_table->setItem(row, 5, new QTableWidgetItem(query.value("total_borrowed").toString()));

        // 冻结状态标红
        if (query.value("status").toString() == "frozen") {
            for (int col = 0; col < 6; ++col)
                m_table->item(row, col)->setForeground(QColor(200, 50, 50));
        }
        row++;
    }
}

// ==================== 添加 ====================
void ReaderManageWindow::onAddReader()
{
    ReaderEditDialog dlg(this);
    dlg.setWindowTitle("添加读者");
    if (dlg.exec() != QDialog::Accepted) return;

    QString code = dlg.readerCode();
    QString name = dlg.readerName();

    // 检查证号是否重复
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM readers WHERE reader_code = ? AND status != 'deleted'");
    check.addBindValue(code);
    if (check.exec() && check.next() && check.value(0).toInt() > 0) {
        QMessageBox::warning(this, "提示", "该读者证号已存在！");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO readers (reader_code, name, status, borrowed_count, total_borrowed) "
                  "VALUES (?, ?, 'active', 0, 0)");
    query.addBindValue(code);
    query.addBindValue(name);
    if (query.exec()) {
        LogHelper::record(m_userId, "添加读者",
                          QString("证号:%1 姓名:%2").arg(code).arg(name));
        QMessageBox::information(this, "成功", "读者添加成功！");
        onSearch();
    } else {
        QMessageBox::critical(this, "错误", "添加失败：" + query.lastError().text());
    }
}

// ==================== 编辑 ====================
void ReaderManageWindow::onEditReader()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一位读者");
        return;
    }

    int readerId = m_table->item(row, 0)->text().toInt();
    QString oldCode = m_table->item(row, 1)->text();
    QString oldName = m_table->item(row, 2)->text();

    ReaderEditDialog dlg(this);
    dlg.setWindowTitle("编辑读者");
    dlg.setReaderCode(oldCode);
    dlg.setName(oldName);
    if (dlg.exec() != QDialog::Accepted) return;

    QString newCode = dlg.readerCode();
    QString newName = dlg.readerName();

    // 如果证号变了，检查是否冲突
    if (newCode != oldCode) {
        QSqlQuery check;
        check.prepare("SELECT COUNT(*) FROM readers WHERE reader_code = ? AND reader_id != ? AND status != 'deleted'");
        check.addBindValue(newCode);
        check.addBindValue(readerId);
        if (check.exec() && check.next() && check.value(0).toInt() > 0) {
            QMessageBox::warning(this, "提示", "该读者证号已存在！");
            return;
        }
    }

    QSqlQuery query;
    query.prepare("UPDATE readers SET reader_code = ?, name = ? WHERE reader_id = ?");
    query.addBindValue(newCode);
    query.addBindValue(newName);
    query.addBindValue(readerId);
    if (query.exec()) {
        LogHelper::record(m_userId, "编辑读者",
                          QString("读者ID:%1 证号:%2 姓名:%3").arg(readerId).arg(newCode).arg(newName));
        QMessageBox::information(this, "成功", "读者信息已更新！");
        onSearch();
    } else {
        QMessageBox::critical(this, "错误", "编辑失败：" + query.lastError().text());
    }
}

// ==================== 冻结/解冻 ====================
void ReaderManageWindow::onToggleFreeze()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一位读者");
        return;
    }

    int readerId = m_table->item(row, 0)->text().toInt();
    QString currentStatus = m_table->item(row, 3)->text();
    QString newStatus = (currentStatus == "active") ? "frozen" : "active";

    // 冻结前检查是否有活跃借阅
    if (newStatus == "frozen" && hasActiveBorrows(readerId)) {
        QMessageBox::warning(this, "无法冻结", "该读者尚有未归还的图书，不能冻结！");
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE readers SET status = ? WHERE reader_id = ?");
    query.addBindValue(newStatus);
    query.addBindValue(readerId);
    if (query.exec()) {
        LogHelper::record(m_userId, (newStatus == "frozen") ? "冻结读者" : "解冻读者",
                          QString("读者ID:%1").arg(readerId));
        QMessageBox::information(this, "成功",
            QString(newStatus == "frozen" ? "读者已冻结" : "读者已解冻"));
        onSearch();
    } else {
        QMessageBox::critical(this, "错误", "操作失败：" + query.lastError().text());
    }
}

// ==================== 删除 ====================
void ReaderManageWindow::onDeleteReader()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一位读者");
        return;
    }

    int readerId = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 2)->text();

    if (hasActiveBorrows(readerId)) {
        QMessageBox::warning(this, "无法删除",
            QString("读者「%1」尚有未归还的图书，不能删除！").arg(name));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除读者「%1」吗？\n此操作为软删除！").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("UPDATE readers SET status = 'deleted' WHERE reader_id = ?");
    query.addBindValue(readerId);
    if (query.exec()) {
        LogHelper::record(m_userId, "删除读者",
                          QString("读者ID:%1 姓名:%2").arg(readerId).arg(name));
        QMessageBox::information(this, "成功", "读者已删除！");
        onSearch();
    } else {
        QMessageBox::critical(this, "错误", "删除失败：" + query.lastError().text());
    }
}

// ==================== 工具方法 ====================
bool ReaderManageWindow::hasActiveBorrows(int readerId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM borrow_records "
                  "WHERE reader_id = ? AND status = 'borrowed'");
    query.addBindValue(readerId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// ==================== 导出 CSV ====================
void ReaderManageWindow::onExport()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出读者列表", "readers.csv", "CSV Files (*.csv)");
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
    for (int col = 0; col < m_table->columnCount(); ++col)
        headers << "\"" + m_table->horizontalHeaderItem(col)->text() + "\"";
    out << headers.join(",") << "\n";

    // 写数据行
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QStringList cols;
        for (int col = 0; col < m_table->columnCount(); ++col) {
            QTableWidgetItem *item = m_table->item(row, col);
            QString val = item ? item->text() : "";
            val.replace("\"", "\"\"");
            cols << "\"" + val + "\"";
        }
        out << cols.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "成功", "数据已导出到 " + filePath);
}
