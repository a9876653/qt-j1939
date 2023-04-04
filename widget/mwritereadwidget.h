#ifndef MWRITEREADWIDGET_H
#define MWRITEREADWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include "frmcentericon.h"
#include "mqspinbox.h"

class MWriteReadWidget : public QObject
{
    Q_OBJECT

public:
    explicit MWriteReadWidget(QString des, QVariant min = -999999999, QVariant max = 999999999, QVariant def = 0);
    ~MWriteReadWidget();

private:
signals:
    void sig_update(QVariant value);
    void sig_request_read();

public slots:
    void slot_update(QVariant value);
    void slot_update_finish();

private slots:
    void update();
    void on_readBtn_clicked();

public:
    QLabel        *des_label     = nullptr;
    MQSpinBox     *read_spinbox  = nullptr;
    MQSpinBox     *write_spinbox = nullptr;
    QToolButton   *read_btn      = nullptr;
    QToolButton   *write_btn     = nullptr;
    frmCenterIcon *text_icon     = nullptr;

private:
    QString  des;
    QVariant min;
    QVariant max;
    QVariant def;
};

#endif // MWRITEREADWIDGET_H
