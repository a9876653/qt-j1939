#ifndef MLABELTABLE_H
#define MLABELTABLE_H

#include <QWidget>
#include "mlabel.h"

namespace Ui
{
class MLabelTable;
}

class MLabelTable : public QWidget
{
    Q_OBJECT

public:
    explicit MLabelTable(QString des, int column_count, QWidget *parent = nullptr);
    ~MLabelTable();
    void insert(MLabel *label);

private:
    Ui::MLabelTable *ui;

    int column_count = 0;
    int widget_count = 0;
};

#endif // MLABELTABLE_H
