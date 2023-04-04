#ifndef MQSPINBOX_H
#define MQSPINBOX_H

#include <QObject>
#include <QSpinBox>

class MQSpinBox : public QSpinBox
{
public:
    MQSpinBox()
    {
        this->setAlignment(Qt::AlignCenter);
    }

private:
    void wheelEvent(QWheelEvent *event)
    {
        (void)event;
    }
};

#endif // MQSPINBOX_H
