// addbookwindow.h
#ifndef ADDBOOKWINDOW_H
#define ADDBOOKWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QSpinBox;
class QComboBox;

class AddBookWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AddBookWindow(int userId,QWidget *parent = nullptr);

private slots:
    void onConfirm();

private:
    void setupUI();
    QLineEdit *m_isbnEdit;
    QLineEdit *m_titleEdit;
    QLineEdit *m_authorEdit;
    QLineEdit *m_publisherEdit;
    QSpinBox  *m_countSpin;
    QComboBox *m_categoryCombo;
    QPushButton *m_confirmBtn;
    int m_userId;
};

#endif