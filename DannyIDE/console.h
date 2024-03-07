#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>

namespace Ui {
class Console;
}

class Console : public QWidget
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = nullptr);
    ~Console();
    //overwrite closeEvent
    void closeEvent(QCloseEvent *);
    void onUpdateForm();

private slots:
    void on_DiscardButton_clicked();

    void on_SaveButton_clicked();

private:
    Ui::Console *ui;

};

#endif // CONSOLE_H
