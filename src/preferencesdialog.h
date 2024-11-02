#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>


class PreferencesDialog : public QDialog
{
public:
    PreferencesDialog(QWidget *parent = nullptr);

    QString getStyle();

private:
    QComboBox *styleComboBox;
};

#endif // PREFERENCESDIALOG_H
