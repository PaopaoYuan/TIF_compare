#include "mainwindow.h"
#include "rastershow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->label,SIGNAL(doubleClickSignal(int,int)),ui->label_2,SLOT(RedSquare_slot(int,int)));
    connect(ui->label_2,SIGNAL(doubleClickSignal(int,int)),ui->label,SLOT(RedSquare_slot(int,int)));
    connect(ui->label,SIGNAL(ProjSingnal(double,double,bool)),ui->label_3,SLOT(GetProj_Slot(double,double,bool)));
    connect(ui->label_2,SIGNAL(ProjSingnal(double,double,bool)),ui->label_4,SLOT(GetProj_Slot(double,double,bool)));
    this->setWindowTitle("遥感图像对照工具");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->label->ImageRowCol2Projection();
    ui->label_2->Projection2ImageRowCol(ui->label->Xproj, ui->label->Yproj, ui->label->GetXsize(), ui->label->GetYsize());
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->label_2->ImageRowCol2Projection();
    ui->label->Projection2ImageRowCol(ui->label_2->Xproj, ui->label_2->Yproj, ui->label_2->GetXsize(), ui->label_2->GetYsize());
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->label->ZoomIn();
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->label->ZoomOut();
}

void MainWindow::on_pushButton_8_clicked()
{
    ui->label->RasterReset();
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->label_2->ZoomIn();
}

void MainWindow::on_pushButton_6_clicked()
{
    ui->label_2->ZoomOut();
}

void MainWindow::on_pushButton_7_clicked()
{
    ui->label_2->RasterReset();
}
