#include "readereditdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

ReaderEditDialog::ReaderEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void ReaderEditDialog::setupUI()
{
    setWindowTitle("读者信息");
    resize(350, 180);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    m_codeEdit = new QLineEdit();
    m_nameEdit = new QLineEdit();
    m_confirmBtn = new QPushButton("确认");

    form->addRow("读者证号:", m_codeEdit);
    form->addRow("姓名:", m_nameEdit);

    mainLayout->addLayout(form);
    mainLayout->addWidget(m_confirmBtn);

    connect(m_confirmBtn, &QPushButton::clicked, this, &ReaderEditDialog::onConfirm);
}

void ReaderEditDialog::setReaderCode(const QString &code)
{
    m_codeEdit->setText(code);
}

void ReaderEditDialog::setName(const QString &name)
{
    m_nameEdit->setText(name);
}

QString ReaderEditDialog::readerCode() const
{
    return m_codeEdit->text().trimmed();
}

QString ReaderEditDialog::readerName() const
{
    return m_nameEdit->text().trimmed();
}

void ReaderEditDialog::onConfirm()
{
    if (m_codeEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "读者证号不能为空");
        return;
    }
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名不能为空");
        return;
    }
    accept();
}
