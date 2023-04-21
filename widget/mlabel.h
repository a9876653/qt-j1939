#ifndef MLABEL_H
#define MLABEL_H

#include <QWidget>
#include <QMap>
#include "value_des.h"

namespace Ui
{
class MLabel;
}

class MLabel : public QWidget
{
    Q_OBJECT

public:
    explicit MLabel(QString des, ValueDes value_des = ValueDes(), QWidget *parent = nullptr);
    ~MLabel();

public:
    void slot_update(QVariant value);

private:
    Ui::MLabel *ui;

    ValueDes value_des;
};

#endif // MLABEL_H
