#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "login.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
    , serial_send(new QSerialPort(this)),
    startTime(QDateTime::currentDateTime()),
    centerXSeries(new QLineSeries(this)),
    centerYSeries(new QLineSeries(this)),
    laserSeries(new QLineSeries(this))
{
    ui->setupUi(this);
    mqtt_config();
    init_sql(); // 初始化 MYSQL
    init_timer();
    init_serial();
    working_time();
    setupCharts();
    if(client->state()==QMqttClient::Disconnected){
        client->connectToHost();
        qDebug()<<"client connect successfully!";
    }
 // 连接成功信号到槽函数
//    connect(client, &QMqttClient::connected, this, &MainWindow::onMqttConnected);
    get_k210msg_and_send();
    connect(client, &QMqttClient::connected, this, [this](){
                QMqttTopicFilter topic("/k1gm3ZRZ0Xw/device1/user/get");
                subscription = client->subscribe(topic);
                if(subscription)
                {
            qDebug() << "subscribe successfully!";
                        connect(subscription, &QMqttSubscription::messageReceived, this, [this](QMqttMessage message)
                        {
                            QString msg = message.payload();
                            // 通过串口发送信号
                            if (serial_send->isOpen()) {
                                serial_send->write(msg.toUtf8());
                                qDebug()<<"send message to 1C:  "<<msg<<"  successfully";
                            } else {
                                qDebug() << "串口未连接!";
                            }
                        });
                }
    });
//    {
//        QMqttTopicFilter topic("/k1gm3ZRZ0Xw/device1/user/get");
//        QMqttSubscription *subscription = client->subscribe(topic);

//        if (subscription)
//        {
//            qDebug() << "subscribe successfully!";
//            connect(subscription, &QMqttSubscription::messageReceived, this, [this](QMqttMessage message)
//            {
//                QString msg = message.payload();

//                const QString content = QDateTime::currentDateTime().toString()
//                                        + QLatin1Char('\n')
//                                        + QLatin1String(" Message: ")
//                                        + QLatin1Char('\n')
//                                        + message.payload()
//                                        + QLatin1Char('\n');
//                ui->mqtt_message->insertPlainText(content);

//                // 插入消息到数据库
//                QString sql = QString("INSERT INTO weed_log (msg) VALUES ('%1');").arg(msg);
//                QSqlQuery query;
//                if (query.exec(sql)) {
//                    qDebug() << "Insert successful!";
//                } else {
//                    qDebug() << "Insert failed: " << query.lastError().text();
//                }
//            });
//        }
//    });
}

MainWindow::~MainWindow()
{
    serial->close();
    serial_send->close();
    delete ui;
}

void MainWindow::mqtt_config()
{
    client = new QMqttClient(this);
    client->setPort(1883);
    client->setHostname("iot-06z00ic3nf3rxha.mqtt.iothub.aliyuncs.com");
    client->setClientId("k1gm3ZRZ0Xw.device1|securemode=2,signmethod=hmacsha256,timestamp=1719709338988|");
    client->setUsername("device1&k1gm3ZRZ0Xw");
    client->setPassword("5ee9b0c7b1212af3e95d8aeec7c8bad7503e799837a5eb693d693d02c9aedbe6");
    client->setKeepAlive(120);
}

void MainWindow::init_sql()
{
    //连接数据库
    db = QSqlDatabase::addDatabase("QMYSQL");//加载MYsql数据库驱动
    db.setHostName("localhost");
    db.setDatabaseName("weed_test");
    db.setUserName("root");
    db.setPassword("!Zyzy3412");

        if(db.open()){
            qDebug()<<"打开数据库成功！";
        }
        else{
            qDebug()<<"打开数据库失败"<<db.lastError().text();
        }
}

void MainWindow::init_timer()
{
    timer_mqtt = new QTimer(this);
    timer_mysql = new QTimer(this);
}

void MainWindow::mysql_insert()
{
    QString content;
    int lineCount = ui->msg->document()->blockCount();
    if (lineCount > 0) {
        QTextBlock lastBlock = ui->msg->document()->findBlockByNumber(lineCount - 1);
        content = lastBlock.text().trimmed();
    } else {
        qDebug() << "PlainTextEdit is empty!";
        return;
    }
    QString sql = QString("INSERT INTO weed_log (msg) VALUES ('%1');").arg(content);
    QSqlQuery query;
    if (query.exec(sql)) {
        QMessageBox::information(this, "插入提示", "插入成功！");
    } else {
        QMessageBox::information(this, "插入提示", "插入失败！");
    }
}

void MainWindow::on_pushButton_clicked()
{
    mysql_insert();
}

void MainWindow::on_query_btn_clicked()
{
    QString sql = QString("SELECT * FROM weed_log WHERE DATE(time) = '%1';").arg(dateString);
    qDebug() << dateString;
    QSqlQuery query;

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << "编号" << "中心点坐标" << "长度" << "宽度");

    if (query.exec(sql)) {
        qDebug() << "\nQuery executed successfully!";
        while (query.next()) {
            int id = query.value(0).toInt();
            QString msg = query.value(1).toString();
            QDateTime time = query.value(2).toDateTime();

            qDebug() << "ID:" << id << "Message:" << msg << "Time:" << time;

            // 解析 msg 字符串，假设格式为 "中心点坐标: (%1, %2), 检测框长度: %3, 检测框高度: %4"
            QRegExp regex("中心点坐标: \\((\\d+), (\\d+)\\), 检测框长度: (\\d+), 检测框高度: (\\d+)");
            if (regex.indexIn(msg) != -1) {
                int centerx = regex.cap(1).toInt();
                int centery = regex.cap(2).toInt();
                int w = regex.cap(3).toInt();
                int h = regex.cap(4).toInt();

                QString centerPoint = QString("(%1, %2)").arg(centerx).arg(centery);

                QList<QStandardItem *> row;
                row.append(new QStandardItem(QString::number(id)));
                row.append(new QStandardItem(centerPoint));
                row.append(new QStandardItem(QString::number(w)));
                row.append(new QStandardItem(QString::number(h)));

                model->appendRow(row);
            } else {
                qDebug() << "Failed to parse message:" << msg;
            }
        }
    } else {
        qDebug() << "Query failed: " << query.lastError().text();
    }

    ui->query_table->setModel(model);
    ui->query_table->horizontalHeader()->setMinimumSectionSize(120);
    ui->query_table->horizontalHeader()->setMaximumSectionSize(120);
    ui->query_table->resizeColumnsToContents();
}

void MainWindow::on_date_choice_clicked(const QDate &date)
{
    dateString = date.toString("yyyy-MM-dd");
    qDebug() << dateString;
}

void MainWindow::exportToCsv()
{
    // 查询数据库并更新数据
    queryDatabase();

    // 使用查询日期命名 CSV 文件
    QString fileName = QString("query_result_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8"); // 设置编码为 UTF-8

    // 写入表头
    out << "ID,Message,Time\n";

    // 写入数据行
    for (const QStringList &row : qAsConst(dataRows)) {
        out << row.join(",") << "\n";
    }

    file.close();
    qDebug() << "File saved to:" << filePath;
}


void MainWindow::init_serial()
{
    serial->setPortName("/dev/ttyS1"); // 根据实际情况改串口名称
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    serial_send->setPortName("/dev/ttyS2"); // 根据实际情况修改串口名称
    serial_send->setBaudRate(QSerialPort::Baud115200);
    serial_send->setDataBits(QSerialPort::Data8);
    serial_send->setParity(QSerialPort::NoParity);
    serial_send->setStopBits(QSerialPort::OneStop);
    serial_send->setFlowControl(QSerialPort::NoFlowControl);

}

void MainWindow::HandleSerialData()
{
    buffer.append(serial->readAll());

    // 查找包头和包尾
    int start = buffer.indexOf('\xaa');
    int end = buffer.indexOf('\xbb', start + 1);

    while (start != -1 && end != -1 && end > start) {
        QByteArray data = buffer.mid(start + 1, end - start - 1);
        QDataStream stream(&data, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian); // 设置为小端序

        int x, y, w, h;
        stream >> x >> y >> w >> h;
        int centerx=x+w/2;
        int centery=y+h/2;
        // 数据验证
        if (w > 0 && h > 0 && w < 320 && h < 320 && x > 0 && x < 320 && y > 0 && y < 320&&x>0&&x<320&&centery<180) {
            QString strData = QString("中心点坐标: (%1, %2), 检测框长度: %3, 检测框高度: %4").arg(centerx).arg(centery).arg(w).arg(h);
            ui->msg->append(strData); // 将数据追加显示在 QTextEdit 中

            // 更新图表
            updateCharts(centerx, centery);

            // 插入消息到数据库
            QString sql = QString("INSERT INTO weed_log (msg) VALUES ('%1');").arg(strData);
            QSqlQuery query;
            if (query.exec(sql)) {
//                qDebug() << "Insert successful!";
            } else {
                qDebug() << "Insert failed: " << query.lastError().text();
            }

         initialAngle = 90;
         int angle=initialAngle;
         if(centerx<160&&centerx>=100)
         {
             angle=initialAngle+(160-centerx)*26/160;
         }
         else if(centerx>=160&&centerx<=220)
         {
             angle=initialAngle-(centerx-160)*26/160;
         }
         else if(centerx>=40&&centerx<=100)
         {
             angle=initialAngle+(160-centerx)*23/160;
         }
         else if(centerx>220&&centerx<=280)
         {
             angle=initialAngle-(centerx-160)*23/160;
         }
         else if(centerx>280)
         {
             angle=initialAngle-(centerx-160)*20/160;
         }
         else if(centerx<40)
         {
             angle=initialAngle+(160-centerx)*20/160;
         }

// 激光控制

         if(centery>=0&&centery<=100){
             laser=1;
             updade_weed_record(angle);
         }
         if(y<40)
         {
             QString signal = QString("%1%2").arg(angle, 3, 10, QChar('2')).arg(laser);
             qDebug() << "Signal:" << signal;

     // 通过 MQTT 发布信号
                if (client->state() == QMqttClient::Connected) {
                    client->publish(QMqttTopicName("/k1gm3ZRZ0Xw/device1/user/update"), signal.toUtf8());
                } else {
                    qDebug() << "MQTT client is not connected!";
                }

                 // 通过串口发送信号
                 if (serial_send->isOpen()) {
                     serial_send->write(signal.toUtf8());
                     qDebug()<<"send message to 1C:  "<<signal<<"  successfully";
                 } else {
                     qDebug() << "串口未连接!";
                 }
         }
}
        // 移动到下一个包
        buffer.remove(0, end + 1);
        start = buffer.indexOf('\xaa');
        end = buffer.indexOf('\xbb', start + 1);
    }
}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ReadError) {
        qDebug() << "Serial port read error occurred.";
    } else {
        qDebug() << "Serial port error occurred:" << error;
    }
    // 可以根据具体情况添加其他错误处理逻辑
}

void MainWindow::get_k210msg_and_send()
{
    int flag1=serial_send->open(QIODevice::WriteOnly);
    int flag2=serial->open(QIODevice::ReadOnly);
    if(flag1&&flag2) {
        qDebug() << "Serial port opened successfully.";
        connect(serial, &QSerialPort::readyRead, this, &MainWindow::HandleSerialData);
    } else {
        qDebug() << "Failed to open serial port.";
        qDebug() << serial->errorString();
    }

    // 处理串口错误
    connect(serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(serial_send, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
}

void MainWindow::sendSignal(const QString &signal)
{
    if (serial->isOpen()) {
        serial->write(signal.toUtf8());
    } else {
        qDebug() << "串口未连接!";
    }
}

//void MainWindow::init_lx1c_into90()
//{
//    // 通过 MQTT 发布信号 init_laser
//    if (client->state() == QMqttClient::Connected) {
//        QString signal = QString("0900");
//        client->publish(QMqttTopicName("/k1gm3ZRZ0Xw/device1/user/update"), signal.toUtf8());
//        qDebug()<<"send 0900 signal successfully!";
//    } else {
//        qDebug() << "MQTT client is not connected!";
//    }
//}

//void MainWindow::onMqttConnected()
//{
//    qDebug() << "MQTT client connected successfully!";
//    init_lx1c_into90();
//}



void MainWindow::on_btn_export_clicked()
{
                exportToCsv();
}


void MainWindow::on_btn_database_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btn_index_clicked()
{
        ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_btn_msg_clicked()
{
        ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btn_plot_clicked()
{
        ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::working_time()
{
    // 连接定时器，用于更新时间
    connect(&m_tUpdateTime, &QTimer::timeout, this, [=]()
    {
        // 计算程序运行的总秒数
        int secondsElapsed = startTime.secsTo(QDateTime::currentDateTime());
        int hours = secondsElapsed / 3600;
        int minutes = (secondsElapsed % 3600) / 60;
        int seconds = secondsElapsed % 60;

        // 格式化显示的时间
        QString timeString = QString("%1:%2:%3")
                             .arg(hours, 2, 10, QChar('0'))
                             .arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0'));

        ui->lcdNumber->display(timeString);

        // 显示运行总时间（天、小时、分钟）
        int days = hours / 24;
        int displayHours = hours % 24;
        QString totalTimeString = QString("%1天 %2时 %3分").arg(days).arg(displayHours).arg(minutes);
//        ui->label_totalTime->setText(totalTimeString);
    });

    m_tUpdateTime.start(1000); // 启动定时器，每秒更新一次
}

void MainWindow::updade_weed_record(int angle)
{
    QString sql = QString("INSERT INTO weed_record (angle) VALUES ('%1');").arg(angle);
    QSqlQuery query;
    if (query.exec(sql)) {
//                qDebug() << "Insert successful!";
    } else {
        qDebug() << "Insert failed: " << query.lastError().text();
    }
}


void MainWindow::updateCharts(int x, int y)
{
    static int time = 0;
    centerXSeries->append(time, x);
    centerYSeries->append(time, y);
    time++;

    // 更新X轴范围，使其显示最新的100个点
    QChart *centerXChart = ui->centerx_chart->chart();
    centerXChart->axes(Qt::Horizontal).back()->setRange(qMax(0, time - 50), time);

    QChart *centerYChart = ui->centery_chart->chart();
    centerYChart->axes(Qt::Horizontal).back()->setRange(qMax(0, time - 50), time);
}

void MainWindow::setupCharts()
{
    // 创建中心坐标X图表
    QChart *centerXChart = new QChart();
    centerXChart->legend()->hide();
    centerXChart->addSeries(centerXSeries);
    centerXChart->createDefaultAxes();
    centerXChart->axes(Qt::Horizontal).back()->setRange(0,50); // 时间范围，可根据实际需要调整
    centerXChart->axes(Qt::Vertical).back()->setRange(0, 320);// 中心坐标X的范围
    centerXChart->setTitle("Center X Coordinate");
    ui->centerx_chart->setChart(centerXChart);

    // 创建中心坐标Y图表
    QChart *centerYChart = new QChart();
    centerYChart->legend()->hide();
    centerYChart->addSeries(centerYSeries);
    centerYChart->createDefaultAxes();
    centerYChart->axes(Qt::Horizontal).back()->setRange(0,50); // 时间范围，可根据实际需要调整
    centerYChart->axes(Qt::Vertical).back()->setRange(0, 240); // 中心坐标Y的范围
    centerYChart->setTitle("Center Y Coordinate");
    ui->centery_chart->setChart(centerYChart);

}

void MainWindow::queryDatabase()
{
    dataRows.clear(); // 清空现有数据

    QSqlQuery query("SELECT id, msg, time FROM weed_log ORDER BY time ASC");
    while (query.next()) {
        QStringList row;
        row << query.value(0).toString() // ID
            << query.value(1).toString() // Message
            << query.value(2).toString(); // Time
        dataRows.append(row);
    }
}


