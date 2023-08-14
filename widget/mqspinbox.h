#ifndef MQSPINBOX_H
#define MQSPINBOX_H

#include <QObject>
#include <QSpinBox>

class MQSpinBox : public QDoubleSpinBox
{
public:
    MQSpinBox()
    {
        this->setAlignment(Qt::AlignCenter);
        this->setDecimals(0);
    }

private:
    void wheelEvent(QWheelEvent *event)
    {
        (void)event;
    }
};

#endif // MQSPINBOX_H
