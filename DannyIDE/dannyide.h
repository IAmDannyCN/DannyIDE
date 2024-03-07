#ifndef DANNYIDE_H
#define DANNYIDE_H

#include "console.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class DannyIDE; }
QT_END_NAMESPACE

class DannyIDE : public QMainWindow
{
    Q_OBJECT

public:
    DannyIDE(QWidget *parent = nullptr);
    ~DannyIDE();
    void closeEvent(QCloseEvent *event);

private slots:
    void on_RunButton_clicked();

    void on_EditButton_clicked();

    void on_ConsoleButton_clicked();

    void on_NextButton_clicked();

    void on_UndoButton_clicked();

    void on_SettingButton_clicked();

signals:
    void UpdateFormSignal();

private:
    Ui::DannyIDE *ui;
    Console *ConsoleWindow;
    void on_CodeText_Changed();

private:
    void AddOutput(std::string curContent);
    void ChangeBackgroundColor(int startLineNumber, int endLineNumber, int R, int G, int B);
    void SetBackgroundWhite();
    void UpdateForm();
    void UpdateRownum();
};

bool GetConsoleStatus();
void ChangeConsoleStatus(int curConsoleStatus);

#endif // DANNYIDE_H
