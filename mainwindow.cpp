#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TRUE 1
#define FALSE 0

#define REGISTRY_HASH "AA621736FE16A116452AEF1FCE8E9AB026229C78226E394\
EF3249E75305799C0B43D0E1E4F72DEB8515719021CAE3445318AEB929726ED7D648EB\
D5C20918276SERVERBACKUP"\

#define REGISTRY_SOFTWARE_HASH "L2M9SNR"

#define REGISTRY_KEY_EXISTS "YDG3X2M"

#define REGISTRY_KEY_RECOVERABLE "7N3MPX4"

#define REGISTRY_KEY_HOSTNAME "Q2WU6O9"

#define REGISTRY_KEY_USERNAME "IRV31SZ"

#define REGISTRY_KEY_PASSWORD "16L75HX"

#define REGISTRY_KEY_ATTEMPTS "KTDJ7M9"

#define EXIT_ANIMATION_LENGHT 500

#define EXIT_ANIMATION_CURVE QEasingCurve::InOutCirc

#define FAVORITE_TAB_INDEX 2

#define RESTART_PROGRAM qApp->quit();\
QProcess::startDetached(qApp->arguments()[0], qApp->arguments());

#define RESET_ATTEMPTS attempts = 11;\
settings.setValue(REGISTRY_KEY_ATTEMPTS, 11);

// SHA512 of serverbackup:
// AA621736FE16A116452AEF1FCE8E9AB026229C78226E394EF3249E75305799C0B43D
// 0E1E4F72DEB8515719021CAE3445318AEB929726ED7D648EBD5C20918276

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    AESObject(QAESEncryption::AES_256, QAESEncryption::ECB)
{
    ui->setupUi(this);

    setMinimumSize(0, 0);

    QSettings settings(QString(REGISTRY_HASH), QSettings::NativeFormat, this);

    ui->file2Group->hide();
    ui->file1Text->hide();
    ui->downloadF1Button->hide();

    exitAnimation = new QPropertyAnimation(this, "windowOpacity");
    exitAnimation->setEasingCurve(EXIT_ANIMATION_CURVE);
    exitAnimation->setDuration(EXIT_ANIMATION_LENGHT);
    exitAnimation->setStartValue(qreal(1));
    exitAnimation->setEndValue(qreal(0));

    connect(exitAnimation, SIGNAL(finished()), qApp, SLOT(quit()));

    checkRegistryKey();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();

    settings.setValue(REGISTRY_KEY_ATTEMPTS, attempts);

    ui->centralWidget->hide();

//    exitAnimation->setStartValue(geometry());
//    exitAnimation->setEndValue(QRect(x(), y()+height()/2, width(), 0));
    exitAnimation->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::on_compareCheckBox_clicked()
{
    ui->file2Group->setHidden(ui->compareCheckBox->isChecked() ^ TRUE);
    ui->file1Text->setHidden(ui->compareCheckBox->isChecked() ^ TRUE);
    ui->downloadF1Button->setHidden
            (ui->compareCheckBox->isChecked() ^ TRUE);
}

void MainWindow::on_favoriteFilesButton_clicked()
{
    ui->tabWidget->setCurrentIndex(FAVORITE_TAB_INDEX);
}

void MainWindow::checkRegistryKey()
{
    if (settings.value(REGISTRY_KEY_EXISTS) != TRUE) {
        configureSoftware();
    }
    else {
        LogIn();
    }
}

bool MainWindow::LogIn()
{
    bool ok;
    attempts = settings.value(REGISTRY_KEY_ATTEMPTS).toInt();

    while ((QString::fromStdString(sha256(softwarePassword.toStdString()))
           != settings.value(REGISTRY_SOFTWARE_HASH).toString()) &&
           (--attempts > 0)) {

        softwarePassword = QInputDialog::getText(this, "Log in",
QString::number(attempts) + " attemps remaining.\n" +
"Software password:", QLineEdit::Password, "", &ok);

        if (ok == false) {
            attempts++;
            close();
            return false;
        }
    }

    if (attempts < 1) {
        settings.clear();
        QMessageBox::critical(this, "Reset", "No remaining tries left."
                 " The software has been reset and will now exit.");
        close();
        return false;
    }

    RESET_ATTEMPTS
    return true;
}

void MainWindow::on_resetButton_clicked()
{
    int ok = QMessageBox::warning(this, "Warning",
"Do you really want to reset your data ? "
"You will still be able to recover most of it.",
QMessageBox::Yes, QMessageBox::No);

    if (ok == QMessageBox::No)
        return;

    settings.setValue(REGISTRY_KEY_EXISTS, FALSE);

    QMessageBox::information(this, "Information",
                   "The software will now restart");

    RESTART_PROGRAM
}

void MainWindow::on_hResetButton_clicked()
{
    int ok = QMessageBox::warning(this, "Warning",
"Do you really want to reset your data ? "
"You won't be able to recover any of it.",
QMessageBox::Yes, QMessageBox::No);

    if (ok == QMessageBox::No)
        return;

    settings.clear();

    QMessageBox::information(this, "Information",
                   "The software will now restart");

    RESTART_PROGRAM
}

void MainWindow::configureSoftware()
{
    int ok = QMessageBox::question(this, "Welcome",
"It seems to be the first time you run this program. "
"You must configure it first. Continue ?",
QMessageBox::Yes, QMessageBox::Abort);

    if (ok == QMessageBox::Abort) {
        close();
        return;
    }

    int recover(0);

    if (settings.value(REGISTRY_KEY_RECOVERABLE) == TRUE) {
        recover = QMessageBox::question(this, "Recover",
"A previous installation was detected. "
"Do you want to try to recover data from it ?",
QMessageBox::Yes, QMessageBox::No);
    }

    if (recover == QMessageBox::Yes){
        if (LogIn() != true)
            return;
        QByteArray key = softwarePassword.toLatin1();
        configInstance = new configDialog(
QString::fromLatin1(AESObject.decode(settings.value(
REGISTRY_KEY_HOSTNAME).toByteArray(), key)),

QString::fromLatin1(AESObject.decode(settings.value(
REGISTRY_KEY_USERNAME).toByteArray(), key)),

QString::fromLatin1(AESObject.decode(settings.value(
REGISTRY_KEY_PASSWORD).toByteArray(), key)), this);
        configInstance->show();
    }
    else {
        configInstance = new configDialog(this);
        configInstance->show();
    }

    configInstance->setModal(true);

    connect(configInstance, SIGNAL(rejected()), this, SLOT(close()));
    connect(configInstance, SIGNAL(accepted()), this, SLOT(connectionTest()));

}

void MainWindow::connectionTest()
{
    //test

    //then if it went well:

RESET_ATTEMPTS

QByteArray key = softwarePassword.toLatin1();

QAESEncryption instance(QAESEncryption::AES_256, QAESEncryption::ECB);

settings.setValue(REGISTRY_KEY_EXISTS, TRUE);
settings.setValue(REGISTRY_KEY_RECOVERABLE, TRUE);

settings.setValue(REGISTRY_SOFTWARE_HASH, QString::fromStdString(
sha256(configInstance->sPassPtr->text().toStdString())));

settings.setValue(REGISTRY_KEY_HOSTNAME, AESObject.encode(configInstance
->hostPtr->text().toLatin1(), key));

settings.setValue(REGISTRY_KEY_USERNAME, AESObject.encode(configInstance
->usrPtr->text().toLatin1(), key));

settings.setValue(REGISTRY_KEY_PASSWORD, AESObject.encode(configInstance
->passPtr->text().toLatin1(), key));

    QMessageBox::information(this, "Success", "Data successfully saved.");
}

