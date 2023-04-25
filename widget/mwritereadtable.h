#ifndef MWRITEREADTABLE_H
#define MWRITEREADTABLE_H

#include <QWidget>
#include <QTimer>
#include "mwritereadwidget.h"

namespace Ui
{
class MWriteReadTable;
}

class MWriteReadTable : public QWidget
{
    Q_OBJECT

public:
    explicit MWriteReadTable(QString node, QWidget *parent = nullptr);
    ~MWriteReadTable();

    void insert(MWriteReadWidget *ins);

    int get_row_cnt();

private slots:
    void on_readAllBtn_clicked();

    void slot_read_timeout();
    void slot_write_timeout();

    void on_writeAllBtn_clicked();

    void on_changeBtn_clicked();

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    Ui::MWriteReadTable *     ui;
    QList<MWriteReadWidget *> ins_list;

    int    read_index  = 0;
    int    write_index = 0;
    QTimer read_timer;
    QTimer write_timer;
};

#endif // MWRITEREADTABLE_H
