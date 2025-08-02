#pragma once

#include <QApplication>
#include "./ui_mainwindow.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSql>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QStandardItemModel>
#include <QRect>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionFileExit_triggered();

    void on_actionFileReadDb_triggered();

    void on_actionFileCloseDb_triggered();

    void on_actionFileWriteDb_triggered();

    void on_actionFileDisplayDb_triggered();
    
    void on_actionFileOpenDb_triggered();

    void on_actionAbout_triggered();

private:
    const QList<QString> dbFields = {"CALL", "QSO_DATE", "TIME_ON", "QSO_DATE_OFF", "TIME_OFF", "FREQ", "FREQ_RX", "BAND_RX", "STATION_CALLSIGN", "NAME", "QTH", "GRIDSQUARE", "MY_GRIDSQUARE", "BAND", "MODE", "TX_PWR", "RST_RCVD", "RST_SENT","COMMENT", "STATE"};
    const QList<QString> primaryKey = {"CALL", "QSO_DATE", "TIME_ON"};
    const QString dbFile = "./db.sqlite";
    const QString dbTable = "log";

    int recordCount;

    Ui::MainWindow *ui;
    QLabel *statusLabel;

    QSqlQuery query;

    QSqlDatabase db;
    //QSqlTableModel *model;   // Replacing with QSqlQueryModel
    QSqlQueryModel *model;

    void setFileMenuOptions(bool frad, bool frda, bool fod, bool fdd, bool fcd);
    void loadAdifFile();
    int loadExistingDatabase();
    void setupDatabase();
    bool checkTableExists(QString tableName);
    int createTable();
    void setupModelAndView();
    void setupUI();
};
