#ifndef BANDCHOOSEDIALOG_H
#define BANDCHOOSEDIALOG_H

#include <QDialog>

namespace Ui {
class BandChooseDialog;
}

class BandChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandChooseDialog(QWidget *parent = nullptr);
    ~BandChooseDialog();
    void setBandCount(int);

signals:
    void signal_sendbands(QByteArray bandList);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::BandChooseDialog *ui;
};

#endif // BANDCHOOSEDIALOG_H
