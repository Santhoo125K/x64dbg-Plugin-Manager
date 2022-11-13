// Copyright (c) 2019-2022 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "dialogconvertprocess.h"

#include "ui_dialogconvertprocess.h"

DialogConvertProcess::DialogConvertProcess(QWidget *pParent, Utils::MDATA *pMData, QString sDataPath) : QDialog(pParent), ui(new Ui::DialogConvertProcess) {
    ui->setupUi(this);

    this->pMData = pMData;
    this->sDataPath = sDataPath;

    pConvertProcess = new ConvertProcess;
    pThread = new QThread;

    pConvertProcess->moveToThread(pThread);

    connect(pThread, SIGNAL(started()), pConvertProcess, SLOT(process()));
    connect(pConvertProcess, SIGNAL(completed(qint64)), this, SLOT(onCompleted(qint64)));
    connect(pConvertProcess, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

    pTimer = new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));

    pConvertProcess->setData(pMData, sDataPath);

    bIsRun = true;

    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);

    pThread->start();
    pTimer->start(100);  // 0.1 sec
}

DialogConvertProcess::~DialogConvertProcess() {
    if (bIsRun) {
        pConvertProcess->stop();
    }

    pTimer->stop();

    pThread->quit();
    pThread->wait();

    delete ui;

    delete pThread;
    delete pConvertProcess;
}

void DialogConvertProcess::on_pushButtonCancel_clicked() {
    if (bIsRun) {
        pConvertProcess->stop();
        pTimer->stop();
        bIsRun = false;
    }
}

void DialogConvertProcess::onCompleted(qint64 nElapsed) {
    Q_UNUSED(nElapsed)
    // TODO
    bIsRun = false;
    this->close();
}

void DialogConvertProcess::timerSlot() {
    Utils::STATS stats = pConvertProcess->getCurrentStats();

    ui->labelInfo->setText(stats.sModule);

    if (stats.nTotalModule) {
        ui->progressBar->setValue((int)((stats.nCurrentModule * 100) / stats.nTotalModule));
    }
}
