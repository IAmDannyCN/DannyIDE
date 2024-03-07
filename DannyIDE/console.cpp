#include "console.h"
#include "qevent.h"
#include "ui_console.h"
#include "dannyide.h"
#include "Document.h"

#include <QObject>

Console::Console(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Console)
{
    ui->setupUi(this);
}

Console::~Console()
{
    delete ui;
}

void Console::closeEvent(QCloseEvent *event)
{
    ChangeConsoleStatus(0);
    event->accept();
}

void Console::onUpdateForm()
{
    ui->VarTable->clearContents();
    ui->VarTable->setRowCount(0);
    std::map<std::string, int> rg= Getrg();
    int rowcnt=-1;
    for(const auto &it: rg)
    {
        ui->VarTable->insertRow(++rowcnt);
        QTableWidgetItem *varname= new QTableWidgetItem(QString::fromStdString(it.first));
        QTableWidgetItem *varval= new QTableWidgetItem(QString::fromStdString(std::to_string(it.second)));
//        QTableWidgetItem *varval= new QTableWidgetItem(it.second);
        varname->setFlags(varname->flags() & ~Qt::ItemIsEditable);
        ui->VarTable->setItem(rowcnt, 0, varname);
        ui->VarTable->setItem(rowcnt, 1, varval);
    }
}

void Console::on_DiscardButton_clicked()
{
    onUpdateForm();
}

void Console::on_SaveButton_clicked()
{
    std::map<std::string, int> rg;
    for(int i=0; i<(ui->VarTable->rowCount()); i++)
    {
        std::string varname= ui->VarTable->item(i, 0)->text().toStdString();
        int varval= ui->VarTable->item(i, 1)->text().toInt();
        rg[varname]=varval;
    }
    Updaterg(rg);
}

