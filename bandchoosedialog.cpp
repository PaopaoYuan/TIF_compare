#include "bandchoosedialog.h"
#include "ui_bandchoosedialog.h"
#include <QDebug>

BandChooseDialog::BandChooseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandChooseDialog)
{
    ui->setupUi(this);
    ui->spinBox_3->setPrefix("Band");       //设置输出显示前缀
    ui->spinBox_4->setPrefix("Band");       //设置输出显示前缀
    ui->spinBox_5->setPrefix("Band");       //设置输出显示前缀
}

BandChooseDialog::~BandChooseDialog()
{
    delete ui;
}

void BandChooseDialog::on_buttonBox_accepted()
{

    QByteArray datagram;
    QDataStream out(&datagram,QIODevice::WriteOnly);

       out<<ui->spinBox_3->value()
       <<ui->spinBox_4->value()
       <<ui->spinBox_5->value();

    emit signal_sendbands(datagram);
}

void BandChooseDialog::setBandCount(int a)
{
    ui->spinBox_3->setRange(1,a);
    ui->spinBox_4->setRange(1,a);
    ui->spinBox_5->setRange(1,a);
}
