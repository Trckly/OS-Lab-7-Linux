#include "staticlib.hpp"

void StaticLib::ConfirmationMessageBox(QMainWindow* Parent)
{
    QMessageBox::information(Parent, "Confirmation", "*****This is a Static Library functions");
}

void StaticLib::OutputInfo(QMainWindow *Parent,
                           QTableWidget *Table,
                           QVector<pthread_t> &th,
                           QVector<int> &tid, bool bShowConfirmation)
{
    if(bShowConfirmation)
        ConfirmationMessageBox(Parent);
    if(!th.empty())
        Table->setRowCount(th.size());
    std::vector<QTableWidgetItem *> table_item(Table->columnCount());
    for (int i = 0; i < th.size(); ++i) {
        table_item[0] = new QTableWidgetItem(QString::number(tid[i]));
        struct sched_param param;
        int policy;
        pthread_getschedparam(th[i], &policy, &param);
        int schedPriority = param.sched_priority;
        table_item[1] = new QTableWidgetItem(QString::number(schedPriority));
        for (int j = 0; j < Table->columnCount(); ++j) {
            Table->setItem(i, j, table_item[j]);
        }
    }
}
