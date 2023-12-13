#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidget>

class StaticLib
{
public:
    static void ConfirmationMessageBox(QMainWindow* Parent);
    static void OutputInfo(QMainWindow*Parent, QTableWidget* Table, QVector<pthread_t>& th, QVector<int>& tid, bool bShowConfirmation);
};
