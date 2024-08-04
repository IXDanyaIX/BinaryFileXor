#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , settings(new QSettings("Settings", "MySettings", this))
{
    ui->setupUi(this);
    connect(ui->saveSettingsButton, &QPushButton::clicked, this, &MainWindow::on_saveSettingsButton_clicked);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::on_selectFolderButton_clicked);
    connect(timer, &QTimer::timeout, this, &MainWindow::on_timerTimeout);
    connect(ui->overwriteCheckBox, &QCheckBox::clicked, this, &MainWindow::on_overwriteCheckBox_clicked);
    connect(ui->periodicRunCheckBox, &QCheckBox::clicked, this, &MainWindow::on_periodicRunCheckBox_clicked);
    loadSettings();
    on_overwriteCheckBox_clicked();
    on_periodicRunCheckBox_clicked();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loadSettings() {
    inputMask = settings->value("inputMask", "").toString();
    deleteInputFiles = settings->value("deleteInputFiles", false).toBool();
    outputPath = settings->value("outputPath", "").toString();
    overwriteOutput = settings->value("overwriteOutput", false).toBool();
    periodicRun = settings->value("periodicRun", false).toBool();
    pollInterval = settings->value("pollInterval", 10).toInt();
    modificationValue = settings->value("modificationValue", "").toByteArray();
    inputFolder = settings->value("inputFolder", "").toString();

    ui->inputMaskLineEdit->setText(inputMask);
    ui->deleteFilesCheckBox->setChecked(deleteInputFiles);
    ui->outputPathLineEdit->setText(outputPath);
    ui->overwriteCheckBox->setChecked(overwriteOutput);
    ui->periodicRunCheckBox->setChecked(periodicRun);
    ui->intervalSpinBox->setValue(pollInterval);
    ui->modificationValueLineEdit->setText(QString(modificationValue));
    ui->inputFolderLineEdit->setText(inputFolder);
}

void MainWindow::saveSettings() {
    settings->setValue("inputMask", ui->inputMaskLineEdit->text());
    settings->setValue("deleteInputFiles", ui->deleteFilesCheckBox->isChecked());
    settings->setValue("outputPath", ui->outputPathLineEdit->text());
    settings->setValue("overwriteOutput", ui->overwriteCheckBox->isChecked());
    settings->setValue("periodicRun", ui->periodicRunCheckBox->isChecked());
    settings->setValue("pollInterval", ui->intervalSpinBox->value());
    settings->setValue("modificationValue", ui->modificationValueLineEdit->text().toUtf8());
    settings->setValue("inputFolder", ui->inputFolderLineEdit->text());
}

void MainWindow::on_saveSettingsButton_clicked() {
    saveSettings();
}

void MainWindow::on_selectFolderButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выберите папку"), ui->inputFolderLineEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->inputFolderLineEdit->setText(dir);
    }
}

void MainWindow::on_startButton_clicked() {

    inputFolder = ui->inputFolderLineEdit->text();
    deleteInputFiles = ui->deleteFilesCheckBox->isChecked();
    outputPath = ui->outputPathLineEdit->text();
    overwriteOutput = ui->overwriteCheckBox->isChecked();
    periodicRun = ui->periodicRunCheckBox->isChecked();
    pollInterval = ui->intervalSpinBox->value();
    modificationValue = ui->modificationValueLineEdit->text().toUtf8();
    //saveSettings();  // Сохранение настроек перед запуском
    if (periodicRun) {
        timer->start(ui->intervalSpinBox->value() * 1000);  // Запуск таймера для периодической обработки

    } else {
        processFiles();  // Немедленный запуск одноразовой обработки
    }
}

void MainWindow::on_stopButton_clicked() {
    timer->stop();  // Остановка таймера, если он запущен
}

void MainWindow::on_overwriteCheckBox_clicked(){
    if (ui->overwriteCheckBox->isChecked()){
        ui->outputPathLineEdit->setEnabled(false);

    }
    else {
        ui->outputPathLineEdit->setEnabled(true);

    }
}

void MainWindow::on_periodicRunCheckBox_clicked(){
    if (ui->periodicRunCheckBox->isChecked()){
        ui->intervalSpinBox->setEnabled(true);
    }
    else {
        ui->intervalSpinBox->setEnabled(false);
    }
}

void MainWindow::on_timerTimeout() {
    processFiles();
}

void MainWindow::processFiles() {
    QDir dir(ui->inputFolderLineEdit->text());
    QStringList files = dir.entryList({ui->inputMaskLineEdit->text()}, QDir::Files);

    for (const QString &fileName : files) {
        QString filePath = dir.absoluteFilePath(fileName);
        QFileInfo fileInfo(filePath);
        if (!fileInfo.isWritable()) {
            qWarning() << "File is open or not writable:" << filePath;
            continue;
        }
        modifyFile(filePath);
    }
}

void MainWindow::modifyFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    for (int i = 0; i < fileData.size(); ++i) {
        fileData[i] ^= modificationValue[i % modificationValue.size()];
    }

    QString outputFilePath = outputPath + "/" + QFileInfo(filePath).fileName();
    if (QFile::exists(outputFilePath) && !overwriteOutput) {
        int counter = 1;
        while (QFile::exists(outputFilePath)) {
            outputFilePath = outputPath + "/" + QFileInfo(filePath).completeBaseName()
                             + "_" + QString::number(counter++)
                             + "." + QFileInfo(filePath).suffix();
        }
    }

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << outputFilePath;
        return;
    }

    outputFile.write(fileData);
    outputFile.close();

    if (deleteInputFiles) {
        file.remove();
    }
}
