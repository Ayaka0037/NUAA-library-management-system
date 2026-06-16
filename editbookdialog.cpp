#include "editbookdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

EditBookDialog::EditBookDialog(int bookId, QWidget *parent)
    : QDialog(parent), m_bookId(bookId)
{
    setupUI();
    loadBookData();
}

void EditBookDialog::setupUI()
{
    setWindowTitle("编辑图书");
    resize(400, 400);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    m_isbnEdit = new QLineEdit();
    m_titleEdit = new QLineEdit();
    m_authorEdit = new QLineEdit();
    m_publisherEdit = new QLineEdit();
    m_totalSpin = new QSpinBox();
    m_totalSpin->setRange(0, 9999);
    m_availableSpin = new QSpinBox();
    m_availableSpin->setRange(0, 9999);

    m_categoryCombo = new QComboBox();
    // 加载分类
    QSqlQuery catQuery;
    if (catQuery.exec("SELECT category_id, category_name FROM categories ORDER BY category_id")) {
        while (catQuery.next()) {
            m_categoryCombo->addItem(catQuery.value("category_name").toString(),
                                     catQuery.value("category_id").toInt());
        }
    }

    form->addRow("ISBN:", m_isbnEdit);
    form->addRow("书名:", m_titleEdit);
    form->addRow("作者:", m_authorEdit);
    form->addRow("出版社:", m_publisherEdit);
    form->addRow("分类:", m_categoryCombo);
    form->addRow("总数量:", m_totalSpin);
    form->addRow("可借数量:", m_availableSpin);

    m_confirmBtn = new QPushButton("确认修改");

    mainLayout->addLayout(form);
    mainLayout->addWidget(m_confirmBtn);

    connect(m_confirmBtn, &QPushButton::clicked, this, &EditBookDialog::onConfirm);
}

void EditBookDialog::loadBookData()
{
    QSqlQuery query;
    query.prepare("SELECT isbn, title, author, publisher, category_id, total_count, available_count FROM books WHERE book_id = ?");
    query.addBindValue(m_bookId);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "错误", "加载图书数据失败：" + query.lastError().text());
        return;
    }

    m_isbnEdit->setText(query.value("isbn").toString());
    m_titleEdit->setText(query.value("title").toString());
    m_authorEdit->setText(query.value("author").toString());
    m_publisherEdit->setText(query.value("publisher").toString());
    m_totalSpin->setValue(query.value("total_count").toInt());
    m_availableSpin->setValue(query.value("available_count").toInt());

    // 设置当前分类
    int catId = query.value("category_id").toInt();
    int idx = m_categoryCombo->findData(catId);
    if (idx >= 0) m_categoryCombo->setCurrentIndex(idx);
}

void EditBookDialog::onConfirm()
{
    QString isbn = m_isbnEdit->text().trimmed();
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "警告", "书名不能为空");
        return;
    }

    int total = m_totalSpin->value();
    int available = m_availableSpin->value();
    if (available > total) {
        QMessageBox::warning(this, "警告", "可借数量不能大于总数量");
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE books SET isbn=?, title=?, author=?, publisher=?, category_id=?, total_count=?, available_count=? WHERE book_id=?");
    query.addBindValue(isbn);
    query.addBindValue(title);
    query.addBindValue(m_authorEdit->text().trimmed());
    query.addBindValue(m_publisherEdit->text().trimmed());
    query.addBindValue(m_categoryCombo->currentData());
    query.addBindValue(total);
    query.addBindValue(available);
    query.addBindValue(m_bookId);

    if (query.exec()) {
        QMessageBox::information(this, "成功", "图书信息已更新");
        accept();
    } else {
        QMessageBox::critical(this, "错误", "更新失败：" + query.lastError().text());
    }
}