#ifndef ENUMVALUEVEIW_H
#define ENUMVALUEVEIW_H

#include <QWidget>

namespace Ui {
class EnumValueVeiw;
}

class EnumValueVeiw : public QWidget
{
    Q_OBJECT

public:
    explicit EnumValueVeiw(QWidget *parent = nullptr);
    ~EnumValueVeiw();

private:
    Ui::EnumValueVeiw *ui;
};

#endif // ENUMVALUEVEIW_H
