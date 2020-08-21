#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setupLayout();

    void Get(const QString& cmd, const std::map<QString, QString>& param);

private:
    QPushButton* btnXAscend;
    QPushButton* btnYAscend;
    QPushButton* btnZAscend;
    QPushButton* btnXDescend;
    QPushButton* btnYDescend;
    QPushButton* btnZDescend;

    QPushButton* btnStartManual;
    QPushButton* btnStartAuto;
    QPushButton* btnStopServer;

    QPushButton* btnPause;
    QPushButton* btnStop;

    QCheckBox* checkboxIsFlash;
    QPushButton* btnSetFlashInterval;

    QLineEdit* editFlashInterval;
};
#endif // MAINWINDOW_H
