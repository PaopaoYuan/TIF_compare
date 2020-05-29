#include "rastershow.h"
#include <QMessageBox>
#include <QString>
#include "gdal_priv.h"
#include <QDebug>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QPainter>
#include <QObject>
#include "bandchoosedialog.h"
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"
#include <QTime>

RasterShow::RasterShow(QWidget *parent):QLabel(parent)
{
    this->setMouseTracking(true);
}

RasterShow::~RasterShow()
{
    if(m_pDataset != NULL)
    {
        GDALClose((GDALDatasetH) m_pDataset);
        m_pDataset = NULL;
    }
}

bool RasterShow::ReadFile(QString filename)
{
    QTime time;
    time.start();
    GDALAllRegister();
    m_strInputRaster = filename;
    QByteArray ba = m_strInputRaster.toLatin1();
    m_pDataset = (GDALDataset*)GDALOpen(ba.data(), GA_ReadOnly);
    if (m_pDataset == NULL)
    {
        //QMessageBox::about(this,"提示", "指定的文件不能打开！");
        return false;
    }

    nSizeX = m_pDataset->GetRasterXSize(); // 影像的宽度（像元数目）
    nSizeY = m_pDataset->GetRasterYSize(); // 影像的高度（像元数目）

    qDebug()<<nSizeX<<" "<<nSizeY;

    double m_dScale;//现有图框与图像的比值
    nBandCount = m_pDataset->GetRasterCount(); // 影像波段数

    double adfGeoTransform[6];
    m_pDataset->GetGeoTransform(adfGeoTransform);
    MoveSpeed = 100/adfGeoTransform[1];//根据分辨率设置速度参数的初始值
    qDebug()<<adfGeoTransform[1];

    m_dScale = nSizeY > nSizeX ? nSizeY : nSizeX;
    int iViewHeight = 800;
    m_dScale = iViewHeight/m_dScale;

    iSize = GDALGetDataTypeSize(GDT_Byte) / 8;
    iScaleWidth = static_cast<int>(nSizeX*m_dScale+0.5);
    iScaleHeight = static_cast<int>(nSizeY*m_dScale+0.5);

    iScaleWidth = (iScaleWidth*8+31)/32*4;

    double time1 = time.elapsed()/1000.0;
    qDebug()<<time1<<"s";

    int  ret = QMessageBox::question(this,"question","是否构建金字塔？",QMessageBox::No|QMessageBox::Yes);
    switch (ret) {
    case QMessageBox::Yes:
    {
        time.restart();

        int  anOverviewList[8] = { 2, 4, 8, 16, 32, 64, 128, 256};
        m_pDataset->BuildOverviews( "NEAREST", 7, anOverviewList, 0, nullptr,GDALDummyProgress, nullptr );    //构造金字塔

        double time1 = time.elapsed()/1000.0;
        qDebug()<<time1<<"jis";
        break;
    }
    case QMessageBox::No:
        break;
    default:
        break;
    }

    BandChooseDialog getbands;
    connect(&getbands,SIGNAL(signal_sendbands(QByteArray)),this,SLOT(GetBandList_slot(QByteArray)));
    getbands.setBandCount(nBandCount);
    getbands.show();
    getbands.exec();

    ShowRaster();
}

void RasterShow::ShowRaster()
{
    QTime time;
    time.start();

    unsigned char* pBuffer = new unsigned char[iScaleWidth*iScaleHeight*nBandCount];

    CPLErr err = m_pDataset->RasterIO(GF_Read, pX, pY, nSizeX, nSizeY, pBuffer, iScaleWidth, iScaleHeight,
                                      GDT_Byte, nBandCount, band_list, iSize*nBandCount, iSize*iScaleWidth*nBandCount, iSize);	//读取所有波段数据

    unsigned char* pDataBuffer = NULL;
    if (nBandCount >=3 )
    {
        pDataBuffer = pBuffer;
    }
    else
    {
        pDataBuffer = new unsigned char[iScaleWidth*iScaleHeight*3];
        for (int i=0; i<iScaleWidth*iScaleHeight*3; i++)
            pDataBuffer[i] = pBuffer[i/3];

        delete []pBuffer;
    }
    QImage QImg(pDataBuffer, iScaleWidth, iScaleHeight, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(QImg);
    delete []pDataBuffer;

    this->setPixmap(pixmap);

    double time1 = time.elapsed()/1000.0;
    qDebug()<<time1<<"s";
}

void RasterShow::contextMenuEvent(QContextMenuEvent *event)
{
    QPoint pos = event->pos();
    pos = this->mapToGlobal(pos);
    QMenu *menu = new QMenu(this);

    QAction *loadImage = new QAction(this);
    loadImage->setText("选择图片");
    connect(loadImage, &QAction::triggered, this, &RasterShow::ReadFile_Slot);
    menu->addAction(loadImage);

    QAction *closeData = new QAction(this);
    closeData->setText("关闭数据");
    connect(closeData, &QAction::triggered, this, &RasterShow::CloseData_slot);
    menu->addAction(closeData);

    menu->addSeparator();//添加分割线

    QAction *zoomInAction = new QAction(this);
    zoomInAction->setText("放大");
    connect(zoomInAction, &QAction::triggered, this, &RasterShow::ZoomIn);
    menu->addAction(zoomInAction);

    QAction *zoomOutAction = new QAction(this);
    zoomOutAction->setText("缩小");
    connect(zoomOutAction, &QAction::triggered, this, &RasterShow::ZoomOut);
    menu->addAction(zoomOutAction);

    QAction *presetAction = new QAction(this);
    presetAction->setText("还原");
    connect(presetAction, &QAction::triggered, this, &RasterShow::RasterReset);
    menu->addAction(presetAction);

    menu->addSeparator();//添加分割线

    QAction *getPrjInfo = new QAction(this);
    getPrjInfo->setText("坐标信息");
    connect(getPrjInfo, &QAction::triggered, this, &RasterShow::GetProjectionInfo);
    menu->addAction(getPrjInfo);

    menu->exec(pos);
}

void RasterShow::wheelEvent(QWheelEvent *event)
{
    int value = event->delta();
    if (value > 0)  //放大
        ZoomIn();
    else            //缩小
        ZoomOut();
}


void RasterShow::ZoomIn()
{
   if(m_pDataset != NULL)
   {
        pX = pX + nSizeX / 4;
        pY = pY + nSizeY / 4;
        nSizeX = nSizeX / 2;
        nSizeY = nSizeY / 2;
        MoveSpeed = MoveSpeed / 2;
        ShowRaster();
   }
}

void RasterShow::ZoomOut()
{
    if(m_pDataset != NULL)
    {
        pX = pX - nSizeX / 2;
        pY = pY - nSizeY / 2;
        nSizeX = nSizeX * 2;
        nSizeY = nSizeY * 2;
        MoveSpeed = MoveSpeed * 2;
        //如果缩小范围超过图像则回到原始大小图像
        if(pX < 0)
        {
            RasterReset();
        }
        ShowRaster();
    }
}

//鼠标摁下
void RasterShow::mousePressEvent(QMouseEvent *event)
{
    OldPos = event->pos();
    Pressed = true;
}

//鼠标松开
void RasterShow::mouseReleaseEvent(QMouseEvent *event)
{
    Pressed = false;
    setCursor(Qt::ArrowCursor);
}

//鼠标发射事件
void RasterShow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pDataset != NULL)
    {
        if(Pressed == false){
            const char* projRef = m_pDataset->GetProjectionRef();
            double adfGeoTransform[6];
            m_pDataset->GetGeoTransform(adfGeoTransform);

            double x, y;
            x = adfGeoTransform[0] + adfGeoTransform[1] * event->x() + adfGeoTransform[2] * event->y();
            y = adfGeoTransform[3] + adfGeoTransform[4] * event->x() + adfGeoTransform[5] * event->y();

            OGRSpatialReference fRef, tRef;
            char *tmp = NULL;
            /* 获得projRef的一份拷贝 */
            /* 由于projRef是const char*,下面的一个函数不接受，所以需要转换成非const*/
            tmp = (char *)malloc(strlen(projRef) + 1);
            strcpy_s(tmp, strlen(projRef)+1, projRef);
            /* 设置原始的坐标参数，和test.tif一致 */
            fRef.importFromWkt(&tmp);
            /* 设置转换后的坐标 */
            tRef.SetWellKnownGeogCS("WGS84");
            /* 下面进行坐标转换，到此为止都不需要proj，但是下面的内容如果不安装proj将会无法编译 */
            OGRCoordinateTransformation *coordTrans;
            coordTrans = OGRCreateCoordinateTransformation(&fRef, &tRef);

            if(coordTrans != NULL){
                coordTrans->Transform(1, &x, &y);
                emit ProjSingnal(x,y,1);
            }
            else {
                emit ProjSingnal(0,0,0);
                this->setMouseTracking(false);
            }
        }

        //图像漫游功能
        if (Pressed == true) {
            this->setCursor(Qt::SizeAllCursor);
            QPoint pos = event->pos();
            int xPtInterval = pos.x() - OldPos.x();
            int yPtInterval = pos.y() - OldPos.y();

            pX -= xPtInterval * MoveSpeed;
            pY -= yPtInterval * MoveSpeed;

            //检测移动是否到达图像边界
            if(pX < 0)
                pX = 0;
            if(pY < 0)
                pY = 0;
            if(pX > m_pDataset->GetRasterXSize() - nSizeX)
                pX = m_pDataset->GetRasterXSize() - nSizeX;
            if(pY > m_pDataset->GetRasterYSize() - nSizeY)
                pY = m_pDataset->GetRasterYSize() - nSizeY;

            OldPos = pos;
            ShowRaster();
        }
    }
}

void RasterShow::RasterReset()
{
    if(m_pDataset != NULL)
    {
        pX = 0;
        pY = 0;
        nSizeX = m_pDataset->GetRasterXSize();
        nSizeY = m_pDataset->GetRasterYSize();
        double adfGeoTransform[6];
        m_pDataset->GetGeoTransform(adfGeoTransform);
        MoveSpeed = 100/adfGeoTransform[1];
        ShowRaster();
    }
}

int RasterShow::RasterShow::GetXsize()
{
    return nSizeX;
}

int RasterShow::RasterShow::GetYsize()
{
    return nSizeY;
}

//图像坐标转地理坐标
bool RasterShow::ImageRowCol2Projection()
{
    //adfGeoTransform[6]  数组adfGeoTransform保存的是仿射变换中的一些参数，分别含义见下
    //adfGeoTransform[0]  左上角x坐标
    //adfGeoTransform[1]  东西方向分辨率
    //adfGeoTransform[2]  旋转角度, 0表示图像 "北方朝上"
    //adfGeoTransform[3]  左上角y坐标
    //adfGeoTransform[4]  旋转角度, 0表示图像 "北方朝上"
    //adfGeoTransform[5]  南北方向分辨率
    double adfGeoTransform[6];
    m_pDataset->GetGeoTransform(adfGeoTransform);
    try
    {
        Xproj = adfGeoTransform[0] + adfGeoTransform[1] * pX + adfGeoTransform[2] * pY;
        Yproj = adfGeoTransform[3] + adfGeoTransform[4] * pX + adfGeoTransform[5] * pY;
        qDebug()<<Xproj<<" "<<Yproj;
        return true;
    }
    catch(...)
    {
        return false;
    }
}

//地理坐标转图像坐标
bool RasterShow::Projection2ImageRowCol(double Xpj, double Ypj, int Xsize, int Ysize)
{
    double adfGeoTransform[6];
    m_pDataset->GetGeoTransform(adfGeoTransform);
    try
    {
        double dTemp = adfGeoTransform[1]*adfGeoTransform[5] - adfGeoTransform[2]*adfGeoTransform[4];
        double dCol = 0.0, dRow = 0.0;
        dCol = (adfGeoTransform[5]*(Xpj - adfGeoTransform[0]) -
            adfGeoTransform[2]*(Ypj - adfGeoTransform[3])) / dTemp + 0.5;
        dRow = (adfGeoTransform[1]*(Ypj - adfGeoTransform[3]) -
            adfGeoTransform[4]*(Xpj - adfGeoTransform[0])) / dTemp + 0.5;

        pX = static_cast<int>(dCol);
        pY = static_cast<int>(dRow);
        nSizeX = Xsize;
        nSizeY = Ysize;

        ShowRaster();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

void RasterShow::GetProjectionInfo()
{
    const char *projection;
    projection = m_pDataset->GetProjectionRef();

    QMessageBox::about(this,"坐标信息", projection);
    qDebug()<<projection;
}

void RasterShow::ReadFile_Slot()
{
    QString filename;
    if(filename != m_strInputRaster)
    {
        GDALClose((GDALDatasetH) m_pDataset);
        m_pDataset = NULL;
    }
    filename = QFileDialog::getOpenFileName(this, "Open Image", "./", tr("Images (*tif)"));
    ReadFile(filename);
}

//鼠标双击
void RasterShow::mouseDoubleClickEvent(QMouseEvent *event)
{
    int i = event->x();
    int j = event->y();
    mX = i - 30;
    mY = j - 30;
    update();
    emit doubleClickSignal(mX, mY);
}

//红色小方框
void RasterShow::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    QPen pen;
    pen.setColor(QColor(255,0,0));//设置颜色
    pen.setWidth(2);//设置宽度
    painter.setPen(pen);
    painter.drawRect(mX, mY, 60, 60);//自定义实现方框的左上角坐标和长宽
}



void RasterShow::RedSquare_slot(int x, int y)
{
    mX = x;
    mY = y;
    update();
}

void RasterShow::ZoomIn_Slot()
{
    ZoomIn();
}

void RasterShow::ZoomOut_Slot()
{
    ZoomOut();
}

void RasterShow::ZoomReset_Slot()
{
    RasterReset();
}

void RasterShow::ImageRowCol2Projection_slot()
{
    ImageRowCol2Projection();
}

void RasterShow::GetProjectionInfo_slot()
{
    GetProjectionInfo();
}

void RasterShow::GetBandList_slot(QByteArray datagram)
{

    QDataStream in(&datagram,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in>>band_list[0]
      >>band_list[1]
      >>band_list[2];
}

void RasterShow::CloseData_slot()
{
    if(m_pDataset != NULL)
    {
        GDALClose((GDALDatasetH) m_pDataset);
        m_pDataset = NULL;
        this->setText("右键可打开图像");
    }

}
