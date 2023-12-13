#pragma once

#include "DynamicLib_global.hpp"
#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <chrono>
#include <QMessageBox>
#include <pthread.h>

extern "C" void ConfirmationMessageBox(QMainWindow *Parent){
    QMessageBox::information(Parent, "Confirmation", "******This is a Shared Library function******");
}

extern "C" void HandleExit(QMainWindow *Parent,
                               QTableWidget *Table,
                               QVector<pthread_t> &th,
                               QVector<int> &tid,
                               QLabel *TimeOutput,
                               const int &FinishedThreadCounter,
                               std::chrono::high_resolution_clock::time_point StartTime,
                               bool& bDisplayTime,
                               bool& bShowConfirmation)
{
    if (bShowConfirmation) {
        ConfirmationMessageBox(Parent);
        bShowConfirmation = false;
    }
    if (!th.empty() && FinishedThreadCounter == 0) {
        void *value_ptr;
        for (int i = th.size() - 1; i >= 0; --i) {
            if (pthread_join(th[i], &value_ptr) == 0) {
                th.erase(th.begin() + i);
                tid.erase(tid.begin() + i);
                Table->removeRow(i);
                if (Table->rowCount() == 0) {
                    bDisplayTime = true;
                }
            }
        }
    }
    if (Table->rowCount() == 0 && bDisplayTime) {
        auto EndTime = std::chrono::high_resolution_clock::now();
        auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime);
        TimeOutput->setText(QString::number(Duration.count()));
        bDisplayTime = false;
    }
}

