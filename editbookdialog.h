#ifndef EDITBOOKDIALOG_H
#define EDITBOOKDIALOG_H

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;

class EditBookDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditBookDialog(int bookId, QWidget *parent = nullptr);

private slots:
    void onConfirm();

private:
    void setupUI();
    void loadBookData();

    int m_bookId;
    QLineEdit *m_isbnEdit;
    QLineEdit *m_titleEdit;
    QLineEdit *m_authorEdit;
    QLineEdit *m_publisherEdit;
    QSpinBox  *m_totalSpin;
    QSpinBox  *m_availableSpin;
    QComboBox *m_categoryCombo;
    QPushButton *m_confirmBtn;
};

#endif