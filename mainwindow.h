#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_saveSettingsButton_clicked();
    void on_selectFolderButton_clicked();
    void on_startButton_clicked();         // Слот для кнопки "Старт"
    void on_stopButton_clicked();          // Слот для кнопки "Стоп"
    void on_timerTimeout();
    void on_overwriteCheckBox_clicked();
    void on_periodicRunCheckBox_clicked();


private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QSettings *settings;
    QByteArray modificationValue;
    QString inputMask;
    bool deleteInputFiles;
    QString outputPath;
    bool overwriteOutput;
    bool periodicRun;
    int pollInterval;
    QString inputFolder;

    void processFiles();
    void modifyFile(const QString &filePath);
    void loadSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
