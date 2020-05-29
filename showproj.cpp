#include "showproj.h"

ShowProj::ShowProj(QWidget *parent) : QLabel(parent)
{

}

void ShowProj::GetProj_Slot(double a, double b, bool c)
{
    if(c == 1)
    {
        QString text = QString("坐标：Lat:%1, Lon:%2").arg(b).arg(a);
        this->setText(text);
    }
    else if (c == 0) {
        QString text = QString("坐标：获取坐标失败！");
        this->setText(text);
    }
}
