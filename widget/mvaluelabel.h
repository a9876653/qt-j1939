#ifndef MVALUELABEL_H
#define MVALUELABEL_H

#include <QWidget>
#include <QVariant>
namespace Ui
{
class MValueLabel;
}

class MValueLabel : public QWidget
{
    Q_OBJECT

public:
    explicit MValueLabel(bool read_only = true, QWidget *parent = nullptr);
    ~MValueLabel();
    void set_backcolor(QColor color);
signals:
    void sig_update(QVariant v);

public:
    void slot_update(QVariant value);

private:
    Ui::MValueLabel *ui;
    QVariant         value;
};

#endif // MVALUELABEL_H
