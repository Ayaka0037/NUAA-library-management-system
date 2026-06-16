// addbookwindow.cpp
#include "addbookwindow.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <loghelper.h>

AddBookWindow::AddBookWindow(int userId,QWidget *parent) : QWidget(parent),m_userId(userId)
{
    setupUI();
}

void AddBookWindow::setupUI()
{
    setWindowTitle("添加新书");
    resize(400, 350);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    m_isbnEdit = new QLineEdit();
    m_titleEdit = new QLineEdit();
    m_authorEdit = new QLineEdit();
    m_publisherEdit = new QLineEdit();
    m_countSpin = new QSpinBox();
    m_countSpin->setRange(1, 999);

    m_categoryCombo = new QComboBox();
    // 加载分类
    QSqlQuery catQuery;
    if (catQuery.exec("SELECT category_id, category_name FROM categories ORDER BY category_id")) {
        while (catQuery.next()) {
            m_categoryCombo->addItem(catQuery.value("category_name").toString(),
                                     catQuery.value("category_id").toInt());
        }
    }

    m_confirmBtn = new QPushButton("确认添加");

    formLayout->addRow("ISBN:", m_isbnEdit);
    formLayout->addRow("书名:", m_titleEdit);
    formLayout->addRow("作者:", m_authorEdit);
    formLayout->addRow("出版社:", m_publisherEdit);
    formLayout->addRow("分类:", m_categoryCombo);
    formLayout->addRow("数量:", m_countSpin);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_confirmBtn);

    connect(m_confirmBtn, &QPushButton::clicked, this, &AddBookWindow::onConfirm);
}

void AddBookWindow::onConfirm()
{
    QString isbn = m_isbnEdit->text().trimmed();
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "错误", "书名不能为空");
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO books (isbn, title, author, publisher, category_id, total_count, available_count, status) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, 'available')");
    query.addBindValue(isbn);
    query.addBindValue(title);
    query.addBindValue(m_authorEdit->text().trimmed());
    query.addBindValue(m_publisherEdit->text().trimmed());
    query.addBindValue(m_categoryCombo->currentData());
    int count = m_countSpin->value();
    query.addBindValue(count);
    query.addBindValue(count);
    LogHelper::record(m_userId, "添加图书",
                      QString("ISBN:%1 书名:%2 数量:%3")
                          .arg(isbn).arg(title).arg(count));
    if (query.exec()) {
        QMessageBox::information(this, "成功", "图书添加成功");
        close();
    } else {
        QMessageBox::critical(this, "错误", "添加失败：" + query.lastError().text());
    }
}