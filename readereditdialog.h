#ifndef READEREDITDIALOG_H
#define READEREDITDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class ReaderEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReaderEditDialog(QWidget *parent = nullptr);

    void setReaderCode(const QString &code);
    void setName(const QString &name);
    QString readerCode() const;
    QString readerName() const;

private slots:
    void onConfirm();

private:
    void setupUI();
    QLineEdit   *m_codeEdit;
    QLineEdit   *m_nameEdit;
    QPushButton *m_confirmBtn;
};

#endif // READEREDITDIALOG_H
