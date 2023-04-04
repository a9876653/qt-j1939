#ifndef MLABEL_H
#define MLABEL_H

#include <QWidget>

namespace Ui
{
class MLabel;
}

class MLabel : public QWidget
{
    Q_OBJECT

public:
    explicit MLabel(QString des, QWidget *parent = nullptr);
    ~MLabel();

public:
    void slot_update(QVariant value);

private:
    Ui::MLabel *ui;
};

#endif // MLABEL_H
