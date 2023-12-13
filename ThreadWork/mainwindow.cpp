#include "mainwindow.hpp"
#include "ui_mainwindow.h"

const std::string MainWindow::Info = "Shlomiak Danylo StudentID:BK13914809\n";
int MainWindow::FinishedThreadCounter = false;
bool MainWindow::done = true;
pthread_mutex_t MainWindow::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t MainWindow::condition = PTHREAD_COND_INITIALIZER;
dispatch_queue_t MainWindow::concurrentQueue = dispatch_queue_create("queue",
                                                                     DISPATCH_QUEUE_CONCURRENT);
dispatch_group_t MainWindow::barrierGroup = dispatch_group_create();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
{
    ui->setupUi(this);

    ui->comboBox->addItems({"1", "2", "4", "8", "16"});

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels({"TID", "Priority"});

    ui->ThreadOptionsBox->addItems({"Set Priority", "Detach Thread", "Cancel Thread"});

    const char* libPath = ("//Users//bossofthisgym//Documents//"
                           "build-DynamicLib-Desktop_arm_darwin_generic_mach_o_64bit-Debug//"
                            "libDynamicLib.dylib");
    hDll = dlopen(libPath, RTLD_LAZY);
    if (hDll) {
        *(void**)(&myFunc) = dlsym(hDll, "ConfirmationMessageBox");
        if (!myFunc) {
            QMessageBox::information(this, "Error!", "Failed to load dynamic function!");
        }
    } else {
        QMessageBox::information(this, "Error!", "Failed to open dylib");
    }

    outputTimer = new QTimer;
    displayTimer = new QTimer;

    connect(outputTimer, &QTimer::timeout, this, &MainWindow::HandleExit);
    connect(displayTimer, &QTimer::timeout, this, &MainWindow::OutputInfo);

    outputTimer->setInterval(100);
    outputTimer->start();
    displayTimer->setInterval(1000);
    displayTimer->start();

    ui->spinBox->setMinimum(sched_get_priority_min(SCHED_FIFO));
    ui->spinBox->setMaximum(sched_get_priority_max(SCHED_FIFO));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    bShowStaticConfirmation = true;
    bShowDynamicConfirmation = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OutputInfo()
{
    if(!th.isEmpty ()){
    StaticLib::OutputInfo(this, ui->tableWidget, th, tid, bShowStaticConfirmation);
    bShowStaticConfirmation = false;
    }
}

void MainWindow::HandleExit()
{
    //    DynamicLib::HandleExit(this,
    //                           ui->tableWidget,
    //                           th,
    //                           tid,
    //                           ui->TimeOutput,
    //                           FinishedThreadCounter,
    //                           StartTime,
    //                           DisplayTime,
    //                           bShowDynamicConfirmation);
    //    HandleExitDll(this,
    //                  ui->tableWidget,
    //                  th,
    //                  tid,
    //                  ui->TimeOutput,
    //                  FinishedThreadCounter,
    //                  StartTime,
    //                  DisplayTime,
    //                  bShowDynamicConfirmation);
    myFunc(this);
}

void *MainWindow::th_func(void *args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    int *gap = static_cast<int *>(args);
    int counter = *gap;
    std::ofstream fout(R"(/Users/bossofthisgym/Documents/ThreadWork/Log.txt)", std::ios::app);
    for (int i = 0; i < counter; ++i)
        for (auto Character : Info)
            fout << Character;

    fout.close();
    FinishedThreadCounter--;
    return 0;
}

void *MainWindow::th_func_cond_var(void *args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    int *gap = static_cast<int *>(args);
    int counter = *gap;
    pthread_mutex_lock(&mutex);
    while (!done) {
        pthread_cond_wait(&condition, &mutex);
    }
    done = false;
    std::ofstream fout(R"(/Users/bossofthisgym/Documents/ThreadWork/Log.txt)", std::ios::app);
    for (int i = 0; i < counter; ++i)
        for (auto Character : Info)
            fout << Character;

    fout.close();
    done = true;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    FinishedThreadCounter--;
    return 0;
}

void *MainWindow::th_func_barrier(void *args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    int *gap = static_cast<int *>(args);
    int counter = *gap;
    dispatch_barrier_sync(concurrentQueue,
                          ^{
                              std::ofstream fout(R"(/Users/bossofthisgym/Documents/ThreadWork/Log.txt)", std::ios::app);
                              for (int i = 0; i < counter; ++i)
                                  for (auto Character : Info)
                                      fout << Character;

                              fout.close();
                          });

    FinishedThreadCounter--;
    return 0;
}

void MainWindow::on_commitButton_clicked()
{
    bool ok, found = false;
    int current_tid = ui->lineEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::information(this, "Error!", "TID is not valid!");
        return;
    }
    for (int i = 0; i < tid.size(); ++i) {
        if (current_tid == tid[i]) {
            found = true;
        }
    }

    if (!found) {
        QMessageBox::information(this, "Error!", "TID is not valid!");
        return;
    }

    switch (ui->ThreadOptionsBox->currentIndex()) {
    case CommitAction::Priority:
        struct sched_param param;
        param.sched_priority = ui->spinBox->value();
        if(pthread_setschedparam(th[current_tid], SCHED_FIFO, &param) != 0){
            QMessageBox::information(this, "Error", "Failed to set priority!");
        }
        break;

    case CommitAction::Detach:
        if (pthread_detach(th[current_tid]) != 0) {
            QMessageBox::information(this, "Error", "Failed to detach process!");
        }
        break;

    case CommitAction::Cancel:
        if (pthread_cancel(th[current_tid]) != 0) {
            QMessageBox::information(this, "Error", "Failed to cancel thread!");
        }
        break;
    }
}

void MainWindow::on_startButton_clicked()
{
    if (!th.empty()) {
        for (int i = 0; i < th.size(); ++i) {
            pthread_cancel(th[i]);
        }

        th.clear();
    }

    int ThreadNum = ui->comboBox->currentText().toInt();

    std::ofstream fout(R"(/Users/bossofthisgym/Documents/ThreadWork/Log.txt)", std::ios::trunc);
    fout.close();

    th.resize(ThreadNum);
    tid.resize(ThreadNum);
    FinishedThreadCounter = ThreadNum;

    if (ThreadNum == 1) {
        StartTime = std::chrono::high_resolution_clock::now();
        if (pthread_create(&th[0], &attr, &MainWindow::th_func, &GeneralGap) != 0) {
            QMessageBox::information(this, "Error", "Failed to create thread!");
        }
        tid[0] = 0;
        OutputInfo();
    } else {
        int Gap = GeneralGap / ThreadNum;
        th.resize(ThreadNum);

        bool bConditionalVariables = ui->CriticalRadio->isChecked();
        bool bBarrier = ui->TimerRadio->isChecked();

        if (bConditionalVariables) {
            StartTime = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < ThreadNum; ++i) {
                pthread_create(&th[i], &attr, &MainWindow::th_func_cond_var, &Gap);
                tid[i] = i;
            }
        } else if (bBarrier) {
            StartTime = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < ThreadNum; ++i) {
                pthread_create(&th[i], &attr, &MainWindow::th_func_barrier, &Gap);
                tid[i] = i;
            }
        } else {
            StartTime = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < ThreadNum; ++i) {
                pthread_create(&th[i], &attr, &MainWindow::th_func, &Gap);
                tid[i] = i;
            }

            OutputInfo();
        }
    }
}

