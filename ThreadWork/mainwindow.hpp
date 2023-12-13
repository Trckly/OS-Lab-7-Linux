#pragma once

#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include <dispatch/dispatch.h>
#include <fstream>
#include <QMessageBox>
#include <QTimer>
#include <QtWidgets/QMainWindow>
#include "staticlib.hpp"
#include <dlfcn.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void OutputInfo();

    void HandleExit();

    static void *th_func(void *args);
    static void *th_func_cond_var(void *args);
    static void *th_func_barrier(void *args);

private slots:
    void on_startButton_clicked();
    void on_commitButton_clicked();

private:
    enum CommitAction { Priority, Detach, Cancel };

    typedef struct
    {
        int gap;
    } th_param;

    static const std::string Info;

    int GeneralGap = 10000000;

    QVector<pthread_t> th;
    QVector<int> tid;
    sem_t semaphore;

    std::chrono::high_resolution_clock::time_point StartTime, EndTime;
    std::chrono::microseconds Duration;

    QTimer *outputTimer;
    QTimer *displayTimer;

    static int FinishedThreadCounter;
    bool DisplayTime;

    pthread_attr_t attr;

    static pthread_mutex_t mutex;
    static pthread_cond_t condition;
    static dispatch_queue_t concurrentQueue;
    static dispatch_group_t barrierGroup;

    static bool done;

    bool bShowStaticConfirmation;
    bool bShowDynamicConfirmation;

    void *hDll;

    void (*HandleExitDll)(QMainWindow *Parent,
                          QTableWidget *Table,
                          QVector<pthread_t> &th,
                          QVector<int> &tid,
                          QLabel *TimeOutput,
                          const int &FinishedThreadCounter,
                          std::chrono::high_resolution_clock::time_point StartTime,
                          bool &bDisplayTime,
                          bool &bShowConfirmation);

    void (*myFunc)(QMainWindow *Parent);

private:
    Ui::MainWindow *ui;
};
