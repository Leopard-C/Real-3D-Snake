#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QFrame>
#include <unistd.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupLayout();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupLayout() {
    btnStartManual = new QPushButton("Manual", this);
    btnStartAuto = new QPushButton("AutoRun", this);

    btnPause = new QPushButton("Pause", this);
    btnStop = new QPushButton("Stop", this);

    checkboxIsFlash = new QCheckBox("Flash on/off", this);
    checkboxIsFlash->setCheckState(Qt::Unchecked);
    QLabel* labelFlashInterval = new QLabel("Interval(ms):", this);
    editFlashInterval = new QLineEdit(this);
    editFlashInterval->setMaximumWidth(35);
    editFlashInterval->setText("150");
    btnSetFlashInterval = new QPushButton("Set Interval", this);

    btnStopServer = new QPushButton("Stop Server", this);
    btnXAscend = new QPushButton("X_Ascend", this);
    btnYAscend = new QPushButton("Y_Ascend", this);
    btnZAscend = new QPushButton("Z_Ascend", this);
    btnXDescend = new QPushButton("X_Descend", this);
    btnYDescend = new QPushButton("Y_Descend", this);
    btnZDescend = new QPushButton("Z_Descend", this);

    btnPause->setShortcut(Qt::Key_P);
    btnStop->setShortcut(Qt::Key_T);

    btnYAscend->setShortcut(Qt::Key_W);
    btnXDescend->setShortcut(Qt::Key_A);
    btnYDescend->setShortcut(Qt::Key_S);
    btnXAscend->setShortcut(Qt::Key_D);

    btnZDescend->setShortcut(Qt::Key_J);
    btnZAscend->setShortcut(Qt::Key_K);

    // Frame 1:
    QFrame* frame1 = new QFrame(this);
    QVBoxLayout* layoutCtl1 = new QVBoxLayout(frame1);
    frame1->setFrameShape(QFrame::Panel);
    frame1->setFrameShadow(QFrame::Raised);
    frame1->setStyleSheet("background:#999999");
    frame1->setLineWidth(2);
    frame1->setMidLineWidth(3);

    layoutCtl1->addWidget(btnStartManual);
    layoutCtl1->addWidget(btnStartAuto);
    layoutCtl1->addWidget(btnStopServer);

    // Frame 1.5:
    QFrame* frame1_5 = new QFrame(this);
    QVBoxLayout* layoutCtl1_5 = new QVBoxLayout(frame1_5);
    frame1_5->setFrameShape(QFrame::Panel);
    frame1_5->setFrameShadow(QFrame::Raised);
    frame1_5->setStyleSheet("background:#999999");
    frame1_5->setLineWidth(2);
    frame1_5->setMidLineWidth(3);

    QHBoxLayout* flashIntervalLayout = new QHBoxLayout();
    flashIntervalLayout->addWidget(labelFlashInterval);
    flashIntervalLayout->addWidget(editFlashInterval);

    layoutCtl1_5->addWidget(checkboxIsFlash);
    layoutCtl1_5->addLayout(flashIntervalLayout);
    layoutCtl1_5->addWidget(btnSetFlashInterval);

    // Frame 2:
    QFrame* frame2 = new QFrame(this);
    QVBoxLayout* layoutCtl2 = new QVBoxLayout(frame2);
    frame2->setFrameShape(QFrame::Panel);
    frame2->setFrameShadow(QFrame::Raised);
    frame2->setStyleSheet("background:#777777");
    frame2->setLineWidth(2);
    frame2->setMidLineWidth(3);

    layoutCtl2->addWidget(btnPause);
    layoutCtl2->addWidget(btnStop);

    // Frame 3:
    QFrame* frame3 = new QFrame(this);
    QVBoxLayout* layoutXY = new QVBoxLayout(frame3);
    frame3->setFrameShape(QFrame::Panel);
    frame3->setFrameShadow(QFrame::Raised);
    frame3->setStyleSheet("background:#CCCCCC");
    frame3->setLineWidth(2);
    frame3->setMidLineWidth(3);

    QHBoxLayout* layout1 = new QHBoxLayout();
    layout1->addStretch();
    layout1->addWidget(btnYAscend);
    layout1->addStretch();

    QHBoxLayout* layout2 = new QHBoxLayout();
    layout2->addWidget(btnXDescend);
    layout2->addStretch();
    layout2->addWidget(btnXAscend);

    QHBoxLayout* layout3 = new QHBoxLayout();
    layout3->addStretch();
    layout3->addWidget(btnYDescend);
    layout3->addStretch();

    layoutXY->addLayout(layout1);
    layoutXY->addLayout(layout2);
    layoutXY->addLayout(layout3);


    // Frame 4:
    QFrame* frame4 = new QFrame(this);
    frame4->setFrameShape(QFrame::Panel);
    frame4->setFrameShadow(QFrame::Raised);
    frame4->setStyleSheet("background:#CCCCCC");
    frame4->setLineWidth(2);
    frame4->setMidLineWidth(3);

    QVBoxLayout* layoutZ = new QVBoxLayout(frame4);
    layoutZ->addWidget(btnZAscend);
    layoutZ->addStretch();
    layoutZ->addWidget(btnZDescend);

    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(frame1);
    mainLayout->addWidget(frame1_5);
    mainLayout->addWidget(frame2);
    mainLayout->addWidget(frame3);
    mainLayout->addWidget(frame4);
    this->setCentralWidget(centralWidget);

    connect(btnStartManual, &QPushButton::clicked, this, [this]{
        Get("new-manual", {});
        btnPause->setText("Pause");
        checkboxIsFlash->setCheckState(Qt::Checked);
        checkboxIsFlash->setCheckState(Qt::Unchecked);
    });
    connect(btnStartAuto, &QPushButton::clicked, this, [this]{
        Get("new-auto", {});
        btnPause->setText("Pause");
        checkboxIsFlash->setCheckState(Qt::Unchecked);
        checkboxIsFlash->setCheckState(Qt::Checked);
    });
    connect(btnPause, &QPushButton::clicked, this, [this]{
        if (btnPause->text() == "Pause") {
            Get("pause", { { "state", "on" } });
            btnPause->setText("Resume");
        }
        else {
            Get("pause", { { "state", "off" }});
            btnPause->setText("Pause");
        }
        btnPause->setShortcut(Qt::Key_P);
    });
    connect(btnStop, &QPushButton::clicked, this, [this]{
        Get("stop", {});
        btnPause->setText("Pause");
        checkboxIsFlash->setCheckState(Qt::Checked);
    });

    connect(checkboxIsFlash, &QCheckBox::stateChanged, this, [this](int state){
        if (state == Qt::Checked) {
            Get("flash", { { "state", "on" } });
        }
        else {
            Get("flash", { { "state", "off" } });
        }
    });

    connect(btnSetFlashInterval, &QPushButton::clicked, this, [this](){
        QString interval = editFlashInterval->text();
        if (interval.isNull() || interval.isEmpty()) {
            return;
        }
        Get("set-flash-interval", { { "interval", interval } });
    });

    connect(btnStopServer, &QPushButton::clicked, this, [this]{
        Get("stop-server", {});
    });

    connect(btnXAscend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "X_ASCEND" } });
    });
    connect(btnYAscend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "Y_ASCEND" } });
    });
    connect(btnZAscend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "Z_ASCEND" } });
    });
    connect(btnXDescend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "X_DESCEND" } });
    });
    connect(btnYDescend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "Y_DESCEND" } });
    });
    connect(btnZDescend, &QPushButton::clicked, this, [this]{
        Get("move", { { "dir", "Z_DESCEND" } });
    });
}

void MainWindow::Get(const QString& cmd, const std::map<QString, QString>& params) {
    QNetworkAccessManager *m_pHttpMgr = new QNetworkAccessManager();

    // set url
    QString url = "http://192.168.1.102:1234/" + cmd;
    size_t paramsCount = params.size();
    if (paramsCount > 0) {
        url += "?";
        size_t count = 0;
        for (auto& param : params) {
            url += param.first;
            url += "=";
            url += param.second;
            if (++count < paramsCount)
                url += "&";
        }
    }
    QNetworkRequest requestInfo;
    requestInfo.setUrl(QUrl(url));

    qDebug() << url;

    //添加事件循环机制，返回后再运行后面的
    QEventLoop eventLoop;
    QNetworkReply *reply =  m_pHttpMgr->get(requestInfo);
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();       //block until finish
    //错误处理
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "request protobufHttp NoError";
    }
    else {
        qDebug()<<"request protobufHttp handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug( "request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug(qPrintable(reply->errorString()));
    }
    //请求返回的结果
    QByteArray responseByte = reply->readAll();
    qDebug() << responseByte;
}
