#pragma once

#include <QWidget>
#include <QEvent>
#include <QObject>
#include <QTabWidget>
#include <QGridLayout>
#include <QTabBar>
#include <QMouseEvent>
#include <QCloseEvent>

class CWidget : public QWidget
{
    Q_OBJECT
public:
    CWidget(QWidget *parent = 0) : QWidget(parent)
    {
    }
    ~CWidget()
    {
    }

    int index = 0;

protected:
    void closeEvent(QCloseEvent *e)
    {
        emit sig_close();
        e->accept();
    }

signals:
    void sig_close(); // 点击关闭信号
};

class CTabBar : public QTabBar
{
    Q_OBJECT
public:
    CTabBar(QWidget *parent = 0) : QTabBar(parent), pressFlag(false)
    {
    }
    virtual ~CTabBar()
    {
    }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            pressFlag = true;
        }
        QTabBar::mousePressEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton && pressFlag)
        {
            pressFlag = false;
            if (tabRect(currentIndex()).contains(event->pos()))
                return;
            emit sig_tabDrag(currentIndex(), event->pos());
        }

    } //通过两个事件模拟出tab被拖动的动作
private:
    bool pressFlag;
signals:
    void sig_tabDrag(int, QPoint);
};

class MTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    MTabWidget(QWidget *parent = nullptr) : QTabWidget(parent)
    {
        tabbar = new CTabBar;
        setTabBar(tabbar);                                                       //用自定义的CTabBar替换原来的QTabBar
        connect(tabbar, &CTabBar::sig_tabDrag, this, &MTabWidget::slot_tabDrag); // 拖动信号槽
    }
    ~MTabWidget()
    {
    }

private slots:
    void slot_tabBarClose() //响应关闭的槽函数
    {
        CWidget    *cwidget    = qobject_cast<CWidget *>(sender());
        QObjectList list       = cwidget->children();
        QWidget    *tab_widget = NULL;

        for (int i = 0; i < list.count(); ++i)
        {
            if (list[i]->inherits("QWidget"))
            {
                tab_widget = qobject_cast<QWidget *>(list[i]);
                break;
            }
        }
        if (tab_widget == NULL)
        {
            return;
        }

        tab_widget->setParent(this);
        insertTab(cwidget->index, tab_widget, cwidget->windowTitle());
        delete cwidget;
    }
    void slot_tabDrag(int index, QPoint point) //响应拖动动作的槽函数
    {
        (void)point;
        CWidget *cwidget    = new CWidget;
        QWidget *draged     = widget(index);
        QString  windowName = tabText(index);
        cwidget->index      = index;
        removeTab(index);
        connect(cwidget, &CWidget::sig_close, this, &MTabWidget::slot_tabBarClose);

        QGridLayout *layout = new QGridLayout;

        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(draged);

        cwidget->setContentsMargins(0, 0, 0, 0);

        cwidget->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint);
        cwidget->setLayout(layout);
        cwidget->resize(draged->size());
        cwidget->move(cursor().pos());
        // cwidget->move(point + pos() + tabWidget->pos());
        cwidget->setWindowTitle(windowName);
        cwidget->show();
        draged->show();
    }

private:
    CTabBar *tabbar = nullptr;
};
