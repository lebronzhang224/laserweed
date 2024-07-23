#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    //设置窗口标题
    this->setWindowTitle("系统登录");
    initdatabase();
    //设置密文登录
    login::ui->password->setEchoMode(QLineEdit::Password);
    QApplication::setQuitOnLastWindowClosed(false);

}

login::~login()
{
    delete ui;
}

void login::on_pushButton_clicked()
{
    handleLogin();
}

QString login::getUsername() const
{
    return login::ui->username->text();
}

QString login::getPassword() const
{
    return login::ui->password->text();
}

void login::initdatabase()
{
    // Replace with your database connection details
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("weed_test");
    db.setUserName("root");
    db.setPassword("!Zyzy3412");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Connection Error", db.lastError().text());
        return;
    }
}

void login::handleLogin()
{
    QString username = getUsername();
    QString password = getPassword();
    QString sql = QString("SELECT * FROM user WHERE username = '%1' AND password ='%2' ").arg(username).arg(password);
    QSqlQuery query;

    if (query.exec(sql)) {
        if(query.next())
        {
            QString message = tr("尊敬的用户:'%1'").arg(username);
            QMessageBox::about(this, message, tr("欢迎使用自动化激光除草系统!!!"));
            this->close();
            MainWindow *w=new MainWindow;
            w->show();
        }
        else{
                QMessageBox::warning(this,tr("登录失败"),tr("用户名或密码输入错误！"),QMessageBox::Ok);
                login::ui->username->clear();
                login::ui->password->clear();
                login::ui->username->setFocus();
            }
    }else{        QMessageBox::critical(this, "Query Error", query.lastError().text());
        return;}
}
