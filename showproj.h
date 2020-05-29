#ifndef SHOWPROJ_H
#define SHOWPROJ_H

#include <QObject>
#include <QLabel>
#include <QWidget>

class ShowProj : public QLabel
{
    Q_OBJECT
public:
    explicit ShowProj(QWidget *parent = nullptr);

signals:

public slots:
    void GetProj_Slot(double a, double b, bool c);
};

#endif // SHOWPROJ_H
