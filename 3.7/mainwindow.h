#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "login.h"
#include <QMainWindow>
#include <QTimer>
#include <QPixmap>
//弹窗
#include <QMessageBox>
#include <QtMqtt/qmqttclient.h>
#include <QtDebug>
#include<QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <qmqttsubscription.h>
#include <QTextBlock>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDataStream>
#include <QPixmap>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QDateTime>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>

QT_CHARTS_USE_NAMESPACE

//using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_query_btn_clicked();
    void on_date_choice_clicked(const QDate &date);
    void exportToCsv();
    void HandleSerialData();
    void handleError(QSerialPort::SerialPortError error);
    void updateCharts(int x, int y);
    void queryDatabase();

    void on_btn_export_clicked();

    void on_btn_database_clicked();

    void on_btn_index_clicked();

    void on_btn_msg_clicked();

    void on_btn_plot_clicked();

private:
    Ui::MainWindow *ui;
    QMqttClient *client;
    QTimer *timer_mqtt,*timer_mysql;
    QString dateString;
    QMqttSubscription *subscription;
    QString latestMessage;
    QSqlDatabase db;
    QTextBlock *lastBlock;
    QList<QStringList> dataRows;
    QSerialPort *serial,*serial_send;
    QByteArray buffer; // 临时缓冲区
    int laser=0;
    int initialAngle;
    QDateTime startTime;			//开始计时的时间
    QDateTime currentTime;			//当前时间
    QString strInterval;			//运行总时间
    QTimer m_tUpdateTime;			//定时器
    QLineSeries *centerXSeries;
    QLineSeries *centerYSeries;
    QLineSeries *laserSeries;
//    QDateTime startTime;
    QDateTime lastLaserOnTime;
    void init_camera();
    void mqtt_config();
    void init_sql();
    void init_timer();
    void mysql_insert();
    void init_serial();
    void get_k210msg_and_send();
    void init_lx1c_into90();
    void sendSignal(const QString &signal);
    void working_time();
    void updade_weed_record(int angle);
    void setupCharts();
};
#endif // MAINWINDOW_H

