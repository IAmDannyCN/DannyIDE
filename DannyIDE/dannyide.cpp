#include "dannyide.h"
#include "Document.h"
#include "console.h"
#include "ui_dannyide.h"

#include <iostream>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QDialog>

DannyIDE::DannyIDE(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DannyIDE)
{

    ui->setupUi(this);

    QTextEdit* CodeText = findChild<QTextEdit*>("CodeText");
    QTextBrowser* RownumText = findChild<QTextBrowser*>("RownumText");

    RownumText->setOpenExternalLinks(0);

    connect(CodeText, &QTextEdit::textChanged, this, &DannyIDE::on_CodeText_Changed);

    RownumText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(CodeText->verticalScrollBar(), &QScrollBar::valueChanged, RownumText->verticalScrollBar(), &QScrollBar::setValue);//滚动绑定

    RownumText->setAlignment(Qt::AlignCenter); // 设置文本居中对齐
    RownumText->setWordWrapMode(QTextOption::NoWrap); // 禁用文本自动换行



//    RownumText->setStyleSheet("letter-spacing: -1px;");
}

DannyIDE::~DannyIDE()
{
    delete ui;
}

void DannyIDE::closeEvent(QCloseEvent *event)
{
    if(GetConsoleStatus())
        ConsoleWindow->close();
    event->accept();
}

bool isRunning=0;

void DannyIDE::UpdateRownum()
{
    QTextEdit* CodeText = QObject::findChild<QTextEdit*>("CodeText");
    QTextDocument *document = CodeText->document();

    std::vector<int> height; height.clear();
    int lineCount = document->blockCount();
    for (int lineNumber = 0; lineNumber < lineCount; lineNumber++)
    {
        QTextBlock block = document->findBlockByLineNumber(lineNumber);
        QTextLayout *textLayout = block.layout();
        height.push_back(textLayout->lineCount());
    }

    ui->RownumText->setPlainText(QString::fromStdString(MakeLineNumber(height)));
    ui->RownumText->verticalScrollBar()->setValue(ui->CodeText->verticalScrollBar()->value());
}

void DannyIDE::on_CodeText_Changed()
{
    if(isRunning) return ;
    //Check the code
    std::string curText= ui->CodeText->toPlainText().toStdString();
    SetPassage(curText);
    Check();
    if(GetFoundError())
        ui->OutputText->setPlainText(QString::fromStdString(UpdateCheckOutput(0)));
    else
    {
        CheckGoto();
        ui->OutputText->setPlainText(QString::fromStdString(UpdateCheckOutput(1)));
    }

    //Generate the line number and draw
    UpdateRownum();
}

#define Status_OK -1
#define Status_Empty 0

void DannyIDE::ChangeBackgroundColor(int startLineNumber, int endLineNumber, int R, int G, int B)
{
    QTextEdit* CodeText = findChild<QTextEdit*>("CodeText");
    QTextDocument* document = CodeText->document();

    QTextBlockFormat format;
    QColor Color(R,G,B);
    QBrush Color_brush(Color);
    format.setBackground(Color_brush);

    QTextBlock block = document->findBlockByLineNumber(startLineNumber - 1);
    QTextCursor cursor(block);
    while (block.isValid() && block.blockNumber() <= endLineNumber - 1)
    {
        cursor.setBlockFormat(format);
        block = block.next();
        cursor.setPosition(block.position());
    }
}

void DannyIDE::on_RunButton_clicked()
{
    isRunning=1;
    ui->RunButton->setEnabled(0);
    ui->EditButton->setEnabled(1);
    ui->SettingButton->setEnabled(0);
    ui->CodeText->setReadOnly(1);

    int Status= GetStatus();
    if(Status==-1)
    {
        QPixmap OK_Status(":/icon/Resources/Status_OK.ico");
        ui->StatusLabel->setPixmap(OK_Status);

        ui->NextButton->setEnabled(1);
        ui->UndoButton->setEnabled(1);
        ui->ConsoleButton->setEnabled(1);

        ui->OutputText->clear();
        Set_curLine(0);
        InitChangeHistory();
        return ;
    }

    QPixmap Error_Status(":/icon/Resources/Status_Error.ico");
    ui->StatusLabel->setPixmap(Error_Status);

    if(Status==0)
        ui->OutputText->setPlainText(QString::fromStdString("Nothing to run."));
    else
        ChangeBackgroundColor(Status,Status, 255,192,203);
}

void DannyIDE::SetBackgroundWhite()
{
    QTextEdit* CodeText = findChild<QTextEdit*>("CodeText");
    //Set white color
    QTextDocument* document = CodeText->document();

    QTextBlockFormat format;
    QColor white(255,255,255);
    QBrush white_brush(white);
    format.setBackground(white_brush);

    QTextBlock block = document->begin();
    QTextCursor cursor(block);
    while (block.isValid())
    {
        cursor.setBlockFormat(format);
        block = block.next();
        cursor.setPosition(block.position());
    }
}

void DannyIDE::on_EditButton_clicked()
{
    isRunning=0;
    ui->RunButton->setEnabled(1);
    ui->EditButton->setEnabled(0);
    ui->SettingButton->setEnabled(1);
    ui->CodeText->setReadOnly(0);
    ui->NextButton->setEnabled(0);
    ui->UndoButton->setEnabled(0);
    ui->ConsoleButton->setEnabled(0);
    QPixmap Edit_Status(":/icon/Resources/Status_Edit.ico");
    ui->StatusLabel->setPixmap(Edit_Status);

    SetBackgroundWhite();

    if(GetConsoleStatus())
        ConsoleWindow->close();
}

bool ConsoleStatus=0;
bool GetConsoleStatus()
{
    return ConsoleStatus;
}
void ChangeConsoleStatus(int curConsoleStatus)
{
    ConsoleStatus= curConsoleStatus;
}

void DannyIDE::on_ConsoleButton_clicked()
{
    if(GetConsoleStatus()!=0)
        return ;
    ChangeConsoleStatus(1);
    ConsoleWindow= new Console;
    ConsoleWindow->setAttribute(Qt::WA_DeleteOnClose);
    UpdateForm();
    ConsoleWindow->show();
}

void DannyIDE::AddOutput(std::string newContent)
{
    std::string curOutput= ui->OutputText->toPlainText().toStdString();
    ui->OutputText->setPlainText(QString::fromStdString(curOutput+newContent+"\n"));
    QTextCursor cursor = ui->OutputText->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->OutputText->setTextCursor(cursor);
}

void DannyIDE::UpdateForm()
{
    ConsoleWindow->onUpdateForm();
}

void DannyIDE::on_NextButton_clicked()
{
    std::pair<int, std::pair<std::string, int > > curans= Go();
    int curLine= curans.first;
    std::string varname= curans.second.first;
    int varval= curans.second.second;

    assert(varname!="[e]");

    if(varname=="[t]")
    {
        AddOutput(">> Already at the end of the program.");
        return ;
    }

    SetBackgroundWhite();
    ChangeBackgroundColor(curLine,curLine, 153,217,234);

    if(varname=="[o]")
    {
        AddOutput(std::to_string(varval));
    }

    if(GetConsoleStatus()) UpdateForm();
}


void DannyIDE::on_UndoButton_clicked()
{
    std::pair<std::string, int> curans= Undo();
    int curLine= curans.second;
    std::string Status= curans.first;

    if(Status=="[s]")
    {
        AddOutput(">> Already at the beginning of the program.");
        return ;
    }

    assert(Status=="[ok]");

    SetBackgroundWhite();
    ChangeBackgroundColor(curLine,curLine, 153,217,234);

    if(GetConsoleStatus()) UpdateForm();
}


void DannyIDE::on_SettingButton_clicked()
{
    std::string ExampleCode="ans=1\nbase=2\nline=100\n\n@loop\ngotoif end ans>=line\nans=ans*base\ngoto loop\n\n@end\noutput ans\noutput ans/base\n";
    ui->CodeText->setPlainText(QString::fromStdString(ExampleCode));
}

