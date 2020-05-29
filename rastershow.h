#ifndef RASTERSHOW_H
#define RASTERSHOW_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include "gdal_priv.h"
#include <QString>
#include <QLabel>

class RasterShow:public QLabel
{
     Q_OBJECT
public:
    explicit RasterShow(QWidget *parent = 0);
    ~RasterShow();
    bool RasterShow::ReadFile(QString filename);
    void RasterShow::ZoomIn();
    void RasterShow::ZoomOut();
    void RasterShow::RasterReset();
    void RasterShow::ShowRaster();
    void RasterShow::mousePressEvent(QMouseEvent *event);          //鼠标摁下
    void RasterShow::mouseMoveEvent(QMouseEvent *event) ;           //鼠标松开
    void RasterShow::mouseReleaseEvent(QMouseEvent *event) ;        //鼠标发射事件
    void RasterShow::wheelEvent(QWheelEvent *event) ;               //鼠标滚轮滚动
    void RasterShow::contextMenuEvent(QContextMenuEvent *event) ;   //右键菜单
    void RasterShow::mouseDoubleClickEvent(QMouseEvent *event);  //鼠标双击事件
    void RasterShow::paintEvent(QPaintEvent *event);            //绘制方框
    void RasterShow::Transform();
    bool RasterShow::Projection2ImageRowCol(double Xpj, double Ypj, int Xsize, int Ysize);
    bool RasterShow::ImageRowCol2Projection();
    int RasterShow::GetXsize();
    int RasterShow::GetYsize();
    void RasterShow::GetProjectionInfo();
    double Xproj, Yproj;

signals:
    void doubleClickSignal(int a, int b);
    void ProjSingnal(double x, double y, bool c);

public slots:
    void RedSquare_slot(int a, int b);

private slots:
    void ReadFile_Slot();
    void ZoomIn_Slot();
    void ZoomOut_Slot();
    void ZoomReset_Slot();
    void ImageRowCol2Projection_slot();
    void GetProjectionInfo_slot();
    void GetBandList_slot(QByteArray datagram);
    void CloseData_slot();

private:
    QString m_strInputRaster = NULL;     //文件名
    GDALDataset *m_pDataset = NULL;
    int pX = 0, pY = 0;
    int nSizeX, nSizeY;
    int iSize;
    int iScaleWidth;
    int iScaleHeight;
    int nBandCount;
    QPoint OldPos;          //旧的鼠标位置
    bool Pressed = false;   //鼠标是否被摁压
    double MoveSpeed;
    int mX = 0, mY = 0;
    int	band_list[3] = {1,2,3};
};

#endif // RASTERSHOW_H
